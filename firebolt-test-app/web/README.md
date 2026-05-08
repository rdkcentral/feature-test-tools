# Firebolt® API Test Tool

A Lightning.js-based web application for testing RDK Firebolt® API functionality on RDK devices and in local development.

---

## Project Structure

```
web/
├── src/
│   ├── App.js                        # Main Lightning component (header, clock, layout)
│   ├── index.js                      # Application entry point
│   ├── components/
│   │   ├── Menu.js                   # Category navigation menu
│   │   ├── TestRunner.js             # Test execution and progress display
│   │   ├── ResultsPanel.js           # Pass/fail results panel
│   │   └── DimensionRuler.js         # Optional debug overlay
│   ├── lib/
│   │   ├── FireboltAPI.js            # API wrapper, connection, test runner
│   │   ├── DeviceConnectionConfig.js # Endpoint resolution (IP, port)
│   │   └── AppSettings.js            # Layout, colors, typography from settings
│   └── data/
│       ├── categories.json           # Menu category definitions
│       └── tests/                    # Per-module test definitions (JSON)
│           ├── account.json
│           ├── accessibility.json
│           ├── advertising.json
│           ├── device.json
│           ├── discovery.json
│           ├── display.json
│           ├── lifecycle.json
│           ├── localization.json
│           ├── metrics.json
│           ├── mockstress.json
│           ├── network.json
│           ├── presentation.json
│           ├── stats.json
│           └── texttospeech.json
├── static/                           # Static assets (fonts, icons)
├── build/                            # Build output (generated)
├── test/                             # Jest unit tests
├── index.html                        # HTML entry point
├── package.json
├── settings.json                     # Lightning stage and platform settings
├── app-settings.json                 # App layout, colors, typography overrides
└── metadata.json                     # App metadata (name, version, icon)
```

---

## Prerequisites

- Node.js v18 or higher
- npm

---

## Installation

```bash
npm install
```

---

## Running

### Development (hot reload)

```bash
npm start
```

Available at `http://localhost:9090`

### Production build

```bash
npm run build
```

Output goes to `build/`. Deploy the contents of `build/` to the device at `/usr/share/fbttest/`.

### Serve production build locally

```bash
npm run serve
```

Serves `build/` at `http://localhost:9090`

### Run unit tests

```bash
npm test
```

---

## Deploying to a Device

Copy the build output to the device:

```bash
scp -r build/* root@<device-ip>:/usr/share/fbttest/
```

The app is loaded by the RDK BrowserLauncher. On launch the BrowserLauncher injects the Firebolt endpoint as a user script at document start:

```js
window.__firebolt = { endpoint: 'ws://127.0.0.1:3473/?session=<token>' };
```

The app reads this automatically — no manual configuration is needed on device.

---

## Connection & Endpoint Resolution

`FireboltAPI.js` resolves the WebSocket endpoint once at startup in this priority order:

| Priority | Source | Use case |
|---|---|---|
| 1 | `window.__firebolt.endpoint` | Real RDK device — injected by BrowserLauncher |
| 2 | `?__firebolt_endpoint=ws://...` URL param | Legacy / dev fallback |
| 3 | `DeviceConnectionConfig` default | Local dev (`ws://127.0.0.1:3473`) |

To connect to a remote device during local development, append `?deviceIP=<ip>` to the URL:

```
http://localhost:9090?deviceIP=192.168.1.100
```

Or use the legacy endpoint override:

```
http://localhost:9090?__firebolt_endpoint=ws://192.168.1.100:3473
```

---

## API Backend Architecture

The app uses two connection paths:

### 1. `@firebolt-js/core-client` SDK (primary)

Used for: `Accessibility`, `Advertising`, `Device`, `Discovery`, `Localization`, `Metrics`, `Network`

The SDK reads `window.__FIREBOLT_CONFIG__.endpoint` (set from the resolved endpoint above) and manages its own persistent WebSocket internally. Method calls go through `module.method()` and events through `module.listen('eventName', callback)`.

### 2. Raw WebSocket JSON-RPC (gateway)

Used for modules not yet in the SDK package: `Display`, `Lifecycle`, `Presentation`, `Stats`, `TextToSpeech`

Three call patterns, all using the resolved `connectionInfo.endpoint`:

| Method | Purpose | Socket lifetime |
|---|---|---|
| `_callViaGateway(method, params)` | Single request/response | Open → send → close (5s timeout) |
| `_subscribeViaGateway(method)` | One-shot event wait (per test run) | Open → wait for event → close |
| `_subscribeGatewayEventPersistent(method)` | Persistent event monitor (startup) | Open, stays open for app lifetime |

---

## Test Categories & Types

Each test is defined in `src/data/tests/<module>.json`. Every test has a `type` field:

| Type | Description | Backend |
|---|---|---|
| `getter` | Calls a Firebolt getter method | SDK or gateway |
| `gateway` | Raw JSON-RPC call (non-SDK module) | Raw WebSocket |
| `event` | Subscribes via SDK `listen()`, waits 7.5s for event | SDK |
| `gateway-event` | Subscribes via raw WebSocket, waits 7.5s for event | Raw WebSocket |

### Covered modules (14 total)

| Module | Backend | Notes |
|---|---|---|
| Account | Mock only | Not in OpenRPC spec |
| Accessibility | SDK | |
| Advertising | SDK | |
| Device | SDK | |
| Discovery | SDK | |
| Display | Gateway | Not in SDK package |
| Lifecycle | Gateway | Not in SDK package |
| Localization | SDK | |
| Metrics | SDK | |
| MockStress | Mock | 35 synthetic methods for stress testing |
| Network | SDK | |
| Presentation | Gateway | Not in SDK package |
| Stats | Gateway | Not in SDK package |
| TextToSpeech | Gateway | Not in SDK package |

---

## Adding New Tests

Add an entry to the relevant `src/data/tests/<module>.json` file:

```json
{
  "id": "module_methodName",
  "name": "Module.methodName()",
  "description": "What this test verifies",
  "method": "Module.methodName",
  "type": "getter",
  "params": [],
  "expectedType": "string"
}
```

For a raw gateway call:

```json
{
  "id": "module_methodName",
  "name": "Module.methodName [gateway]",
  "description": "What this test verifies",
  "method": "Module.methodName",
  "type": "gateway",
  "gatewayParams": {}
}
```

For an event:

```json
{
  "id": "module_onSomethingChanged",
  "name": "Module.onSomethingChanged [event]",
  "description": "Fires when something changes",
  "method": "Module.onSomethingChanged",
  "type": "event",
  "listenEvent": "onSomethingChanged"
}
```

No code changes are required — the test definitions are loaded automatically at runtime.

---

## References

- [RDK Firebolt® API Specification](https://wiki.rdkcentral.com/pages/viewpage.action?pageId=433961686)
- [Firebolt JS Core Client](https://www.npmjs.com/package/@firebolt-js/core-client)
- [Lightning.js SDK Documentation](https://lightningjs.io/)
- [entservices-runtime BrowserLauncher](https://github.com/rdkcentral/entservices-runtime/tree/develop/BrowserLauncher)
