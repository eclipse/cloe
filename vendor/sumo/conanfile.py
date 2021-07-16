from conans import ConanFile, CMake, tools


class SumoConan(ConanFile):
    name = "sumo"
    version = "1.6.0"
    default_user = "sumo"
    default_channel = "stable"
    license = "EPL-2.0"
    author = "Author details [To be added]"
    url = "https://github.com/eclipse/sumo"
    description = "Interface to provide access to a running road traffic simulation."
    topics = ("traici", "foreign tcpip")
    settings = "os", "compiler", "build_type", "arch"
    
    options = {
        "shared": [True, False], 
        "fPIC": [True, False]
    }
    default_options = {
        "shared": False, 
        "fPIC": True
    }
    
    generators = "cmake"
    _src_dir_sumo = "sumo"
    _src_tag = "v1_6_0"
    _dst_include_path = "include/"
    _dst_lib_path = "lib"
    _src_traci_include_path = "sumo/src/utils/traci/"
    _src_foreign_include_path = "sumo/src/foreign/tcpip/"
    _src_libsumo_include_path = "sumo/src/libsumo/"
    _src_traci_lib_path = "src/utils/traci/"
    _src_foreign_lib_path = "src/foreign/tcpip/"
    _sumo_binary = "sumo/bin/"

    def source(self):
        git = tools.Git(folder=self._src_dir_sumo)
        git.clone("https://github.com/eclipse/sumo.git", self._src_tag, "", "True")

    def configure(self):
        if self.develop:
            tools.check_min_cppstd(self, "98")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=self._src_dir_sumo)
        cmake.build()

    def package(self):
        self.copy(
            "*.h",
            dst="include/utils/traci/",
            src=self._src_traci_include_path,
            keep_path=False,
        )
        self.copy(
            "*.h",
            dst="include/foreign/tcpip/",
            src=self._src_foreign_include_path,
            keep_path=False,
        )
        self.copy(
            "TraCIDefs.h",
            dst="include/libsumo/",
            src=self._src_libsumo_include_path,
            keep_path=False,
        )
        self.copy(
            "TraCIConstants.h",
            dst="include/libsumo/",
            src=self._src_libsumo_include_path,
            keep_path=False,
        )

        self.copy(
            "*.a", dst=self._dst_lib_path, src=self._src_traci_lib_path, keep_path=False
        )
        self.copy(
            "*.a",
            dst=self._dst_lib_path,
            src=self._src_foreign_lib_path,
            keep_path=False,
        )
        
        self.copy("*", dst="/usr/bin/", src=self._sumo_binary)

    def package_info(self):
        self.cpp_info.libs = ["utils_traci", "foreign_tcpip"]
    
        