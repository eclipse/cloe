from pathlib import Path
import os
from shutil import rmtree
import tarfile
from conans import CMake, ConanFile, RunEnvironment, tools


class CloeSimulatorVTD(ConanFile):
    name = "cloe-plugin-vtd"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe simulator plugin that binds to Vires VTD"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "test": [True, False],
        "pedantic": [True, False],
    }
    default_options = {
        "test": True,
        "pedantic": True,
    }
    generators = "cmake"
    exports_sources = [
        "bin/*",
        "cmake/*",
        "src/*",
        "CMakeLists.txt",
        "module.py",
    ]
    no_copy_source = True
    requires = [
        "open-simulation-interface/3.2.0@cloe/stable",
        "vtd-api/2.2.0@cloe/stable",
    ]

    _setup_folder = "contrib/setups"
    _cmake = None

    def set_version(self):
        for version_path in ["VERSION", "../../VERSION"]:
            version_file = Path(self.recipe_folder) / version_path
            if version_file.exists():
                self.version = tools.load(version_file).strip()
                return
        git = tools.Git(folder=self.recipe_folder)
        self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires(f"cloe-models/{self.version}@cloe/develop")

        # Overrides, same as in the cloe conanfile.py:
        #self.requires("protobuf/[>=3.9.1]", override=True)
        self.requires("zlib/1.2.12", override=True)
        self.requires("fmt/[~=8.1.1]", override=True)
        self.requires("inja/[~=3.3.0]", override=True)
        self.requires("nlohmann_json/[~=3.10.5]", override=True)
        self.requires("incbin/[~=0.88.0]@cloe/stable", override=True)

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~1.10]")

    def _compress_and_remove(self, dir):
        def reset(tarinfo):
            tarinfo.uid = tarinfo.gid = 0
            tarinfo.uname = ""
            tarinfo.gname = ""
            tarinfo.mtime = 1
            return tarinfo

        if not dir.is_dir():
            return
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

    def _configure_cmake(self):
        vtd_api_version = str(self.requires.get("vtd-api"))
        if "vtd-api/2.2.0" in vtd_api_version:
            self.vtd_api_version = "2.2.0"
        else:
            self.vtd_api_version = "2022.3"

        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["VTD_API_VERSION"] = self.vtd_api_version
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["BuildTests"] = self.options.test
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.configure()
        return self._cmake

    def configure(self):
        self.options["open-simulation-interface"].shared = False
        self.options["open-simulation-interface"].fPIC = True

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.test:
            with tools.environment_append(RunEnvironment(self).vars):
                cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.copy("vtd-launch", dst="bin", src=f"{self.source_folder}/bin")
        self.copy(
            "*.tar",
            dst=self._setup_folder,
            src=f"{self.source_folder}/{self._setup_folder}/{self.vtd_api_version}"
        )

    def package_id(self):
        # Changes in a dependency's package_id need to be accounted for since
        # the package statically links against vtd-object-lib.
        self.info.requires["cloe-models"].full_package_mode()
        self.info.requires["cloe-runtime"].full_package_mode()
        self.info.requires["open-simulation-interface"].full_package_mode()
        self.info.requires["vtd-api"].full_package_mode()

    def package_info(self):
        self.env_info.VTD_LAUNCH = f"{self.package_folder}/bin/vtd-launch"
        self.env_info.VTD_SETUP_DIR = f"{self.package_folder}/{self._setup_folder}"

    def package_id(self):
        del self.info.options.test
        del self.info.options.pedantic
