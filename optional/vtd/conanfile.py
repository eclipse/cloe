# mypy: ignore-errors
# pylint: skip-file

import os
import tarfile
from pathlib import Path
from shutil import rmtree

from conan import ConanFile
from conan.tools import cmake, files, scm, env

required_conan_version = ">=1.52.0"


class CloeSimulatorVTD(ConanFile):
    name = "cloe-plugin-vtd"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe simulator plugin that binds to Vires VTD"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "pedantic": [True, False],
    }
    default_options = {
        "pedantic": True,
    }
    generators = "CMakeDeps", "VirtualRunEnv"
    no_copy_source = True
    exports_sources = [
        "bin/*",
        "cmake/*",
        "src/*",
        "CMakeLists.txt",
        "module.py",
    ]

    _setup_folder = "contrib/setups"

    def set_version(self):
        for version_path in ["VERSION", "../../VERSION"]:
            version_file = Path(self.recipe_folder) / version_path
            if version_file.exists():
                self.version = files.load(self, version_file).strip()
                return
        git = scm.Git(self, self.recipe_folder)
        self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires(f"cloe-models/{self.version}@cloe/develop")
        self.requires("open-simulation-interface/3.2.0@cloe/stable")
        self.requries("vtd-api/2.2.0@cloe/stable")

        # Overrides, same as in the cloe conanfile.py:
        self.requires("protobuf/[>=3.9.1]", override=True)
        self.requires("zlib/1.2.12", override=True)
        self.requires("fmt/[~=8.1.1]", override=True)
        self.requires("inja/[~=3.3.0]", override=True)
        self.requires("nlohmann_json/[~=3.10.5]", override=True)
        self.requires("incbin/cci.20211107", override=True),

    def build_requirements(self):
        self.test_requires("gtest/[~1.10]")

    def _compress_and_remove(self, dir):
        if not dir.is_dir():
            return
        with tarfile.open(f"{dir.name}.tgz", "w:gz") as tar:
            tar.add(dir.path, arcname=os.path.basename(dir.path))
            rmtree(dir.path)

    def _compress_setups(self):
        cwd = os.getcwd()
        os.chdir(f"{self.export_sources_folder}/{self._setup_folder}")
        with os.scandir() as scan:
            for dir in scan:
                self._compress_and_remove(dir)
        os.chdir(cwd)

    def export_sources(self):
        self.copy("*", dst=self._setup_folder, src=self._setup_folder, symlinks=True)
        self._compress_setups()

    def configure(self):
        self.options["open-simulation-interface"].shared = False
        self.options["open-simulation-interface"].fPIC = True

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["CLOE_PROJECT_VERSION"] = self.version
        tc.cache_variables["TargetLintingExtended"] = self.options.pedantic
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        cm.configure()
        cm.build()
        cm.test()

    def package(self):
        cm = cmake.CMake(self)
        cm.install()
        self.copy("vtd-launch", dst="bin", src=f"{self.source_folder}/bin")
        self.copy(
            "*.tgz",
            dst=self._setup_folder,
            src=f"{self.source_folder}/{self._setup_folder}",
        )

    def package_id(self):
        # Changes in a dependency's package_id need to be accounted for since
        # the package statically links against vtd-object-lib.
        self.info.requires["cloe-models"].full_package_mode()
        self.info.requires["cloe-runtime"].full_package_mode()
        self.info.requires["open-simulation-interface"].full_package_mode()
        self.info.requires["vtd-api"].full_package_mode()
        self.info.requires["boost"].full_package_mode()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cloe-plugin-vtd")
        self.cpp_info.set_property("pkg_config_name", "cloe-plugin-vtd")
        self.env_info.VTD_LAUNCH = f"{self.package_folder}/bin/vtd-launch"
        self.env_info.VTD_SETUP_DIR = f"{self.package_folder}/{self._setup_folder}"

    def package_id(self):
        del self.info.options.pedantic
