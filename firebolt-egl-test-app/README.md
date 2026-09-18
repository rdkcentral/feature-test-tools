# Firebolt C++ EGL Test Application

Native C++ EGL test application conforming to Firebolt Lifecycle

## Prerequisites

| Requirement | Notes |
|---|---|
| **CMake ≥ 3.13** | |
| **C++17 compiler** | GCC 7+ or Clang 5+ |
| **FireboltClient v0.7.0** installed | Build from [firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) |
| **FireboltTransport v1.1.12** installed | Bundled from [firebolt-cpp-transport](https://github.com/rdkcentral/firebolt-cpp-transport) |
| **wayland-client / wayland-egl** | Required for the GL display window (`gl.cpp`) |
| **EGL / OpenGL ES 3 (GLES3 headers)** | Required for the GL display window (`gl.cpp`) |
| **Cairo / cairo-ft / FreeType** | Required for the GL display window (`gl.cpp`) |
| **xkbcommon** (optional) | Enables XKB keymap translation in the GL window; falls back to raw evdev codes without it |

Logging level is controlled via the `GLLOGLEVEL` (GL module) and `APPLOGLEVEL` (app module) environment variables.

---

## Building

```bash
cmake -S . -B build
cmake --build build --parallel
```

<details>
  <summary>Sample bitbake recipe & bolt package configuration</summary>

### Bitbake recipe

```bash
SUMMARY = "Firebolt C++ EGL Test Application"
DESCRIPTION = "Native C++ EGL test application conforming to Firebolt Lifecycle"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../../LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

inherit cmake pkgconfig

SRC_URI = "${CMF_GITHUB_ROOT}/feature-test-tools;${CMF_GITHUB_SRC_URI_SUFFIX}"
SRCREV = "${AUTOREV}"  <=== Replace with SHA
PV = "1.0.0"
PR = "r0"

S = "${WORKDIR}/git/firebolt-egl-test-app"

DEPENDS = "firebolt-cpp-client nlohmann-json cairo virtual/egl virtual/libgles2 freetype westeros-simpleshell libxkbcommon"
RDEPENDS:${PN} += "firebolt-cpp-client firebolt-cpp-transport cairo westeros-simpleshell libxkbcommon xkeyboard-config"

FILES:${PN} += " /usr/share/*"
```

### Bolt package configuration

```json
{
  "id": "com.rdkcentral.fbtegltest",
  "version": "0.0.1",
  "name": "fbtegltest",
  "packageType": "application",
  "entryPoint": "/usr/bin/firebolt-egl-test-app",
  "dependencies": {
    "com.rdkcentral.base": ">= 0.3.0"
  },
  "permissions": [
      "urn:rdk:permission:firebolt"
  ],
  "configuration": {
      "urn:rdk:config:env": {
          "PATTERN_MODE": "GRID",
          "WIDTH": "1920",
          "HEIGHT": "1080",
          "GLLOGLEVEL":"DEBUG",
          "APPLOGLEVEL":"DEBUG"
      }
  }
}
```

</details>

### Font License Note

[`LICENSE`](../firebolt-test-app/native/assets/LICENSE) is the license text installed from the Liberation font package for
`LiberationSans-Bold.ttf`.

---

## Running

This is a lifecycle-driven Firebolt application; ensure `WAYLAND_DISPLAY`, `XDG_RUNTIME_DIR`, and `FIREBOLT_ENDPOINT` are configured.

### Binary name
```
firebolt-egl-test-app
```

Runtime env vars:

| Variable | Description |
|---|---|
| `FIREBOLT_ENDPOINT` | Firebolt endpoint for the app session provided by AppManager |
| `WAYLAND_DISPLAY` | Wayland socket name used by the GL app (default `wayland-0`) |
| `WIDTH` | GL window width (default `1920`) |
| `HEIGHT` | GL window height (default `1080`) |
| `PATTERN_MODE` | GL background pattern (`GRID` or `DOT`) |

---

## License

Apache-2.0 – see [LICENSE](./../../LICENSE)

---

## Third-Party Attributions

#### Font used in this app: Liberation Sans Bold (LiberationSans-Bold.ttf)

| Field | Value |
|---|---|
| **Font** | Liberation Sans Bold |
| **Copyright holders** | Google Corporation (digitized data); Red Hat, Inc. |
| **Reserved Font Names** | Arimo, Tinos, Cousine, Liberation |
| **License** | [SIL Open Font License, Version 1.1](../firebolt-test-app/native/assets/LICENSE) |
| **Source** | https://github.com/liberationfonts/liberation-fonts |
| **Bundled at** | `../firebolt-test-app/native/assets/LiberationSans-Bold.ttf` |
