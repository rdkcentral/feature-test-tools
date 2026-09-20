# Event Handler Validation Failure Reporting

## Usage in Event Handlers

Event handlers can now report validation failures when they detect mismatches without incrementing progress (since the subscription itself already reported progress).

### Example: Accessibility Event Handler with Mismatch Detection

```cpp
auto r = IFireboltAccessor::Instance()
    .AccessibilityInterface()
    .subscribeOnAudioDescriptionChanged([this](bool enabled) {
        std::cout << "  [EVENT] onAudioDescriptionChanged: enabled="
                  << std::boolalpha << enabled << std::endl;

        // Verify event value matches query response
        auto r2 = IFireboltAccessor::Instance()
                    .AccessibilityInterface()
                    .audioDescription();

        if (checkResult(r2, "Query Accessibility.audioDescription"))
        {
            std::cout << "  Query Response audioDescription enabled: "
                      << std::boolalpha << *r2 << std::endl;

            // Detect mismatch
            if (enabled != *r2)
            {
                std::string mismatchDetails = std::string("event enabled=")
                    + (enabled ? "true" : "false") + " vs query="
                    + (*r2 ? "true" : "false");
                reportEventValidationFailure("onAudioDescriptionChanged", mismatchDetails);
            }
        }
     });

if (checkResult(r, method)) {
    onAudioDescriptionChangedSubId_ = *r;
    std::cout << "  Subscribed. Subscription ID: " << onAudioDescriptionChangedSubId_ << std::endl;
}
```

## API Reference

### TestModuleBase Methods

#### `reportStepCompletion(bool failed = false)`
- Used for **explicit progress reporting** on non-Result methods
- **Increments progress counter**
- Example: `reportStepCompletion(false);` for `unsubscribeAll()`

#### `reportEventValidationFailure(const std::string& eventName, const std::string& details)`
- Used for **validation failures in event handlers**
- **Does NOT increment progress** (subscription already reported)
- **Sets failure flag** to indicate test issues
- Constructs full message as: `Module.eventName: details`

### Example Details Format

```cpp
// Good detail descriptions:
reportEventValidationFailure("onAudioDescriptionChanged",
    "event enabled=true vs query=false");

reportEventValidationFailure("onClosedCaptionsSettingsChanged",
    "enabled mismatch: event has 5 languages, query has 3");

reportEventValidationFailure("onHdrChanged",
    "state mismatch: event=DOLBY_VISION, query=HDR10");
```

## Data Flow

```
Event Handler Execution (Async)
    ↓
Detect Mismatch
    ↓
Call reportEventValidationFailure(eventName, details)
    ↓
GetTestProgressTracker()->reportValidationFailure(...)
    ↓
ProgressController::reportValidationFailure()
    ↓
Store in validationFailures_ vector
    ↓
Set failDetected = true (no progress increment)
```

## Retrieving Validation Failures

After tests complete, retrieve all validation failures:

```cpp
ProgressController& pc = /* get controller reference */;
auto failures = pc.getValidationFailures();

for (const auto& failure : failures) {
    std::cerr << "[VALIDATION ERROR] " << failure << std::endl;
}
```

## Key Differences

| Method | Progress Impact | Use Case |
|--------|---------|----------|
| `checkResult()` | ✓ Reports success/failure | Main API call results |
| `reportStepCompletion()` | ✓ Increments counter | Methods without Result (unsubscribeAll) |
| `reportEventValidationFailure()` | ✗ No progress increment | Event handler mismatches |

## Thread Safety

All methods are thread-safe:
- Event handlers run on callback threads
- Progress tracker uses mutex protection
- Safe to call from any thread

## Practical Example: Device Test

```cpp
// In deviceTest.cpp, onHdrChanged subscription:
auto r = IFireboltAccessor::Instance()
    .DeviceInterface()
    .subscribeOnHdrChanged([this](bool hdrEnabled) {
        std::cout << "  [EVENT] onHdrChanged: enabled="
                  << std::boolalpha << hdrEnabled << std::endl;

        auto r2 = IFireboltAccessor::Instance()
                    .DeviceInterface()
                    .hdr();

        if (checkResult(r2, "Query Device.hdr")) {
            if (hdrEnabled != *r2) {
                reportEventValidationFailure("onHdrChanged",
                    std::string("event=") + (hdrEnabled ? "true" : "false") +
                    ", query=" + (*r2 ? "true" : "false"));
            }
        }
    });

if (checkResult(r, method)) {
    onHdrChangedSubId_ = *r;
}
```

This ensures all mismatches are tracked and flagged at the end of test execution.
