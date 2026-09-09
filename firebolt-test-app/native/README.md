# Firebolt C++ Test Application

A native C++ firebolt test application that exercises the
[firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) APIs
and events/notifications across all supported Firebolt modules.

---

## Project Layout

```
native/
├── CMakeLists.txt          # Top-level CMake project
├── assets/
│   ├── LiberationSans-Bold.ttf # Embedded font for the GL display window (OFL 1.1)
│   └── OFL.txt                 # License text installed from the Liberation font package (OFL 1.1)
└── src/
    ├── main.cpp            # Entry point, connection management, run-mode dispatch
    ├── utils.h / utils.cpp # Shared helpers: AppConfig, fireboltVersion, chooseFromList, TestModuleBase
    ├── gl.h                # GlApp class declaration (Wayland/EGL/GLES keycode display window)
    ├── gl.cpp              # GlApp implementation
    └── tests/
        ├── accessibilityTest.h/.cpp
        ├── actionsTest.h/.cpp
        ├── advertisingTest.h/.cpp
        ├── deviceTest.h/.cpp
        ├── discoveryTest.h/.cpp
        ├── displayTest.h/.cpp
        ├── lifecycleTest.h/.cpp
        ├── localizationTest.h/.cpp
        ├── metricsTest.h/.cpp
        ├── networkTest.h/.cpp
        ├── presentationTest.h/.cpp
        ├── SpeechSynthesisTest.h/.cpp
        ├── statsTest.h/.cpp
        ├── texttospeechTest.h/.cpp
        └── VideoOutputTest.h/.cpp
```

---

## Prerequisites

| Requirement | Notes |
|---|---|
| **CMake ≥ 3.12** | |
| **C++17 compiler** | GCC 7+ or Clang 5+ |
| **FireboltClient** installed | Build from [firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) |
| **FireboltTransport** installed | Bundled in the firebolt-cpp-client build |
| **nlohmann-json** installed | Used for JSON input/response validation in tests |
| **wayland-client / wayland-egl** | Required for the GL display window (`gl.cpp`) |
| **EGL / GLESv2** | Required for the GL display window (`gl.cpp`) |
| **Cairo / cairo-ft / FreeType** | Required for the GL display window (`gl.cpp`) |

The `FireboltClient`, `FireboltTransport`, and `nlohmann_json` CMake packages must be findable via
`CMAKE_PREFIX_PATH` (or `SYSROOT_PATH` for cross-compilation).

---

## Building

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

<details>
  <summary>Sample bitbake recipe</summary>

```bash
SUMMARY = "Firebolt C++ Test Application"
DESCRIPTION = "Native C++ test application for exercising firebolt-cpp-client APIs and events"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../../LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

inherit cmake pkgconfig

SRC_URI = "${CMF_GITHUB_ROOT}/feature-test-tools;${CMF_GITHUB_SRC_URI_SUFFIX}"
SRCREV = "${AUTOREV}"  <=== Replace with SHA
PV = "2.0.0"
PR = "r0"

S = "${WORKDIR}/git/firebolt-test-app/native"

DEPENDS = "firebolt-cpp-client nlohmann-json cairo virtual/egl virtual/libgles2 freetype westeros-simpleshell libxkbcommon"
RDEPENDS:${PN} += "firebolt-cpp-client firebolt-cpp-transport cairo westeros-simpleshell libxkbcommon xkeyboard-config"

EXTRA_OECMAKE:append = " \
    -DBUILD_FIREBOLT_APP=ON \
    -DBUILD_GL_TEST=ON \
    -DGL_MODULE_SHARED=OFF \
    "

FILES:${PN} += " /usr/share/fonts"
```

</details>

### Font License Note

`assets/OFL.txt` is the license text installed from the Liberation font package for
`LiberationSans-Bold.ttf`.

---

## Running

This is a lifecycle-driven Firebolt application; ensure `WAYLAND_DISPLAY`, `XDG_RUNTIME_DIR`, and `FIREBOLT_ENDPOINT` are configured. `MODE_AUTO_RUN` is optional and enables auto mode without `--auto`.

Note: Command-line options are supported for development/testing; interactive and piped modes are deprecated in the lifecycle-driven app flow.

### Binary name
```
firebolt-test-app
```

### Command-line options
```
firebolt-test-app [--auto] [--url <URL>]
                  [--legacy | --rpc-v2] [--dbg]
                  [--firebolt8 | --firebolt9 | --firebolt-all] [--help]
```

| Option | Description |
|---|---|
| `--auto` | Run all methods for all modules without user input |
| `--url <URL>` | Use a custom WebSocket endpoint |
| `--legacy` | Force legacy (v1) RPC protocol |
| `--rpc-v2` | Force JSON-RPC v2 compliant protocol |
| `--dbg` | Enable debug logging |
| `--firebolt8` | Firebolt 8 modules only — excludes all Firebolt 9 modules and v9-specific methods within shared modules |
| `--firebolt9` | Firebolt 8 base modules + Firebolt 9 modules (default) — includes all base APIs plus Actions, SpeechSynthesis, Stats, VideoOutput, and v9-specific methods within shared modules |
| `--firebolt-all` | All modules across all Firebolt versions |
| `--help` | Print usage and exit |

Endpoint priority: `--url` > `FIREBOLT_ENDPOINT` env var

Additional runtime env vars:

| Variable | Description |
|---|---|
| `MODE_AUTO_RUN` | If set to a non-empty value other than `0`/`false`, enables auto mode even without `--auto` |
| `WAYLAND_DISPLAY` | Wayland socket name used by the GL app (default `wayland-0`) |
| `WIDTH` | GL window width (default `1920`) |
| `HEIGHT` | GL window height (default `1080`) |
| `PATTERN_MODE` | GL background pattern (`GRID` or `DOT`) |

### GL display window

During lifecycle-driven startup, the app initializes a Wayland/EGL overlay window and renders the last
received key code using the bundled Liberation Sans Bold font. `XDG_RUNTIME_DIR` must be set for GL init.
The following environment variables control it:

| Variable | Default | Description |
|---|---|---|
| `WAYLAND_DISPLAY` | `wayland-0` | Wayland socket name |
| `WIDTH` | `1920` | Window width in pixels |
| `HEIGHT` | `1080` | Window height in pixels |
| `PATTERN_MODE` | *(none)* | Background pattern: `GRID` or `DOT` |

---

## Version-Aware Modules

Some modules expose additional methods depending on the selected Firebolt version at runtime:

| Module | Firebolt 8 methods | Additional Firebolt 9 methods |
|---|---|---|
| **Device** | `chipsetId`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged` (subscribe / unsubscribe), `unsubscribeAll` | `deviceClass`, `dolbyAtmosExperienceAvailable`, `onDolbyAtmosExperienceAvailableChanged` (subscribe / unsubscribe) |
| **Localization** | `country`, `preferredAudioLanguages`, `presentationLanguage`, `onCountryChanged` (subscribe / unsubscribe), `onPreferredAudioLanguagesChanged` (subscribe / unsubscribe), `onPresentationLanguageChanged` (subscribe / unsubscribe), `unsubscribeAll` | `timezone`, `onTimezoneChanged` (subscribe / unsubscribe) |

---

## Run Modes

### 1. Lifecycle-driven app flow
The app subscribes to lifecycle state changes and runs module tests only after an `INITIALIZING -> PAUSED -> ACTIVE` transition.

### 2. Auto mode (`--auto` or `MODE_AUTO_RUN`)
Runs every registered method sequentially once active. In auto mode, `.unsubscribe` and `.unsubscribeAll` methods are deferred and executed during shutdown cleanup.
```bash
# Run all Firebolt 8 + 9 modules (default)
firebolt-test-app --auto

# Run Firebolt 8 base modules only
firebolt-test-app --auto --firebolt8

# Same module set as --firebolt9 in current implementation
firebolt-test-app --auto --firebolt-all
```

`runInteractiveMode()` and `runPipedMode()` helper functions are kept but not maintained due to requirement changes to be upgraded as a true Firebolt bolt app running inside container.

---

## Covered Modules & APIs

### Base modules (always included in `--firebolt9`/`--firebolt-all`, and also available in `--firebolt8`)

| Module | Methods / Events |
|---|---|
| **Accessibility** | `audioDescription`, `closedCaptionsSettings`, `highContrastUI`, `voiceGuidanceSettings`, `onAudioDescriptionChanged` (subscribe / unsubscribe), `onClosedCaptionsSettingsChanged` (subscribe / unsubscribe), `onHighContrastUIChanged` (subscribe / unsubscribe), `onVoiceGuidanceSettingsChanged` (subscribe / unsubscribe), `unsubscribeAll` |
| **Advertising** | `advertisingId` |
| **Device** | `chipsetId`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged` (subscribe / unsubscribe), `unsubscribeAll`, `deviceClass`, `dolbyAtmosExperienceAvailable`, `onDolbyAtmosExperienceAvailableChanged` (subscribe / unsubscribe; v9+) |
| **Discovery** | `watched`, `watchedV2` |
| **Display** | `size`, `maxResolution`, `edid` |
| **Localization** | `country`, `preferredAudioLanguages`, `presentationLanguage`, `onCountryChanged` (subscribe / unsubscribe), `onPreferredAudioLanguagesChanged` (subscribe / unsubscribe), `onPresentationLanguageChanged` (subscribe / unsubscribe), `unsubscribeAll` *(+ v9 additions — see above)* |
| **Metrics** | `ready`, `signIn`, `signOut`, `startContent`, `stopContent`, `page`, `error`, `mediaLoadStart`, `mediaPlay`, `mediaPlaying`, `mediaPause`, `mediaWaiting`, `mediaSeeking`, `mediaSeeked`, `mediaRateChanged`, `mediaRenditionChanged`, `mediaEnded`, `event` *(validates schema + JSON data input)*, `appInfo` |
| **Network** | `connected`, `onConnectedChanged` (subscribe / unsubscribe / unsubscribeAll) |
| **Presentation** | `focused`, `onFocusedChanged` (subscribe / unsubscribe / unsubscribeAll) |
| **TextToSpeech** | `speak`, `getSpeechState`, `listVoices`, `pause`, `resume`, `cancel`, `onSpeechStart` (subscribe / unsubscribe), `onSpeechPause` (subscribe / unsubscribe), `onSpeechResume` (subscribe / unsubscribe), `onWillSpeak` (subscribe / unsubscribe), `onSpeechComplete` (subscribe / unsubscribe), `onSpeechInterrupted` (subscribe / unsubscribe), `onNetworkError` (subscribe / unsubscribe), `onPlaybackError` (subscribe / unsubscribe), `unsubscribeAll` |

### Firebolt 9 modules (`--firebolt9` or `--firebolt-all`)

| Module | Methods / Events |
|---|---|
| **Actions** | `intent` *(validates response schema)*, `start` *(validates JSON input)*, `onIntent` (subscribe / unsubscribe / unsubscribeAll). Intent payloads follow the `{ action, context.source?, intentId }` model. |
| **SpeechSynthesis** | `voices`, `speak`, `cancel`, `pause`, `resume`, `onVoicesChanged` (subscribe / unsubscribe), `onUtteranceEvent` (subscribe / unsubscribe), `unsubscribeAll` |
| **Stats** | `memoryUsage` |
| **VideoOutput** | `resolution`, `hdcp`, `cecState`, `refreshRate`, `colorDepth`, `colorFormat`, `colorimetry`, `dynamicRange`, `quantizationRange`, `onResolutionChanged` (subscribe / unsubscribe), `onHdcpChanged` (subscribe / unsubscribe), `onCecStateChanged` (subscribe / unsubscribe), `onRefreshRateChanged` (subscribe / unsubscribe), `unsubscribeAll` |

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
| **License** | [SIL Open Font License, Version 1.1](./assets/OFL.txt) |
| **Source** | https://github.com/liberationfonts/liberation-fonts |
| **Bundled at** | `assets/LiberationSans-Bold.ttf` |
