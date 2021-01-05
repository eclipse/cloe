from pathlib import Path
import shutil
import os.path

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
    _archive_osi = "vtd.2.2.0.addOns.OSI.20190513.tgz"
    _archive_rod = "vtd.2.2.0.addOns.ROD.Base.20190313.tgz"
    _root_dir = "VTD.2.2"

    def export_sources(self):
        self.copy(self._archive_base, symlinks=False)
        if os.path.isfile(self._archive_osi):
            self.copy(self._archive_osi, symlinks=False)
        if os.path.isfile(self._archive_rod):
            self.copy(self._archive_rod, symlinks=False)

    def configure(self):
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("VTD binaries do not exist for Windows")

    def build(self):
        src = Path(self.source_folder)
        dst = Path(self.build_folder)

        def extract_archive(archive):
            print("Extracting: {}".format(archive))
            tools.untargz(src / archive, dst)

        extract_archive(self._archive_base)
        if self.options.with_osi:
            extract_archive(self._archive_osi)
        if self.options.with_road_designer:
            extract_archive(self._archive_rod)

    def package(self):
        self.copy("*", src=self._root_dir, symlinks=True)

    def package_info(self):
        bindir = Path(self.package_folder) / "bin"
        self.output.info("Appending PATH environment variable: {}".format(bindir))
        self.env_info.PATH.append(str(bindir))
        self.env_info.VTD_ROOT = self.package_folder
