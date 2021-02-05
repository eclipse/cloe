import os.path
from conans import ConanFile, CMake, tools


class Fable(ConanFile):
    name = "fable"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "JSON schema and configuration library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "test": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "test": True,
    }
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "include/*",
        "src/*",
        "CMakeLists.txt",
    ]
    requires = [
        "boost/[>=1.65.1]",
        "fmt/[~=6.2.0]",
        "nlohmann_json/[~=3.9.1]",
    ]

    _cmake = None

    def _project_version(self):
        version_file = os.path.join(self.recipe_folder, "..", "VERSION")
        return tools.load(version_file).strip()

    def set_version(self):
        self.version = self._project_version()

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~1.10]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["FABLE_VERSION"] = self.version
        self._cmake.definitions["BuildTests"] = self.options.test
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.test:
            cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        if self.in_local_cache:
            self.cpp_info.libs = tools.collect_libs(self)
        else:
            self.cpp_info.libs = ["fable"]
