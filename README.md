![build status](https://github.com/kkosciusz/raceintospace/actions/workflows/CI.yaml/badge.svg?event=push)

Race into Space
===============

Race  into  Space is  the  free  software  version of  Interplay's  Buzz
Aldrin's Race into Space.  This is the  reworked version  following  the
source release  for the computer version  of the Liftoff! board  game by
Fritz Bronner.  The original was developed by Strategic Visions and published by
Interplay as a disk-based game in 1992 and a CD-ROM in 1994.

The most recent stable releases are available from
[SourceForge](http://sourceforge.net/projects/raceintospace/).
There is a more recent (interim) 1.2 release on Flatpak (which mistakenly shows as 2.0), 
https://flathub.org/apps/details/org.raceintospace.Raceintospace.
More recent yet is the 2.0 beta available here at 
https://github.com/raceintospace/raceintospace/releases/tag/v.2.0beta.
The repository here contains a work-in-progress modernization of the game, 
so if you want to run the very latest version, you will have to download
and compile the source.  Instructions are below.

We have made a number of improvements to the game, and are hoping to do a 
release soon.  The changes made can be viewed in ChangeLog.md above.

License
=======

Race  Into Space  is distributed  under GNU  General Public  License
(GPL)  version 2.  You can find  the  terms and  conditions in  file
`COPYING`.

Contributing
============

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

Building
========

You need CMake.

The build system automatically finds, downloads, and/or compiles:

* SDL
* Boost
* zlib
* libpng
* jsoncpp
* libogg
* libvorbis
* libtheora
* libprotobuf / protoc

The specifics of which things come from where will depend on your platform.

On UNIXy systems (including Mac OS X), you can build everything with:

    $ git clone git://github.com/raceintospace/raceintospace.git
    $ mkdir raceintospace-build; cd raceintospace-build
    $ cmake ../raceintospace
    $ make

To download a copy of the game, go to the folder where you want the raceintospace 
subfolder to go, and run:

    git clone https://github.com/raceintospace/raceintospace


Music
=====

Due to copyright concerns, the game's music has been placed in a nonfree repository.
To add music to the game, run the following:

    $ cd ~/ && git clone https://github.com/raceintospace/raceintospace-nonfree && cp -r ~/raceintospace-nonfree/data/audio/music/ ~/raceintospace/data/audio/ && cp -r ~/raceintospace-nonfree/data/midi/ ~/raceintospace/data/audio/


Linux
-----

### Debian/Ubuntu

We rely on the operating system to provide most of the dependencies. If you're
on a Debian-based system, you can get everything with:

    sudo apt-get install cmake libsdl1.2-dev libboost-dev libpng-dev \
        libjsoncpp-dev libogg-dev libvorbis-dev libtheora-dev \
        libprotobuf-dev protobuf-compiler

Or enter this all on one line:

    sudo apt-get install cmake libsdl1.2-dev libboost-dev libpng-dev libjsoncpp-dev libogg-dev libvorbis-dev libtheora-dev libprotobuf-dev protobuf-compiler

If you don't already have git installed, you'll also have to run:

    sudo apt install git

You may also need to run (especially if you get "No CMAKE_CXX_COMPILER could be found."):

    sudo apt-get install build-essential

### Fedora

If you are on a Fedora-based system, you can get everything with:

    $ sudo dnf install cmake gcc-c++ SDL-devel protobuf-devel boost-devel \
        libogg-devel libvorbis-devel libtheora-devel jsoncpp-devel \
        physfs-devel libpng-devel

### Build

To compile the game quickly you can run a series of commands in-line. This 
creates a folder where the compiled game will be installed. If you want it named 
something other than raceintospace-build, simply change the name. The last two 
commands also run the game automatically.

    mkdir raceintospace-build && cd raceintospace-build && cmake -DCMAKE_BUILD_TYPE=Debug ../raceintospace && make run
Or:
    mkdir raceintospace-build && cd raceintospace-build && cmake ../raceintospace && make && sudo make install && cd src/game && ./raceintospace


Once built, you can automatically delete the -build folder and recompile by entering this (assuming raceintospace and raceintospace-build are on the root of your home folder):

    cd ~/ && sudo mkdir raceintospace-build && cd raceintospace-build && cmake ../raceintospace && make && sudo make install && cd src/game && ./raceintospace

Used to be:

    cd ~/ && sudo rm -rf ~/raceintospace-build && mkdir raceintospace-build && cd raceintospace-build && cmake -DCMAKE_BUILD_TYPE=Debug ../raceintospace && make && sudo make install && cd src/game && ./raceintospace


After the first build, you can download the latest version of the source with:

    cd ~/ && sudo rm -r raceintospace && git clone https://github.com/raceintospace/raceintospace

Then delete raceintospace-build and recompile:

    cd ~/ && sudo rm -r raceintospace-build && mkdir raceintospace-build && cd raceintospace-build && cmake ../raceintospace && make && sudo make install && cd src/game && ./raceintospace


You can run these commands separately, or just run the ones you need (for instance, 
if you already have the folder you want the game installed to, there's of course 
no need to create it.

    mkdir raceintospace-build
    cd raceintospace-build
    cmake -DCMAKE_BUILD_TYPE=Debug ../raceintospace
    make run

Mac OS X
--------

You need CMake, which is readily obtained from
[Homebrew](http://mxcl.github.com/homebrew/), which you probably have anyway.
Homebrew also has a nice Boost package that doesn't cause any runtime linkage
issues, so install that along with SDL:

    $ brew install cmake boost sdl

CMake automatically handles the other dependencies using the stuff in `lib/`.
Mac OS X sometimes includes libpng, and sometimes it doesn't, so we build our
own. We do, however, rely on the platform-provided zlib.

You might want to use Xcode for development. CMake can generate an Xcode
project file:

    $ git clone git://github.com/raceintospace/raceintospace.git
    $ rm -r raceintospace-build; cd raceintospace-build
    $ cmake -G Xcode ../raceintospace
    $ open raceintospace.xcodeproj

Windows
-------

Ingredients:

* [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) ($0, but read the fine print)
    * Install at least *MSVC*, *C++ CMake for Windows* and *Windows SDK* components!
    * Full IDE is not needed
* [CMake](https://www.cmake.org/cmake/resources/software.html) (free, also included above)
* [Git](https://git-scm.com/downloads) or [GitHub for Windows](https://desktop.github.com/) (free)

Open the command prompt and make sure git, your compiler and cmake are accessible (check [Visual Studio Developer Command Prompt and Developer PowerShell](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022))

Then clone, configure and build the game:

```
cd <some_directory>
git clone --recurse-submodules https://github.com/raceintospace/raceintospace.git
cd raceintospace
cmake --build --preset windows-release
```

After building, you can start the game:

```
.\build\release\src\game\raceintospace.exe BARIS_DATA=data BARIS_SAVE=.\save
```

The following instructions may be relevant for setting up in Visual Studio IDE:

- Start Visual Studio by opening "raceintospace.sln".
- Right-click "ALL_BUILD" and choose "Build"  (Everything will download and install)
- Right-click on "Race Into Space" and choose "Set as startup project"
- Right-click on "Race Into Space" and choose "Properties"
- Open "Configuration Properties" and choose "Output Directory" and set it to the path for "raceintospace-build\local\bin"
- Click on "Debugging" and edit "Environment".
Add:
   BARIS_DATA=[path to raceintospace\data]
   BARIS_SAVE=.\save
Click "run"
