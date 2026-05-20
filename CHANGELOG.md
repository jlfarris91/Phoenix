# Changelog

## [1.2.0](https://github.com/jlfarris91/Phoenix/compare/v1.1.0...v1.2.0) (2026-05-20)


### Features

* Add BlockBuffer SyncTo using write-watch ([eb72420](https://github.com/jlfarris91/Phoenix/commit/eb72420c79af2f6eb41adf7acc2ef1dc0a92bdfb))
* Add BlockBuffer SyncTo using write-watch ([f91591d](https://github.com/jlfarris91/Phoenix/commit/f91591db7fb0f455eb2c24207615359cbda8602f))
* Add dirty-page tracking and double-buffered world view ([1c2239a](https://github.com/jlfarris91/Phoenix/commit/1c2239aa307dad6de0f0c8fff82932caed9271dd))
* add doctest unit testing framework ([b8894ee](https://github.com/jlfarris91/Phoenix/commit/b8894ee52aff1cd29d172d768fbbeb4c5d14f222))
* add doctest unit testing framework and CI integration ([47c35e5](https://github.com/jlfarris91/Phoenix/commit/47c35e5ba6269539334963bd1e302a957c64604a))
* Add enum reflection, BaseDescriptor & UI controls ([2c866b3](https://github.com/jlfarris91/Phoenix/commit/2c866b3b0a257ac1a224557315fd5d71e8d27cfe))
* add IScriptBindings DDL system with Entity/Unit OOP Lua API ([4a2830c](https://github.com/jlfarris91/Phoenix/commit/4a2830c54f65d931720e4bf16fed6ca62f18fa94))
* add LuaRuntime, RTSScriptRegistration, and scripting binding architecture ([41c2a40](https://github.com/jlfarris91/Phoenix/commit/41c2a406f7402b8ce570916ce1a95b9a1e0d8ffd))
* add missing TestRTS source files and catalog data ([14d30b2](https://github.com/jlfarris91/Phoenix/commit/14d30b24319f69cfb69d265cfa16a40ce9796493))
* Add Phoenix.Editor.PropertyGrid project ([15df4b2](https://github.com/jlfarris91/Phoenix/commit/15df4b24877deb7611fd908b456ff102458abc5c))
* Add Phoenix.Editor.PropertyGrid project ([5ab37d9](https://github.com/jlfarris91/Phoenix/commit/5ab37d9106a508728d9bafdaef427d3a11668064))
* Add profiling zones to FeatureTask executions ([0f3aec8](https://github.com/jlfarris91/Phoenix/commit/0f3aec86f6d07e86cc8589467b1fd9760f6bff23))
* add TemplateMethod/TemplateFunction, Blackboard/Random/World bindings, and Random API overhaul ([d70d2b0](https://github.com/jlfarris91/Phoenix/commit/d70d2b0712dfe2be6d3f856edbcf55e247d8f0c2))
* add tests/unit directory with initial FName tests ([46c9599](https://github.com/jlfarris91/Phoenix/commit/46c95995bc904d072bfdaf6d64cfbf0dd5e01139))
* Add zoomable FlameGraph control and Memory debug tool ([8c043d6](https://github.com/jlfarris91/Phoenix/commit/8c043d66f41081d5f735c9aa0de09cffeff7f99e))
* adding linux configuration to cmake ([0271e51](https://github.com/jlfarris91/Phoenix/commit/0271e513b075d77fb0abed036c7340aa7aa53a60))
* copy Phoenix.d.lua to src/PhoenixLua for EmmyLua IDE support ([f9a4903](https://github.com/jlfarris91/Phoenix/commit/f9a4903774423e16506b2e3e8e797f27e7548486))
* copying imgui UX from ace-utils ([003429d](https://github.com/jlfarris91/Phoenix/commit/003429dcea57c67269f7478c39c696ab5b81f4e6))
* copying imgui UX from ace-utils ([436abb3](https://github.com/jlfarris91/Phoenix/commit/436abb3f7229cc1d603bbebebf1be98ef23a735a))
* ECS job system with DAG-based parallel scheduling and command buffers ([6caf2df](https://github.com/jlfarris91/Phoenix/commit/6caf2df91a2e543f73757208d88dcf876f62cc35))
* Enable PE_BUILD_STANDALONE in windows preset ([4eed0f8](https://github.com/jlfarris91/Phoenix/commit/4eed0f85a104ecc1df62754ee466b867e23217ab))
* Enhance debug UI: tables, blackboard, tasks ([3b7ee5f](https://github.com/jlfarris91/Phoenix/commit/3b7ee5fc631be917a433e675f967654cd1969df0))
* implement tasks API for synchronous entity logic (as an alternative for parallel jobs) ([7bc82ae](https://github.com/jlfarris91/Phoenix/commit/7bc82ae73b83781e067a791e9d66c2a55a1c9a68))
* Import Phoenix.App, Phoenix.Editor, Phoenix.Runtime from PhoenixEditor ([dc3e93c](https://github.com/jlfarris91/Phoenix/commit/dc3e93cb08a423dc02977e2d3b35c140034e7ed2))
* Lua scripting via WASM — IScriptBindings, script events, cross-platform build fixes ([6167a8a](https://github.com/jlfarris91/Phoenix/commit/6167a8a8c024f98284888cb947de1d0f0899c1c0))
* migrate Lua WASM runtime to tools, add code generation pipeline ([f22d2b2](https://github.com/jlfarris91/Phoenix/commit/f22d2b22553a1c14f4c92e97c5e2d68de27d600b))
* migrate PhysicsSystem, SteeringSystem, PeriodicEffectSystem to IJob scheduler ([29be563](https://github.com/jlfarris91/Phoenix/commit/29be56399f7ccf11946f2126824c4a73d994e6f0))
* overhaul reflection system and migrate features to new macro API ([f598896](https://github.com/jlfarris91/Phoenix/commit/f59889667a8b6d4bb263413412c09bcac5741489))
* overhaul reflection system with GenericFunction/MethodDescriptor split ([d9c8dae](https://github.com/jlfarris91/Phoenix/commit/d9c8dae62f7e1a3bbb6121adbd85e503e6bacbce))
* overhaul reflection system with GenericValue, TypeDescriptorBuilder, and TFixed metadata ([4aea9ff](https://github.com/jlfarris91/Phoenix/commit/4aea9ff1bcd4cc1c2cd4fea4cf64ddf488ac20b9))
* refactor ScriptModuleBuilder to use persistent class entries and add SteeringScriptBindings ([c3649b5](https://github.com/jlfarris91/Phoenix/commit/c3649b510d4e9fb022f52a8dd51279459a8040b8))
* Reflection system overhaul — GenericValue, TypeDescriptorBuilder, TFixed metadata ([edc2328](https://github.com/jlfarris91/Phoenix/commit/edc2328b2bc7b55ab4ae099797c2355faba80716))
* Switch projectiles to task-based system ([b73dedc](https://github.com/jlfarris91/Phoenix/commit/b73dedc39f7e4c8f09f6e6f85b5c41b9c2aad041))


### Bug Fixes

* Add extracted feature libs to Phoenix.Tests link list ([7bb1498](https://github.com/jlfarris91/Phoenix/commit/7bb14983196afc113d3712b98fda13567c00eba8))
* add missing &lt;cstring&gt; and &lt;format&gt; includes for Linux/GCC build ([3721c82](https://github.com/jlfarris91/Phoenix/commit/3721c8259056005e463121f030ce976447f805fc))
* add missing final collapse step in CollapseBits ([2e4c61c](https://github.com/jlfarris91/Phoenix/commit/2e4c61cb4ca30816714230a5e78dff5c499311a2))
* add override to GetTypeDescriptor so clang is happy ([211b076](https://github.com/jlfarris91/Phoenix/commit/211b076d10f9e3fbeeb0cd7cf4a284029752aeef))
* Add template keyword for dependent member template calls on GCC/Clang ([dfa4b53](https://github.com/jlfarris91/Phoenix/commit/dfa4b538d50ff9686a208ef8de3cb66ad588c5ec))
* Adding entityid support to natvis ([de62660](https://github.com/jlfarris91/Phoenix/commit/de6266006edfbff4a046c9d906a82fe3614d9bc6))
* adding local linux builds with WSL, fixing linux build issues ([41ec87e](https://github.com/jlfarris91/Phoenix/commit/41ec87e45e8337c1dc081e4670c4816805236a9f))
* adding missing atomic header to Delegates.cpp ([d2f1591](https://github.com/jlfarris91/Phoenix/commit/d2f15914834511958966fd8c7b1fe0dbd29ffc9b))
* adding some missing test files ([d31ef85](https://github.com/jlfarris91/Phoenix/commit/d31ef85d5b9dd404cc778bdbbdd9441db0c8aa7c))
* addressing missing &lt;cstring&gt; includes for memcpy ([0e8b740](https://github.com/jlfarris91/Phoenix/commit/0e8b74072597e21818f18bdf07900a5183b45d3f))
* avoid doctest rvalue-ref bind on std::strong_ordering in FName test ([8bc77f9](https://github.com/jlfarris91/Phoenix/commit/8bc77f9d7ccbedd6b1f7ff8cc61211802e7745ef))
* cap CollapseBits test to 16-bit range ([73da600](https://github.com/jlfarris91/Phoenix/commit/73da600990dc7290e8f130e46f2035192d32e6d0))
* correct overload resolution and ScaleToMortonCode test value ([2c2223b](https://github.com/jlfarris91/Phoenix/commit/2c2223b5b2b085de814ad0968fa247d8dc556345))
* cross-platform linker flags and missing cmath include for Linux ([c3d5e0f](https://github.com/jlfarris91/Phoenix/commit/c3d5e0fdcbd523636be59f7bda60121067ce4ad2))
* disable debug test in FeatureECS ([50c22a6](https://github.com/jlfarris91/Phoenix/commit/50c22a6941feb0f08201a4e3f4e864778020df8f))
* enabling pch for windows ci ([968db56](https://github.com/jlfarris91/Phoenix/commit/968db567a5c5e676367fb42f5c1a5a0944f99ef6))
* enabling sscache for faster compile times ([37f7567](https://github.com/jlfarris91/Phoenix/commit/37f756711bf2da326c998a6bf78e51bd65c58f24))
* fix corrupt COFF with FixedPoint.cpp, ([b87a2ac](https://github.com/jlfarris91/Phoenix/commit/b87a2ac3bfbbd05e41bc0ae053c39ccdac0fdd3d))
* Fix glob pattern list in PhoenixRTS CMakeLists ([1f120b3](https://github.com/jlfarris91/Phoenix/commit/1f120b384292a859822fe6c689309775a07fad60))
* Fix Linux and Emscripten CI failures after rebase ([a724bb0](https://github.com/jlfarris91/Phoenix/commit/a724bb0de85a43f9d3a1c47bd9169e3a968a8be3))
* Fixing a bug with ECS entity sorting ([87f748e](https://github.com/jlfarris91/Phoenix/commit/87f748eb38da636ac051d3978176538cfbbe5cd0))
* Fixing bad names in TestRTS ([28e27e8](https://github.com/jlfarris91/Phoenix/commit/28e27e83df3e20681accb7d8ce711b0252e0509b))
* fixing issue found during ace-utils integration ([31e6041](https://github.com/jlfarris91/Phoenix/commit/31e6041afcae36b87276bc2d8c196f2eb42ea779))
* fixing linux build issues ([f59e728](https://github.com/jlfarris91/Phoenix/commit/f59e7288ca4556430efce811c9144696aaccbaca))
* fixing linux ci issues, adding -Wno-changes-meaning because who cares ([02590c3](https://github.com/jlfarris91/Phoenix/commit/02590c3362bc74651c502f005a5fb947e36f3806))
* ghost entity created by GetOrAddComponent adding a component to a dead entity ([8c319a1](https://github.com/jlfarris91/Phoenix/commit/8c319a12d260cb0b298eafdac4dd8c0f80a281f6))
* **parallel:** CI breaks — Enqueue(const Task&) overload + Delegates.h cstring ([d66572c](https://github.com/jlfarris91/Phoenix/commit/d66572c29d569989fb170bd37e2153650f4e34c0))
* **parallel:** MSVC — std::destroy_at for the InlineCallable Destroy op ([53e64e5](https://github.com/jlfarris91/Phoenix/commit/53e64e52e3fafef1eb1ab22a1776ae7784b7ba7d))
* Qualify delegate macros and friend TypeRegistrar ([89702e1](https://github.com/jlfarris91/Phoenix/commit/89702e13a3f39bf2463b10b267bf8d7c6d516d43))
* re-commenting out debug code ([1a123ad](https://github.com/jlfarris91/Phoenix/commit/1a123adcd9bf69c6677073b174057786a932ae9a))
* re-enabling linux ci builds for release please ([816fb7c](https://github.com/jlfarris91/Phoenix/commit/816fb7c9e38560b295fb23139323e5a18efaf19d))
* Redirect Phoenix.d.lua output from PhoenixLua/ to Phoenix.Sim.Lua/ ([9360541](https://github.com/jlfarris91/Phoenix/commit/93605410afebcea020ae8e725038ea3451894520))
* Refactor shutdown ordering and cleanup ([b358240](https://github.com/jlfarris91/Phoenix/commit/b35824037db12116d40d702d6d5e8cc58fdc430e))
* register FeatureTask to the session in app.cpp ([d75f4df](https://github.com/jlfarris91/Phoenix/commit/d75f4df853866036c77ea9a928f2d43541bfd480))
* Remove dynamic StepHz (should always be 1/64 to match Time) and … ([6e79db6](https://github.com/jlfarris91/Phoenix/commit/6e79db6f468ba3fae9c635297083c3a213f31b7b))
* Remove dynamic StepHz (should always be 1/64 to match Time) and introduce sim speed multiplier ([8ecdb1e](https://github.com/jlfarris91/Phoenix/commit/8ecdb1e66093e27111de2a81f071217af8d48ea3))
* removing preset named folder as root of artifact ([55c5b58](https://github.com/jlfarris91/Phoenix/commit/55c5b58c3239b08af0bbf0f4a9a7985f764edfdc))
* rename Tests/unit to tests/unit for case-sensitive Linux filesystem ([a2966b5](https://github.com/jlfarris91/Phoenix/commit/a2966b50913c92c6d13191d6792fce40d6019cc1))
* Repair build and runtime after feature lib extraction ([322901a](https://github.com/jlfarris91/Phoenix/commit/322901a0072fb0480da2627fb39265193878fad0))
* resolve Emscripten and Linux cross-platform build failures ([94a7e4e](https://github.com/jlfarris91/Phoenix/commit/94a7e4e73db26c5575eca6c5951ccb8cf4d2b357))
* resolve Linux and Emscripten CI build failures ([f7f0f31](https://github.com/jlfarris91/Phoenix/commit/f7f0f31f0c2d750f4ae23de15745d898dffd7429))
* Restore PHX_CACHE_LINE_SIZE and fix stale PhoenixSim/ includes ([845f13f](https://github.com/jlfarris91/Phoenix/commit/845f13fa38dd5025030de44233ba5e55c25729d2))
* sim FPS accurately represents step time ([a465d69](https://github.com/jlfarris91/Phoenix/commit/a465d69c8f227c159de0fa58780888bb4bb29d72))
* undoing that last change ([1f69c04](https://github.com/jlfarris91/Phoenix/commit/1f69c04be234b4c883128cb26d4fca90f4f31cd4))
* Update include paths for dot-notation module directories ([a694872](https://github.com/jlfarris91/Phoenix/commit/a6948728d498eed70905ac0cb75f1efa8793e7b7))
* use single-arg array overload in constexpr hash tests ([7d48ebb](https://github.com/jlfarris91/Phoenix/commit/7d48ebb1d0be33b0583fe8e62ab66bb8a10535a1))
* various misc changes ([178a34a](https://github.com/jlfarris91/Phoenix/commit/178a34a5891b231f50941df9d36443d5561be8f9))


### Performance Improvements

* **parallel:** [#1](https://github.com/jlfarris91/Phoenix/issues/1) Cache-line pad MPMC cursors and ThreadPool atomics ([8684ae9](https://github.com/jlfarris91/Phoenix/commit/8684ae9e2e98ae0e484ae62b3a5abb94f20bc078))
* **parallel:** [#2](https://github.com/jlfarris91/Phoenix/issues/2)b/c Wire ThreadPool to per-worker Chase-Lev deques + slab ([af0069b](https://github.com/jlfarris91/Phoenix/commit/af0069b408f3fe7fc283251e2326474dc0e224ac))
* **parallel:** [#3](https://github.com/jlfarris91/Phoenix/issues/3) ParallelForEach -&gt; ParallelRange shim; fix WaitIdle TOCTOU ([2e20825](https://github.com/jlfarris91/Phoenix/commit/2e2082583c0a41b08c44c857f32a28c713acaa2f))
* **parallel:** [#4](https://github.com/jlfarris91/Phoenix/issues/4) Replace std::function in Task body with inline 128B callable ([a1c0fc4](https://github.com/jlfarris91/Phoenix/commit/a1c0fc4ab9872dbef3881f5a91b39ed340864b79))
* **parallel:** [#6](https://github.com/jlfarris91/Phoenix/issues/6) PAUSE backoff on caller-side waits; emit real PAUSE on POSIX ([7f2ce68](https://github.com/jlfarris91/Phoenix/commit/7f2ce682fb9a3ddfcd77b5cffafad7272a55e550))

## [1.1.0](https://github.com/jlfarris91/PhoenixSim/compare/v1.0.2...v1.1.0) (2026-03-14)


### Features

* adding support for emscripten builds ([c4efc0f](https://github.com/jlfarris91/PhoenixSim/commit/c4efc0fd2ea8e3b3141345fe6c3358ebb78190d7))
* updating ci for windows and emscripten ([74d34c7](https://github.com/jlfarris91/PhoenixSim/commit/74d34c74928a64126600221ff2dde24fcba97d1e))
* updating docs ([8761cd0](https://github.com/jlfarris91/PhoenixSim/commit/8761cd01132989df6d088cc7ecf4e6b318960dd5))


### Bug Fixes

* adding step to install required build tools (which I thought should already be pre-installed....) ([dedca6f](https://github.com/jlfarris91/PhoenixSim/commit/dedca6f802d65302d3ecbce34298526f53dd7a7f))
* addressing "zip-inside-a-zip" artifact generated ([dd2e9f8](https://github.com/jlfarris91/PhoenixSim/commit/dd2e9f88b0eb2279f95b085f6f5a50a7778df713))
* attempting vcpkg cache fix and removing TestRTS from ci builds ([5790543](https://github.com/jlfarris91/PhoenixSim/commit/57905434443e9453082d5ac3b17b4616d5d3d824))
* enabling artifact generation for ci ([3169a85](https://github.com/jlfarris91/PhoenixSim/commit/3169a853137ab14a4f704f9cf5145ff3724ab2be))
* fixing a bug in install step where build dir was not specified ([cda6b2b](https://github.com/jlfarris91/PhoenixSim/commit/cda6b2b49814dcbd34083e4f1f1877e29de5d221))
* fixing invalid path for upload-artifact ([0cf6370](https://github.com/jlfarris91/PhoenixSim/commit/0cf6370e036c3002a6db718249e37692a12b75ab))
* giving build.yml appropriate permissions ([a84d0f9](https://github.com/jlfarris91/PhoenixSim/commit/a84d0f90e566e41629343cb99e19007c3ec634e2))
* implementing vcpkg caching using actions/cache ([98c4db8](https://github.com/jlfarris91/PhoenixSim/commit/98c4db84671d63bff5f2d5fb07372c38ad920b1d))
* improving ci, artifact generation and attachment to release ([aae5a4f](https://github.com/jlfarris91/PhoenixSim/commit/aae5a4f4a4a2527176c637590eb6c6f8ab4e59c7))
* include all files recursively in artifact zip ([2f5e8c4](https://github.com/jlfarris91/PhoenixSim/commit/2f5e8c4182475aa9296ef26bffcc35373162cc59))
* last attempt ([c5188de](https://github.com/jlfarris91/PhoenixSim/commit/c5188de2d03d0ced24c6046056e141937d0e8b33))
* Make build workflows callable; conditional uploads ([8190270](https://github.com/jlfarris91/PhoenixSim/commit/81902707ad1e56e54ab5803db6b0fd2cf845fa24))
* more ci cleanup and another attempt at fixing the heap memory issue ([d55fac2](https://github.com/jlfarris91/PhoenixSim/commit/d55fac2ca61f0e572f218975c243953c96a8278f))
* more fixes for upload artifact step ([82d9cbe](https://github.com/jlfarris91/PhoenixSim/commit/82d9cbe4682024cd55ce88748b014bc948a9425b))
* more permissions needed ([c0f8d4d](https://github.com/jlfarris91/PhoenixSim/commit/c0f8d4d58e9e2b8891081dbb05c305b819a04068))
* removing &lt;execution&gt; which might be causing the compiler to run out of heap memory ([668cdfa](https://github.com/jlfarris91/PhoenixSim/commit/668cdfa5d430c9364ed3f082f62961527b39d8af))
* removing hard-coded path to TracyClient.dll ([5396f4d](https://github.com/jlfarris91/PhoenixSim/commit/5396f4d670dcc0561ae73a569c749c5525bfa009))
* Skip CI for release-please branches ([0a1d14d](https://github.com/jlfarris91/PhoenixSim/commit/0a1d14d7e2c6ac7d6a3acbffa27655ee07dcb2b6))
* still trying to fix out of heap memory failure ([31723d2](https://github.com/jlfarris91/PhoenixSim/commit/31723d2f0b82eff187f4268db471bf45f0d7cbed))
* switching build runner to ubuntu-latest since windows sucks ([5749ee8](https://github.com/jlfarris91/PhoenixSim/commit/5749ee88d1bee9791d30dadd3ea562c6840d03c2))
* the upload artifact is still not finding any files to upload, adding some logging ([5cd8f82](https://github.com/jlfarris91/PhoenixSim/commit/5cd8f825c95344a095d4e07aefb9f955c7d25a2b))
* trying another path for the artifact zip ([9e79fb6](https://github.com/jlfarris91/PhoenixSim/commit/9e79fb6ab78994ba4ab2694a95077a2fda486e31))
* trying anything I can at this point, nothing is working ([4afb312](https://github.com/jlfarris91/PhoenixSim/commit/4afb312b20457b1c76aaa8959508a0cf7c3e1196))
* trying to address a compiler out of heap memory error ([f97df46](https://github.com/jlfarris91/PhoenixSim/commit/f97df46b5ef2fc2d92a98b8dbfb4b46f9efffac7))
* trying to address install error ([8c70554](https://github.com/jlfarris91/PhoenixSim/commit/8c70554e4be59a1bdcb27b12528c1e6c0c23ec26))
* trying to force x64 compiler ([940ffba](https://github.com/jlfarris91/PhoenixSim/commit/940ffba64954b31e73ab405231d30109f3943cf1))
* ugh ([dad458a](https://github.com/jlfarris91/PhoenixSim/commit/dad458a9021c586d51250f977082a1f033c80ee5))
* Use Release preset and remove config input ([31ce428](https://github.com/jlfarris91/PhoenixSim/commit/31ce42834bd0e46f65fadf8b2a918153a19a9285))
* what was I thinking of course I need to use windows ([b252b3c](https://github.com/jlfarris91/PhoenixSim/commit/b252b3c5c6ca3f148f531aa4599a010fffc29216))
* wow this is such a pain in the ass ([f03f0f8](https://github.com/jlfarris91/PhoenixSim/commit/f03f0f83fa87208673bce8d2ddd3be3ad6fb67ef))

## [1.0.2](https://github.com/jlfarris91/PhoenixSim/compare/v1.0.1...v1.0.2) (2026-03-12)


### Bug Fixes

* touching to force re-release ([4e47be5](https://github.com/jlfarris91/PhoenixSim/commit/4e47be5c558f57a08c22a4c1f1f59788a9dd7504))

## [1.0.1](https://github.com/jlfarris91/PhoenixSim/compare/v1.0.0...v1.0.1) (2026-03-12)


### Bug Fixes

* Ignoring compile options for clang ([b5cf44f](https://github.com/jlfarris91/PhoenixSim/commit/b5cf44fdf9a6f9fa58321d602c0f6b4d136e690e))

## 1.0.0 (2026-03-12)


### Features

* Initial draft of release-please and build workflows for CI/CD ([aa3c012](https://github.com/jlfarris91/PhoenixSim/commit/aa3c01272488ade09ff35470b80fc3b4894b57cd))


### Bug Fixes

* Improving some of the cmake configuration, prepping for CI/CD ([cec209a](https://github.com/jlfarris91/PhoenixSim/commit/cec209acbe270788148ec90ad7aab25fb1eff4f1))
