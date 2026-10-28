# Source2Mod SDK

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Website](https://img.shields.io/badge/Website-source2mod.net-blue)](https://www.source2mod.net)
[![Discord](https://img.shields.io/badge/Discord-join-7289da?logo=discord&logoColor=white)](https://discord.gg/4Ck56eDNXj)

**Source2Mod SDK** is what [Source2Mod](https://github.com/Source2Mod/source2mod)
plugins are built against -- in **C++** or in **SourcePawn**. Both kinds are
first-class and use the same API: the C++ headers live in `public/source2mod`,
the SourcePawn include set that mirrors them name for name in
`public/scripting`.

It bundles everything a native plugin needs -- headers, s2sdk, the hooking
system and build helpers -- and the includes a script needs, so you can focus
purely on writing your plugin.

---

## What is this?

Source2Mod SDK is a lightweight development layer that provides:

- Preconfigured **s2sdk (CS2)** -- AlliedModders' Source 2 SDK, formerly hl2sdk's `cs2` branch  
- Integrated **KHook** hooking (virtual, vtable and function detours), shared with Metamod  
- Ready-to-use **Source 2 headers & interfaces**  
- Cross-platform build configuration  
- Simple plugin build system  

No setup. No hunting dependencies. Just build.

---

## Why use it?

- **Zero setup** – everything included (SDK, hooks, protobufs)  
- **Fast builds** – optimized CMake configuration  
- **Clean integration** – designed specifically for Source2Mod  
- **Cross-platform** – Windows & Linux support out of the box  
- **Minimal boilerplate** – create plugins in seconds  

---

## Quick Start (C++)

### 1. Add SDK to your project

```bash
git submodule add https://github.com/Source2Mod/source2mod-sdk.git
git submodule update --init --recursive
```

### 2. Get the dependencies

**s2sdk** ([alliedmodders/s2sdk](https://github.com/alliedmodders/s2sdk), `cs2`
branch) comes with the SDK as the `vendor/s2sdk` submodule, pinned to the
commit Source2Mod headers are built against -- the `--recursive` above
already fetched it, game protobufs included (s2sdk's own `.proto` files, which
the SDK generates into its `Protobufs` library). Nothing else to set up.

If you keep your own s2sdk checkout instead, point any one of these at it; the
first one set wins over the submodule:

```
S2SDK_CS2, S2SDKCS2, S2SDK, HL2SDK_CS2, HL2SDKCS2
```

`tools/deps.py` remembers that choice and keeps everything current:

```bash
python source2mod-sdk/tools/deps.py init
python source2mod-sdk/tools/deps.py update
```

`init` asks, once, whether s2sdk is a checkout you already have or the
submodule, and saves the answer in `deps.json`. `update` pulls your checkout
or puts the submodule on the pinned commit, moves `vendor/khook` to the KHook
commit metamod-source pins (anything else refuses to load on Source2Mod core)
and moves `vendor/s2sdk-manifests` to its latest. `deps.py status` shows what
is checked out.

What a binary needs from s2sdk -- include paths, defines, libraries with the
Steam API, protoc -- comes from [s2sdk-manifests](https://github.com/Source2Mod/s2sdk-manifests),
for both build systems: CMake through s2sdk's `s2sdk::s2sdk`, AMBuild through
its `SdkHelpers.ambuild`. Both use the `vendor/s2sdk-manifests` submodule;
`S2SDK_MANIFESTS_DIR` (or AMBuild's `--s2sdk-manifests`) points them at another
checkout. Answer up front with `--s2sdk submodule` when there is no
terminal.

---

### 3. Minimal plugin setup (4 lines)

```cmake
cmake_minimum_required(VERSION 3.18)
project(my-plugin CXX)

add_subdirectory(source2mod-sdk)
add_source2mod_plugin(my_plugin plugin.cpp)
```

---

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Output:
```
my_plugin.smx
```

---

## Quick Start (SourcePawn)

A script needs no build system: `#include <source2mod/api>` and compile it with
`spcomp`. Every Source2Mod build ships both in `addons/source2mod/scripting`
(`spcomp` and `include/`, copied from this SDK's `public/scripting`).

```sourcepawn
#include <source2mod/api>

public void OnPluginStart()
{
    Source2ModCommands.RegisterConCommand("hello", Cmd_Hello);
}

public void Cmd_Hello(Source2ModCommandContext context, Source2ModCommandArgs args, bool post)
{
    Source2ModCommands.ReplyToCommand(context, "Hello!");
}
```

```bash
cd addons/source2mod/scripting
./spcomp -i include hello.sp -o ../plugins/hello.smx
```

The script API mirrors the C++ one: `ISource2ModCommands::RegisterConCommand`
is `Source2ModCommands.RegisterConCommand`, a schema class is a methodmap of
the same name with its fields as properties.

---

## What's Included

- **s2sdk (CS2)** -- the `vendor/s2sdk` submodule, or your own checkout  
- **Protobufs** -- the game's, generated from s2sdk's own `.proto` files  
- **KHook** (virtual, vtable & function hooks) -- as the `vendor/khook` submodule, headers only    
- **Tier0 / Tier1 / Mathlib**  
- **Schema system headers**  
- **C++ headers** -- `public/source2mod`: every `ISource2Mod*` interface and the generated schema classes  
- **SourcePawn include set** -- `public/scripting` (`#include <source2mod/api>`), shipped by the core in `addons/source2mod/scripting/include`  
- **Preconfigured compiler flags & linking**  

### Interface revisions

Each interface is one class (`ISource2ModCommands`) whose version string
carries a revision (`SOURCE2MOD_COMMANDS_INTERFACE` = `"ISource2ModCommands004"`);
`SOURCE2MOD_SAVEVARS()` asks the core for the revision the plugin was built
against. A newer core keeps serving the revisions it froze through compat
adapters, so a plugin does not have to be rebuilt when an interface changes;
an older core than the plugin needs refuses it with `update the core`. See
[Compatibility](https://www.source2mod.net/docs/development/compatibility).

---

## Helper API

### add_source2mod_plugin

```cmake
add_source2mod_plugin(my_plugin plugin.cpp)
```

Automatically:
- Links SDK  
- Sets correct output (`.smx`)  
- Applies all required flags  

---

## Requirements

- CMake 3.18+  
- C++20 compiler  
- Python 3 and git (for `tools/deps.py`)  
- Source2Mod installed on server  

---

## Documentation

- [source2mod.net](https://www.source2mod.net): guides with every example in C++
  and SourcePawn, and the [API reference](https://www.source2mod.net/docs/core-api)
  generated from these headers.
- The headers themselves: every interface in `public/source2mod` is documented
  where it is declared, and the include set in `public/scripting` says the same
  for scripts.
- Samples: `samples/cs2/cpp_sample` and `samples/cs2/sp_sample` in the
  [Source2Mod repository](https://github.com/Source2Mod/source2mod) -- the same
  plugin in C++ and in SourcePawn.

---

## Contributing

Fork the repository, branch off `main`, and open the pull request against
`main` -- `dev` is the maintainer's own working branch. The full walkthrough
(upstream remote, rebasing, what a pull request needs) is in
[CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the GNU General Public License v3.0, with a
linking exception for Valve's engines and games, a dual-licensing exception
and an MIT exception for derivative works. See [LICENSE_INFO.txt](LICENSE_INFO.txt) for the terms
and [LICENSE](LICENSE) for the full GPLv3 text.

### Do I have to open-source my plugins?

No. The GPLv3 license carries an explicit MIT exception for derivative works:
your plugins, C++ or SourcePawn, or anything built against the SDK, can stay
closed-source or commercial. The exception covers what you build on
Source2Mod, not Source2Mod itself -- a modified Source2Mod or SDK stays under
the GPLv3. The parts of the include set that come from SourceMod keep
AlliedModders LLC's own license terms.

---

<div align="center">
  <strong>C++ or SourcePawn -- everything you need to build Source2Mod plugins.</strong>
</div>
