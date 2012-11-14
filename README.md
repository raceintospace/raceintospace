Race into Space [![Build Status](https://secure.travis-ci.org/raceintospace/raceintospace.png?branch=master)](https://travis-ci.org/raceintospace/raceintospace)
===============

Race  into  Space is  the  free  software  version of  Interplay's  Buzz
Aldrin's Race into Space.  This is the  reworked version  following  the
source release  for the computer version  of the Liftoff! board  game by
Fritz Bronner.  This was developed by Strategic Visions and published by
Interplay as a disk-based game in 1992 and a CD-ROM in 1994.

The Race Into Space project officially lives on
[SourceForge](http://sourceforge.net/projects/raceintospace). This repository
contains a work-in-progress modernization of the game.

License
=======

Race  Into Space  is distributed  under GNU  General Public  License
(GPL)  version 2.  You can find  the  terms and  conditions in  file
`COPYING`.

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

The specifics of which things come from where depend on your platform.

On UNIXy systems (including Mac OS X), you can build everything with:

    $ git clone git://github.com/raceintospace/raceintospace.git
    $ cd raceintospace/
    $ mkdir build; cd build
    $ cmake ..
    $ make

Linux
=====

We rely on the operating system to provide most of the dependencies. If you're
on a Debian-based system, you can get everything with:

    $ sudo apt-get install cmake libsdl-dev libboost-dev libpng-dev \
        libogg-dev libvorbis-dev libtheora-dev libprotobuf-dev protobuf-compiler

Mac OS X
========

You need CMake, which is readily obtained from
[Homebrew](http://mxcl.github.com/homebrew/), which you probably have anyway.
Homebrew also has a nice Boost package that doesn't cause any runtime linkage
issues, so install that too:

    $ brew install cmake boost

CMake automatically handles the other dependencies using the stuff in `lib/`.
Mac OS X sometimes includes libpng, and sometimes it doesn't, so we build our
own. We do, however, rely on the platform-provided zlib.

You might want to use Xcode for development. CMake can generate an Xcode
project file:

    $ rm -r build
    $ mkdir build; cd build
    $ cmake -G Xcode ..
    $ open raceintospace.xcodeproj

Windows
=======

Ingredients:

* [Visual C++ Express 2010](http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express) ($0; other versions likely work too)
* [CMake](http://www.cmake.org/cmake/resources/software.html) (free)
* A checkout of the source (e.g. from [GitHub for Windows](http://windows.github.com/))

From here, use CMake-GUI or the command-line CMake to generate Visual Studio
project files. Literally every dependency will be downloaded and compiled
automatically.
