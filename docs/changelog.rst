Changelog
=========

The Cloe repository, which you can find at https://github.com/eclipse/cloe, is
composed of multiple "packages", which could potentially be versioned
individually. At the moment, however, we are versioning all together and noting
here what changes for each package.

The following change-log contains grouped selections of commits that are part
of a release; some commits may be missing. See the :doc:`news` for a more
readable perspective on new releases.

.. note::
   We currently do not release any binaries, each "release" is just a way of
   referring to a tagged set of all packages.

..
   TODO(release) // Update release change log

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
      - changes to documentation
      - changes to vendored packages
      - changes to CI
      - changes to Node dependencies
      - refactoring
      - net-zero changes (commits within the release that introduce bugs and
        then fix them, for example)

   4. Change or delete items that are not worded clearly:
      - "Fix bugs" is not very helpful.

   5. Use the following format for the section heading:

         VERSION (YYYY-MM-DD)
         --------------------

         Followed by one or more sentences or paragraphs describing on a high
         level what the release is about or if there are any important breaking
         changes that are relevant.

         Link to the human-readable news article.

      Note that the most recent release is at the *top* of the document.

0.20.1 (2023-07-28)
-------------------

This is a bugfix release to 0.20.0 that fixes some compile issues.

**Core Libraries:**

fable: Instantiate missing Number<signed char> `[035ed74] <https://github.com/eclipse/cloe/commit/035ed7423365e0629592d00b768db96670ba7538>`_

**Tooling & Dependencies:**

tooling: Ensure minimum GCC version of 8 `[4a9bbbf] <https://github.com/eclipse/cloe/commit/4a9bbbfbd1668c7acab31efc3bd82efbb2423f79>`_

0.20.0 (2023-04-03)
-------------------

This is the third public minor release of the Cloe packages.
Read all about it :doc:`here <news/release-0.20.0>`.

**CLI:**

- cli: Provide better error message behavior when prepare fails `[cff17e3] <https://github.com/eclipse/cloe/commit/cff17e3ee8d2cff1783ba1c3602b1bcf5450cfbf>`_

**Engine:**

- engine: Support stack minor versions and bump to "4.1" `[751fa28] <https://github.com/eclipse/cloe/commit/751fa28317407cd8b9a215ed2bc8bc634f6a8d45>`_
- engine: Add `conceal` key to trigger conf `[385b5e4] <https://github.com/eclipse/cloe/commit/385b5e40285cb8a25f94ba0ffa94ad071f9acc8f>`_
- engine: Add `optional` parameter to trigger configuration `[918f795] <https://github.com/eclipse/cloe/commit/918f79587bb05bc20c80204bdb7a6a0911b29917>`_
- engine: Improve error handling of invalid triggers `[87b6cf5] <https://github.com/eclipse/cloe/commit/87b6cf5a94bab60a5da5599d322345dce6e583a7>`_

**Core Libraries:**

- fable: Update examples to use modern CMake `[6f06b12] <https://github.com/eclipse/cloe/commit/6f06b128f435ed7ed1199df4c92df13610e5e360>`_
- fable: Extend String schema with enum_of method `[70d5760] <https://github.com/eclipse/cloe/commit/70d57607ad7b3c54946ef86ce77b8ba64f3ec4e8>`_
- fable: Extend gtest.hpp utility header `[1a97427] <https://github.com/eclipse/cloe/commit/1a97427804599d977c94444ba74ea1b0fff93e3c>`_
- fable: Extract Number<T> implementation into number_impl.hpp `[e104e76] <https://github.com/eclipse/cloe/commit/e104e7677959f023fc4d5cd00b37b590de6be5a6>`_
- fable: Check key existence with contains method, not at `[b9aafa8] <https://github.com/eclipse/cloe/commit/b9aafa80873e69817032d7941bb0cecf05419238>`_

- runtime: Add ModelStop exception to signal simulation stop `[c78a4ef] <https://github.com/eclipse/cloe/commit/c78a4ef3d3e6bd58eb69fba9c5ebf97283fa8a5c>`_
- runtime: Fix error in utility/inja.hpp header `[68634ca] <https://github.com/eclipse/cloe/commit/68634ca448ed1940d04be5e2086850ac00e33a36>`_

- models: Add driver request component `[11a5dfe] <https://github.com/eclipse/cloe/commit/11a5dfe391a44642f799125b940b432e2bf627be>`_
- models: Add vehicle state model component `[157e999] <https://github.com/eclipse/cloe/commit/157e9997e2c235131ff87c2922becd1f68cd8f6f>`_
- models: Allow overriding of actuation methods `[9e738c4] <https://github.com/eclipse/cloe/commit/9e738c44d7fc5c75e08f4320151604517b1a0266>`_
- models: Add geometry utility functions `[9e9169e] <https://github.com/eclipse/cloe/commit/9e9169ed55df235282a18ad05524c8fa57f43c07>`_

**Plugins:**

- basic: Add option for setting driver request `[dd7ec17] <https://github.com/eclipse/cloe/commit/dd7ec174a9531dbaf381feaf4b227296ad8c622b>`_

- minimator: Fix assertion failed on abort `[b60f8be] <https://github.com/eclipse/cloe/commit/b60f8bedb25010fa2f2e60c8c1d98f77dcc9d6bb>`_

- vtd: Add external ego model class `[e2c724f] <https://github.com/eclipse/cloe/commit/e2c724f08bf152876253fb80161913220f5407c8>`_
- vtd: Set actuation after sensor update `[42a5ec9] <https://github.com/eclipse/cloe/commit/42a5ec9d84623691370e29cc3261e5fdc88a09f2>`_
- vtd: Support actuation requests from driver `[2c7f356] <https://github.com/eclipse/cloe/commit/2c7f35690e712f1f53d3108e05166651c2b93ee8>`_
- vtd: Add SCP Action `[f356001] <https://github.com/eclipse/cloe/commit/f356001b2df4fdd9b5a58254348414705108cfc0>`_
- vtd: Allow vendor package selection orthogonal to cloe `[4969e08] <https://github.com/eclipse/cloe/commit/4969e088a577ce1db6b71815b0ecd71537483499>`_
- vtd: Fix use of protobuf deprecated function use ByteSize `[a6a0548] <https://github.com/eclipse/cloe/commit/a6a0548d026aee02f302dcb2d7d8b57603bd36d7>`_
- vtd: Handle scenario where VTD sends Stop signal `[3dc3236] <https://github.com/eclipse/cloe/commit/3dc323664aa75d050aaa6b9639319a2643c42d41>`_
- vtd: Change compression method to avoid revision change `[f3a8b17] <https://github.com/eclipse/cloe/commit/f3a8b170b7bc981dcd45bfe17e8e702aa61e9b14>`_
- vtd: Add vtd setups for 2022.3 `[ec3a14c] <https://github.com/eclipse/cloe/commit/ec3a14c57c6732a7c5a819de48c29c3c5f952040>`_
- vtd: Add vtd-2022.3 package `[880bb2e] <https://github.com/eclipse/cloe/commit/880bb2e295c688b64a212e478bf23ec99baf8a7b>`_
- vtd: Add vtd-api-2022.3 package `[f564d1b] <https://github.com/eclipse/cloe/commit/f564d1b9d4619a5bf7af6bd344c8d66262244306>`_
- vtd: Move vtd to vtd-2.2.0 and vtd-api to vtd-api-2.2.0 `[74ffe1c] <https://github.com/eclipse/cloe/commit/74ffe1ca30bde93e47eb4f6ef43743c561952ade>`_
- vtd: Add support for xosc v1.0 `[83103e6] <https://github.com/eclipse/cloe/commit/83103e6853f82385cfa44109a356ea67a42ab2c9>`_

**Tooling & Dependencies:**

Some notable changes that didn't fit cleanly in the changelog below are:

- Most Conan packages have been updated to support use with Conan 2.0.
  The tooling in the project is still limited to Conan 1.X though.

- Smoketests in the project have been renamed from ``tests/profile_*``
  to ``tests/conanfile_`` to prevent confusion with Conan profiles.

- tooling: Change Conan build policy to outdated by default `[61fba38] <https://github.com/eclipse/cloe/commit/61fba381a72d077b747d5cd9580e2e9aaa1a98e2>`_
- tooling: Fix incorrect installation of Conan profiles `[aa8d04a] <https://github.com/eclipse/cloe/commit/aa8d04a44e7b3d67b09c8d25d6a70cb48857692d>`_
- tooling: Allow conanfiles used by smoketest to be overridden `[50c9d95] <https://github.com/eclipse/cloe/commit/50c9d95458e81fad58cee1900ff53d1cac647ab6>`_
- tooling: Fix smoketest-deps continuing after failure `[19cd6cc] <https://github.com/eclipse/cloe/commit/19cd6cc33a1cc4c53502c9d68e27ab323b7bcc6c>`_
- tooling: Fix warning from missing default build profile `[2038c80] <https://github.com/eclipse/cloe/commit/2038c80fa94ba3e033e966796da15f9fdfd35272>`_
- tooling: Limit Conan installation to <2.0 `[d27bbcb] <https://github.com/eclipse/cloe/commit/d27bbcbed577ce38ba7abb8c3dee6121b703d92a>`_
- tooling: Handle GCC versions >= 11 `[64936d6] <https://github.com/eclipse/cloe/commit/64936d6b306a58f704d95ccb879fc646ed0fd589>`_
- tooling: Use CMake standard BUILD_TESTING variable `[1b31578] <https://github.com/eclipse/cloe/commit/1b3157898dbaad9073f5a7b8cfb48853bb2d5963>`_
- tooling: Ensure an up-to-date (>= 3.15) CMake is configured `[f5ffe92] <https://github.com/eclipse/cloe/commit/f5ffe929b514e94aab254758a00a0c90895d2f31>`_
- tooling: Bump required CMake version from 3.7 to 3.15 `[37e6078] <https://github.com/eclipse/cloe/commit/37e6078037780c1d0808eda799702fa8397afb0d>`_

- docker: Provide more robust setup.sh.example file `[1fc57ed] <https://github.com/eclipse/cloe/commit/1fc57edf74cdb057d9c1104be87392d6f0305a03>`_
- docker: Fix and extend setup.sh.example `[e304d15] <https://github.com/eclipse/cloe/commit/e304d1520d3bc8bd481d72c31d59b90921376312>`_
- docker: Use /bin/bash as SHELL to support setup.sh functions `[0d58bf5] <https://github.com/eclipse/cloe/commit/0d58bf59caf1086b600eaefaafebdda47b43c3a7>`_
- docker: Fix --build-arg passing from Makefile `[fab9c13] <https://github.com/eclipse/cloe/commit/fab9c13c8af34bdef77e736aa59e2ae6ba5e5c58>`_

- vendor: Update openssl require to 1.1.1t for cpp-netlib `[3f793df] <https://github.com/eclipse/cloe/commit/3f793dfe81d4ca94cad603d7ff3ac125e01155a7>`_
- vendor: Update cpp-netlib requires openssl/1.1.1s `[a942a45] <https://github.com/eclipse/cloe/commit/a942a45fda67be3a7af6da18a7b54699800eab9c>`_
- vendor: Use incbin from Conan Center `[1dd42fc] <https://github.com/eclipse/cloe/commit/1dd42fc2a46936a75bf63b44fcf0532a0bbbd0dd>`_
- vendor: Remove bundled libbacktrace `[df6994c] <https://github.com/eclipse/cloe/commit/df6994c4a8e4afb77a3dee9d079f6f8d040e6883>`_

0.19.0 (2022-12-05)
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

- fable: Fix incorrect JSON schema output in some edge cases `[ec5b8cb] <https://github.com/eclipse/cloe/commit/ec5b8cb81dad81623e6fd9b54504ef3c463ce4bd>`_
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

- tooling: Improve Makefile maintainability `[454e5bc] <https://github.com/eclipse/cloe/commit/454e5bc65af69995452d63bf054b57973c97e801>`_
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
- tooling: Fix make status broken `[ee9b264] <https://github.com/eclipse/cloe/commit/ee9b264773f0dc9f031324abd3aa79b86df64418>`_
- tooling: Improve handling of editable files `[2a8c994] <https://github.com/eclipse/cloe/commit/2a8c994e4c61513414e51263febbc796a2ce2cd4>`_
- tooling: Don't set default BUILD_TYPE in Makefile `[771a7f5] <https://github.com/eclipse/cloe/commit/771a7f55025dbfc0359b1de810085c3092d44148>`_
- tooling: Add set_version() to conanfiles `[fb4741f] <https://github.com/eclipse/cloe/commit/fb4741ff38dfd203280d23935455c6b83ca9466a>`_
- tooling: Add option to specify lockfile generation `[382828a] <https://github.com/eclipse/cloe/commit/382828ae652342da76bc4ce54edfaf6e39288668>`_
- tooling: Verify options are set to 0 or 1 `[3068330] <https://github.com/eclipse/cloe/commit/3068330051057906af8a7775b1d6619b6d5c4143>`_
- tooling: Fix KEEP_SOURCES build-arg set by WITH_VTD `[a4ade4f] <https://github.com/eclipse/cloe/commit/a4ade4f806e9bc5e5765ac6410dc4edc573718c3>`_

- docker: Simplify Docker builds `[e7aa389] <https://github.com/eclipse/cloe/commit/e7aa389b3d5a35ff84e24d6522d16470165983f2>`_
- docker: Remove VTD configuration and drop support for Ubuntu 16.04 `[907095d] <https://github.com/eclipse/cloe/commit/907095dacdbd1dbe5fbc1800330c3ee4e260ae60>`_
- docker: Remove DEBUG option in favor of BUILDKIT_PROGRESS `[eabb9da] <https://github.com/eclipse/cloe/commit/eabb9da0c7867eea77f8c545ab66872b424ddf95>`_

- vendor: Remove bincrafters/stable dependencies `[c621be9] <https://github.com/eclipse/cloe/commit/c621be94279395f38367c0beb084f448bd639735>`_
- vendor: Improve documentation of vtd installation `[f93a949] <https://github.com/eclipse/cloe/commit/f93a949a7d0ab1f24b66af157f48188db975a6e7>`_
- vendor: Export cloe/vtd-conan-package Docker image with Ubuntu 18:04 `[40b9abe] <https://github.com/eclipse/cloe/commit/40b9abe108fccb1d9b1d7fd34d27a2258ef92954>`_


0.18.0 (2022-04-26)
-------------------

This marks the initial "release" of the Cloe packages.

**CLI:**

- cli: Use logging library functions instead of print `[0617841] <https://github.com/eclipse/cloe/commit/0617841>`_
- cli: Fix broken logging statements `[dfc3452] <https://github.com/eclipse/cloe/commit/dfc3452>`_
- cli: Add Makefile with install and editable targets `[33d831d] <https://github.com/eclipse/cloe/commit/33d831d>`_
- cli: Pass extra arguments to shell command `[154828f] <https://github.com/eclipse/cloe/commit/154828f>`_
- cli: Add --conan-arg and --conan-setting options to exec and shell commands `[734944c] <https://github.com/eclipse/cloe/commit/734944c>`_
- cli: Add activate command `[9aca3db] <https://github.com/eclipse/cloe/commit/9aca3db>`_

**Engine:**

- engine: Provide better errors when simulation errors occur `[e4c94ca] <https://github.com/eclipse/cloe/commit/e4c94ca>`_
- engine: Add interpolation for ${THIS_STACKFILE_DIR} and -FILE `[072e577] <https://github.com/eclipse/cloe/commit/072e577>`_
- engine: Fix in ComponentConf serialization `[0ab2bc2] <https://github.com/eclipse/cloe/commit/0ab2bc2>`_
- engine: Fix package bin path for in-source builds `[988bf3d] <https://github.com/eclipse/cloe/commit/988bf3d>`_
- engine: Fix plugin clobbering not working `[820ff72] <https://github.com/eclipse/cloe/commit/820ff72>`_
- engine: Stream JSON api data to a file `[08938d6] <https://github.com/eclipse/cloe/commit/08938d6>`_
- engine: Avoid compiler bug in xenial build `[4c08424] <https://github.com/eclipse/cloe/commit/4c08424>`_
- engine: Fix missing CXX_STANDARD_REQUIRED for libstack `[db0a41f] <https://github.com/eclipse/cloe/commit/db0a41f>`_

**Core Libraries:**

- fable: Set version to project version from conanfile.py `[cea763a] <https://github.com/eclipse/cloe/commit/cea763a>`_
- fable: Forward-declare make_prototype<> in interface.hpp `[a868f9a] <https://github.com/eclipse/cloe/commit/a868f9a>`_
- fable: Add extra type traits for working with schema types `[b0ae81b] <https://github.com/eclipse/cloe/commit/b0ae81b>`_
- fable: Add and use gtest utility functions `[902dfc9] <https://github.com/eclipse/cloe/commit/902dfc9>`_
- fable: Fix unorthogonal interface of Struct schema `[de9d324] <https://github.com/eclipse/cloe/commit/de9d324>`_
- fable: Fix un-reusable interface of Factory class `[d771921] <https://github.com/eclipse/cloe/commit/d771921>`_
- fable: Add to_json() method to all schema types `[a97ee64] <https://github.com/eclipse/cloe/commit/a97ee64>`_
- fable: Add CustomDeserializer schema type `[d42419e] <https://github.com/eclipse/cloe/commit/d42419e>`_
- fable: Add set_factory() method to Factory schema `[3d26e0a] <https://github.com/eclipse/cloe/commit/3d26e0a>`_
- fable: Add examples and documentation `[599da29] <https://github.com/eclipse/cloe/commit/599da29>`_
- fable: Relax version fmt version requirement `[d990c19] <https://github.com/eclipse/cloe/commit/d990c19>`_

- runtime: Fix Vehicle error handling `[5376189] <https://github.com/eclipse/cloe/commit/5376189>`_
- runtime: Add SetVariable and SetData trigger actions `[d21fbd7] <https://github.com/eclipse/cloe/commit/d21fbd7>`_
- runtime: Support components with multiple inputs `[c867eab] <https://github.com/eclipse/cloe/commit/c867eab>`_

- models: Add existence probability to lane boundary and object `[8e25a97] <https://github.com/eclipse/cloe/commit/8e25a97>`_
- models: Add utility function for coordinate transformation `[f24216c] <https://github.com/eclipse/cloe/commit/f24216c>`_
- models: Fix actuation state is_consistent() method `[34ba08e] <https://github.com/eclipse/cloe/commit/34ba08e>`_
- models: Fix compile error in actuation_state.cpp `[8698921] <https://github.com/eclipse/cloe/commit/8698921>`_
- models: Initialize members of LaneBoundary class `[f688e32] <https://github.com/eclipse/cloe/commit/f688e32>`_

**Plugins:**

- gndtruth_extractor: Fix compiler warning `[6ee61e4] <https://github.com/eclipse/cloe/commit/6ee61e4>`_
- gndtruth_extractor: Replace enumconfable by fable `[21e8f53] <https://github.com/eclipse/cloe/commit/21e8f53>`_

- minimator: Provide lanes of a straight road `[f9b60c2] <https://github.com/eclipse/cloe/commit/f9b60c2>`_
- minimator: Fix JSON api `[5df6e9d] <https://github.com/eclipse/cloe/commit/5df6e9d>`_

- noisy_sensor: Extend to lane boundaries and refine noise configuration `[a00f64f] <https://github.com/eclipse/cloe/commit/a00f64f>`_
- noisy_sensor: Add trigger for noise activation `[f8e488f] <https://github.com/eclipse/cloe/commit/f8e488f>`_

- nop: Provide a NopLaneSensor component `[fc75ea1] <https://github.com/eclipse/cloe/commit/fc75ea1>`_

- virtue: Add irrational event `[c672e06] <https://github.com/eclipse/cloe/commit/c672e06>`_
- virtue: Add safety event `[83ee4d5] <https://github.com/eclipse/cloe/commit/83ee4d5>`_
- virtue: Add missing_lane_boundaries event `[43af6a6] <https://github.com/eclipse/cloe/commit/43af6a6>`_

- vtd: Set object existence probabilities `[8d31704] <https://github.com/eclipse/cloe/commit/8d31704>`_
- vtd: Obtain OSI lane boundaries from ground truth `[3310de6] <https://github.com/eclipse/cloe/commit/3310de6>`_
- vtd: Fix missing CXX_STANDARD option `[8dd562c] <https://github.com/eclipse/cloe/commit/8dd562c>`_
- vtd: Use vendored vtd package `[a62a118] <https://github.com/eclipse/cloe/commit/a62a118>`_
- vtd: Avoid spin-logging on empty RDB message queue `[886c562] <https://github.com/eclipse/cloe/commit/886c562>`_
- vtd: Enable VTD dynamics models `[08e64ce] <https://github.com/eclipse/cloe/commit/08e64ce>`_
- vtd: Remove non-recommended startup options `[69aa806] <https://github.com/eclipse/cloe/commit/69aa806>`_
- vtd: Add linking of external models to runtime directory `[45587b5] <https://github.com/eclipse/cloe/commit/45587b5>`_
- vtd: Add vtd-launch script to conan package `[c7b1826] <https://github.com/eclipse/cloe/commit/c7b1826>`_
- vtd: Add logging option to startup script `[5712175] <https://github.com/eclipse/cloe/commit/5712175>`_
- vtd: Add vtd-setups to conan package `[955a980] <https://github.com/eclipse/cloe/commit/955a980>`_
- vtd: Only remove simulation artifacts `[daa98b2] <https://github.com/eclipse/cloe/commit/daa98b2>`_
- vtd: Add more timers for performance analysis `[1598272] <https://github.com/eclipse/cloe/commit/1598272>`_

**Web UI:**

- ui: Add option to render object labels `[06e1c25] <https://github.com/eclipse/cloe/commit/06e1c25>`_
- ui: Change rendering color palette `[3d8585b] <https://github.com/eclipse/cloe/commit/3d8585b>`_
- ui: Fix existence probability output `[d77a66a] <https://github.com/eclipse/cloe/commit/d77a66a>`_
- ui: Fix orbit control axes orientation `[4094d04] <https://github.com/eclipse/cloe/commit/4094d04>`_
- ui: Add replay functionality `[f88eba5] <https://github.com/eclipse/cloe/commit/f88eba5>`_
- ui: Add canvas recording functionality `[798b3f9] <https://github.com/eclipse/cloe/commit/798b3f9>`_
- ui: Add web server for replay feature `[4ee6475] <https://github.com/eclipse/cloe/commit/4ee6475>`_
- ui: Add Plotly graph import function for replay `[a9102fd] <https://github.com/eclipse/cloe/commit/a9102fd>`_
- ui: Add python cli script to launch data replay `[3ed385c] <https://github.com/eclipse/cloe/commit/3ed385c>`_
- ui: Add button to switch between label attributes `[aa6ae75] <https://github.com/eclipse/cloe/commit/aa6ae75>`_

**Tooling & Dependencies:**

- depends: Pin cli11 dependency to 2.1.2 `[0cdb2e8] <https://github.com/eclipse/cloe/commit/0cdb2e8>`_
- depends: Pin boost dependency to 1.69 `[0e04650] <https://github.com/eclipse/cloe/commit/0e04650>`_
- depends: Pin fmt dependency to 8.1.1 `[2dc7902] <https://github.com/eclipse/cloe/commit/2dc7902>`_
- depends: Pin inja dependency to 3.3.0 `[9e23f02] <https://github.com/eclipse/cloe/commit/9e23f02>`_
- depends: Pin nlohmann_json dependency to 3.10.5 `[5dd97d7] <https://github.com/eclipse/cloe/commit/5dd97d7>`_
- depends: Pin incbin dependency to 0.88.0 `[66caf6b] <https://github.com/eclipse/cloe/commit/66caf6b>`_

- tooling: Remove export of VERSION file `[db93f33] <https://github.com/eclipse/cloe/commit/db93f33>`_
- tooling: Package the cloe meta-package by default `[75fb6c5] <https://github.com/eclipse/cloe/commit/75fb6c5>`_
- tooling: Simplify and streamline Makefiles `[0d75409] <https://github.com/eclipse/cloe/commit/0d75409>`_
- tooling: Do not build vtd plugin by default `[7422e3e] <https://github.com/eclipse/cloe/commit/7422e3e>`_
- tooling: Add package-debug target to Makefile.package `[67fec7e] <https://github.com/eclipse/cloe/commit/67fec7e>`_
- tooling: Skip build of VTD related vendor packages by default `[86dac87] <https://github.com/eclipse/cloe/commit/86dac87>`_
- tooling: Add pre-commit configuration `[0833719] <https://github.com/eclipse/cloe/commit/0833719>`_
- tooling: Add BUILD_TYPE argument to Makefile `[4cb2bef] <https://github.com/eclipse/cloe/commit/4cb2bef>`_
- tooling: Add Ubuntu 16.04 Dockerfile `[e893a98] <https://github.com/eclipse/cloe/commit/e893a98>`_
- tooling: Add authentication and extra parameters to Dockerfiles `[2bd67c8] <https://github.com/eclipse/cloe/commit/2bd67c8>`_
- tooling: Add package-auto target to Makefile.all `[570e05a] <https://github.com/eclipse/cloe/commit/570e05a>`_
- tooling: Use buildkit frontend for building Docker images `[875b93c] <https://github.com/eclipse/cloe/commit/875b93c>`_
- tooling: Derive package version from git describe `[fe8a3e2] <https://github.com/eclipse/cloe/commit/fe8a3e2>`_
- tooling: Remove VTD dependency from cloe and cloe-plugin-vtd `[83265ee] <https://github.com/eclipse/cloe/commit/83265ee>`_
- tooling: Upgrade Doxyfile for compatibility with latest Doxygen `[f118108] <https://github.com/eclipse/cloe/commit/f118108>`_
- tooling: Fix installation of documentation dependencies `[e0d8c33] <https://github.com/eclipse/cloe/commit/e0d8c33>`_
- tooling: Set boost dependency to full package mode `[d5447a6] <https://github.com/eclipse/cloe/commit/d5447a6>`_
- tooling: Add test UUIDs `[9e850c1] <https://github.com/eclipse/cloe/commit/9e850c1>`_
- tooling: Specify override=True in meta-package for overrides `[e8a17a1] <https://github.com/eclipse/cloe/commit/e8a17a1>`_

- all: Make C++14 required for all packages `[77a135a] <https://github.com/eclipse/cloe/commit/77a135a>`_
- all: Ensure editable mode works for all packages `[2b5cf81] <https://github.com/eclipse/cloe/commit/2b5cf81>`_
- all: Simplify CMakeLists.txt for all plugins `[5e61078] <https://github.com/eclipse/cloe/commit/5e61078>`_
- all: Add CMAKE_EXPORT_COMPILE_COMMANDS to Conan recipes `[fd28630] <https://github.com/eclipse/cloe/commit/fd28630>`_
