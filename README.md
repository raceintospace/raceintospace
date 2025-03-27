![build status](https://github.com/raceintospace/raceintospace/actions/workflows/CI.yaml/badge.svg?event=push)

# Race into Space

Race  into  Space is  the  free  software  version of  Interplay's  Buzz
Aldrin's Race into Space.  This is the  reworked version  following  the
source release  for the computer version  of the Liftoff! board  game by
Fritz Bronner.  The original was developed by Strategic Visions and published by
Interplay as a disk-based game in 1992 and a CD-ROM in 1994.

The most recent stable releases are available from this page, on the righthand 
side.  The 2.0.0 release can be downloaded directly from 
https://github.com/raceintospace/raceintospace/releases/tag/v2.0.0.
If you want to run the very latest version, you will have to download
and compile the source.  Instructions are below.

# License

Race  Into Space  is distributed  under GNU  General Public  License
(GPL)  version 2.  You can find  the  terms and  conditions in  file
`COPYING`.

# Getting help

We have a [Matrix space](https://matrix.to/#/#raceintospace:matrix.org), where 
you can obtain help, discuss the game in general, and also find others to play
against, The space consists of two rooms:

| Room                  | Description                           |
| --------------------- | ------------------------------------- |
| [raceintospace-general](https://matrix.to/#/#raceintospace-general:matrix.org) | General discussions about the game.   |
| [raceintospace-games](https://matrix.to/#/#raceintospace-games:matrix.org)   | Arrange games to play against others. |

# Contributing

We coordinate development through the [GitHub issue
tracker](https://github.com/raceintospace/raceintospace/issues). Feel free to
report bugs, discuss tasks, or pick up work there. If you want to make
changes, please fork, edit, and [send us a pull
request](https://github.com/raceintospace/raceintospace/pull/new/master).

There's a `.git-hooks` directory in the root. This contains a `pre-commit`
hook that verifies code styling before accepting changes. You can add this to
your local repository's `.git/hooks/` directory like:

    $ cd raceintospace
    $ ln -s ../../.git-hooks/pre-commit .git/hooks/pre-commit

Pull requests that don't match the project code style are still likely to be
accepted after manually formatting and amending your changeset. The formatting
tool (`astyle`) is completely automated; please try to use it.

# Obtaining source and data files

Clone the git repository:

    git clone --recurse-submodules git://github.com/raceintospace/raceintospace.git

Or try via https if you encounter issues with git protocol:

    git clone --recurse-submodules https://github.com/raceintospace/raceintospace

This creates sub-directory `raceintospace` where you can build the game.

In case you did not specify `--recurse-submodules` when cloning the repository,
you will need to initialize submodules inside the cloned repo:

    git submodule init --recursive

# Building

You need a modern C++ compiler and CMake.

The build system uses [vcpkg.io](https://vcpkg.io/) to automatically download
and build all dependencies.

The creation of binary packages for releases is described in a [set of separate instructions](Release.md).

## Linux

### Debian/Ubuntu

We rely on the operating system to provide most of the dependencies. If you're
on a Debian-based system, you can get everything with:

    sudo apt-get install cmake libsdl1.2-dev libboost-dev libpng-dev \
        libjsoncpp-dev libogg-dev libvorbis-dev libtheora-dev \
        libphysfs-dev libcereal-dev libprotobuf-dev protobuf-compiler

Or enter this all on one line:

    sudo apt-get install cmake libsdl1.2-dev libboost-dev libpng-dev libjsoncpp-dev libogg-dev libvorbis-dev libtheora-dev libphysfs-dev libcereal-dev libprotobuf-dev protobuf-compiler

If you don't already have git installed, you'll also have to run:

    sudo apt install git

You may also need to run (especially if you get "No CMAKE_CXX_COMPILER could be found."):

    sudo apt-get install build-essential

### Fedora

If you are on a Fedora-based system, you can get everything with:

    $ sudo dnf install cmake gcc-c++ SDL-devel protobuf-devel boost-devel libogg-devel libvorbis-devel libtheora-devel jsoncpp-devel physfs-devel libpng-devel cereal-devel

### Build with system dependencies

To compile and run the game quickly run the following commands from the source directory:

    cmake --preset linux-release
    cmake --build --preset linux-release --target run

The build directory is `build/release`, you can always delete it to start from a clean build.

### Build without system depencencies

Alternatively, you can have all dependencies downloaded and built automatically:

    cmake --preset linux-vcpkg
    cmake --build --preset linux-vcpkg --target run

### Installing

To install in the default system location:

    sudo cmake --build --preset XXX --target install

To install in your home directory, you need to pass `--prefix` to configure first:

    cmake --preset XXX --prefix "$HOME"
    cmake --build --preset XXX --target install

## Mac OS X

Building is supported on MacOS 15 and later, on Apple chips.

You need CMake, which is readily obtained from
[Homebrew](http://mxcl.github.com/homebrew/), which you probably have anyway. You also need `pkg-config`, 
if you don't have it already. The only external dependency needed is `sdl`.

    $ brew install cmake sdl pkg-config

CMake and vcpkg automatically handle all other dependencies.

To obtain source code and build use:

    $ git clone --recurse-submodules git://github.com/raceintospace/raceintospace.git
    $ cd raceintospace
    $ cmake --preset macos-vcpkg
    $ cmake --build --preset macos-vcpkg

You might want to use Xcode for development. CMake can generate an Xcode
project file:

    $ cmake --preset macos-vcpkg -G Xcode
    $ open raceintospace.xcodeproj

## Windows

The easiest way to compile the game is the following:

1. Before you start, ensure that the following is installed:

   * [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) ($0, but read the fine print)
       * Install at least *MSVC*, *C++ CMake for Windows* and *Windows SDK* components!  It may show as "Desktop development with C++"
       * If you have already installed Visual Studio but are missing a component, you can launch the Visual Studio installer app, choose "Modify" and select further components for installation.
   * [Git](https://git-scm.com/downloads) (You can accept the defaults, though you may want to tell it to use Notepad for the default editor.) You may need to reboot for git to be added to your $PATH.
2. Start Visual Studio and select Continue without code
3. Launch Tools -> Command Line -> [Developer Command Prompt](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022). This is important and puts cmake and ninja into your path.
4. In that command prompt (not the Windows one), clone, configure, and build the game:

    ```
    git clone --recurse-submodules https://github.com/raceintospace/raceintospace.git
    cd raceintospace
    cmake --preset windows-release
    cmake --build --preset windows-release
    ```

5. After building, navigate to the raceintospace folder, and you can start the game:

    `.\build\release\src\game\raceintospace.exe BARIS_DATA=data`

The following instructions may be relevant for setting up in Visual Studio IDE:

- Start Visual Studio by opening "raceintospace.sln".
- Right-click "ALL_BUILD" and choose "Build"  (Everything will download and install)
- Right-click on "Race Into Space" and choose "Set as startup project"
- Right-click on "Race Into Space" and choose "Properties"
- Open "Configuration Properties" and choose "Output Directory" and set it to the path for "raceintospace-build\local\bin"
- Click on "Debugging" and edit "Environment".
Add:
```
   BARIS_DATA=[path to raceintospace\data]
   BARIS_SAVE=.\save
```
Click "run"

# Music

Due to copyright concerns, the game's music has been placed in a nonfree repository.
To add music to the game, obtain the files from the repository:

    git clone https://github.com/raceintospace/raceintospace-nonfree 

Then copy the `data/audio/music` directory from `raceintospace-nonfree` to:

1. `data/audio` in the source code directory - if you're building the game from source, or
2. `data/audio` in the directory where you installed the game.
