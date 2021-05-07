import os.path
import shutil
import subprocess
import sys
from pathlib import Path

from conans import CMake, ConanFile, tools
from conans.errors import ConanInvalidConfiguration


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

    _archive_base = "vtd.2.2.0.Base.20181231.tgz"
    _archive_osi = "vtd.2.2.0.addOns.OSI.20210120.tgz"
    _archive_rod = "vtd.2.2.0.addOns.ROD.Base.20190313.tgz"
    _root_dir = "VTD.2.2"

    def export_sources(self):
        self.copy("library_filelist.txt")
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
        libdir = vtddir / "lib"

        def extract_archive(archive):
            print(f"Extracting: {archive}")
            tools.untargz(src / archive, dst)

        extract_archive(self._archive_base)
        libdir.mkdir()
        if self.options.with_osi:
            extract_archive(self._archive_osi)
        if self.options.with_road_designer:
            extract_archive(self._archive_rod)

        # Get a list of all binaries we need to set the RPATH of:
        result = subprocess.run(
            """find -type f -executable -exec sh -c "file -i '{}' | grep -q '; charset=binary'" \; -print""",
            shell=True,
            stdout=subprocess.PIPE,
            check=True,
        )
        binary_files = [Path(x) for x in result.stdout.decode().splitlines()]
        # Set RPATH of all the collected binaries:
        for file in binary_files:
            assert file.exists()
            rpath = os.path.relpath(libdir, (dst / file).parent)
            try:
                print(f"Setting RPATH: {file} -> $ORIGIN/{rpath}")
                result = subprocess.run(
                    ["patchelf", "--set-rpath", f"$ORIGIN/{rpath}", str(file)],
                    check=True,
                )
            except:
                print(f"Error: cannot set RPATH of {file}", file=sys.stderr)

        # Bundle libraries that we need at runtime so that this package is portable:
        with Path(src / "library_filelist.txt").open() as file:
            libraries = [Path(x.strip()) for x in file.read().splitlines()]
        for path in libraries:
            print(f"Bundling system library: {path}")
            shutil.copy(path, libdir, follow_symlinks=False)

    def package(self):
        self.copy("*", src=self._root_dir, symlinks=True)

    def package_info(self):
        bindir = Path(self.package_folder) / "bin"
        self.output.info(f"Appending PATH environment variable: {bindir}")
        self.env_info.PATH.append(str(bindir))
        self.env_info.VTD_ROOT = self.package_folder
        if self.options.with_osi:
            osi_path = (
                Path(self.package_folder)
                / "Data/Setups/Standard.OSI3/Bin/libopen_simulation_interface.so"
            )
            if not osi_path.exists():
                raise ConanInvalidConfiguration("VTD OSI library not found.")
            self.env_info.VTD_EXTERNAL_MODELS.append(f"{osi_path}")
