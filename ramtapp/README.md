# ramtapp
This folder contains sample applications for testing RDK-E RDK App Managers.

## Building

<details>
  <summary>Sample bitbake recipe </summary>
  
### Bitbake recipe

```bash
SUMMARY = "Appmanager test application
DESCRIPTION = "Native C++ test application for appmanagers using COMRPC communication"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

inherit cmake pkgconfig

SRC_URI = "${CMF_GITHUB_ROOT}/feature-test-tools;${CMF_GITHUB_SRC_URI_SUFFIX}"
SRCREV = "45807b09decfcb4cef9344661cf75ad3cabe129d"
PV = "1.0.0"
PR = "r0"

S = "${WORKDIR}/git/ramtapp"

DEPENDS = "entservices-apis jsoncpp"
RDEPENDS:${PN} +=  "entservices-apis jsoncpp"
```
