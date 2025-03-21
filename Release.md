# Release instructions

## Prerequisites
Ensure that the main CMakeLists.txt generates the correct version number. However, the packaging system also requires the correct release tag to be set, therefore it is best to first create the release on Github without any files, build all the binary packages according to these instructions, and finally add the resulting binaries to the already existing release. Alternatively, you can tag the release using `git tag`, push it to the repository, and only create the Github release in the end.

## Create the release on Github
This requires write access to the raceintospace repository.
1. Go to Add Release on the main page of the repository
2. Add a new release tag (e.g., "v2.0.0")
3. Enter the remaining information and publish. You can add the binary packages later on.

## Source distribution
1. Start with a fresh clone from master

   `git clone https://github.com/raceintospace/raceintospace`

   Note that this doesn't contain --recurse-submodules, as we don't want vcpkg to be included in the source release.
3. Change into a separate build directory
4. cmake $PATH_TO_SOURCE_DIRECTORY
5. make package_source
This produces a tarball in the build directory. 

## Linux binaries
We build a generic tarball, a DEB package, and an RPM package. This requires the packages dpkg and rpm being installed.
1. Copy the music files from raceintospace-nonfree
2. cmake --preset linux-release $PATH_TO_SOURCE_DIRECTORY
3. navigate to the location it gives, e.g. ~/raceintospace/build/release
4. make package (probably needs to be sudo make package)
5. cmake -DCMAKE_INSTALL_PREFIX=/usr -DCPACK_GENERATOR=DEB $PATH_TO_SOURCE_DIRECTORY (e.g., ~/raceintospace)
6. cpack -G DEB
7. cpack -G RPM

## Windows binaries
In addition to the requirements to build the game, creating the installer also requires [NSIS](https://nsis.sourceforge.io/Download) being present. The safest bet is to install NSIS into the default location (e.g., `C:\Progam Files (x86)\NSIS`) and ensure that makensis.exe is present in that folder.
1. Start with a fresh clone from master
2. Copy the music files from raceintospace-nonfree
3. Follow the [instructions to compile the game](README.md#windows)
4. Change into the build\release subdirectory and run cpack there

## macOS binaries
TODO

## Push to Flathub
TODO

## Further information
https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository
