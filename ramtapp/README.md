# Introduction
This folder contains sample applications for testing RDK-E RDK App Managers.
# Building the code
There are two ways you can build the code.
## Using Yocto
Try this command
```
devtool add -S <SRCREV> ramtapp https://github.com/rdkcentral/feature-test-tools.git --src-subdir ramtapp --srcbranch develop && devtool build ramtapp
```
or use this recipe
```
SUMMARY          = "ramtapp - RDK app managers test app"
DESCRIPTION      = "C++ utility to test the various components in \
RDK app managers framework and perform basic benchmark testing."

HOMEPAGE         = "https://github.com/rdkcentral/feature-test-tools/tree/develop/ramtapp"

LICENSE          = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"
SRC_URI          = "git://github.com/rdkcentral/feature-test-tools.git;protocol=https;no-branch=1"
SRCREV           = "<SRCREV>"
PV              ?= "1.0.0"
PR              ?= "r0"
S                = "${WORKDIR}/git/ramtapp"

DEPENDS          = "wpeframework entservices-apis jsoncpp curl"
PACKAGE_ARCH = "${MIDDLEWARE_ARCH}"

inherit cmake pkgconfig
```
## Using CMake
Make sure you build all dependencies first. These can be built using CMake.
~~~bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="<path to dependency installation>" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_FLAGS="-g -I<dependency header path>" -DCMAKE_EXE_LINKER_FLAGS="-L<dependency library path>"
cmake --build build
~~~
