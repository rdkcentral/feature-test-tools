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
 * FireboltAPI.test.js — Unit tests for the FireboltAPI wrapper.
 *
 * Tests category definitions, runnable/event filtering,
 * runTest dispatch (mock mode), and event listener tracking.
 */
import FireboltAPI from 'src/lib/FireboltAPI'
import networkTests from 'src/data/tests/network.json'
import accessibilityTests from 'src/data/tests/accessibility.json'
import deviceTests from 'src/data/tests/device.json'

// -------------------------------------------------------------------
// Mock WebSocket globally (no real network opened)
// -------------------------------------------------------------------
let _originalWebSocket

class MockWebSocket {
  constructor() {
    this.readyState = MockWebSocket.OPEN
    this.sent = []
    // Trigger open async so ws.onopen can be assigned first
    setTimeout(() => this.onopen && this.onopen(), 0)
  }
  send(data) { this.sent.push(data) }
  close() { this.onclose && this.onclose() }
}
MockWebSocket.OPEN = 1

beforeAll(() => {
  _originalWebSocket = global.WebSocket
  global.WebSocket = MockWebSocket
})
afterAll(() => { global.WebSocket = _originalWebSocket })

// -------------------------------------------------------------------
// Mock DeviceConnectionConfig
// -------------------------------------------------------------------
jest.mock('src/lib/DeviceConnectionConfig', () => ({
  getConnectionInfo: jest.fn().mockReturnValue({
    endpoint: 'ws://127.0.0.1:3473',
    display: 'Local Device (127.0.0.1)'
  })
}))

// -------------------------------------------------------------------
// Suppress console noise during tests
// -------------------------------------------------------------------
beforeAll(() => {
  jest.spyOn(console, 'log').mockImplementation(() => {})
  jest.spyOn(console, 'error').mockImplementation(() => {})
})
afterAll(() => {
  console.log.mockRestore()
  console.error.mockRestore()
})

// -------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------
function makeApi() {
  const api = new FireboltAPI()
  // Prevent actual module init; mark as already initialized
  api.initialized = true
  return api
}

// -------------------------------------------------------------------
// Suite: test-definition loading
// -------------------------------------------------------------------
describe('FireboltAPI — TEST_DEFINITIONS_BY_CATEGORY', () => {
  let api

  beforeAll(() => { api = makeApi() })

  const EXPECTED_CATEGORIES = [
    'account', 'accessibility', 'advertising', 'device', 'discovery',
    'display', 'lifecycle', 'localization', 'metrics', 'network',
    'presentation', 'stats', 'texttospeech'
  ]

  it('should have all expected categories', () => {
    const allDefs = api._getAllTestDefinitions()
    const categories = [...new Set(allDefs.map(d => d._category).filter(Boolean))]
    EXPECTED_CATEGORIES.forEach(cat => {
      expect(categories).toContain(cat)
    })
  })

  EXPECTED_CATEGORIES.forEach(cat => {
    it(`should include at least one test definition for "${cat}"`, () => {
      const defs = api._getTestsForCategory(cat)
      expect(Array.isArray(defs)).toBe(true)
      expect(defs.length).toBeGreaterThan(0)
    })
  })
})

// -------------------------------------------------------------------
// Suite: event vs runnable filtering
// -------------------------------------------------------------------
describe('FireboltAPI — runnable vs event test separation', () => {
  let api

  beforeAll(() => { api = makeApi() })

  it('_getTestsForCategory("network") should include getter tests', () => {
    const tests = api._getTestsForCategory('network')
    const getters = tests.filter(t => t.type === 'getter')
    expect(getters.length).toBeGreaterThan(0)
  })

  it('_getTestsForCategory("network") should include event tests (before filter)', () => {
    const tests = api._getTestsForCategory('network')
    const events = tests.filter(t => t.type === 'event')
    expect(events.length).toBeGreaterThan(0)
  })

  it('event tests should have a listenEvent field', () => {
    const tests = api._getTestsForCategory('network')
    const events = tests.filter(t => t.type === 'event')
    events.forEach(e => {
      expect(e).toHaveProperty('listenEvent')
    })
  })
})

// -------------------------------------------------------------------
// Suite: mock mode runTest
// -------------------------------------------------------------------
describe('FireboltAPI — runTest (mock mode)', () => {
  let api

  beforeAll(() => { api = makeApi() })

  it('runTest on a mock test definition should return a result object', async () => {
    const mockDef = {
      id: 'test_mock',
      name: 'Mock.test()',
      method: 'mock.test',
      type: 'getter',
      params: [],
      expectedType: 'string',
      _isMock: true,
      execute: jest.fn().mockResolvedValue({ value: 'mock-result', type: 'string', backend: 'mock' })
    }
    const result = await api.runTest(mockDef)
    expect(result).toBeDefined()
    expect(result).toHaveProperty('success')
    expect(result).toHaveProperty('message')
  })
})

// -------------------------------------------------------------------
// Suite: event listener tracking
// -------------------------------------------------------------------
describe('FireboltAPI — event listener tracking', () => {
  it('_eventListeners should start empty', () => {
    const api = makeApi()
    expect(api._eventListeners).toEqual([])
  })

  it('_eventSockets should start empty', () => {
    const api = makeApi()
    expect(api._eventSockets).toEqual([])
  })

  it('unsubscribeAllEvents should clear _eventListeners', () => {
    const api = makeApi()
    const mockClear = jest.fn()
    api._eventListeners = [{ moduleObj: { clear: mockClear }, listenerId: 42 }]
    api.unsubscribeAllEvents()
    expect(mockClear).toHaveBeenCalledWith(42)
    expect(api._eventListeners).toEqual([])
  })

  it('unsubscribeAllEvents should close and clear _eventSockets', () => {
    const api = makeApi()
    const mockClose = jest.fn()
    api._eventSockets = [{ close: mockClose }]
    api.unsubscribeAllEvents()
    expect(mockClose).toHaveBeenCalled()
    expect(api._eventSockets).toEqual([])
  })
})

// -------------------------------------------------------------------
// Suite: onEventLog callback
// -------------------------------------------------------------------
describe('FireboltAPI — onEventLog hook', () => {
  it('onEventLog should be undefined by default', () => {
    const api = makeApi()
    expect(api.onEventLog).toBeUndefined()
  })

  it('onEventLog can be assigned and is callable', () => {
    const api = makeApi()
    const cb = jest.fn()
    api.onEventLog = cb
    api.onEventLog('Test.onSomething', '{"value":1}')
    expect(cb).toHaveBeenCalledWith('Test.onSomething', '{"value":1}')
  })
})
