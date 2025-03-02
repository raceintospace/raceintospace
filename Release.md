# Release instructions
Most of the steps require write access to the raceintospace directory.

## Prerequisites
Ensure that the main CMakeLists.txt generates the correct version number.

## Source distribution
1. Start with a fresh clone from master
2. Change into a separate build directory
3. cmake $PATH_TO_SOURCE_DIRECTORY
4. make package_source
This produces a tarball in the build directory. 

## Linux binaries
1. Copy the music files from raceintospace-nonfree
2. cmake --preset linux-release $PATH_TO_SOURCE_DIRECTORY
3. make package
4. TODO: deb/rpm files

## Windows binaries
TODO

## macOS binaries
TODO

## Create the release on Github
1. Go to Add Release on the main page of the repository
2. Add a release tag (e.g., "v2.0.0")
3. Enter the remaining information, upload the files, and publish

## Push to Flathub
TODO

## Further information
https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository
