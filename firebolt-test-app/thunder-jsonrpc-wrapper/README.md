# thunder-jsonrpc-wrapper

A small consumer-facing wrapper around Thunder JSON-RPC for request/response calls and event callbacks.

The goal is to keep client usage simple:
- initialize once
- send methods with optional params/timeout
- optionally send-and-ignore response
- register/unregister event callbacks

## Public API

Header: [include/thunder/JsonRpcBridge.h](include/thunder/JsonRpcBridge.h)

```cpp
namespace Thunder {
class JsonRpcBridge {
public:
    using EventCallback = std::function<void(const std::string& eventName, const nlohmann::json& params)>;

    bool initialize(std::optional<std::string> thunderAccess = std::nullopt);

    nlohmann::json send(const std::string& method,
                        const nlohmann::json& params = nlohmann::json::object(),
                        std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    void sendWithoutResponse(const std::string& method,
                             const nlohmann::json& params = nlohmann::json::object(),
                             std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    void registerEvent(const std::string& eventName, EventCallback callback);
    void unregisterEvent(const std::string& eventName);
};
}
```

## Endpoint Resolution

`initialize()` resolves endpoint in this order:
1. `thunderAccess` argument (if provided and non-empty)
2. `THUNDER_URL` environment variable
3. `THUNDER_ACCESS` environment variable
4. default: `ws://127.0.0.1:9998/jsonrpc`

## Basic Usage

```cpp
#include <thunder/JsonRpcBridge.h>
#include <chrono>
#include <iostream>

Thunder::JsonRpcBridge bridge;

if (!bridge.initialize()) {
    std::cerr << "Bridge initialization failed" << std::endl;
    return;
}

const nlohmann::json result = bridge.send(
    "DeviceInfo.firmwareversion",
    nlohmann::json::object(),
    std::chrono::milliseconds(3000));

if (result.is_object() && result.contains("error")) {
    std::cerr << "RPC failed: "
              << result["error"].value("message", "unknown error")
              << std::endl;
} else {
    std::cout << "Firmware response: " << result.dump() << std::endl;
}
```

## Fire-and-Forget Style

`sendWithoutResponse()` still waits internally for an RPC completion/timeout, but it intentionally discards the returned payload:

```cpp
bridge.sendWithoutResponse("Metrics.ready");
```

Use this when caller does not need the returned JSON.

## Events

```cpp
bridge.registerEvent("Some.Event.Name", [](const std::string& eventName, const nlohmann::json& params) {
    std::cout << "Event: " << eventName << " payload=" << params.dump() << std::endl;
});

// Later...
bridge.unregisterEvent("Some.Event.Name");
```

## Return Contract for send()

- Success: returns JSON-RPC `result` value directly.
- Failure: returns

```json
{
  "error": {
    "code": -32099,
    "message": "..."
  }
}
```

Additional parse/shape errors may use code `-32700`.

## Initialization Notes

- Initialize once per bridge instance.
- Repeated `initialize()` on an already initialized instance returns `false` and logs a warning.
- Current implementation owns request correlation and dispatch-thread management internally.

## Build

CMake target exposed by this module:
- `ThunderJsonRpcWrapper::ThunderJsonRpcWrapper`

If used by sibling `native` project via `add_subdirectory`, linking is already wired in that project CMake.
