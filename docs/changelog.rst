Changelog
=========

The Cloe repository, which you can find at https://github.com/eclipse/cloe, is
composed of multiple "packages", which could potentially be versioned
individually. At the moment, however, we are versioning all together and noting
here what changes for each package.

.. note::
   We currently do not release any binaries, each "release" is just a way of
   refering to a tagged set of all packages.

0.18.0 (2022-04-25)
-------------------

This marks the initial "release" of the Cloe packages.

..
   These are all the commits from this release, except for merge commits.
   They have been grouped into each component.
   Commits not particularly relevant have been commented out.

**Cloe-Launch CLI:**

- cli: Add activate command                                              `[9aca3db] <https://github.com/eclipse/cloe/commit/9aca3db>`_
- cli: Add --conan-arg and --conan-setting options to exec and shell commands `[734944c] <https://github.com/eclipse/cloe/commit/734944c>`_
- cli: Add Makefile with install and editable targets                    `[33d831d] <https://github.com/eclipse/cloe/commit/33d831d>`_
- cli: Fix broken logging statements                                     `[dfc3452] <https://github.com/eclipse/cloe/commit/dfc3452>`_
- cli: Pass extra arguments to shell command                             `[154828f] <https://github.com/eclipse/cloe/commit/154828f>`_
- cli: Use logging library functions instead of print                    `[0617841] <https://github.com/eclipse/cloe/commit/0617841>`_
.. - cli: Bump click to v8.x                                             `[c105f28] <https://github.com/eclipse/cloe/commit/c105f28>`_
.. - cli: Document how to use pipx to install cloe-launch                `[12e4aca] <https://github.com/eclipse/cloe/commit/12e4aca>`_
.. - cli: Revert click dependency change back to 7.x                     `[715a9d2] <https://github.com/eclipse/cloe/commit/715a9d2>`_
.. - cli: Update installation documentation                              `[3032d05] <https://github.com/eclipse/cloe/commit/3032d05>`_

**Engine:**

- engine: Add interpolation for ${THIS_STACKFILE_DIR} and -FILE          `[072e577] <https://github.com/eclipse/cloe/commit/072e577>`_
- engine: Avoid compiler bug in xenial build                             `[4c08424] <https://github.com/eclipse/cloe/commit/4c08424>`_
- engine: Fix in ComponentConf serialization                             `[0ab2bc2] <https://github.com/eclipse/cloe/commit/0ab2bc2>`_
- engine: Fix missing CXX_STANDARD_REQUIRED for libstack                 `[db0a41f] <https://github.com/eclipse/cloe/commit/db0a41f>`_
- engine: Fix package bin path for in-source builds                      `[988bf3d] <https://github.com/eclipse/cloe/commit/988bf3d>`_
- engine: Fix plugin clobbering not working                              `[820ff72] <https://github.com/eclipse/cloe/commit/820ff72>`_
- engine: Provide better errors when simulation errors occur             `[e4c94ca] <https://github.com/eclipse/cloe/commit/e4c94ca>`_
- engine: Stream json api data to a file                                 `[08938d6] <https://github.com/eclipse/cloe/commit/08938d6>`_
.. - engine: Add system tests for dump command                           `[bf23c50] <https://github.com/eclipse/cloe/commit/bf23c50>`_
.. - engine: Add unit test for component configuration                   `[4e35da4] <https://github.com/eclipse/cloe/commit/4e35da4>`_
.. - engine: Fix duplicate test name in BATS tests                       `[0f55110] <https://github.com/eclipse/cloe/commit/0f55110>`_
.. - engine: Fix minor spelling issues                                   `[a380a86] <https://github.com/eclipse/cloe/commit/a380a86>`_
.. - engine: Fix stack schema tests                                      `[04ad387] <https://github.com/eclipse/cloe/commit/04ad387>`_
.. - engine: Fix system test for dump command                            `[2748e38] <https://github.com/eclipse/cloe/commit/2748e38>`_
.. - engine: Migrate to fixed fable Factory implementation               `[d2fb80c] <https://github.com/eclipse/cloe/commit/d2fb80c>`_
.. - engine: Refactor result file handling                               `[709c80e] <https://github.com/eclipse/cloe/commit/709c80e>`_
.. - oak: Add utility functions for endpoint serialization               `[9851742] <https://github.com/eclipse/cloe/commit/9851742>`_

**Core Libraries:**

- fable: Add and use gtest utility functions                             `[902dfc9] <https://github.com/eclipse/cloe/commit/902dfc9>`_
- fable: Add CustomDeserializer schema type                              `[d42419e] <https://github.com/eclipse/cloe/commit/d42419e>`_
- fable: Add examples and documentation                                  `[599da29] <https://github.com/eclipse/cloe/commit/599da29>`_
- fable: Add extra type traits for working with schema types             `[b0ae81b] <https://github.com/eclipse/cloe/commit/b0ae81b>`_
- fable: Add set_factory() method to Factory schema                      `[3d26e0a] <https://github.com/eclipse/cloe/commit/3d26e0a>`_
- fable: Add to_json() method to all schema types                        `[a97ee64] <https://github.com/eclipse/cloe/commit/a97ee64>`_
- fable: Fix unorthogonal interface of Struct schema                     `[de9d324] <https://github.com/eclipse/cloe/commit/de9d324>`_
- fable: Fix un-reusable interface of Factory class                      `[d771921] <https://github.com/eclipse/cloe/commit/d771921>`_
- fable: Forward-declare make_prototype<> in interface.hpp               `[a868f9a] <https://github.com/eclipse/cloe/commit/a868f9a>`_
- fable: Relax version fmt version requirement                           `[d990c19] <https://github.com/eclipse/cloe/commit/d990c19>`_
- fable: Set version to project version from conanfile.py                `[cea763a] <https://github.com/eclipse/cloe/commit/cea763a>`_
.. -
- runtime: Add SetVariable and SetData trigger actions                   `[d21fbd7] <https://github.com/eclipse/cloe/commit/d21fbd7>`_
- runtime: Fix Vehicle error handling                                    `[5376189] <https://github.com/eclipse/cloe/commit/5376189>`_
- runtime: Support components with multiple inputs                       `[c867eab] <https://github.com/eclipse/cloe/commit/c867eab>`_
.. - runtime: Add simple json file serializer                            `[61436e4] <https://github.com/eclipse/cloe/commit/61436e4>`_
.. - runtime: Annotate fallthrough using a boost macro                   `[3216a5e] <https://github.com/eclipse/cloe/commit/3216a5e>`_
.. - runtime: Move output serializer definitions                         `[7e984f4] <https://github.com/eclipse/cloe/commit/7e984f4>`_
- models: Add existence probability to lane boundary and object          `[8e25a97] <https://github.com/eclipse/cloe/commit/8e25a97>`_
- models: Add utility function for coordinate transformation             `[f24216c] <https://github.com/eclipse/cloe/commit/f24216c>`_
- models: Fix actuation state is_consistent() method                     `[34ba08e] <https://github.com/eclipse/cloe/commit/34ba08e>`_
- models: Fix compile error in actuation_state.cpp                       `[8698921] <https://github.com/eclipse/cloe/commit/8698921>`_
- models: Initialize members of LaneBoundary class                       `[f688e32] <https://github.com/eclipse/cloe/commit/f688e32>`_

**Plugins:**

.. - basic: Fix missing PROJECT_SOURCE_DIR definition                    `[6956ee7] <https://github.com/eclipse/cloe/commit/6956ee7>`_
- gndtruth_extractor: Fix compiler warning                               `[6ee61e4] <https://github.com/eclipse/cloe/commit/6ee61e4>`_
- gndtruth_extractor: Replace enumconfable by fable                      `[21e8f53] <https://github.com/eclipse/cloe/commit/21e8f53>`_
.. - gndtruth_extractor: Add unit test                                   `[baae3c3] <https://github.com/eclipse/cloe/commit/baae3c3>`_
- minimator: Fix json api                                                `[5df6e9d] <https://github.com/eclipse/cloe/commit/5df6e9d>`_
- minimator: Provide lanes of a straight road                            `[f9b60c2] <https://github.com/eclipse/cloe/commit/f9b60c2>`_
.. -
- noisy_sensor: Add trigger for noise activation                         `[f8e488f] <https://github.com/eclipse/cloe/commit/f8e488f>`_
- noisy_sensor: Extend to lane boundaries and refine noise configuration `[a00f64f] <https://github.com/eclipse/cloe/commit/a00f64f>`_
.. - noisy_sensor: Refactor configuration and add unit test              `[2c28cc6] <https://github.com/eclipse/cloe/commit/2c28cc6>`_
.. - noisy_sensor: Remove duplicated code                                `[fbb1610] <https://github.com/eclipse/cloe/commit/fbb1610>`_
.. - noisy_object_sensor: Apply develop branch changes                   `[ea59553] <https://github.com/eclipse/cloe/commit/ea59553>`_
- nop: Provide a NopLaneSensor component                                 `[fc75ea1] <https://github.com/eclipse/cloe/commit/fc75ea1>`_
.. -
- virtue: Add irrational event                                           `[c672e06] <https://github.com/eclipse/cloe/commit/c672e06>`_
- virtue: Add missing_lane_boundaries event                              `[43af6a6] <https://github.com/eclipse/cloe/commit/43af6a6>`_
- virtue: Add safety event                                               `[83ee4d5] <https://github.com/eclipse/cloe/commit/83ee4d5>`_
.. -
- vtd: Add linking of external models to runtime directory               `[45587b5] <https://github.com/eclipse/cloe/commit/45587b5>`_
- vtd: Add logging option to startup script                              `[5712175] <https://github.com/eclipse/cloe/commit/5712175>`_
- vtd: Add more timers for performance analysis                          `[1598272] <https://github.com/eclipse/cloe/commit/1598272>`_
- vtd: Add vtd-launch script to conan package                            `[c7b1826] <https://github.com/eclipse/cloe/commit/c7b1826>`_
- vtd: Add vtd-setups to conan package                                   `[955a980] <https://github.com/eclipse/cloe/commit/955a980>`_
- vtd: Avoid spin-logging on empty RDB message queue                     `[886c562] <https://github.com/eclipse/cloe/commit/886c562>`_
- vtd: Enable VTD dynamics models                                        `[08e64ce] <https://github.com/eclipse/cloe/commit/08e64ce>`_
- vtd: Fix missing CXX_STANDARD option                                   `[8dd562c] <https://github.com/eclipse/cloe/commit/8dd562c>`_
- vtd: Obtain OSI lane boundaries from ground truth                      `[3310de6] <https://github.com/eclipse/cloe/commit/3310de6>`_
- vtd: Only remove simulation artifacts                                  `[daa98b2] <https://github.com/eclipse/cloe/commit/daa98b2>`_
- vtd: Remove non-recommended startup options                            `[69aa806] <https://github.com/eclipse/cloe/commit/69aa806>`_
- vtd: Set object existence probabilities                                `[8d31704] <https://github.com/eclipse/cloe/commit/8d31704>`_
- vtd: Use vendored vtd package                                          `[a62a118] <https://github.com/eclipse/cloe/commit/a62a118>`_
.. - vtd: Adjust comments and formatting                                 `[51c3919] <https://github.com/eclipse/cloe/commit/51c3919>`_
.. - vtd: Fix conan dependency issue                                     `[db31c21] <https://github.com/eclipse/cloe/commit/db31c21>`_
.. - vtd: Fix minor mistakes in vtd config and test output               `[ad84139] <https://github.com/eclipse/cloe/commit/ad84139>`_
.. - vtd: Fix minor spelling issues                                      `[4acbb68] <https://github.com/eclipse/cloe/commit/4acbb68>`_
.. - vtd: Fix osi dummy sensor model smoketest                           `[4b7ff24] <https://github.com/eclipse/cloe/commit/4b7ff24>`_
.. - vtd: Fix undeclared index variable                                  `[fcb8731] <https://github.com/eclipse/cloe/commit/fcb8731>`_
.. - vtd: Test proper VTD multi-agent behavior                           `[f364c40] <https://github.com/eclipse/cloe/commit/f364c40>`_

**Web UI:**

- ui: Add button to switch between label attributes                      `[aa6ae75] <https://github.com/eclipse/cloe/commit/aa6ae75>`_
- ui: Add canvas recording functionality                                 `[798b3f9] <https://github.com/eclipse/cloe/commit/798b3f9>`_
- ui: Add option to rendor object labels                                 `[06e1c25] <https://github.com/eclipse/cloe/commit/06e1c25>`_
- ui: Add Plotly graph import function for replay                        `[a9102fd] <https://github.com/eclipse/cloe/commit/a9102fd>`_
- ui: Add python cli script to launch data replay                        `[3ed385c] <https://github.com/eclipse/cloe/commit/3ed385c>`_
- ui: Add replay functionality                                           `[f88eba5] <https://github.com/eclipse/cloe/commit/f88eba5>`_
- ui: Add webserver for replay feature                                   `[4ee6475] <https://github.com/eclipse/cloe/commit/4ee6475>`_
- ui: Change rendering color palette                                     `[3d8585b] <https://github.com/eclipse/cloe/commit/3d8585b>`_
- ui: Fix existence probability output                                   `[d77a66a] <https://github.com/eclipse/cloe/commit/d77a66a>`_
- ui: Fix orbit control axes orientation                                 `[4094d04] <https://github.com/eclipse/cloe/commit/4094d04>`_
.. - ui: Bump axios from 0.19.2 to 0.21.1                                `[14c1e85] <https://github.com/eclipse/cloe/commit/14c1e85>`_
.. - ui: Bump three from 0.125.2 to 0.137.0                              `[de9ef0d] <https://github.com/eclipse/cloe/commit/de9ef0d>`_
.. - ui: Code formatting                                                 `[3e74119] <https://github.com/eclipse/cloe/commit/3e74119>`_
.. - ui: Refactoring and layout fixes                                    `[b9cdfe3] <https://github.com/eclipse/cloe/commit/b9cdfe3>`_
.. - ui: Refactor rendering settings                                     `[4a0d21b] <https://github.com/eclipse/cloe/commit/4a0d21b>`_
.. - ui: Several small fixes                                             `[b6c2095] <https://github.com/eclipse/cloe/commit/b6c2095>`_
.. - ui: Update LICENSE-3RD-PARTY.txt                                    `[ce08931] <https://github.com/eclipse/cloe/commit/ce08931>`_
.. - ui: Update math.js to 7.5.1                                         `[98aa6d3] <https://github.com/eclipse/cloe/commit/98aa6d3>`_
.. - ui: Update ThreeJS to 0.125.2                                       `[ba57417] <https://github.com/eclipse/cloe/commit/ba57417>`_
.. - ui: Upgrading some recommended dependencies                         `[a17bed9] <https://github.com/eclipse/cloe/commit/a17bed9>`_

**Vendored Packages:**

- vendor: Fix cpp-netlib package recipe                                  `[cebff63] <https://github.com/eclipse/cloe/commit/cebff63>`_
- vendor: Fix osi sensor object output                                   `[2dea0a0] <https://github.com/eclipse/cloe/commit/2dea0a0>`_
- vendor: Add libbacktrace dependency                                    `[a03739d] <https://github.com/eclipse/cloe/commit/a03739d>`_
- vendor: Add osi dummy sensor for use with vtd                          `[c04414c] <https://github.com/eclipse/cloe/commit/c04414c>`_
- vendor: Add vtd 2.2.0 packages                                         `[6f7d94d] <https://github.com/eclipse/cloe/commit/6f7d94d>`_
- vendor: Fix osi sensor in-source build                                 `[dc28fa9] <https://github.com/eclipse/cloe/commit/dc28fa9>`_
- vendor: Link vtd osi library to runtime directory                      `[2c3c69a] <https://github.com/eclipse/cloe/commit/2c3c69a>`_
- vendor: Make C++11 required for OSI package                            `[f154961] <https://github.com/eclipse/cloe/commit/f154961>`_
- vendor: Update vtd osi plugin version                                  `[032447e] <https://github.com/eclipse/cloe/commit/032447e>`_
- vendor: Use different osi versions for cloe and vtd                    `[476d8ee] <https://github.com/eclipse/cloe/commit/476d8ee>`_
.. - vendor: Bundle needed system libraries with the vtd package         `[2d03edb] <https://github.com/eclipse/cloe/commit/2d03edb>`_
.. - vendor: Change vtd package export skip condition                    `[e376be1] <https://github.com/eclipse/cloe/commit/e376be1>`_
.. - vendor: Fix .gitignore for cpp-netlib and incbin                    `[149938c] <https://github.com/eclipse/cloe/commit/149938c>`_

**Tooling & Dependencies:**

- depends: Pin cli11 dependency to 2.1.2                                 `[0cdb2e8] <https://github.com/eclipse/cloe/commit/0cdb2e8>`_
- depends: Pin boost dependency to 1.69                                  `[0e04650] <https://github.com/eclipse/cloe/commit/0e04650>`_
- depends: Pin fmt dependency to 8.1.1                                   `[2dc7902] <https://github.com/eclipse/cloe/commit/2dc7902>`_
- depends: Pin incbin dependency to 0.88.0                               `[66caf6b] <https://github.com/eclipse/cloe/commit/66caf6b>`_
- depends: Pin inja dependency to 3.3.0                                  `[9e23f02] <https://github.com/eclipse/cloe/commit/9e23f02>`_
- depends: Pin nlohmann_json dependency to 3.10.5                        `[5dd97d7] <https://github.com/eclipse/cloe/commit/5dd97d7>`_
.. - testing: Group tests by component under test                        `[06df551] <https://github.com/eclipse/cloe/commit/06df551>`_
- tooling: Disable boost semver versioning mode                          `[896f2bc] <https://github.com/eclipse/cloe/commit/896f2bc>`_
- tooling: Add authentication and extra parameters to Dockerfiles        `[2bd67c8] <https://github.com/eclipse/cloe/commit/2bd67c8>`_
- tooling: Add BUILD_TYPE argument to Makefile                           `[4cb2bef] <https://github.com/eclipse/cloe/commit/4cb2bef>`_
- tooling: Add package-auto target to Makefile.all                       `[570e05a] <https://github.com/eclipse/cloe/commit/570e05a>`_
- tooling: Add package-debug target to Makefile.package                  `[67fec7e] <https://github.com/eclipse/cloe/commit/67fec7e>`_
- tooling: Add pre-commit configuration                                  `[0833719] <https://github.com/eclipse/cloe/commit/0833719>`_
- tooling: Add test UUIDs                                                `[9e850c1] <https://github.com/eclipse/cloe/commit/9e850c1>`_
- tooling: Add Ubuntu 16.04 Dockerfile                                   `[e893a98] <https://github.com/eclipse/cloe/commit/e893a98>`_
- tooling: Derive package version from git describe                      `[fe8a3e2] <https://github.com/eclipse/cloe/commit/fe8a3e2>`_
- tooling: Do not build vtd plugin by default                            `[7422e3e] <https://github.com/eclipse/cloe/commit/7422e3e>`_
- tooling: Fix installation of documentation dependencies                `[e0d8c33] <https://github.com/eclipse/cloe/commit/e0d8c33>`_
- tooling: Package the cloe meta-package by default                      `[75fb6c5] <https://github.com/eclipse/cloe/commit/75fb6c5>`_
- tooling: Remove export of VERSION file                                 `[db93f33] <https://github.com/eclipse/cloe/commit/db93f33>`_
- tooling: Remove VTD dependency from cloe and cloe-plugin-vtd           `[83265ee] <https://github.com/eclipse/cloe/commit/83265ee>`_
- tooling: Set boost dependency to full package mode                     `[d5447a6] <https://github.com/eclipse/cloe/commit/d5447a6>`_
- tooling: Simplify and streamline Makefiles                             `[0d75409] <https://github.com/eclipse/cloe/commit/0d75409>`_
- tooling: Skip build of VTD related vendor packages by default          `[86dac87] <https://github.com/eclipse/cloe/commit/86dac87>`_
- tooling: Specify override=True in meta-package for overrides           `[e8a17a1] <https://github.com/eclipse/cloe/commit/e8a17a1>`_
- tooling: Upgrade Doxyfile for compatibility with latest Doxygen        `[f118108] <https://github.com/eclipse/cloe/commit/f118108>`_
- tooling: Use buildkit frontend for building Docker images              `[875b93c] <https://github.com/eclipse/cloe/commit/875b93c>`_
.. - tooling: Refactor Makefile.all and Makefile.package                 `[7ca7161] <https://github.com/eclipse/cloe/commit/7ca7161>`_
.. - tooling: Replace &>/dev/null with >/dev/null 2>&1                   `[c4c3561] <https://github.com/eclipse/cloe/commit/c4c3561>`_
.. - tooling: Rework VTD Docker integration                              `[711cc92] <https://github.com/eclipse/cloe/commit/711cc92>`_
.. - tooling: Run unit tests in Conan environment                        `[527fc02] <https://github.com/eclipse/cloe/commit/527fc02>`_
.. - tooling: Add CODEOWNERS configuration                               `[b228ecb] <https://github.com/eclipse/cloe/commit/b228ecb>`_
.. - tooling: Add preliminary release targets                            `[5b46f6c] <https://github.com/eclipse/cloe/commit/5b46f6c>`_
.. - tooling: Add psmisc to required dependencies                        `[07305e8] <https://github.com/eclipse/cloe/commit/07305e8>`_
.. - tooling: Add status target to Makefile                              `[dd14422] <https://github.com/eclipse/cloe/commit/dd14422>`_
.. - tooling: Don't cd to project root in Vim                            `[76cb9de] <https://github.com/eclipse/cloe/commit/76cb9de>`_
.. - tooling: Fix boost dependency mismatch                              `[457cf11] <https://github.com/eclipse/cloe/commit/457cf11>`_
.. - tooling: Fix bugs                                                   `[f748f97] <https://github.com/eclipse/cloe/commit/f748f97>`_
.. - tooling: Fix build failure not registered in Makefile               `[5693d31] <https://github.com/eclipse/cloe/commit/5693d31>`_
.. - tooling: Fix Dockerfiles not cleaning up user auth                  `[32be85d] <https://github.com/eclipse/cloe/commit/32be85d>`_
.. - tooling: Fix docker-release make target                             `[f133856] <https://github.com/eclipse/cloe/commit/f133856>`_
.. - tooling: Fix in Makefile.all for builds with VTD                    `[aa30a90] <https://github.com/eclipse/cloe/commit/aa30a90>`_
.. - tooling: Fix make package not working                               `[1721839] <https://github.com/eclipse/cloe/commit/1721839>`_
.. - tooling: Fix minor errors in Makefiles                              `[7c0cb5f] <https://github.com/eclipse/cloe/commit/7c0cb5f>`_
.. - tooling: Fix missing package version                                `[d096bda] <https://github.com/eclipse/cloe/commit/d096bda>`_
.. - tooling: Fix missing tmux dependency                                `[4542ebb] <https://github.com/eclipse/cloe/commit/4542ebb>`_
.. - tooling: Fix modification of make variables set from command line   `[c1a80be] <https://github.com/eclipse/cloe/commit/c1a80be>`_
.. - tooling: Fix pipx setup option                                      `[4b7e6ff] <https://github.com/eclipse/cloe/commit/4b7e6ff>`_
.. - tooling: Fix reference to non-existent conan.mk                     `[30ba3bc] <https://github.com/eclipse/cloe/commit/30ba3bc>`_
.. - tooling: Fix testname command not found on Ubuntu 18.04             `[974ef64] <https://github.com/eclipse/cloe/commit/974ef64>`_
.. - tooling: Ignore .clangd/ folders                                    `[c99aada] <https://github.com/eclipse/cloe/commit/c99aada>`_
.. - tooling: Improve Makefile documentation                             `[ab3e971] <https://github.com/eclipse/cloe/commit/ab3e971>`_
.. - tooling: Improve Vim codebase navigation                            `[6655837] <https://github.com/eclipse/cloe/commit/6655837>`_
.. - tooling: Move vtd-api from cloe-restricted to cloe user             `[8ec8957] <https://github.com/eclipse/cloe/commit/8ec8957>`_
- all: Add CMAKE_EXPORT_COMPILE_COMMANDS to Conan recipes                `[fd28630] <https://github.com/eclipse/cloe/commit/fd28630>`_
- all: Ensure editable mode works for all packages                       `[2b5cf81] <https://github.com/eclipse/cloe/commit/2b5cf81>`_
- all: Make C++14 required for all packages                              `[77a135a] <https://github.com/eclipse/cloe/commit/77a135a>`_
- all: Simplify CMakeLists.txt for all plugins                           `[5e61078] <https://github.com/eclipse/cloe/commit/5e61078>`_
.. - all: Apply low-hanging automatic Python 3.6+ improvements           `[3beede7] <https://github.com/eclipse/cloe/commit/3beede7>`_
.. - all: Fix boost dependency version to >=1.65.1                       `[5ac0f25] <https://github.com/eclipse/cloe/commit/5ac0f25>`_
.. - all: Fix trailing whitespace and missing line endings               `[4de9afe] <https://github.com/eclipse/cloe/commit/4de9afe>`_
.. - all: Fix typos highlighted by codespell                             `[f996c6c] <https://github.com/eclipse/cloe/commit/f996c6c>`_
.. - all: Strip newline from project version string in conanfiles        `[bf02981] <https://github.com/eclipse/cloe/commit/bf02981>`_
- ci: Add Github Actions workflow to build all Cloe packages             `[bd5266f] <https://github.com/eclipse/cloe/commit/bd5266f>`_
- ci: Add Github Actions workflow to build documentation                 `[78a9dd5] <https://github.com/eclipse/cloe/commit/78a9dd5>`_
- ci: Build cloe on Ubuntu 18.04 and 20.04                               `[70d22ec] <https://github.com/eclipse/cloe/commit/70d22ec>`_
- ci: Build cloe UI with node versions 10, 12, and 14                    `[9001753] <https://github.com/eclipse/cloe/commit/9001753>`_
.. - ci: Only run workflows when they are needed                         `[6ba289e] <https://github.com/eclipse/cloe/commit/6ba289e>`_
.. - ci: Only upload API documentation on release/** branches            `[5ef0d9e] <https://github.com/eclipse/cloe/commit/5ef0d9e>`_
.. - docs: Add usage guide on optimizing performance                     `[082bc68] <https://github.com/eclipse/cloe/commit/082bc68>`_
.. - docs: Add documention for new contributors                          `[71edf40] <https://github.com/eclipse/cloe/commit/71edf40>`_
.. - docs: Add initial Sphinx documentation                              `[fb9719a] <https://github.com/eclipse/cloe/commit/fb9719a>`_
.. - docs: Apply Eclipse branding to index page                          `[48b358c] <https://github.com/eclipse/cloe/commit/48b358c>`_
.. - docs: Fix bug in README and incorrect docs folder name              `[e77c5cd] <https://github.com/eclipse/cloe/commit/e77c5cd>`_
.. - docs: Fix incorrect reference in README                             `[61a559c] <https://github.com/eclipse/cloe/commit/61a559c>`_
.. - docs: Reformat and add some additional notices                      `[e73c0f5] <https://github.com/eclipse/cloe/commit/e73c0f5>`_
.. - docs: Remove duplicate LICENSE file                                 `[380962e] <https://github.com/eclipse/cloe/commit/380962e>`_
.. - docs: Remove outdated reference in NOTICE.md                        `[e78ce0b] <https://github.com/eclipse/cloe/commit/e78ce0b>`_
.. - docs: Use file names recommended by the Eclipse Foundation          `[1db9e17] <https://github.com/eclipse/cloe/commit/1db9e17>`_
.. - docs: Troubleshoot possible Conan-Center timeout                    `[a3d046e] <https://github.com/eclipse/cloe/commit/a3d046e>`_

