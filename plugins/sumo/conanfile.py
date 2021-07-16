import os.path

from conans import ConanFile, CMake, tools


class CloeSimulatorSumo(ConanFile):
    name = "cloe-plugin-sumo"
    url = "https://github.com/eclipse/cloe"
    description = "SUMO traffic simulator binding plugin for Cloe"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "pedantic": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "pedantic": True,
        "fPIC": True,
    }
    generators = "cmake"
    exports_sources = [
        "src/*",
        "module.py",
        "CMakeLists.txt",
    ]
    no_copy_source = True
    build_requires = [
        "sumo/1.6.0@cloe/stable",
    ]
    
    _cmake = None
    

    def requirements(self):
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires(f"cloe-models/{self.version}@cloe/develop")
        
    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
