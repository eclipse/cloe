# mypy: ignore-errors
# pylint: skip-file

from conan import ConanFile
from conan.tools import cmake, files, scm

required_conan_version = ">=1.52.0"


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
    generators = "CMakeDeps"
    no_copy_source = True
    requires = [
        # CppNetlib does not work with a boost that is newer than 1.69
        "boost/[>=1.65.0 <1.70]",
        "openssl/1.1.1s",
    ]

    def source(self):
        git = scm.Git(self, self.recipe_folder)
        git.clone(self.git_url, self.source_folder, args=["-b", self.git_branch, "--recursive", "--depth=1"])

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["CPP-NETLIB_BUILD_EXAMPLES"] = False
        tc.cache_variables["CPP-NETLIB_BUILD_SHARED_LIBS"] = self.options.shared
        tc.cache_variables["CPP-NETLIB_BUILD_TESTS"] = False
        tc.cache_variables["CPP-NETLIB_ENABLE_HTTPS"] = True
        tc.cache_variables["CPP-NETLIB_STATIC_BOOST"] = True
        tc.cache_variables["CPP-NETLIB_STATIC_OPENSSL"] = True
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        #cm.configure(build_script_folder=self.recipe_folder + "/src")
        cm.configure()
        cm.build()

    def package(self):
        cm = cmake.CMake(self)
        cm.install()

    def package_id(self):
        self.info.requires["boost"].full_package_mode()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cppnetlib")
        self.cpp_info.set_property("cmake_target_name", "cppnetlib::cppnetlib")
        self.cpp_info.set_property("pkg_config_name", "cppnetlib")
        self.cpp_info.libs = files.collect_libs(self)
