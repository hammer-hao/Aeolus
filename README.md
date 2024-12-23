# Aeolus

[![Build](https://github.com/cpp-sc2/blank-bot/actions/workflows/ci.yml/badge.svg)](https://github.com/cpp-sc2/blank-bot/actions/workflows/ci.yml)

Starter bot for StarCraft II with integrated cpp-sc2.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Aeolus](#Aeolus)
    - [About Aeolus](#about-aeolus)
        - [Key Features](#key-features)
        - [Dependencies](#dependencies)
        - [Acknowledgements](#acknowledgements)
    - [Build instructions](#build-instructions)
        - [Windows](#windows)
        - [macOS](#macos)
        - [Linux](#linux)
    - [Additional options](#additional-options)
        - [WSL2 Support](#wsl2-support)
        - [Game client version](#game-client-version)
        - [AIArena ladder build](#aiarena-ladder-build)
    - [Managing CMake dependencies](#managing-cmake-dependencies)
    - [Troubleshooting](#troubleshooting)
        - [CMake options don't take effect](#cmake-options-dont-take-effect)
        - [Build freezes (Linux or macOS)](#build-freezes-linux-or-macos)
    - [License](#license)

<!-- markdown-toc end -->
## About Aeolus

**Aeolus** is a modern StarCraft II bot framework written in C++, inspired by the modular and scalable design principles of [ares-sc2](https://github.com/AresSC2/ares-sc2). Named after the Greek keeper of the winds, Aeolus is built to dominate the dynamic environment of StarCraft II with swift decision-making, strategic precision, and efficient resource management.

### Key Features

- **Behavior-Driven Architecture**  
  Aeolus employs a behavior executor system to handle core bot actions such as mining, unit production, and combat. This modular approach simplifies development and encourages clean, reusable code.

- **Optimized Resource Management**  
  Includes a state-of-the-art mineral mining optimization system, assigning workers perfectly at the start to ensure maximum resource collection efficiency.

- **Advanced Unit Handling**  
  Implements algorithms for unit prioritization, distance sorting, and effective micro-management during combat.

- **Manager Pattern Design**  
  Core functionalities are structured around a manager pattern, with every manager implementing the `ProcessRequest` interface for consistent, scalable logic.

- **Utility Toolkit**  
  A comprehensive set of utility functions for common tasks like sorting units by distance, pathfinding, and filtering.

### Dependencies
This project uses the following third-party library:

- **[nanoflann](https://github.com/jlblancoc/nanoflann)**:  
  A lightweight C++ library for KDTree-based nearest neighbor search.  
  Licensed under the BSD 2-Clause License. See the `thirdparty/nanoflann/LICENSE.txt` file for details.

### Acknowledgements

Aeolus draws inspiration from [ares-sc2](https://github.com/davechurchill/ares-sc2), an excellent framework by Dave Churchill. Special thanks to the ares-sc2 project for setting the bar for StarCraft II bot development and serving as a foundation for learning and innovation.

---

Aeolus is designed for developers who want to build high-performance StarCraft II bots, whether to compete, experiment with AI, or simply learn the intricacies of bot programming. With its focus on clean architecture and strategic flexibility, Aeolus is your gateway to StarCraft II automation.


## Build instructions

> :construction: This project requires a compiler with C++17 support.

1. Before proceeding further download the [actual map pack](https://aiarena.net/wiki/maps/).

1. Put the downloaded maps into the `Maps` folder (create it if the folder doesn't exist):
   * Windows: `C:\Program Files\StarCraft II\Maps`
   * OS X: `/Applications/StarCraft II/Maps`
   * Linux: anywhere.

### Windows

For building for Windows under WSL2, see [WSL2 Support](#wsl2-support).

1. Install [CMake](https://cmake.org/download/).

1. Download and install Visual Studio ([2017](https://www.visualstudio.com/downloads/) or newer).

1. Get the project.

   ```bat
   git clone git@github.com:hammer-hao/aeolus.git
   ```

1. Enter the working directory.

   ```bat
   cd aeolus
   ```

1. Create Visual Studio project files in the directory "build".
   E.g. for Visual Studio 2022:

   ```bat
   cmake -B build -G "Visual Studio 17 2022"
   ```

1. Build the project using Visual Studio.

   ```bat
   start build\Aeolus.sln
   ```

1. Launch the bot with the specified path to a SC2 map, e.g:

   ```bat
   build\bin\Debug\Aeolus.exe Ladder2019Season3/AcropolisLE.SC2Map
   ```

### macOS

1. Install [CMake](https://cmake.org/download/).

1. Install XCode.

1. Install XCode command-line tools.

1. Clone the project.

   ```bash
   git clone git@github.com:hammer-hao/aeolus.git && cd Aeolus
   ```

1. Generate CMake build tree.

   ```bash
   cmake -B build
   ```

1. Build the project.

   ```bash
   cmake --build build --parallel $(nproc)
   ```

1. Launch the bot with the specified absolute path to a SC2 map, e.g.:

   ```bash
   ./build/bin/Aeolus "/home/alkurbatov/Ladder2019Season3/AcropolisLE.SC2Map"
   ```

### Linux

1. Install [CMake](https://cmake.org/download/).

1. Install `gcc-c++`.

1. Install the `make` utility.

1. Get the project.

   ```bash
   git clone git@github.com:hammer-hao/aeolus.git && cd aeolus
   ```

1. Generate CMake build tree.

   ```bash
   cmake -B build
   ```

1. Build the project.

   ```bash
   cmake --build build --parallel $(nproc)
   ```

1. Launch the bot with the specified absolute path to a SC2 map, e.g.:

   ```bash
   ./build/bin/Aeolus "/home/alkurbatov/Ladder2019Season3/AcropolisLE.SC2Map"
   ```

## Additional options

### WSL2 Support

Cross compiling for Windows under WSL2 is supported through `cpp-sc2`. See the `cpp-sc2` [documentation](https://github.com/cpp-sc2/cpp-sc2/blob/master/docs/building.md#wsl2-support) for build requirements. The build flag remains the same, setting `-DWSL2_CROSS_COMPILE=ON`.

### Game client version
By default, the API assumes the latest version of the game client. The assumed version can be found in cmake's output, e.g.:
```bash
$ cmake -B build grep 'SC2 version'
Target SC2 version: 5.0.5
...
```

However, sometimes one may need to compile with an older version of the game, e.g. to play with a Linux build which is
always behind the Windows version. It is possible by specifying the game version manually, e.g.:
```bash
cmake -B build -DSC2_VERSION=4.10.0
```

### AIArena ladder build
To compile a bot capable to play on [the AIArena ladder](https://aiarena.net), configure the project in the following way:
```bash
cmake -B build -DBUILD_FOR_LADDER=ON -DSC2_VERSION=4.10.0
```

## Managing CMake dependencies

`Aeolus` uses the CMake `FetchContent` module to manage and collect dependencies including `cpp-sc2`, `eigen`, and `SDL2`. You can also use your own versions of these dependencies. For example, to use a version of `cpp-sc2` outside of the pinned commit, modify the `GIT_REPOSITORY` and/or the `GIT_TAG` in `cmake/cpp_sc2.cmake`:
```
...
FetchContent_Declare(
    cpp_sc2
    GIT_REPOSITORY <target-cpp-sc2-git-rep>
    GIT_TAG <git-commit-hash>
)
FetchContent_MakeAvailable(cpp_sc2)
...
```
With the updated configuration, re-run the build configuration and the dependency will be updated to the new configuration:
```
cmake -B build
```

## Troubleshooting

### CMake options don't take effect

If you see that some of cmake options don't take effect
(e.g. project was configured with `cmake -B build -DBUILD_FOR_LADDER=ON` and a bit later with `cmake -B build`)
remove the `build` folder or the `CMakeCache.txt` file, or explicitly re-specify the desired default behavior (e.g. `cmake -B build -DBUILD_FOR_LADDER=OFF`).

### Build freezes (Linux or macOS)

If project compilation freezes, decrease nproc to 1 or more, e.g.:

``` bash
cmake --build build --parallel $(nproc --ignore=1)
```

## License

Licensed under the [MIT license](LICENSE).
