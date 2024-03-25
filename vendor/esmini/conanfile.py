import os

from conan import ConanFile
from conan.tools import cmake, files, env
from conan.tools.system import package_manager

required_conan_version = ">=1.52.0"


class ESMini(ConanFile):
    name = "esmini"
    license = "MPL-2.0"
    url = "https://github.com/esmini/esmini"
    description = "Basic OpenScenario player"
    topics = ("Environment Simulator", "OpenScenario", "OpenDrive")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_osg": [True, False],
        "with_osi": [True, False],
        "with_sumo": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "with_osg": True,
        "with_osi": True,
        "with_sumo": True,
    }
    generators = "CMakeDeps"

    def export_sources(self):
        files.export_conandata_patches(self)

    def configure(self):
        if self.options.with_osg:
            self.options.with_osi = True
        if self.options.with_osi:
            self.options["open-simulation-interface"].shared = self.options.shared
            self.options["protobuf"].shared = True
        self.options["open-simulation-interface"].shared = self.options.shared
        self.options["protobuf"].shared = self.options.shared

    def layout(self):
        # We can't use cmake_layout because ESmini *really* doesn't like stuff to
        # not be directly in "build/": unit-tests provide wrong results and segfault.
        # Conan likes putting stuff in "build/Debug" and "build/Release".
        self.folders.src = "."
        self.folders.build = "build"
        self.folders.generators = "build/generators"
        # TODO: I am not sure whether these are necessary...
        self.cpp.source.includedirs = ["include"]
        self.cpp.build.libdirs = ["."]
        self.cpp.build.bindirs = ["."]

    def system_requirements(self):
        packages = None
        if self.options.with_osg:
            # TODO: add all system requirements
            packages = [
                "libfontconfig1-dev",
                "libgl-dev",
                "libxrandr-dev",
                "libxinerama-dev",
            ]
        if packages:
            apt = package_manager.Apt(self)
            apt.install(packages, update=True, check=True)

    def requirements(self):
        # Currently, esmini needs all dependencies whether you use them or not
        self.requires("open-simulation-interface/3.5.0@cloe/stable")

    def source(self):
        files.get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        # This is the same environment that is used by scripts/run_tests.sh:
        base_folder = os.path.join(self.build_folder, "..")
        test_env = env.Environment()
        test_env.define(
            "LSAN_OPTIONS",
            f"print_suppressions=false:suppressions={base_folder}/scripts/LSAN.supp",
        )
        test_env.define(
            "ASAN_OPTIONS",
            f"detect_invalid_pointer_pairs=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:fast_unwind_on_malloc=0:suppressions={base_folder}/scripts/ASAN.supp",
        )
        test_vars = test_env.vars(self)
        test_vars.save_script("esminitest")

        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_PROJECT_VERSION"] = self.version
        tc.cache_variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.cache_variables["USE_OSG"] = self.options.with_osg
        tc.cache_variables["USE_OSI"] = self.options.with_osi
        tc.cache_variables["USE_SUMO"] = self.options.with_sumo
        tc.cache_variables["USE_GTEST"] = True
        tc.cache_variables["DYN_PROTOBUF"] = self.options.shared
        tc.generate()

    def build(self):
        files.apply_conandata_patches(self)

        cm = cmake.CMake(self)
        if self.should_configure:
            cm.configure()
        if self.should_build:
            cm.build()
            cm.install()
        if self.should_test:
            cm.test(env=["conanrun", "conanbuild", "esminitest"])

    def package(self):
        files.copy(
            self,
            "*",
            src=os.path.join(self.source_folder, "bin"),
            dst=os.path.join(self.package_folder, "bin"),
            excludes=["*.a", "*.so"],
        )
        files.copy(
            self,
            "*.hpp",
            src=os.path.join(self.source_folder, "EnvironmentSimulator", "Libraries"),
            dst=os.path.join(self.package_folder, "include"),
            keep_path=False,
        )
        files.copy(
            self,
            "*.a",
            src=os.path.join(self.source_folder, "bin"),
            dst=os.path.join(self.package_folder, "lib"),
            keep_path=False,
        )
        files.copy(
            self,
            "*.so",
            src=os.path.join(self.source_folder, "bin"),
            dst=os.path.join(self.package_folder, "lib"),
            keep_path=False,
        )
        files.copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
        files.copy(
            self,
            "*",
            src=os.path.join(self.source_folder, "3rd_party_terms_and_licenses"),
            dst=os.path.join(self.package_folder, "licenses"),
        )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "esmini")
        self.cpp_info.set_property("cmake_target_name", "esmini::esmini")
        if self.options.shared:
            self.cpp_info.components["esmini"].libs = ["esminiLib", "esminiRMLib"]
            self.cpp_info.components["esminiLib"].libs = ["esminiLib"]
        else:
            self.cpp_info.components["esmini"].libs = [
                "esminiLib_static",
                "esminiRMLib",
            ]
            self.cpp_info.components["esminiLib"].libs = ["esminiLib_static"]
        self.cpp_info.components["esminiRMLib"].libs = ["esminiRMLib"]
        if self.options.with_osi:
            self.cpp_info.components["esmini"].requires = [
                "open-simulation-interface::open-simulation-interface"
            ]
            self.cpp_info.components["esminiLib"].requires = [
                "open-simulation-interface::open-simulation-interface"
            ]

        self.runenv_info.prepend_path("PATH", os.path.join(self.package_folder, "bin"))
