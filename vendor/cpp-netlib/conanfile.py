from conans import CMake, ConanFile, tools


class CppNetlib(ConanFile):
    name = "cpp-netlib"
    version = "0.13.0"
    license = "BSL-1.0"
    url = "https://github.com/cpp-netlib/cpp-netlib"
    git_url = "https://github.com/cpp-netlib/cpp-netlib.git"
    git_branch = "cpp-netlib-0.13.0-final"
    description = "Modern C++ network programming libraries"
    topics = "http"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
    }
    default_options = {
        "shared": False,
    }
    exports_sources = [
        "CMakeLists.txt",
    ]
    generators = "cmake"
    requires = [
        # CppNetlib does not work with a boost that is newer than 1.69
        "boost/[>=1.65.0 <1.70]",
        "openssl/1.1.1g",
    ]

    _cmake = None
    _source_folder = "cpp-netlib"

    def source(self):
        git = tools.Git(folder=self._source_folder)
        git.clone(self.git_url, self.git_branch, args="--recursive", shallow=True)

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["CPP-NETLIB_BUILD_EXAMPLES"] = False
        self._cmake.definitions["CPP-NETLIB_BUILD_SHARED_LIBS"] = self.options.shared
        self._cmake.definitions["CPP-NETLIB_BUILD_TESTS"] = False
        self._cmake.definitions["CPP-NETLIB_ENABLE_HTTPS"] = True
        self._cmake.definitions["CPP-NETLIB_STATIC_BOOST"] = True
        self._cmake.definitions["CPP-NETLIB_STATIC_OPENSSL"] = True
        self._cmake.definitions["Boost_NO_BOOST_CMAKE"] = True
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_id(self):
        self.info.requires["boost"].full_package_mode()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
