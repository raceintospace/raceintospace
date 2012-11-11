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

You need:

* CMake
* SDL
* Boost
* libpng (and zlib)
* libogg
* libvorbis
* libtheora

On UNIXy systems (including Mac OS X), you can build everything with:

    $ git clone git://github.com/raceintospace/raceintospace.git
    $ cd raceintospace/
    $ mkdir build; cd build
    $ cmake ..
    $ make

Linux
=====

Build as described above.

If you're on a Debian-based system, you can get all the dependencies with:

    $ sudo apt-get install cmake libsdl-dev libboost-dev libpng-dev \
        libogg-dev libvorbis-dev libtheora-dev

Mac OS X
========

Mac OS X ships with libpng, but platform help ends there. You'll want to grab
[SDL.framework](http://www.libsdl.org/download-1.2.php) if you don't have it
already. It's easiest to use [Homebrew](http://mxcl.github.com/homebrew/) to
get the rest:

    $ brew install boost libogg libvorbis theora

You might want to use Xcode for development. CMake can generate an Xcode
project file:

    $ rm -r build
    $ mkdir build; cd build
    $ cmake -G Xcode ..
    $ open raceintospace.xcodeproj

Note that the resulting executables are dynamically linked against the
Homebrew-compiled Ogg/Vorbis/Theora libs, adding a runtime dependency. We need
a way to internalize these dependencies before shipping anything generated
from this build process.

Windows
=======

Ingredients:

* [Visual Studio Express 2012](http://www.microsoft.com/visualstudio/eng/downloads#d-express-windows-desktop) ($0; other versions likely work too)
* [CMake](http://www.cmake.org/cmake/resources/software.html) (free)
* A checkout of the source (e.g. from [GitHub for Windows](http://windows.github.com/))

From here, use CMake-GUI or the command-line CMake to generate Visual Studio
project files.

CMake will fail until you get all the other dependencies into your search path
somehow. We should really work on getting those packed up into `lib/`...
