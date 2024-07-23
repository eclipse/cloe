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
        # Check for both VERSION and ../../VERSION because when building inside
        # Docker only the plugin directory is exported.
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
        self.requires(f"cloe-osi/{self.version}@cloe/develop")
        self.requires("open-simulation-interface/3.5.0@cloe/stable")
        self.requires("vtd-api/2.2.0@cloe/stable")

        # Overrides, same as in the cloe conanfile.py:
        self.requires("protobuf/[>=3.9.1]", override=True)
        self.requires("zlib/1.2.12", override=True)
        self.requires("fmt/9.1.0", override=True)
        self.requires("inja/3.4.0", override=True)
        self.requires("nlohmann_json/3.11.2", override=True)
        self.requires("incbin/cci.20211107", override=True)

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")

    def _compress_and_remove(self, dir):
        # reset() will remove the packages metadata
        def reset(tarinfo):
            tarinfo.uid = tarinfo.gid = 0
            tarinfo.uname = ""
            tarinfo.gname = ""
            tarinfo.mtime = 1
            return tarinfo

        if not dir.is_dir():
            return
        # Compressing will add a timestamp to the package and therefore
        # leads to different package_hashes everytime we export with conan.
        # To avoid that, changing from .tgz to .tar was necessary
        with tarfile.open(f"{dir.name}.tar", "w:") as tar:
            tar.add(dir.path, arcname=os.path.basename(dir.path), filter=reset)
            rmtree(dir.path)

    def _compress_setups(self):
        cwd = os.getcwd()
        setup_dir = f"{self.export_sources_folder}/{self._setup_folder}"
        os.chdir(setup_dir)
        versions = [name for name in os.listdir(setup_dir) if os.path.isdir(os.path.join(setup_dir, name))]
        for version in versions:
            os.chdir(os.path.join(setup_dir, version))
            with os.scandir() as scan:
                for dir in scan:
                    self._compress_and_remove(dir)
        os.chdir(cwd)

    def export_sources(self):
        self.copy("*", dst=self._setup_folder, src=self._setup_folder, symlinks=True)
        self._compress_setups()

    def configure(self):
        self.options["open-simulation-interface"].shared = False

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["CLOE_PROJECT_VERSION"] = self.version
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        if self.should_configure:
            cm.configure()
        if self.should_build:
            cm.build()
        if self.should_test:
            cm.test()

    def package(self):
        vtd_api_version = self.dependencies["vtd-api"].ref.version

        cm = cmake.CMake(self)
        if self.should_install:
            cm.install()
            self.copy("vtd-launch", dst="bin", src=f"{self.source_folder}/bin")
            self.copy(
                "*.tar",
                dst=self._setup_folder,
                src=f"{self.source_folder}/{self._setup_folder}/{vtd_api_version}"
            )

    def package_id(self):
        # Changes in a dependency's package_id need to be accounted for since
        # the package statically links against vtd-object-lib.
        self.info.requires["cloe-models"].full_package_mode()
        self.info.requires["cloe-runtime"].full_package_mode()
        self.info.requires["open-simulation-interface"].full_package_mode()
        self.info.requires["vtd-api"].full_package_mode()
        self.info.requires["boost"].full_package_mode()
        self.info.requires["nlohmann_json"].full_package_mode()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cloe-plugin-vtd")
        self.cpp_info.set_property("pkg_config_name", "cloe-plugin-vtd")
        self.runenv_info.define("VTD_LAUNCH", f"{self.package_folder}/bin/vtd-launch")
        self.runenv_info.define("VTD_SETUP_DIR", f"{self.package_folder}/{self._setup_folder}")
