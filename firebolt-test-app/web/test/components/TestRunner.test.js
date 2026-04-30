/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

/**
 * TestRunner.test.js — Unit tests for the TestRunner component.
 *
 * Tests test loading, filtering of event-type tests,
 * single-test execution, and navigation guards.
 */
import App from 'src/index'
import settings from '../../settings.json'

// -------------------------------------------------------------------
// Mock FireboltAPI
// -------------------------------------------------------------------
jest.mock('src/lib/FireboltAPI', () => {
  // Defined inside the factory so Jest hoisting can access these before module-scope consts
  const stubTests = [
    { id: 'network_status', name: 'Network.status()', method: 'Network.status', type: 'getter', execute: jest.fn().mockResolvedValue({ value: 'connected', type: 'string', backend: 'mock' }) },
    { id: 'network_type', name: 'Network.type()', method: 'Network.type', type: 'getter', execute: jest.fn().mockResolvedValue({ value: 'wifi', type: 'string', backend: 'mock' }) },
    { id: 'network_onStatusChanged', name: 'Network.onStatusChanged [event]', method: 'Network.onStatusChanged', type: 'event', listenEvent: 'onStatusChanged' },
    { id: 'network_onInternetStatusChanged', name: 'Network.onInternetStatusChanged [gateway-event]', method: 'Network.onInternetStatusChanged', type: 'gateway-event', listenEvent: 'onInternetStatusChanged' }
  ]
  return jest.fn().mockImplementation(() => ({
    init: jest.fn().mockResolvedValue(undefined),
    getConnectionStatus: jest.fn().mockResolvedValue({ state: 'connected', endpoint: 'ws://127.0.0.1:3473' }),
    getVersionInfo: jest.fn().mockResolvedValue({ sdkVersion: '1.0.0-test' }),
    getTestsForCategory: jest.fn().mockResolvedValue(stubTests),
    runTest: jest.fn().mockResolvedValue({ success: true, message: 'mock-value', backend: 'mock' }),
    subscribeAllEvents: jest.fn(),
    unsubscribeAllEvents: jest.fn(),
    onEventLog: null
  }))
})

function bootApp() {
  return App(
    {
      stage: {
        ...settings.appSettings.stage,
        w: 1920,
        h: 1080,
        useImageWorker: false,
        debug: false
      },
      debug: false
    },
    {
      ...settings.platformSettings,
      log: false,
      fontLoader: jest.fn()
    }
  )
}

const NETWORK_CATEGORY = { id: 'network', name: 'Network', description: 'Network connectivity status and events' }

async function navigateToTestRunner(app, category = NETWORK_CATEGORY) {
  app._setState('Menu')
  const testRunner = app.tag('SafeContainer.Content.TestRunner')
  testRunner.loadTests(category)
  app._setState('TestRunner')
  // Flush pending microtasks so the getTestsForCategory.then() callback runs before we return
  await new Promise(resolve => setTimeout(resolve, 0))
  return testRunner
}

describe('TestRunner — test loading', () => {
  let app, testRunner

  beforeAll(async () => {
    app = bootApp()
    testRunner = await navigateToTestRunner(app)
  })

  it('should load tests for the "network" category', () => {
    expect(testRunner._tests).toBeDefined()
    expect(testRunner._tests.length).toBeGreaterThan(0)
  })

  it('should NOT include event-type tests in _tests', () => {
    const hasEventType = testRunner._tests.some(t => t.type === 'event' || t.type === 'gateway-event')
    expect(hasEventType).toBe(false)
  })

  it('should only include getter/gateway tests', () => {
    testRunner._tests.forEach(t => {
      expect(['getter', 'gateway']).toContain(t.type)
    })
  })
})

describe('TestRunner — navigation guards', () => {
  let app, testRunner

  beforeEach(async () => {
    app = bootApp()
    testRunner = await navigateToTestRunner(app)
  })

  it('_isRunning should be false after loadTests', () => {
    expect(testRunner._isRunning).toBe(false)
  })

  it('Back should navigate to Menu when not running', () => {
    // Simulate back from TestRunner state
    app._setState('TestRunner')
    app._handleBack()
    // After back, Menu should be visible
    expect(app.tag('SafeContainer.Content.Menu')?.visible).not.toBe(false)
  })

  it('Back should be blocked (return true) when running', () => {
    testRunner._isRunning = true
    app._setState('TestRunner')
    const result = app._handleBack()
    expect(result).toBe(true)
    testRunner._isRunning = false
  })
})

describe('TestRunner — UI structure', () => {
  let app, testRunner

  beforeAll(async () => {
    app = bootApp()
    testRunner = await navigateToTestRunner(app)
  })

  it('should have a RunButton', () => {
    expect(testRunner.tag('RunButton')).toBeDefined()
  })

  it('should have a TestListContainer', () => {
    expect(testRunner.tag('TestListContainer')).toBeDefined()
  })

  it('should have a ProgressBar', () => {
    expect(testRunner.tag('ProgressBar')).toBeDefined()
  })
})

describe('TestRunner — single test execution', () => {
  let app, testRunner, mockFireboltAPI

  beforeAll(async () => {
    app = bootApp()
    testRunner = await navigateToTestRunner(app)
    // Grab the mocked instance off the testRunner
    mockFireboltAPI = testRunner._fireboltAPI
  })

  it('should call runTest with a test definition when a test is run', async () => {
    if (testRunner._tests.length === 0) return
    await testRunner._runSingleTest(0)
    expect(mockFireboltAPI.runTest).toHaveBeenCalledWith(testRunner._tests[0])
  })

  it('runTest result should have success and message fields', async () => {
    if (testRunner._tests.length === 0) return
    const result = await mockFireboltAPI.runTest(testRunner._tests[0])
    expect(result).toHaveProperty('success')
    expect(result).toHaveProperty('message')
  })
})
