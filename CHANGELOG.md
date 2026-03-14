# Changelog

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
