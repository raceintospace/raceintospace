# Release instructions

## Prerequisites
Ensure that the main CMakeLists.txt generates the correct version number.

## Source distribution
1. Start with a fresh clone from master
2. Change into a separate build directory
3. cmake $PATH_TO_SOURCE_DIRECTORY
4. make package_source
This produces a tarball in the build directory. 

## Linux binaries
We build a generic tarball, a DEB package, and an RPM package. This requires the packages dpkg and rpm being installed.
1. Copy the music files from raceintospace-nonfree
2. cmake --preset linux-release $PATH_TO_SOURCE_DIRECTORY
3. make package
4. cmake -DCMAKE_INSTALL_PREFIX=/usr -DCPACK_GENERATOR=DEB $PATH_TO_SOURCE_DIRECTORY
5. cpack -G DEB
6. cpack -G RPM

## Windows binaries
In addition to the requirements to build the game, creating the installer also requires [NSIS](https://nsis.sourceforge.io/Download) being present.
1. Start with a fresh clone from master
2. Copy the music files from raceintospace-nonfree
3. Follow the instructions to compile the game
4. Run cpack in the build/release subdirectory

## macOS binaries
TODO

## Create the release on Github
This requires write access to the raceintospace repository.
1. Go to Add Release on the main page of the repository
2. Add a release tag (e.g., "v2.0.0")
3. Enter the remaining information, upload the files, and publish

## Push to Flathub
TODO

## Further information
https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository
