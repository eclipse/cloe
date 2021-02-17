from conans import ConanFile, CMake, tools
import shutil
import os


class OpenSimulationInterfaceConan(ConanFile):
    name = "open-simulation-interface"
    version = "3.0.1"
    license = "Mozilla Public License 2.0"
    url = "https://github.com/OpenSimulationInterface/open-simulation-interface"
    description = "A generic interface for the environmental perception of automated driving functions in virtual scenarios."
    topics = ("Sensor Simulation", "HAD")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "protoc_from_protobuf": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "protoc_from_protobuf": False,
    }
    generators = "cmake"
    build_policy = "missing"
    no_copy_source = False
    exports_sources = [
        "CMakeLists.txt",
    ]
    requires = [
        "protobuf/[>=2.6.1]@bincrafters/stable",
    ]

    _git_url = (
        "https://github.com/OpenSimulationInterface/open-simulation-interface.git"
    )
    _git_dir = "osi"
    _git_ref = "v{}".format(version)

    _cmake = None

    def build_requirements(self):
        if not self.options.protoc_from_protobuf:
            self.build_requires("protoc_installer/[>=2.6.1]@bincrafters/stable")

    def source(self):
        git = tools.Git(folder=self._git_dir)
        git.clone(self._git_url, self._git_ref, shallow=True)
        dst = os.path.join(self.source_folder, self._git_dir)
        shutil.copy("CMakeLists.txt", dst)

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_PROJECT_VERSION"] = self.version
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.configure(source_folder=self._git_dir)
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
