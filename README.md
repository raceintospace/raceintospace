Race into Space
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

* git
* SDL
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
