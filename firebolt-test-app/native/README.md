# Firebolt C++ Test Application

A native C++ firebolt test application that exercises the
[firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) APIs
and events/notifications across all supported Firebolt modules.

It uses thunder APIs to alter the system configurations to test the Firebolt events and it never restores to original.
Explicitly do a device factory reset when the app exits.

---

## Project Layout

```
native/
├── CMakeLists.txt              # Top-level CMake project
├── assets/
│   ├── LiberationSans-Bold.ttf # Embedded font for the GL display window (OFL 1.1)
│   └── LICENSE                 # License text installed from the Liberation font package (OFL 1.1)
└── src/
    ├── main.cpp                # Entry point, Firebolt connection, lifecycle monitoring, GL app management
    ├── utils.h / utils.cpp     # Shared helpers: AppConfig, fireboltVersion, chooseFromList, TestModuleBase
    ├── gl.h                    # GlApp class declaration (Wayland/EGL/GLES keycode display window)
    ├── gl.cpp                  # GlApp implementation
    ├── native_logger.hpp       # Shared logging infrastructure (DBG/INFO/WARN/ERR/FATAL macros)
    └── tests/
        ├── accessibilityTest.h/.cpp
        ├── actionsTest.h/.cpp
        ├── advertisingTest.h/.cpp
        ├── deviceTest.h/.cpp
        ├── discoveryTest.h/.cpp
        ├── displayTest.h/.cpp
        ├── lifecycleTest.h/.cpp           # *(Disabled — app itself is a lifecycle client)*
        ├── localizationTest.h/.cpp
        ├── metricsTest.h/.cpp
        ├── networkTest.h/.cpp
        ├── presentationTest.h/.cpp
        ├── SpeechSynthesisTest.h/.cpp
        ├── statsTest.h/.cpp
        ├── texttospeechTest.h/.cpp
        ├── VideoOutputTest.h/.cpp
        └── ws_comm_tester.h    # Internal WebSocket/Thunder communication test helper
```

---

## Prerequisites

| Requirement | Notes |
|---|---|
| **CMake ≥ 3.13** | |
| **C++17 compiler** | GCC 7+ or Clang 5+ |
| **FireboltClient v0.7.0** installed | Build from [firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) |
| **FireboltTransport v1.1.12** installed | Bundled from [firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-transport) |
| **nlohmann-json** installed | Used for JSON input/response validation in tests |
| **OpenSSL ≥ 3.0** | Required for secure WebSocket transport (libssl, libcrypto) |
| **websocketpp** | Required for WebSocket communication |
| **wayland-client / wayland-egl** | Required for the GL display window (`gl.cpp`) |
| **EGL / GLESv2** | Required for the GL display window (`gl.cpp`) |
| **Cairo / cairo-ft / FreeType** | Required for the GL display window (`gl.cpp`) |
| **xkbcommon** (optional) | Enables XKB keymap translation in the GL window; falls back to raw evdev codes without it |

The `FireboltClient`, `FireboltTransport`, `nlohmann_json`, `OpenSSL`, and `websocketpp` CMake packages must be findable via
`CMAKE_PREFIX_PATH` (or `CMAKE_SYSROOT` for cross-compilation).

Logging level is controlled via the `GLLOGLEVEL` (GL module) and `APPLOGLEVEL` (app module) environment variables.

---

## Building

```bash
cmake -S . -B build -DBUILD_FIREBOLT_APP=ON -DGL_MODULE_SHARED=ON
cmake --build build --parallel
```

<details>
  <summary>Sample bitbake recipe & bolt package configuration</summary>

### Bitbake recipe

```bash
SUMMARY = "Firebolt C++ Test Application"
DESCRIPTION = "Native C++ test application for exercising firebolt-cpp-client APIs and events"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../../LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

inherit cmake pkgconfig

SRC_URI = "${CMF_GITHUB_ROOT}/feature-test-tools;${CMF_GITHUB_SRC_URI_SUFFIX}"
SRCREV = "${AUTOREV}"  <=== Replace with SHA
PV = "3.0.0"
PR = "r0"

S = "${WORKDIR}/git/firebolt-test-app/native"

DEPENDS = "firebolt-cpp-client nlohmann-json cairo virtual/egl virtual/libgles2 freetype westeros-simpleshell libxkbcommon websocketpp asio openssl"
RDEPENDS:${PN} += "firebolt-cpp-client firebolt-cpp-transport cairo westeros-simpleshell libxkbcommon xkeyboard-config openssl"

EXTRA_OECMAKE:append = " \
    -DBUILD_FIREBOLT_APP=ON \
    -DGL_MODULE_SHARED=ON \
    "

FILES:${PN} += " /usr/share/*"
```

### Bolt package configuration

```json
{
  "id": "com.rdkcentral.fbttest",
  "version": "0.0.3",
  "name": "fbttest",
  "packageType": "application",
  "entryPoint": "/usr/bin/firebolt-test-app",
  "dependencies": {
    "com.rdkcentral.base": "0.3.1"
  },
  "permissions": [
      "urn:rdk:permission:firebolt",
      "urn:rdk:permission:thunder"
  ],
  "configuration": {
      "urn:rdk:config:env": {
          "PATTERN_MODE": "DOT",
          "WIDTH": "1920",
          "HEIGHT": "1080",
          "GLLOGLEVEL":"DEBUG",
          "MODE_AUTO_RUN":"true",
          "APPLOGLEVEL":"DEBUG"
      }
  }
}
```

</details>

### Font License Note

`assets/LICENSE` is the license text installed from the Liberation font package for
`LiberationSans-Bold.ttf`.

---

## Running

This is a **lifecycle-driven Firebolt application** with automatic testing via environment variables. The app subscribes to lifecycle state changes and runs module tests only after an `INITIALIZING → PAUSED → ACTIVE` transition.

### Binary name
```
firebolt-test-app
```

### Required environment variables

| Variable | Required | Description |
|---|---|---|
| `FIREBOLT_ENDPOINT` | Yes | WebSocket endpoint URL |
| `WAYLAND_DISPLAY` | Yes | Wayland socket name (e.g., `wayland-0`) |
| `XDG_RUNTIME_DIR` | Yes | Runtime directory for Wayland socket |

### Optional runtime environment variables

| Variable | Default | Description |
|---|---|---|
| `MODE_AUTO_RUN` | *(disabled)* | If set to a non-empty value other than `0`/`false`, runs all module tests automatically after app activation |
| `APPLOGLEVEL` | `Info` | App logging level: `Debug`, `Info`, `Notice`, `Warning`, `Error`, `Fatal` |
| `GLLOGLEVEL` | `Info` | GL module logging level: `Debug`, `Info`, `Notice`, `Warning`, `Error`, `Fatal` |
| `WIDTH` | `1920` | GL window width in pixels |
| `HEIGHT` | `1080` | GL window height in pixels |
| `PATTERN_MODE` | *(none)* | GL background pattern: `GRID` or `DOT` |

### GL display window

The app initializes a Wayland/EGL overlay window during lifecycle-driven startup and renders the last
received key code using the bundled Liberation Sans Bold font. `XDG_RUNTIME_DIR` must be set for GL initialization.

Example startup:
```bash
export FIREBOLT_ENDPOINT="<valid firebolt session token having end-point>"
export WAYLAND_DISPLAY="<provided by window manager>"
export XDG_RUNTIME_DIR="<provided by window management framework>"
export MODE_AUTO_RUN="true"
export WIDTH="1920"
export HEIGHT="1080"
export PATTERN_MODE="DOT"
firebolt-test-app
```

---

## Version-Aware Modules

Some modules expose additional methods depending on the selected Firebolt version at runtime:

| Module | Firebolt 8 methods | Additional Firebolt 9 methods |
|---|---|---|
| **Device** | `chipsetId`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged` (subscribe / unsubscribe), `unsubscribeAll` | `deviceClass`, `dolbyAtmosExperienceAvailable`, `onDolbyAtmosExperienceAvailableChanged` (subscribe / unsubscribe) |
| **Localization** | `country`, `preferredAudioLanguages`, `presentationLanguage`, `onCountryChanged` (subscribe / unsubscribe), `onPreferredAudioLanguagesChanged` (subscribe / unsubscribe), `onPresentationLanguageChanged` (subscribe / unsubscribe), `unsubscribeAll` | `timeZone`, `onTimeZoneChanged` (subscribe / unsubscribe) |

---

## Automatic Test Execution

When `MODE_AUTO_RUN` is enabled (set to any non-empty value except `0` or `false`), the app automatically runs every registered method and event subscription/unsubscription for all modules sequentially after the app reaches the `ACTIVE` lifecycle state. In auto mode, `.unsubscribe` and `.unsubscribeAll` methods are deferred and executed during shutdown cleanup.

Example:
```bash
export FIREBOLT_ENDPOINT="<valid firebolt session token having end-point>"
export WAYLAND_DISPLAY="<provided by window manager>"
export XDG_RUNTIME_DIR="<provided by window management framework>"
export MODE_AUTO_RUN="true"
firebolt-test-app
```

---

## Covered Modules & APIs

The app tests all Firebolt modules across all supported versions. The following modules are currently enabled:

### Base modules (Firebolt 8+)

| Module | Methods / Events |
|---|---|
| **Accessibility** | `audioDescription`, `closedCaptionsSettings`, `highContrastUI`, `voiceGuidanceSettings`, `onAudioDescriptionChanged` (subscribe / unsubscribe), `onClosedCaptionsSettingsChanged` (subscribe / unsubscribe), `onHighContrastUIChanged` (subscribe / unsubscribe), `onVoiceGuidanceSettingsChanged` (subscribe / unsubscribe), `unsubscribeAll` |
| **Advertising** | `advertisingId` |
| **Device** | `chipsetId`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged` (subscribe / unsubscribe), `unsubscribeAll`, `deviceClass` *(v9+)*, `dolbyAtmosExperienceAvailable` *(v9+)*, `onDolbyAtmosExperienceAvailableChanged` *(v9+)* (subscribe / unsubscribe) |
| **Discovery** | `watched`, `watchedV2` *(returns void; reports success/failure)* |
| **Display** | `size`, `maxResolution`, `edid` |
| **Localization** | `country`, `preferredAudioLanguages`, `presentationLanguage`, `onCountryChanged` (subscribe / unsubscribe), `onPreferredAudioLanguagesChanged` (subscribe / unsubscribe), `onPresentationLanguageChanged` (subscribe / unsubscribe), `unsubscribeAll`, `timeZone` *(v9+)*, `onTimeZoneChanged` *(v9+)* (subscribe / unsubscribe) |
| **Metrics** | `ready`, `signIn`, `signOut`, `startContent`, `stopContent`, `page`, `error`, `mediaLoadStart`, `mediaPlay`, `mediaPlaying`, `mediaPause`, `mediaWaiting`, `mediaSeeking`, `mediaSeeked`, `mediaRateChanged`, `mediaRenditionChanged`, `mediaEnded`, `event` *(validates schema + JSON data input)*, `appInfo` |
| **Network** | `connected`, `onConnectedChanged` (subscribe / unsubscribe / unsubscribeAll) |
| **Presentation** | `focused`, `onFocusedChanged` (subscribe / unsubscribe / unsubscribeAll) |
| **TextToSpeech** | `speak`, `getSpeechState`, `listVoices`, `pause`, `resume`, `cancel`, `onSpeechStart` (subscribe / unsubscribe), `onSpeechPause` (subscribe / unsubscribe), `onSpeechResume` (subscribe / unsubscribe), `onWillSpeak` (subscribe / unsubscribe), `onSpeechComplete` (subscribe / unsubscribe), `onSpeechInterrupted` (subscribe / unsubscribe), `onNetworkError` (subscribe / unsubscribe), `onPlaybackError` (subscribe / unsubscribe), `unsubscribeAll` |

### Firebolt 9+ additional modules

| Module | Methods / Events |
|---|---|
| **Actions** | `intent` *(validates response schema)*, `start` *(validates JSON input)*, `onIntent` (subscribe / unsubscribe / unsubscribeAll). Intent payloads follow the `{ action, context.source?, intentId }` model. |
| **SpeechSynthesis** | `voices`, `speak`, `cancel`, `pause`, `resume`, `onVoicesChanged` (subscribe / unsubscribe), `onUtteranceEvent` (subscribe / unsubscribe), `unsubscribeAll` |
| **Stats** | `memoryUsage` |
| **VideoOutput** | `resolution`, `hdcp`, `cecState`, `refreshRate`, `colorDepth`, `colorFormat`, `colorimetry`, `dynamicRange`, `quantizationRange`, `onResolutionChanged` (subscribe / unsubscribe), `onHdcpChanged` (subscribe / unsubscribe), `onCecStateChanged` (subscribe / unsubscribe), `onRefreshRateChanged` (subscribe / unsubscribe), `unsubscribeAll` |

### Disabled modules

| Module | Status | Reason |
|---|---|---|
| **Lifecycle** | Disabled | Disabled in `buildModuleList()` — the app itself is a lifecycle client and should not directly test lifecycle APIs |

---

## Adding a New Module

1. Create `src/tests/myModuleTest.h` and `.cpp` following the same pattern:
   - Inherit from `TestModuleBase`
   - Accept `fireboltVersion version` in the constructor if the module has version-specific methods; use `version != FIREBOLT_VERSION_8` to gate v9 method registration
   - Register method names in the constructor
   - Implement `runMethod(const std::string& method)` calling the firebolt interface
2. `#include` the new header in `src/main.cpp`
3. Add `std::make_unique<MyModuleTest>(version)` to the appropriate version block inside `buildModuleList()` in `main.cpp`
4. CMake automatically globs `src/tests/*.cpp` – no `CMakeLists.txt` edit needed

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
| **License** | [SIL Open Font License, Version 1.1](./assets/LICENSE) |
| **Source** | https://github.com/liberationfonts/liberation-fonts |
| **Bundled at** | `assets/LiberationSans-Bold.ttf` |
