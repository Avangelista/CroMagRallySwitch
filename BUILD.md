# How to build Cro-Mag Rally

## The easy way: build.py (automated build script)

`build.py` can produce a game executable from a fresh clone of the repo in a single command. It will work on macOS, Windows and Linux, provided that your system has Python 3, CMake, and an adequate C++ compiler.

```
git clone --recurse-submodules https://github.com/jorio/CroMagRally
cd CroMagRally
python3 build.py
```

If you want to build the game **manually** instead, the rest of this document describes how to do just that on each of the big 3 desktop operating systems.

## How to build the game manually on macOS

1. Install the prerequisites:
    - Xcode 12+
    - [CMake](https://formulae.brew.sh/formula/cmake) 3.16+ (installing via Homebrew is recommended)
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/jorio/CroMagRally
    cd CroMagRally
    ```
1. Download [SDL2-2.30.0.dmg](https://libsdl.org/release/SDL2-2.30.0.dmg), open it, and copy **SDL2.framework** to the **extern** folder
1. Prep the Xcode project:
    ```
    cmake -G Xcode -S . -B build
    ```
1. Now you can open `build/CroMagRally.xcodeproj` in Xcode, or you can just go ahead and build the game:
    ```
    cmake --build build --config RelWithDebInfo
    ```
1. The game gets built in `build/RelWithDebInfo/CroMagRally.app`. Enjoy!

## How to build the game manually on Windows

1. Install the prerequisites:
    - Visual Studio 2022 with the C++ toolchain
    - [CMake](https://cmake.org/download/) 3.16+
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/jorio/CroMagRally
    cd CroMagRally
    ```
1. Download [SDL2-devel-2.30.0-VC.zip](https://libsdl.org/release/SDL2-devel-2.30.0-VC.zip), extract it, and copy **SDL2-2.30.0** to the **extern** folder
1. Prep the Visual Studio solution:
    ```
    cmake -G "Visual Studio 17 2022" -A x64 -S . -B build
    ```
1. Now you can open `build/CroMagRally.sln` in Visual Studio, or you can just go ahead and build the game:
    ```
    cmake --build build --config Release
    ```
1. The game gets built in `build/Release/CroMagRally.exe`. Enjoy!

## How to build the game manually on Linux et al.

1. Install the prerequisites from your package manager:
    - Any C++20 compiler
    - CMake 3.16+
    - SDL2 development library (e.g. "libsdl2-dev" on Ubuntu, "sdl2" on Arch, "SDL2-devel" on Fedora)
    - OpenGL development libraries (e.g. "libgl1-mesa-dev" on Ubuntu)
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/jorio/CroMagRally
    cd CroMagRally
    ```
1. Build the game:
    ```
    cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build
    ```
    If you'd like to enable runtime sanitizers, append `-DSANITIZE=1` to the **first** `cmake` call above.
1. The game gets built in `build/CroMagRally`. Enjoy!

## How to build for Nintendo Switch (homebrew)

This produces the unofficial Switch homebrew `.nro`. You need Linux (or WSL2 on Windows) with [devkitPro](https://devkitpro.org/wiki/Getting_Started) installed.

1. Install the devkitPro toolchain and the Switch packages:
    ```
    sudo dkp-pacman -S switch-dev switch-sdl2 switch-mesa
    ```
    (`switch-mesa` provides `libGLESv1_CM`, the GLES1 driver the renderer runs on.)
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/Avangelista/CroMagRallySwitch
    cd CroMagRallySwitch
    ```
1. Configure and build with the devkitA64 toolchain file:
    ```
    export DEVKITPRO=/opt/devkitpro
    cmake -S . -B build-switch -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake -DCMAKE_BUILD_TYPE=Release
    cmake --build build-switch -j
    ```
1. The homebrew build is `build-switch/CroMagRally.nro`. Copy it to the `/switch/` folder on your SD card. Enjoy!
