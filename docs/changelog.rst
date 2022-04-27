Changelog
=========

The Cloe repository, which you can find at https://github.com/eclipse/cloe, is
composed of multiple "packages", which could potentially be versioned
individually. At the moment, however, we are versioning all together and noting
here what changes for each package.

.. note::
   We currently do not release any binaries, each "release" is just a way of
   referring to a tagged set of all packages.

..
   When creating new release notes, use the following procedure:

   1. Use changelog-gen.sh script to generate grouped entries of commits for
      the release. You can pass it the range it should consider, such as
      `v0.18.0..HEAD` for everything since v0.18.0:

         ./changelog-gen.sh v0.18.0..HEAD

   2. Add **bold** "headings" for the following groups:
      - CLI
      - Engine
      - Core Libraries
      - Plugins
      - Web UI
      - Tooling & Dependencies

   3. Delete items that are not really relevant for the end-user:
      - Changes to documentation
      - Changes to vendored packages
      - Changes to CI
      - Changes to Node dependencies
      - Refactoring
      - Net-zero changes (commits within the release that introduce bugs and
        then fix them, for example)

   4. Change or delete items that are not worded clearly:
      - "Fix bugs" is not very helpful.

   5. Use the following format for the section heading:

         VERSION (YYYY-MM-DD)
         --------------------

         Followed by one or more sentences or paragraphs describing on a high
         level what the release is about or if there are any important breaking
         changes that are relevant.

      Note that the most recent release is at the *top* of the document.


0.18.0 (2022-04-26)
-------------------

This marks the initial "release" of the Cloe packages.

**CLI:**

- cli: Use logging library functions instead of print                    `[0617841] <https://github.com/eclipse/cloe/commit/0617841>`_
- cli: Fix broken logging statements                                     `[dfc3452] <https://github.com/eclipse/cloe/commit/dfc3452>`_
- cli: Add Makefile with install and editable targets                    `[33d831d] <https://github.com/eclipse/cloe/commit/33d831d>`_
- cli: Pass extra arguments to shell command                             `[154828f] <https://github.com/eclipse/cloe/commit/154828f>`_
- cli: Add --conan-arg and --conan-setting options to exec and shell commands `[734944c] <https://github.com/eclipse/cloe/commit/734944c>`_
- cli: Add activate command                                              `[9aca3db] <https://github.com/eclipse/cloe/commit/9aca3db>`_

**Engine:**

- engine: Provide better errors when simulation errors occur             `[e4c94ca] <https://github.com/eclipse/cloe/commit/e4c94ca>`_
- engine: Add interpolation for ${THIS_STACKFILE_DIR} and -FILE          `[072e577] <https://github.com/eclipse/cloe/commit/072e577>`_
- engine: Fix in ComponentConf serialization                             `[0ab2bc2] <https://github.com/eclipse/cloe/commit/0ab2bc2>`_
- engine: Fix package bin path for in-source builds                      `[988bf3d] <https://github.com/eclipse/cloe/commit/988bf3d>`_
- engine: Fix plugin clobbering not working                              `[820ff72] <https://github.com/eclipse/cloe/commit/820ff72>`_
- engine: Stream JSON api data to a file                                 `[08938d6] <https://github.com/eclipse/cloe/commit/08938d6>`_
- engine: Avoid compiler bug in xenial build                             `[4c08424] <https://github.com/eclipse/cloe/commit/4c08424>`_
- engine: Fix missing CXX_STANDARD_REQUIRED for libstack                 `[db0a41f] <https://github.com/eclipse/cloe/commit/db0a41f>`_

**Core Libraries:**

- fable: Set version to project version from conanfile.py                `[cea763a] <https://github.com/eclipse/cloe/commit/cea763a>`_
- fable: Forward-declare make_prototype<> in interface.hpp               `[a868f9a] <https://github.com/eclipse/cloe/commit/a868f9a>`_
- fable: Add extra type traits for working with schema types             `[b0ae81b] <https://github.com/eclipse/cloe/commit/b0ae81b>`_
- fable: Add and use gtest utility functions                             `[902dfc9] <https://github.com/eclipse/cloe/commit/902dfc9>`_
- fable: Fix unorthogonal interface of Struct schema                     `[de9d324] <https://github.com/eclipse/cloe/commit/de9d324>`_
- fable: Fix un-reusable interface of Factory class                      `[d771921] <https://github.com/eclipse/cloe/commit/d771921>`_
- fable: Add to_json() method to all schema types                        `[a97ee64] <https://github.com/eclipse/cloe/commit/a97ee64>`_
- fable: Add CustomDeserializer schema type                              `[d42419e] <https://github.com/eclipse/cloe/commit/d42419e>`_
- fable: Add set_factory() method to Factory schema                      `[3d26e0a] <https://github.com/eclipse/cloe/commit/3d26e0a>`_
- fable: Add examples and documentation                                  `[599da29] <https://github.com/eclipse/cloe/commit/599da29>`_
- fable: Relax version fmt version requirement                           `[d990c19] <https://github.com/eclipse/cloe/commit/d990c19>`_

- runtime: Fix Vehicle error handling                                    `[5376189] <https://github.com/eclipse/cloe/commit/5376189>`_
- runtime: Add SetVariable and SetData trigger actions                   `[d21fbd7] <https://github.com/eclipse/cloe/commit/d21fbd7>`_
- runtime: Support components with multiple inputs                       `[c867eab] <https://github.com/eclipse/cloe/commit/c867eab>`_

- models: Add existence probability to lane boundary and object          `[8e25a97] <https://github.com/eclipse/cloe/commit/8e25a97>`_
- models: Add utility function for coordinate transformation             `[f24216c] <https://github.com/eclipse/cloe/commit/f24216c>`_
- models: Fix actuation state is_consistent() method                     `[34ba08e] <https://github.com/eclipse/cloe/commit/34ba08e>`_
- models: Fix compile error in actuation_state.cpp                       `[8698921] <https://github.com/eclipse/cloe/commit/8698921>`_
- models: Initialize members of LaneBoundary class                       `[f688e32] <https://github.com/eclipse/cloe/commit/f688e32>`_

**Plugins:**

- gndtruth_extractor: Fix compiler warning                               `[6ee61e4] <https://github.com/eclipse/cloe/commit/6ee61e4>`_
- gndtruth_extractor: Replace enumconfable by fable                      `[21e8f53] <https://github.com/eclipse/cloe/commit/21e8f53>`_

- minimator: Provide lanes of a straight road                            `[f9b60c2] <https://github.com/eclipse/cloe/commit/f9b60c2>`_
- minimator: Fix JSON api                                                `[5df6e9d] <https://github.com/eclipse/cloe/commit/5df6e9d>`_

- noisy_sensor: Extend to lane boundaries and refine noise configuration `[a00f64f] <https://github.com/eclipse/cloe/commit/a00f64f>`_
- noisy_sensor: Add trigger for noise activation                         `[f8e488f] <https://github.com/eclipse/cloe/commit/f8e488f>`_

- nop: Provide a NopLaneSensor component                                 `[fc75ea1] <https://github.com/eclipse/cloe/commit/fc75ea1>`_

- virtue: Add irrational event                                           `[c672e06] <https://github.com/eclipse/cloe/commit/c672e06>`_
- virtue: Add safety event                                               `[83ee4d5] <https://github.com/eclipse/cloe/commit/83ee4d5>`_
- virtue: Add missing_lane_boundaries event                              `[43af6a6] <https://github.com/eclipse/cloe/commit/43af6a6>`_

- vtd: Set object existence probabilities                                `[8d31704] <https://github.com/eclipse/cloe/commit/8d31704>`_
- vtd: Obtain OSI lane boundaries from ground truth                      `[3310de6] <https://github.com/eclipse/cloe/commit/3310de6>`_
- vtd: Fix missing CXX_STANDARD option                                   `[8dd562c] <https://github.com/eclipse/cloe/commit/8dd562c>`_
- vtd: Use vendored vtd package                                          `[a62a118] <https://github.com/eclipse/cloe/commit/a62a118>`_
- vtd: Avoid spin-logging on empty RDB message queue                     `[886c562] <https://github.com/eclipse/cloe/commit/886c562>`_
- vtd: Enable VTD dynamics models                                        `[08e64ce] <https://github.com/eclipse/cloe/commit/08e64ce>`_
- vtd: Remove non-recommended startup options                            `[69aa806] <https://github.com/eclipse/cloe/commit/69aa806>`_
- vtd: Add linking of external models to runtime directory               `[45587b5] <https://github.com/eclipse/cloe/commit/45587b5>`_
- vtd: Add vtd-launch script to conan package                            `[c7b1826] <https://github.com/eclipse/cloe/commit/c7b1826>`_
- vtd: Add logging option to startup script                              `[5712175] <https://github.com/eclipse/cloe/commit/5712175>`_
- vtd: Add vtd-setups to conan package                                   `[955a980] <https://github.com/eclipse/cloe/commit/955a980>`_
- vtd: Only remove simulation artifacts                                  `[daa98b2] <https://github.com/eclipse/cloe/commit/daa98b2>`_
- vtd: Add more timers for performance analysis                          `[1598272] <https://github.com/eclipse/cloe/commit/1598272>`_

**Web UI:**

- ui: Add option to render object labels                                 `[06e1c25] <https://github.com/eclipse/cloe/commit/06e1c25>`_
- ui: Change rendering color palette                                     `[3d8585b] <https://github.com/eclipse/cloe/commit/3d8585b>`_
- ui: Fix existence probability output                                   `[d77a66a] <https://github.com/eclipse/cloe/commit/d77a66a>`_
- ui: Fix orbit control axes orientation                                 `[4094d04] <https://github.com/eclipse/cloe/commit/4094d04>`_
- ui: Add replay functionality                                           `[f88eba5] <https://github.com/eclipse/cloe/commit/f88eba5>`_
- ui: Add canvas recording functionality                                 `[798b3f9] <https://github.com/eclipse/cloe/commit/798b3f9>`_
- ui: Add web server for replay feature                                   `[4ee6475] <https://github.com/eclipse/cloe/commit/4ee6475>`_
- ui: Add Plotly graph import function for replay                        `[a9102fd] <https://github.com/eclipse/cloe/commit/a9102fd>`_
- ui: Add python cli script to launch data replay                        `[3ed385c] <https://github.com/eclipse/cloe/commit/3ed385c>`_
- ui: Add button to switch between label attributes                      `[aa6ae75] <https://github.com/eclipse/cloe/commit/aa6ae75>`_

**Tooling & Dependencies:**

- depends: Pin cli11 dependency to 2.1.2                                 `[0cdb2e8] <https://github.com/eclipse/cloe/commit/0cdb2e8>`_
- depends: Pin boost dependency to 1.69                                  `[0e04650] <https://github.com/eclipse/cloe/commit/0e04650>`_
- depends: Pin fmt dependency to 8.1.1                                   `[2dc7902] <https://github.com/eclipse/cloe/commit/2dc7902>`_
- depends: Pin inja dependency to 3.3.0                                  `[9e23f02] <https://github.com/eclipse/cloe/commit/9e23f02>`_
- depends: Pin nlohmann_json dependency to 3.10.5                        `[5dd97d7] <https://github.com/eclipse/cloe/commit/5dd97d7>`_
- depends: Pin incbin dependency to 0.88.0                               `[66caf6b] <https://github.com/eclipse/cloe/commit/66caf6b>`_

- tooling: Remove export of VERSION file                                 `[db93f33] <https://github.com/eclipse/cloe/commit/db93f33>`_
- tooling: Package the cloe meta-package by default                      `[75fb6c5] <https://github.com/eclipse/cloe/commit/75fb6c5>`_
- tooling: Simplify and streamline Makefiles                             `[0d75409] <https://github.com/eclipse/cloe/commit/0d75409>`_
- tooling: Do not build vtd plugin by default                            `[7422e3e] <https://github.com/eclipse/cloe/commit/7422e3e>`_
- tooling: Add package-debug target to Makefile.package                  `[67fec7e] <https://github.com/eclipse/cloe/commit/67fec7e>`_
- tooling: Skip build of VTD related vendor packages by default          `[86dac87] <https://github.com/eclipse/cloe/commit/86dac87>`_
- tooling: Add pre-commit configuration                                  `[0833719] <https://github.com/eclipse/cloe/commit/0833719>`_
- tooling: Add BUILD_TYPE argument to Makefile                           `[4cb2bef] <https://github.com/eclipse/cloe/commit/4cb2bef>`_
- tooling: Add Ubuntu 16.04 Dockerfile                                   `[e893a98] <https://github.com/eclipse/cloe/commit/e893a98>`_
- tooling: Add authentication and extra parameters to Dockerfiles        `[2bd67c8] <https://github.com/eclipse/cloe/commit/2bd67c8>`_
- tooling: Add package-auto target to Makefile.all                       `[570e05a] <https://github.com/eclipse/cloe/commit/570e05a>`_
- tooling: Use buildkit frontend for building Docker images              `[875b93c] <https://github.com/eclipse/cloe/commit/875b93c>`_
- tooling: Derive package version from git describe                      `[fe8a3e2] <https://github.com/eclipse/cloe/commit/fe8a3e2>`_
- tooling: Remove VTD dependency from cloe and cloe-plugin-vtd           `[83265ee] <https://github.com/eclipse/cloe/commit/83265ee>`_
- tooling: Upgrade Doxyfile for compatibility with latest Doxygen        `[f118108] <https://github.com/eclipse/cloe/commit/f118108>`_
- tooling: Fix installation of documentation dependencies                `[e0d8c33] <https://github.com/eclipse/cloe/commit/e0d8c33>`_
- tooling: Set boost dependency to full package mode                     `[d5447a6] <https://github.com/eclipse/cloe/commit/d5447a6>`_
- tooling: Add test UUIDs                                                `[9e850c1] <https://github.com/eclipse/cloe/commit/9e850c1>`_
- tooling: Specify override=True in meta-package for overrides           `[e8a17a1] <https://github.com/eclipse/cloe/commit/e8a17a1>`_

- all: Make C++14 required for all packages                              `[77a135a] <https://github.com/eclipse/cloe/commit/77a135a>`_
- all: Ensure editable mode works for all packages                       `[2b5cf81] <https://github.com/eclipse/cloe/commit/2b5cf81>`_
- all: Simplify CMakeLists.txt for all plugins                           `[5e61078] <https://github.com/eclipse/cloe/commit/5e61078>`_
- all: Add CMAKE_EXPORT_COMPILE_COMMANDS to Conan recipes                `[fd28630] <https://github.com/eclipse/cloe/commit/fd28630>`_
