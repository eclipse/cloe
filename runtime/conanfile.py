import os.path
from conans import ConanFile, CMake, tools


class CloeRuntime(ConanFile):
    name = "cloe-runtime"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe runtime"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "test": [True, False],
        "pedantic": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "test": True,
        "pedantic": True,
    }
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "cmake/*",
        "include/*",
        "src/*",
        "CMakeLists.txt",
    ]
    requires = [
        "boost/[>=1.65.1]",
        "inja/[~=3.0.0]",
        "spdlog/[~=1.5.0]",
        "incbin/[~=1.74.0]@cloe/stable",
    ]

    _cmake = None

    def _project_version(self):
        version_file = os.path.join(self.recipe_folder, "..", "VERSION")
        return tools.load(version_file).strip()

    def set_version(self):
        self.version = self._project_version()

    def requirements(self):
        self.requires("fable/{}@cloe/develop".format(self.version))

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~1.10]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["BuildTests"] = self.options.test
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.definitions["CLOE_PROJECT_VERSION"] = self.version
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
        # Make sure we can find the libs and *.cmake files, both in editable
        # mode and in the normal package mode:
        if not self.in_local_cache:
            self.cpp_info.builddirs.append("cmake")
            self.cpp_info.libs = ["cloe-runtime"]
        else:
            self.cpp_info.builddirs.append(os.path.join("lib", "cmake", "cloe"))
            self.cpp_info.libs = tools.collect_libs(self)
