# mypy: ignore-errors
# pylint: skip-file

import os.path
import shutil
import subprocess
import sys
from pathlib import Path
from typing import List

from conan import ConanFile
from conan.tools import files
from conan.errors import ConanInvalidConfiguration

required_conan_version = ">=1.52.0"


def patch_rpath(file: Path, rpath: List[str]):
    """Set the RPATH of an executable or library.

    This will fail silently if the file is not an ELF executable.
    """
    assert file.exists()

    # Get original RPATH of file, or fail.
    result = subprocess.run(
        ["patchelf", "--print-rpath", str(file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.stderr.decode().strip() == "not an ELF executable":
        # Silently ignore binaries that don't apply.
        return
    original = result.stdout.decode().strip()
    if original != "":
        rpath.append(original)

    file_arg = str(file)
    rpath_arg = ":".join(rpath)
    print(f"Setting RPATH: {file_arg} -> {rpath_arg}")
    subprocess.run(
        ["patchelf", "--set-rpath", rpath_arg, file_arg],
        check=True,
    )


def find_binary_files() -> List[Path]:
    """Return a list of all file paths that are of the binary type."""
    result = subprocess.run(
        """find -type f -executable -exec sh -c "file -i '{}' | grep -q '; charset=binary'" \; -print""",
        shell=True,
        stdout=subprocess.PIPE,
        check=True,
    )
    return [Path(x) for x in result.stdout.decode().splitlines()]


class VtdConan(ConanFile):
    name = "vtd"
    version = "2.2.0"
    license = "Proprietary"
    url = "https://vires.mscsoftware.com"
    no_copy_source = True
    description = "Include binary files in C/C++"
    topics = ("simulator", "vires")
    settings = "os", "arch"
    options = {
        "with_osi": [True, False],
        "with_road_designer": [True, False],
    }
    default_options = {
        "with_osi": True,
        "with_road_designer": False,
    }

    _archive_base = "dl/vtd.2.2.0.Base.20181231.tgz"
    _archive_osi = "dl/vtd.2.2.0.addOns.OSI.20210120.tgz"
    _archive_rod = "dl/vtd.2.2.0.addOns.ROD.Base.20190313.tgz"
    _root_dir = "VTD.2.2"

    def export_sources(self):
        self.copy("libdeps_pruned.txt")
        self.copy(self._archive_base, symlinks=False)
        if Path(self._archive_osi).exists():
            self.copy(self._archive_osi, symlinks=False)
        if Path(self._archive_rod).exists():
            self.copy(self._archive_rod, symlinks=False)

    def configure(self):
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("VTD binaries do not exist for Windows")

    def build(self):
        src = Path(self.source_folder)
        dst = Path(self.build_folder)
        vtddir = dst / "VTD.2.2"
        libdir = vtddir / "bundled"

        def extract_archive(archive):
            print(f"Extracting: {archive}")
            files.untargz(src / archive, dst)

        extract_archive(self._archive_base)
        libdir.mkdir()
        if self.options.with_osi:
            extract_archive(self._archive_osi)
        if self.options.with_road_designer:
            extract_archive(self._archive_rod)

        # Patch RPATH of several critical binaries.
        patch_rpath(
            vtddir / "Runtime/Core/ModuleManager/moduleManager.4.7.0",
            ["$ORIGIN/../Framework/lib", "$ORIGIN/lib"],
        )
        patch_rpath(
            vtddir / "Runtime/Core/ParamServer/paramServer-1.1.0", ["$ORIGIN/../Lib"]
        )
        patch_rpath(
            vtddir / "Runtime/Core/Traffic/ghostdriver.2.2.0.837",
            ["$ORIGIN/../Lib"],
        )

        # Patch RPATH of all the binaries with the path to our bundled system libraries:
        for file in find_binary_files():
            try:
                patch_rpath(
                    file, [f"$ORIGIN/{os.path.relpath(libdir, (dst / file).parent)}"]
                )
            except:
                # Not all files can be set, but even if this happens it doesn't appear
                # to be a big deal.
                print(f"Error: cannot set RPATH of {file}", file=sys.stderr)

        # Bundle libraries that we need at runtime so that this package is portable:
        with Path(src / "libdeps_pruned.txt").open() as file:
            libraries = [Path(x.strip()) for x in file.read().splitlines()]
        for path in libraries:
            print(f"Bundling system library: {path}")
            if path.is_symlink():
                shutil.copy(path, libdir, follow_symlinks=False)
                path = path.resolve(strict=True)
            assert not path.is_symlink()
            shutil.copy(path, libdir)
            patch_rpath(libdir / path.name, ["$ORIGIN"])

    def package(self):
        self.copy("*", src=self._root_dir, symlinks=True)

    def package_info(self):
        bindir = Path(self.package_folder) / "bin"
        self.output.info(f"Appending PATH environment variable: {bindir}")
        self.runenv_info.append_path("PATH", str(bindir))
        self.runenv_info.define("VTD_ROOT", self.package_folder)
        self.buildenv_info.define("VTD_ROOT", self.package_folder)

        if self.options.with_osi:
            osi_path = (
                Path(self.package_folder)
                / "Data/Setups/Standard.OSI3/Bin/libopen_simulation_interface.so"
            )
            if not osi_path.exists():
                raise ConanInvalidConfiguration("VTD OSI library not found.")
            self.runenv_info.append_path("VTD_EXTERNAL_MODELS", f"{osi_path}")
