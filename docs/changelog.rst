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

0.19.0 (2022-11-18)
-------------------

This is the second public minor "release" of the Cloe packages, although there
are some significant changes to the way Cloe is built and tested.

Read all about it :doc:`here <news/release-0.19.0>`.

**CLI**:

- cli: Fix catching recursive shells not working `[d878767] <https://github.com/eclipse/cloe/commit/d8787672d6a3afaf4ef211dd320e99f5e04b9980>`_
- cli: Add --version flag to cloe-launch `[70f3d7d] <https://github.com/eclipse/cloe/commit/70f3d7dbe05e2d3b3b5f82c23f98f6009ca893e7>`_
- cli: Add [cloe-shell] prefix to prompt `[9261331] <https://github.com/eclipse/cloe/commit/92613312ba604d7fc410858cc52d72d5c772a163>`_
- cli: Source "cloe_launch_env.sh" if generated `[14be6ca] <https://github.com/eclipse/cloe/commit/14be6ca76693ef0aab711af16e41acb1ec35c91f>`_
- cli: Add prepare command `[1f6c907] <https://github.com/eclipse/cloe/commit/1f6c90738d205da62836f07fcd1e108f896f7745>`_

**Engine:**

- engine: Add file exporting exit codes of cloe-engine `[01d6138] <https://github.com/eclipse/cloe/commit/01d6138f6634e011a3a1436cc0b0741558441081>`_
- engine: Add brake, steering, wheel, and powertrain sensor to NopVehicle `[8caa31d] <https://github.com/eclipse/cloe/commit/8caa31dace95bf026b4358967f334754729a881d>`_
- engine: Add comment on refresh_buffer() performance `[5fdff7a] <https://github.com/eclipse/cloe/commit/5fdff7a6c1a66d3c91e80fe2860a1cea6c72df62>`_
- engine: Fix Cloe state machine `[ea791f4] <https://github.com/eclipse/cloe/commit/ea791f402b9bc03bd9eb9198331877de6383a58e>`_
- engine: Allow $schema key to be present in a cloe stack file `[d306efa] <https://github.com/eclipse/cloe/commit/d306efa0bef6bdd255341f7c84468466c592b263>`_
- engine: Read several options from environment variables `[8f9731c] <https://github.com/eclipse/cloe/commit/8f9731c67e0d0bf4de123586d9c936e24d5cac1b>`_
- engine: Add --strict and --secure flags `[f44eeb5] <https://github.com/eclipse/cloe/commit/f44eeb5c4c00883f560b88d381079d09401fa4b3>`_
- engine: Make server an optional component `[1a4ab65] <https://github.com/eclipse/cloe/commit/1a4ab6564caf86cd8eaed07490aa41c5853d2da8>`_
- engine: Replace direct use of oak types with ServerRegistrar interface `[ac3a7fc] <https://github.com/eclipse/cloe/commit/ac3a7fcc2d027c12ac1d226b01ebd747caa69ff1>`_
- engine: Refactor server into interface and implementation `[d8c826a] <https://github.com/eclipse/cloe/commit/d8c826a21f1a2acb1ed9039552d693f32b45037e>`_
- engine: Fix compilation error due to missing <thread> include `[68ec539] <https://github.com/eclipse/cloe/commit/68ec539cb3292389ebd7fc666af60f3810547d99>`_
- engine: Fix compilation error due to unused variable `[b95bdd4] <https://github.com/eclipse/cloe/commit/b95bdd48c4a27c6eb33191e1e5a36d6940dbb9fc>`_
- engine: Remove deprecated use of std::binary_function `[806b8ea] <https://github.com/eclipse/cloe/commit/806b8eabe6b4ceee5e81b7692b8f7bf1e56d4364>`_

**Core Libraries:**

- fable: Fix incorrect JSON schema output in some edge cases (WIP) `[ec5b8cb] <https://github.com/eclipse/cloe/commit/ec5b8cb81dad81623e6fd9b54504ef3c463ce4bd>`_
- fable: Accept // comments in JSON files `[b891da9] <https://github.com/eclipse/cloe/commit/b891da96d7be47d9cd34a2e2eb12157f64963a55>`_

- models: Add gearbox, pedal and steering actuator. `[40d128e] <https://github.com/eclipse/cloe/commit/40d128e492b697d7658b381a5c860f1f18bfb33d>`_
- models: Add brake, steering, wheel, and powertrain sensors `[09e14fd] <https://github.com/eclipse/cloe/commit/09e14fdaeb49a0ec23b52525a2576525f59afed1>`_
- models: Bump eigen dependency from 3.3.7 to 3.4.0 `[1a390ac] <https://github.com/eclipse/cloe/commit/1a390ac24a88f44804d6cc5c6998e01ab905672d>`_

- runtime: Use fable::parse_json instead of Json::parse `[e8fd51a] <https://github.com/eclipse/cloe/commit/e8fd51a9afe2e71c81e38f2bab4e682602a54be3>`_
- runtime: Fix assignment of temporary reference `[64cf1f2] <https://github.com/eclipse/cloe/commit/64cf1f29a6e1a7ea61c3de92c6b77c95e1d96b8e>`_

**Plugins:**

- vtd: Add git describe to profile_default `[658efcc] <https://github.com/eclipse/cloe/commit/658efcc936c8fae45b9591ad5b96ac98480d9cd9>`_
- vtd: Move vtd with dependencies into optional/vtd directory `[c69fc3c] <https://github.com/eclipse/cloe/commit/c69fc3c32ad9edcf99079399663e125ea398fa7b>`_

**Web UI:**

- ui: Fix wrong dir in Makefile and remove timeout in webserver `[7d2e5f4] <https://github.com/eclipse/cloe/commit/7d2e5f43227b96a2be74881f11d7e23da481bffc>`_
- ui: Fix github run pipeline for node > 16 `[d36cddb] <https://github.com/eclipse/cloe/commit/d36cddb83bccbd676cb5ed6ba41c0a3bfcbed019>`_

**Tooling & Dependencies:**

- tooling: Refactor tests significantly `[9ef417d] <https://github.com/eclipse/cloe/commit/9ef417dd3a237b2fbffd8573cb34d055bafe17b3>`_
- tooling: Modify test profiles to specify environment variables `[1fd969d] <https://github.com/eclipse/cloe/commit/1fd969de0499406a28dae0c6af02d8c4c62aee22>`_
- tooling: Build ui with current supported Node versions `[9ed0d2e] <https://github.com/eclipse/cloe/commit/9ed0d2e0dac681d101b39dd76b2df84639699321>`_
- tooling: Simplify Makefiles and make them more user-friendly `[cd20202] <https://github.com/eclipse/cloe/commit/cd2020299cabbde650db41d446d5b1851932ac4d>`_
- tooling: Rename package-auto target to package `[55645a2] <https://github.com/eclipse/cloe/commit/55645a237676963b32fff5496dbe59ae4740eb2b>`_
- tooling: Streamline in-source builds `[fe1882b] <https://github.com/eclipse/cloe/commit/fe1882bef55bb3b1feb5e4eb475378baa4136b34>`_
- tooling: Add setup-conan target to Makefile.setup `[de41391] <https://github.com/eclipse/cloe/commit/de413913260aa129dfe8cd106c13689b140573b9>`_
- tooling: Fix version "unknown" when using git worktree `[4227f93] <https://github.com/eclipse/cloe/commit/4227f93695ef13fd62ce7f08b7f613c7d7970c4e>`_
- tooling: Fix mismatch of fmt version between engine and cloe `[e903bea] <https://github.com/eclipse/cloe/commit/e903bea4d74095cf761b51d9342948c8c4b5b784>`_
- tooling: Add boost override if engine server enabled `[fe6751e] <https://github.com/eclipse/cloe/commit/fe6751e1a0b7311ffe536ea425e74a9307c57663>`_
- tooling: Fix package_id affected by test and pedantic options `[3f0a62c] <https://github.com/eclipse/cloe/commit/3f0a62c14227430dceabcf0d5dc917b9b41bc184>`_
- tooling: Don't build unnecessary vendor packages `[0205b3e] <https://github.com/eclipse/cloe/commit/0205b3e71f8d0433c253f2822219d7b9df1b06bc>`_
- tooling: Fix .editorconfig rst indent setting from 3 to 4 `[a9160e4] <https://github.com/eclipse/cloe/commit/a9160e41e7ab6eef02fe4c61fce75588cadc0b25>`_
- tooling: Fix `make status` broken `[ee9b264] <https://github.com/eclipse/cloe/commit/ee9b264773f0dc9f031324abd3aa79b86df64418>`_
- tooling: Improve handling of editable files `[2a8c994] <https://github.com/eclipse/cloe/commit/2a8c994e4c61513414e51263febbc796a2ce2cd4>`_
- tooling: Don't set default BUILD_TYPE in Makefile `[771a7f5] <https://github.com/eclipse/cloe/commit/771a7f55025dbfc0359b1de810085c3092d44148>`_
- tooling: Add set_version() to conanfiles `[fb4741f] <https://github.com/eclipse/cloe/commit/fb4741ff38dfd203280d23935455c6b83ca9466a>`_
- tooling: Add option to specify lockfile generation `[382828a] <https://github.com/eclipse/cloe/commit/382828ae652342da76bc4ce54edfaf6e39288668>`_
- tooling: Verify options are set to 0 or 1 `[3068330] <https://github.com/eclipse/cloe/commit/3068330051057906af8a7775b1d6619b6d5c4143>`_
- tooling: Fix KEEP_SOURCES build-arg set by WITH_VTD `[a4ade4f] <https://github.com/eclipse/cloe/commit/a4ade4f806e9bc5e5765ac6410dc4edc573718c3>`_

- docker: Remove VTD configuration and drop support for Ubuntu 16.04 `[907095d] <https://github.com/eclipse/cloe/commit/907095dacdbd1dbe5fbc1800330c3ee4e260ae60>`_
- docker: Remove DEBUG option in favor of BUILDKIT_PROGRESS `[eabb9da] <https://github.com/eclipse/cloe/commit/eabb9da0c7867eea77f8c545ab66872b424ddf95>`_

- vendor: Remove bincrafters/stable dependencies `[c621be9] <https://github.com/eclipse/cloe/commit/c621be94279395f38367c0beb084f448bd639735>`_
- vendor: Improve documentation of vtd installation `[f93a949] <https://github.com/eclipse/cloe/commit/f93a949a7d0ab1f24b66af157f48188db975a6e7>`_
- vendor: Export cloe/vtd-conan-package Docker image with Ubuntu 18:04 `[40b9abe] <https://github.com/eclipse/cloe/commit/40b9abe108fccb1d9b1d7fd34d27a2258ef92954>`_


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
