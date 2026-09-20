# Progress & Failure Tracking Implementation

## Overview
Implemented a mechanism for test modules to report progress and failure detection to the central `ProgressController` without requiring direct dependencies on the main application.

## Architecture

### 1. Interface: `ITestProgressTracker` (utils.h)
```cpp
class ITestProgressTracker {
public:
    virtual ~ITestProgressTracker() = default;
    virtual void reportStepCompleted(bool failDetected = false) = 0;
};
```

**Purpose:** Defines the contract for progress tracking, allowing test modules to report step completions without coupling to main.cpp's `ProgressController`.

### 2. Global Accessors (utils.h & utils.cpp)
```cpp
ITestProgressTracker* GetTestProgressTracker();
void SetTestProgressTracker(ITestProgressTracker* tracker);
```

**Implementation:**
- Thread-safe singleton pattern using static pointer + mutex
- Allows main app to inject the ProgressController before test execution
- Returns nullptr if not registered (safe no-op in test modules)

### 3. Enhanced `TestModuleBase` (utils.h)
Added two new protected methods:

#### `checkResult()` - Updated template method
- Now calls `reportStepCompletion()` after logging result
- Automatically reports pass/fail to progress tracker
- **No changes needed** in derived test classes for backward compatibility

#### `reportStepCompletion()` - New helper method
- Explicit reporting for non-Result methods (e.g., `unsubscribeAll()`)
- Usage: `reportStepCompletion(failed);`
- Safe to call from any derived test module

### 4. `ProgressController` Enhancement (main.cpp)
Made `ProgressController` inherit from `ITestProgressTracker`:

```cpp
class ProgressController : public ITestProgressTracker {
    // ... existing implementation ...

    // Override ITestProgressTracker
    void reportStepCompleted(bool failDetected = false) override {
        increment_progress(failDetected);
    }
};
```

### 5. Registration (main.cpp)
In the test execution thread (`startRunTestModules`):

```cpp
// Before running tests
SetTestProgressTracker(&PC);

// Run tests
runAutoMode(testModules, PC, thunderClient, exitRequested);
runAutoModeDeferredUnsubscribeCleanup(testModules, PC);

// After cleanup
SetTestProgressTracker(nullptr);
```

## Usage in Test Modules

### Automatic Progress Tracking (Existing Code)
Already works with no changes needed:
```cpp
auto r = IFireboltAccessor::Instance().SomeModule().someMethod();
checkResult(r, "Module.method");  // Reports pass/fail automatically
```

### Manual Progress Tracking (For Non-Result Methods)
```cpp
void AccessibilityTest::runMethod(const std::string& method) {
    // ...
    else if (method == "Accessibility.unsubscribeAll") {
        IFireboltAccessor::Instance().AccessibilityInterface().unsubscribeAll();
        reportStepCompletion(false);  // Report success (no Result object)
    }
}
```

## Data Flow

```
Test Module runMethod()
    ↓
checkResult(result, label)  [OR]  reportStepCompletion(failed)
    ↓
GetTestProgressTracker()->reportStepCompleted(failDetected)
    ↓
ProgressController::increment_progress(failDetected)
    ↓
Updates: count++, currentPercentage, failDetected flag
    ↓
Notifies CV for glAppProgressUpdateThread
    ↓
Progress update sent to GL display
```

## Benefits

1. **Decoupling:** Test modules no longer depend on main.cpp
2. **Thread-Safe:** Uses mutex-protected singleton
3. **Backward Compatible:** Existing code continues to work unchanged
4. **Extensible:** Can swap tracker implementation if needed
5. **Null-Safe:** Handles cases where tracker isn't registered

## Files Modified

1. **src/utils.h**
   - Added `ITestProgressTracker` interface
   - Added `GetTestProgressTracker()` / `SetTestProgressTracker()` declarations
   - Enhanced `TestModuleBase::checkResult()` template
   - Added `TestModuleBase::reportStepCompletion()` method

2. **src/utils.cpp**
   - Implemented global tracker accessors
   - Thread-safe singleton with mutex

3. **src/main.cpp**
   - Made `ProgressController` inherit from `ITestProgressTracker`
   - Implemented `reportStepCompleted()` override
   - Register/unregister tracker around test execution

## Example: Tracking Silent Methods

For methods that don't produce explicit logging, add reporting at the end:

```cpp
else if (method == "VideoOutput.unsubscribeAll") {
    IFireboltAccessor::Instance().VideoOutputInterface().unsubscribeAll();
    onResolutionChangedSubId_ = 0;
    onHdcpChangedSubId_ = 0;
    // ... reset other IDs ...
    std::cout << "  Unsubscribed from all VideoOutput events." << std::endl;
    reportStepCompletion(false);  // Explicitly report completion
}
```

This ensures the 36 "silent" steps are properly accounted for in the progress percentage calculation.
