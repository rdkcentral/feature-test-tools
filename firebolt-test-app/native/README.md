# Firebolt C++ Test Application

A native C++ firebolt test application that exercises the
[firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) APIs
and events/notifications across all supported Firebolt modules.

This app runs firebolt module tests automatically - executes the registered module methods and
subscriptions in sequence after startup.

It uses thunder APIs to alter the system configurations to test the Firebolt events and never restores to original.

**Explicitly do a device factory reset when the app exits.**

---

## Project layout

```text
native/
├── CMakeLists.txt
├── README.md
├── assets/
│   ├── LiberationSans-Bold.ttf
│   └── LICENSE
├── cmake/
│   └── FindWesterosSimpleShell.cmake
├── src/
│   ├── gl.cpp
│   ├── gl.h
│   ├── main.cpp
│   ├── native_logger.hpp
│   ├── utils.cpp
│   ├── utils.h
│   ├── gltest/
│   │   └── gl_standalone_test.cpp
│   └── tests/
│       ├── accessibilityTest.cpp/.h
│       ├── actionsTest.cpp/.h
│       ├── advertisingTest.cpp/.h
│       ├── deviceTest.cpp/.h
│       ├── discoveryTest.cpp/.h
│       ├── displayTest.cpp/.h
│       ├── lifecycleTest.cpp/.h
│       ├── localizationTest.cpp/.h
│       ├── metricsTest.cpp/.h
│       ├── networkTest.cpp/.h
│       ├── presentationTest.cpp/.h
│       ├── SpeechSynthesisTest.cpp/.h
│       ├── statsTest.cpp/.h
│       ├── texttospeechTest.cpp/.h
│       ├── VideoOutputTest.cpp/.h
│       └── ws_comm_tester.h
└── build/
```

---

## Features

- Connects to a Firebolt WebSocket endpoint using the installed Firebolt client libraries
- Enumerates and exercises registered test modules
- Supports Firebolt version-aware behavior for v8 and v9+ APIs
- Runs all registered methods and event subscriptions automatically by default
- Optionally exercises Thunder JSON-RPC connectivity using `THUNDER_ACCESS` dependent on `PROFILE` configured.
- Initializes a GL/Wayland window for lifecycle and visual feedback
- Uses a bundled Liberation Sans Bold font for display output

---

## Prerequisites

The native app depends on the following system and CMake packages:

| Requirement | Notes |
|---|---|
| **CMake ≥ 3.13** | |
| **C++17 compiler** | GCC 7+ or Clang 5+ |
| **FireboltClient** installed | Build from [firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) |
| **FireboltTransport** installed | Bundled from [firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-transport) |
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
    "com.rdkcentral.base": "0.4.0"
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
          "PROFILE":"STB",
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

This is a **lifecycle-driven Firebolt application** with automatic testing enabled by default. The app subscribes to lifecycle state changes and runs module tests automatically after startup and after an `INITIALIZING → PAUSED → ACTIVE` transition.

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
| `APPLOGLEVEL` | `Info` | App logger verbosity: `Debug`, `Info`, `Notice`, `Warning`, `Error`, `Fatal` |
| `GLLOGLEVEL` | `Info` | GL logger verbosity |
| `WIDTH` | `1920` | GL window width |
| `HEIGHT` | `1080` | GL window height |
| `PATTERN_MODE` | none | `GRID` or `DOT` background pattern |
| `THUNDER_ACCESS` | unset | Optional host:port for Thunder JSON-RPC tests |
| `PROFILE` | unset | Used to control Thunder JSONRPC API calls. Optional: `STB` (default) or `TV` |

Example:

```bash
export FIREBOLT_ENDPOINT="<valid firebolt session end-point>"
export WAYLAND_DISPLAY="<provided by window manager>"
export XDG_RUNTIME_DIR="<provided by window management framework>"
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

## Covered modules

The app tests all accessible Firebolt modules supported by the runtime version. The current implementation includes the following modules (methods marked *v9+* are available only in Firebolt 9 and later):

| Module | Methods / Events |
|---|---|
| `Accessibility` | `audioDescription`, `closedCaptionsSettings`, `highContrastUI`, `voiceGuidanceSettings`, `onAudioDescriptionChanged`, `onClosedCaptionsSettingsChanged`, `onHighContrastUIChanged`, `onVoiceGuidanceSettingsChanged`, `unsubscribeAll` |
| `Actions` *v9+* | `intent`, `start`, `onIntent` |
| `Advertising` | `advertisingId` |
| `Device` | `chipsetId`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged`, `unsubscribeAll`, `deviceClass` *(v9+)*, `dolbyAtmosExperienceAvailable` *(v9+)*, `onDolbyAtmosExperienceAvailableChanged` *(v9+)*, `osName` *(v9+)*, `setOsName` *(v9+)*, `osVersion` *(v9+)*, `setOsVersion` *(v9+)*, `firmware` *(v9+)* |
| `Discovery` | `watched`, `watchedV2` |
| `Display` | `size`, `maxResolution`, `edid` |
| `Localization` | `country`, `preferredAudioLanguages`, `presentationLanguage`, `onCountryChanged`, `onPreferredAudioLanguagesChanged`, `onPresentationLanguageChanged`, `unsubscribeAll`, `timeZone` *(v9+)*, `onTimeZoneChanged` *(v9+)* |
| `Metrics` | `ready`, `signIn`, `signOut`, `startContent`, `stopContent`, `page`, `error`, `mediaLoadStart`, `mediaPlay`, `mediaPlaying`, `mediaPause`, `mediaWaiting`, `mediaSeeking`, `mediaSeeked`, `mediaRateChanged`, `mediaRenditionChanged`, `mediaEnded`, `event`, `appInfo` |
| `Network` | `connected`, `onConnectedChanged` |
| `Presentation` | `focused`, `onFocusedChanged` |
| `SpeechSynthesis` *v9+* | `voices`, `speak`, `cancel`, `pause`, `resume`, `onVoicesChanged`, `onUtteranceEvent`, `unsubscribeAll` |
| `Stats` *v9+* | `memoryUsage` |
| `TextToSpeech` | `speak`, `getSpeechState`, `listVoices`, `pause`, `resume`, `cancel`, `onSpeechStart`, `onSpeechPause`, `onSpeechResume`, `onWillSpeak`, `onSpeechComplete`, `onSpeechInterrupted`, `onNetworkError`, `onPlaybackError`, `unsubscribeAll` |
| `VideoOutput` *v9+* | `resolution`, `hdcp`, `cecState`, `refreshRate`, `colorDepth`, `colorFormat`, `colorimetry`, `dynamicRange`, `quantizationRange`, `onResolutionChanged`, `onHdcpChanged`, `onCecStateChanged`, `onRefreshRateChanged`, `unsubscribeAll` |

### Disabled by design

| Module | Status | Reason |
|---|---|---|
| `Lifecycle` | Disabled | The app itself acts as a lifecycle client and should not directly test lifecycle APIs |

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
