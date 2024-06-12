# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, files, scm

required_conan_version = ">=1.52.0"


class CloeControllerExampleExternal(ConanFile):
    """
    This package is used for internal tests, and is not part of the
    cloe package distribution. This is used for testing combining
    external packages and cloe with Conan.
    """

    name = "cloe-plugin-example-external"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe controller plugin mocks used for testing integration"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    no_copy_source = True
    exports_sources = [
        "src/*",
        "CMakeLists.txt",
    ]

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires(f"cloe-models/{self.version}@cloe/develop")

    def layout(self):
        cmake.cmake_layout(self)

    def build(self):
        cm = cmake.CMake(self)
        if self.should_configure:
            cm.configure()
        if self.should_build:
            cm.build()

    def package(self):
        cm = cmake.CMake(self)
        if self.should_install:
            cm.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", self.name)
        self.cpp_info.set_property("pkg_config_name", self.name)

        if not self.in_local_cache: # editable mode
            libdir = Path(self.build_folder) / "lib"
            self.runenv_info.append_path("LD_LIBRARY_PATH", str(libdir))
