# Firebolt C++ Test Application

A native C++ test application that exercises the
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

DEPENDS = "firebolt-cpp-client nlohmann-json cairo virtual/egl virtual/libgles2 freetype westeros-simpleshell"
RDEPENDS:${PN} += "firebolt-cpp-client firebolt-cpp-transport cairo westeros-simpleshell"

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

### GL display window (optional)

When `XDG_RUNTIME_DIR` is set, a Wayland/EGL overlay window launches in a background thread after
connecting to Firebolt. It renders the last received key code using the bundled Liberation Sans Bold font.
The following environment variables control it:

| Variable | Default | Description |
|---|---|---|
| `WAYLAND_DISPLAY` | `wayland-0` | Wayland socket name |
| `WIDTH` | `1280` | Window width in pixels |
| `HEIGHT` | `720` | Window height in pixels |
| `PATTERN_MODE` | *(none)* | Background pattern: `GRID` or `DOT` |

---

## Version-Aware Modules

Some modules expose additional methods depending on the selected Firebolt version at runtime:

| Module | Firebolt 8 methods | Additional Firebolt 9 methods |
|---|---|---|
| **Device** | `chipsetId`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged` (subscribe / unsubscribe), `unsubscribeAll` | `deviceClass` |
| **Localization** | `country`, `preferredAudioLanguages`, `presentationLanguage`, `onCountryChanged` (subscribe / unsubscribe), `onPreferredAudioLanguagesChanged` (subscribe / unsubscribe), `onPresentationLanguageChanged` (subscribe / unsubscribe), `unsubscribeAll` | `timezone`, `onTimezoneChanged` (subscribe / unsubscribe) |

---

## Run Modes

### 1. Interactive (default when stdin is a TTY)
Shows a two-level menu:
1. Select a **module** (Accessibility, Device, Lifecycle, …)
2. Select a **method** within that module
3. Enter `q` or press Enter to go back / quit

### 2. Auto mode (`--auto`)
Runs every method of every module sequentially. The set of modules and methods exercised is determined by the active version flag — only the methods registered for the selected version are run. Use this for CI / smoke testing.
```bash
# Run all Firebolt 8 + 9 modules (default)
firebolt-test-app --auto

# Run Firebolt 8 base modules only
firebolt-test-app --auto --firebolt8

# Run all modules including any future additions
firebolt-test-app --auto --firebolt-all
```

### 3. Piped stdin mode
Reads one `Module.method` name per line from stdin. Only methods registered for the active version are recognized — requests for methods outside the active version will print `Method not found:` and be skipped.
```bash
# Firebolt 8 methods (default --firebolt9 mode includes these)
printf "Device.uid\nNetwork.connected\nLifecycle.state\n" | firebolt-test-app --url ws://127.0.0.1:9998

# Firebolt 9 methods — require --firebolt9 or --firebolt-all
printf "Actions.intent\nVideoOutput.resolution\nSpeechSynthesis.voices\n" | firebolt-test-app --url ws://127.0.0.1:9998 --firebolt9

# Version-specific method on a shared module — only available in --firebolt9 or --firebolt-all
printf "Device.deviceClass\nLocalization.timezone\n" | firebolt-test-app --url ws://127.0.0.1:9998 --firebolt9
```

---

## Covered Modules & APIs

### Firebolt 8 modules (available in all modes and restricted to `--firebolt8`)

| Module | Methods / Events |
|---|---|
| **Accessibility** | `audioDescription`, `closedCaptionsSettings`, `highContrastUI`, `voiceGuidanceSettings`, `onAudioDescriptionChanged` (subscribe / unsubscribe), `onClosedCaptionsSettingsChanged` (subscribe / unsubscribe), `onHighContrastUIChanged` (subscribe / unsubscribe), `onVoiceGuidanceSettingsChanged` (subscribe / unsubscribe), `unsubscribeAll` |
| **Advertising** | `advertisingId` |
| **Device** | `chipsetId`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged` (subscribe / unsubscribe), `unsubscribeAll` *(+ v9 additions — see above)* |
| **Discovery** | `watched`, `watchedV2` |
| **Display** | `size`, `maxResolution`, `edid` |
| **Lifecycle** | `state`, `close`, `onStateChanged` (subscribe / unsubscribe / unsubscribeAll) |
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
