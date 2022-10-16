# Changelog

## [0.2.0-prealpha](https://github.com/RigCLang/rigc-lang/compare/v0.1.1-prealpha...v0.2.0-prealpha) (2022-10-16)


### Features

* add --logfile option ([fc72e80](https://github.com/RigCLang/rigc-lang/commit/fc72e8007d5bdb393a5b3ea24a12442dafd3722a))
* add --logfile option ([6161a49](https://github.com/RigCLang/rigc-lang/commit/6161a491c2b2c7bdbec9c52e3d674da14cfb4487))
* **app:** added basic destructor support ([b7ce690](https://github.com/RigCLang/rigc-lang/commit/b7ce690a4b811c64a9fed0a69f7b49631b3e487f))
* clear logs on opening ([18ab9bc](https://github.com/RigCLang/rigc-lang/commit/18ab9bcd393bc7c76bb69f56e4d49d8f79b239cd))
* clear logs on opening ([0b56185](https://github.com/RigCLang/rigc-lang/commit/0b5618515d9f320c4fc3e18a63fd9f55a2f36297))
* don't send frame debug message if the frame is `nullptr`. ([cf011c2](https://github.com/RigCLang/rigc-lang/commit/cf011c24a5de951c369007626bd8da6559776160))
* format the stack frame' labels in a nicer way ([54aecf2](https://github.com/RigCLang/rigc-lang/commit/54aecf276ce93eaf57b126642746b38c6d2db283))
* send actual context to the debugger ([1171f0e](https://github.com/RigCLang/rigc-lang/commit/1171f0efdece7b537c3be8b22d96776d5046f3e4))
* send debug messages about the stack frames and allocations ([54b39d7](https://github.com/RigCLang/rigc-lang/commit/54b39d786a9534405c66db064fdc9acfc7ca26e1))
* **vm:** `--wait-for-connection` program argument ([358625d](https://github.com/RigCLang/rigc-lang/commit/358625d55dbc67b6ae332a468a8c1db74a7c31f6))
* **vm:** `Breakpoint` struct for a DevServer ([7005316](https://github.com/RigCLang/rigc-lang/commit/7005316cbec07e7e7ce7b146614c0423672de578))
* **vm:** `builtin::printCharacters` function. ([3d1355a](https://github.com/RigCLang/rigc-lang/commit/3d1355a5836900c0b870e7006fff536af690f25a))
* **vm:** `devserverLog` function ([729c165](https://github.com/RigCLang/rigc-lang/commit/729c16554c64a54a09bfaa8c2e4b985a247a85c4))
* **vm:** added `json` package and some aliases. ([ccb19fe](https://github.com/RigCLang/rigc-lang/commit/ccb19fe944f3a0844a575c0b5f3eaffef59e21b1))
* **vm:** added `Void` type ([e4d6b7c](https://github.com/RigCLang/rigc-lang/commit/e4d6b7c81bea666d9a712240a9a54020631fa3dc))
* **vm:** added memory allocation / deallocation functions ([fb8a54e](https://github.com/RigCLang/rigc-lang/commit/fb8a54ef300a96557b46a0fed98db2a01c4b34ca))
* **VM:** basic conversions example. ([2939389](https://github.com/RigCLang/rigc-lang/commit/2939389f44032771f2db0e0be9cdacd543fa9a22))
* **VM:** error handling and formatting utilities. ([c443b24](https://github.com/RigCLang/rigc-lang/commit/c443b24aa98f79698bcd62aeecc1402ae2004f5a))
* **VM:** exception handling for runtime_error. ([322b55e](https://github.com/RigCLang/rigc-lang/commit/322b55e4b569958bcf5dcb3b8fb411b7a7a18de0))
* **vm:** generate and use copy constructors for each type. ([ea54c96](https://github.com/RigCLang/rigc-lang/commit/ea54c969eea2a66a8a4f3c60a538a36ff0fd112c))
* **VM:** internal exception class and utils. ([08ddecb](https://github.com/RigCLang/rigc-lang/commit/08ddecbaf7812cc41ca6bfab26150646edea6277))
* **vm:** support constructor templates ([ff4044b](https://github.com/RigCLang/rigc-lang/commit/ff4044b9cc237c95c41288ae93bccb1785cb2a52))
* **vm:** working on a dev server ([f7b80fa](https://github.com/RigCLang/rigc-lang/commit/f7b80fac0209a4b1a1ca026cec753fd99e95ab39))
* **vm:** working on a dev server and destructors. ([16bb61f](https://github.com/RigCLang/rigc-lang/commit/16bb61fc0a0a112afb5698ebb9cc3487222c2543))


### Bug Fixes

* **automation:** fixed invalid tag for artifacts ([f8fe4e0](https://github.com/RigCLang/rigc-lang/commit/f8fe4e055c85427e9bb9c67151b80d8c98658c11))
* **examples:** made examples conforming to the new changes ([61792b3](https://github.com/RigCLang/rigc-lang/commit/61792b3f49c5552e2c60144ec7b950524b3e69e3))
* proper case for program argument (now): `--skip-root-exception-catching` ([d991b41](https://github.com/RigCLang/rigc-lang/commit/d991b41c571eceab0692fa6f09be749c4fcb0909))
* proper case for program argument (now): `--skip-root-exception-catching` ([afe08a1](https://github.com/RigCLang/rigc-lang/commit/afe08a11f1888618d1a56fd8633e3c588e5e0851))
* proper formatting in stack json requests ([5878c27](https://github.com/RigCLang/rigc-lang/commit/5878c276684a1536085f513bf0c3caf2411ed760))
* removed accidentally added test files. ([0da83b1](https://github.com/RigCLang/rigc-lang/commit/0da83b11d392b0ca7039347f7f42f34005122f26))
* typo in json debug message alllocation -> allocation ([2970c0d](https://github.com/RigCLang/rigc-lang/commit/2970c0d25d73f527a0c2b7ea28741da48fc5016f))
* **vm:** added missing return types for `readInt` and `readFloat` ([175a128](https://github.com/RigCLang/rigc-lang/commit/175a128008573e2db0ae164683d6f8332fd329bd))
* **VM:** allocateOnStack condition in assert. ([332388d](https://github.com/RigCLang/rigc-lang/commit/332388d14ba0586a102addf0266c0848bb9a79f5))
* **VM:** conversion operator wrong name extraction. ([bad55d9](https://github.com/RigCLang/rigc-lang/commit/bad55d9485eef194e2c85c82c9db71bb43128022))
* **vm:** do not allow execution if error ocurred during argument parsing ([4154ef9](https://github.com/RigCLang/rigc-lang/commit/4154ef984e6b5e57e3056f438fb15e573c5b6cab))
* **VM:** evalPostfixOperator's paramString concatenation loop. ([7bf77d3](https://github.com/RigCLang/rigc-lang/commit/7bf77d3cf9fa1e1ba954fe1aef09cb73230e7bf3))
* **vm:** method templates not being handled properly (problem with `self` param) ([29d1e1e](https://github.com/RigCLang/rigc-lang/commit/29d1e1e5ba27bd95ef8c78562a693e7a00b12362))
* **vm:** non-debug build used debug-only code. ([83d4eed](https://github.com/RigCLang/rigc-lang/commit/83d4eed8be22c4a8f7541dedca481290e1e41cdd))
* **vm:** order of core type creation ([7bfb829](https://github.com/RigCLang/rigc-lang/commit/7bfb829a925e0b68bf5273d9d1d509cdbaebe7af))
* **vm:** path starting with `.\` bug ([337b4c3](https://github.com/RigCLang/rigc-lang/commit/337b4c32e9535350878bccf5fa9f39b5fe7b4925))
* **VM:** unintended ADL-related error ([53ae9af](https://github.com/RigCLang/rigc-lang/commit/53ae9afa4ae808e04861d8cef284655ffadf306e))
* **vm:** variable getting invalid address when created ([1c5b34c](https://github.com/RigCLang/rigc-lang/commit/1c5b34cb144656798cf5138c65f8a833bb324115))

## [0.1.1-prealpha](https://github.com/PoetaKodu/rigc-lang/compare/v0.1.0-prealpha...v0.1.1-prealpha) (2022-06-23)


### Bug Fixes

* **automation:** fixed invalid tag for artifacts ([f8fe4e0](https://github.com/PoetaKodu/rigc-lang/commit/f8fe4e055c85427e9bb9c67151b80d8c98658c11))
* **automation:** invalid tag being generated ([eaeb64f](https://github.com/PoetaKodu/rigc-lang/commit/eaeb64f795f6a467efa52ae7560e458fbcbc15cf))
* missing `CHANGELOG.md` ([eaeb64f](https://github.com/PoetaKodu/rigc-lang/commit/eaeb64f795f6a467efa52ae7560e458fbcbc15cf))
* **VM:** EnumType == operator in postInitialize. ([352e9b2](https://github.com/PoetaKodu/rigc-lang/commit/352e9b2122b10dcec8154f388a50362707b23202))
