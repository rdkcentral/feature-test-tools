# Firebolt C++ Test Application

A native C++ test application that exercises the
[firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) APIs
and events/notifications across all supported Firebolt modules.

---

## Project Layout

```
native/
├── CMakeLists.txt          # Top-level CMake project
└── src/
    ├── main.cpp            # Entry point, connection management, run-mode dispatch
    ├── utils.h / utils.cpp # Shared helpers: AppConfig, chooseFromList, TestModuleBase
    └── tests/
        ├── accessibilityTest.h/.cpp
        ├── advertisingTest.h/.cpp
        ├── deviceTest.h/.cpp
        ├── discoveryTest.h/.cpp
        ├── displayTest.h/.cpp
        ├── lifecycleTest.h/.cpp    ← includes event subscription/unsubscription
        ├── localizationTest.h/.cpp
        ├── metricsTest.h/.cpp
        ├── networkTest.h/.cpp
        ├── presentationTest.h/.cpp ← includes onFocusedChanged event
        ├── statsTest.h/.cpp
        └── texttospeechTest.h/.cpp ← includes TTS events (start/pause/resume/willSpeak)
```

---

## Prerequisites

| Requirement | Notes |
|---|---|
| **CMake ≥ 3.12** | |
| **C++17 compiler** | GCC 7+ or Clang 5+ |
| **FireboltClient** installed | Build from [firebolt-cpp-client](https://github.com/rdkcentral/firebolt-cpp-client) |
| **FireboltTransport** installed | Bundled in the firebolt-cpp-client build |

The `FireboltClient` and `FireboltTransport` CMake packages must be findable via
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
PV = "1.0.0"
PR = "r0"

S = "${WORKDIR}/git/firebolt-test-app/native"

DEPENDS = "firebolt-cpp-client"
RDEPENDS:${PN} += "firebolt-cpp-client"

EXTRA_OECMAKE = ""

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/firebolt-test-app ${D}${bindir}/firebolt-test-app
}

FILES:${PN} += "${bindir}/firebolt-test-app"
```

</details>

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
                  [--firebolt8 | --firebolt9] [--help]
```

| Option | Description |
|---|---|
| `--auto` | Run all methods for all modules without user input |
| `--url <URL>` | Use a custom WebSocket endpoint |
| `--legacy` | Force legacy (v1) RPC protocol |
| `--rpc-v2` | Force JSON-RPC v2 compliant protocol |
| `--dbg` | Enable debug logging |
| `--firebolt8` | Restrict to Firebolt 8 APIs only — excludes Firebolt 9 modules (currently: Actions). This is the default mode. |
| `--firebolt9` | Enable Firebolt 9 APIs and include Firebolt 9 modules (currently: Actions). Requires building with `-DENABLE_FIREBOLT9=ON`; otherwise this option is unavailable. |
| `--help` | Print usage and exit |

Endpoint priority: `--url` > `FIREBOLT_ENDPOINT` env var

---

## Run Modes

### 1. Interactive (default when stdin is a TTY)
Shows a two-level menu:
1. Select a **module** (Accessibility, Device, Lifecycle, …)
2. Select a **method** within that module
3. Enter `q` or press Enter to go back / quit

### 2. Auto mode (`--auto`)
Runs every method of every module sequentially. Use this for CI / smoke testing.
```bash
firebolt-test-app --auto
```

### 3. Piped stdin mode
Reads one `Module.method` name per line from stdin. Unknown names print a warning.
```bash
printf "Device.uid\nNetwork.connected\nLifecycle.state\n" | firebolt-test-app --url ws://127.0.0.1:9998
```

---

## Covered Modules & APIs

| Module | Methods / Events |
|---|---|
| **Accessibility** | `audioDescription`, `closedCaptionsSettings`, `highContrastUI`, `voiceGuidanceSettings`, `onAudioDescriptionChanged` (subscribe / unsubscribe), `onClosedCaptionsSettingsChanged` (subscribe / unsubscribe), `onHighContrastUIChanged` (subscribe / unsubscribe), `onVoiceGuidanceSettingsChanged` (subscribe / unsubscribe), `unsubscribeAll` |
| **Actions** *(Firebolt 9)* | `intent`, `onIntent` (subscribe / unsubscribe / unsubscribeAll) |
| **Advertising** | `advertisingId` |
| **Device** | `chipsetId`, `deviceClass`, `hdr`, `timeInActiveState`, `uid`, `uptime`, `onHdrChanged` (subscribe / unsubscribe), `unsubscribeAll` |
| **Discovery** | `watched` |
| **Display** | `edid`, `maxResolution`, `size` |
| **Lifecycle** | `state`, `close`, `onStateChanged` (subscribe / unsubscribe / unsubscribeAll) |
| **Localization** | `country`, `preferredAudioLanguages`, `presentationLanguage`, `onCountryChanged` (subscribe / unsubscribe), `onPreferredAudioLanguagesChanged` (subscribe / unsubscribe), `onPresentationLanguageChanged` (subscribe / unsubscribe), `unsubscribeAll` |
| **Metrics** | `ready`, `signIn`, `signOut`, `startContent`, `stopContent`, `page`, `error`, `mediaLoadStart`, `mediaPlay`, `mediaPlaying`, `mediaPause`, `mediaWaiting`, `mediaSeeking`, `mediaSeeked`, `mediaRateChanged`, `mediaRenditionChanged`, `mediaEnded`, `event`, `appInfo` |
| **Network** | `connected`, `onConnectedChanged` (subscribe / unsubscribe / unsubscribeAll) |
| **Presentation** | `focused`, `onFocusedChanged` (subscribe / unsubscribe / unsubscribeAll) |
| **Stats** | `memoryUsage` |
| **TextToSpeech** | `speak`, `getSpeechState`, `listVoices`, `pause`, `resume`, `cancel`, `onSpeechStart` (subscribe / unsubscribe), `onSpeechPause` (subscribe / unsubscribe), `onSpeechResume` (subscribe / unsubscribe), `onWillSpeak` (subscribe / unsubscribe), `onSpeechComplete` (subscribe / unsubscribe), `onSpeechInterrupted` (subscribe / unsubscribe), `onNetworkError` (subscribe / unsubscribe), `onPlaybackError` (subscribe / unsubscribe), `unsubscribeAll` |

---

## Adding a New Module

1. Create `src/tests/myModuleTest.h` and `.cpp` following the same pattern:
   - Inherit from `TestModuleBase`
   - Register method names in the constructor
   - Implement `runMethod(const std::string& method)` calling the firebolt interface
2. `#include` the new header in `src/main.cpp`
3. Add `std::make_unique<MyModuleTest>()` to `buildModuleList()` in `main.cpp`
4. CMake automatically globs `src/tests/*.cpp` – no `CMakeLists.txt` edit needed

---

## License

Apache-2.0 – see [LICENSE](./../../LICENSE)
