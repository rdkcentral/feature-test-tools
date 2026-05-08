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

// Firebolt API Wrapper
// SDK modules (via @firebolt-js/core-client): Accessibility, Advertising, Device, Discovery, Localization, Metrics, Network
// Spec-defined modules not yet in SDK package (called via raw WebSocket JSON-RPC Gateway):
//   Display, Lifecycle2, Presentation, Stats, TextToSpeech
// Account: NOT in OpenRPC spec or SDK — always mock

import DeviceConnectionConfig from './DeviceConnectionConfig'
import AppSettings from './AppSettings'
import accountTests from '../data/tests/account.json'
import accessibilityTests from '../data/tests/accessibility.json'
import advertisingTests from '../data/tests/advertising.json'
import deviceTests from '../data/tests/device.json'
import discoveryTests from '../data/tests/discovery.json'
import displayTests from '../data/tests/display.json'
import lifecycleTests from '../data/tests/lifecycle.json'
import localizationTests from '../data/tests/localization.json'
import metricsTests from '../data/tests/metrics.json'
import networkTests from '../data/tests/network.json'
import presentationTests from '../data/tests/presentation.json'
import statsTests from '../data/tests/stats.json'
import texttospeechTests from '../data/tests/texttospeech.json'
import mockstressTests from '../data/tests/mockstress.json'

// Verbose debug logger — only emits when app-settings.json debug.verbose is true
const dbg = (msg) => AppSettings.debug.verbose && console.log(msg)

let FireboltModules = {}
let useNpmPackage = false
let connectionInfo = null
let _connectionSetup = false
let _modulesInitPromise = null

// Maps category IDs to their statically-imported JSON test definitions.
// Includes Firebolt module categories plus local-only categories (account, mockstress).
const TEST_DEFINITIONS_BY_CATEGORY = {
  account: accountTests,
  accessibility: accessibilityTests,
  advertising: advertisingTests,
  device: deviceTests,
  discovery: discoveryTests,
  display: displayTests,
  lifecycle: lifecycleTests,
  localization: localizationTests,
  metrics: metricsTests,
  network: networkTests,
  presentation: presentationTests,
  stats: statsTests,
  texttospeech: texttospeechTests,
  mockstress: mockstressTests
}

// Validate that a Firebolt endpoint URL uses the ws:// or wss:// scheme.
// This guards against non-WebSocket URL schemes; it does not restrict the target host.
function _isValidFireboltEndpoint(url) {
  try {
    const parsed = new URL(url)
    return parsed.protocol === 'ws:' || parsed.protocol === 'wss:'
  } catch (_) {
    return false
  }
}

// Extract the display hostname from a WebSocket endpoint URL.
function _hostFromEndpoint(endpoint) {
  try { return new URL(endpoint).hostname } catch (_) { return endpoint }
}

const UINT32_MAX = 0xffffffff

function _isUnsigned(value) {
  return Number.isInteger(value) && value >= 0 && value <= UINT32_MAX
}

function _isDouble(value) {
  return typeof value === 'number' && Number.isFinite(value)
}

function _isStringArray(value) {
  return Array.isArray(value) && value.every(item => typeof item === 'string')
}

function _isObject(value) {
  return !!value && typeof value === 'object' && !Array.isArray(value)
}

function _isBooleanRecord(value, keys) {
  return _isObject(value) && keys.every(key => typeof value[key] === 'boolean')
}

function _typeLabel(value) {
  if (value === null) return 'null'
  if (Array.isArray(value)) return 'array'
  return typeof value
}

function _validateField(condition, reason) {
  return condition ? null : reason
}

function _validateBoolean(value, fieldName) {
  return _validateField(typeof value === 'boolean', `${fieldName} must be a boolean`)
}

function _validateString(value, fieldName) {
  return _validateField(typeof value === 'string', `${fieldName} must be a string`)
}

function _validateNumber(value, fieldName) {
  return _validateField(_isDouble(value), `${fieldName} must be a finite number`)
}

function _validateStringArrayField(value, fieldName) {
  return _validateField(_isStringArray(value), `${fieldName} must be an array of strings`)
}

function _validateObject(value, fieldName = 'value') {
  return _validateField(_isObject(value), `${fieldName} must be an object, got ${_typeLabel(value)}`)
}

function _validateOneOf(value, fieldName, allowedValues) {
  return _validateField(allowedValues.includes(value), `${fieldName} must be one of ${allowedValues.join(', ')}`)
}

function _validateBooleanRecordField(value, fieldName, keys) {
  const objectError = _validateObject(value, fieldName)
  if (objectError) return objectError
  for (const key of keys) {
    const error = _validateBoolean(value[key], `${fieldName}.${key}`)
    if (error) return error
  }
  return null
}

function _formatResultValue(result) {
  if (!result) return 'undefined'
  if (result.type === 'error') return String(result.value)
  return JSON.stringify(result.value)
}

function _extractFailureReason(value) {
  if (typeof value === 'string' && value.trim().length > 0) return value
  if (!value || typeof value !== 'object') return String(value)

  // Check for {code, message} first so the formatted [code] message is used when both are present
  const hasCode = typeof value.code === 'number' || typeof value.code === 'string'
  const hasMessage = typeof value.message === 'string' && value.message.trim().length > 0
  if (hasCode && hasMessage) return `[${value.code}] ${value.message}`

  if (hasMessage) return value.message

  if (value.error) {
    if (typeof value.error === 'string') return value.error
    if (typeof value.error.message === 'string' && value.error.message.trim().length > 0) {
      return value.error.message
    }
  }

  return JSON.stringify(value)
}

const JS_SPEC_ERROR_METHODS = new Set([
  'Device.uptime',
  'Device.timeInActiveState',
  'Device.chipsetId',
  'Display.edid',
  'Display.size',
  'Display.maxResolution',
  'Lifecycle2.close',
  'Lifecycle2.state',
  'Lifecycle2.onStateChanged',
  'Metrics.signIn',
  'Metrics.signOut',
  'Presentation.focused',
  'Presentation.onFocusedChanged',
  'Stats.memoryUsage',
  'TextToSpeech.speak',
  'TextToSpeech.pause',
  'TextToSpeech.resume',
  'TextToSpeech.cancel',
  'TextToSpeech.getspeechstate',
  'TextToSpeech.listvoices',
  'TextToSpeech.onWillspeak',
  'TextToSpeech.onSpeechstart',
  'TextToSpeech.onSpeechpause',
  'TextToSpeech.onSpeechresume',
  'TextToSpeech.onSpeechcomplete',
  'TextToSpeech.onSpeechinterrupted',
  'TextToSpeech.onNetworkerror',
  'TextToSpeech.onPlaybackerror'
])

const JS_SPEC_NONE_RETURN_METHODS = new Set([
  'Discovery.watched',
  'Metrics.ready',
  'Metrics.page',
  'Metrics.error',
  'Metrics.event',
  'Metrics.appInfo',
  'Metrics.startContent',
  'Metrics.stopContent',
  'Metrics.mediaLoadStart',
  'Metrics.mediaPlay',
  'Metrics.mediaPlaying',
  'Metrics.mediaPause',
  'Metrics.mediaWaiting',
  'Metrics.mediaSeeking',
  'Metrics.mediaSeeked',
  'Metrics.mediaRateChanged',
  'Metrics.mediaRenditionChanged',
  'Metrics.mediaEnded'
])

const JS_SPEC_VALIDATORS = {
  'Accessibility.audioDescription': value => _validateBoolean(value, 'value'),
  'Accessibility.onAudioDescriptionChanged': value => _validateBoolean(value, 'value'),
  'Accessibility.closedCaptionsSettings': value => _validateObject(value)
    || _validateBoolean(value.enabled, 'enabled')
    || _validateStringArrayField(value.preferredLanguages, 'preferredLanguages'),
  'Accessibility.onClosedCaptionsSettingsChanged': value => _validateObject(value)
    || _validateBoolean(value.enabled, 'enabled')
    || _validateStringArrayField(value.preferredLanguages, 'preferredLanguages'),
  'Accessibility.highContrastUI': value => _validateBoolean(value, 'value'),
  'Accessibility.onHighContrastUIChanged': value => _validateBoolean(value, 'value'),
  'Accessibility.voiceGuidanceSettings': value => _validateObject(value)
    || _validateBoolean(value.enabled, 'enabled')
    || _validateNumber(value.rate, 'rate')
    || _validateField(value.rate >= 0.1 && value.rate <= 10, 'rate must be between 0.1 and 10 inclusive')
    || _validateBoolean(value.navigationHints, 'navigationHints'),
  'Accessibility.onVoiceGuidanceSettingsChanged': value => _validateObject(value)
    || _validateBoolean(value.enabled, 'enabled')
    || _validateNumber(value.rate, 'rate')
    || _validateField(value.rate >= 0.1 && value.rate <= 10, 'rate must be between 0.1 and 10 inclusive')
    || _validateBoolean(value.navigationHints, 'navigationHints'),
  'Advertising.advertisingId': value => _validateObject(value)
    || _validateString(value.ifa, 'ifa')
    || _validateOneOf(value.ifa_type, 'ifa_type', ['dpid', 'sspid', 'sessionid'])
    || _validateField(value.lmt !== undefined, 'missing required field lmt (device returned limit instead of lmt)' )
    || _validateOneOf(value.lmt, 'lmt', ['0', '1']),
  'Device.uid': value => _validateString(value, 'value') || _validateField(value.length > 0, 'value must not be empty'),
  'Device.deviceClass': value => _validateOneOf(value, 'value', ['ott', 'stb', 'tv']),
  'Device.hdr': value => _validateBooleanRecordField(value, 'value', ['hdr10', 'hdr10Plus', 'dolbyVision', 'hlg']),
  'Device.onHdrChanged': value => _validateBooleanRecordField(value, 'value', ['hdr10', 'hdr10Plus', 'dolbyVision', 'hlg']),
  'Localization.country': value => _validateString(value, 'value'),
  'Localization.onCountryChanged': value => _validateString(value, 'value'),
  'Localization.preferredAudioLanguages': value => _validateStringArrayField(value, 'value'),
  'Localization.onPreferredAudioLanguagesChanged': value => _validateStringArrayField(value, 'value'),
  'Localization.presentationLanguage': value => _validateString(value, 'value'),
  'Localization.onPresentationLanguageChanged': value => _validateString(value, 'value'),
  'Network.connected': value => _validateBoolean(value, 'value'),
  'Network.onConnectedChanged': value => _validateBoolean(value, 'value')
}

function _validateAgainstJsSpec(test, result) {
  const actualHasError = result.type === 'error' || _looksLikeGatewayErrorPayload(result.value)

  if (JS_SPEC_ERROR_METHODS.has(test.method)) {
    const returned = _formatResultValue(result)
    const reason = _extractFailureReason(result.value)
    return {
      success: actualHasError,
      message: actualHasError
        ? `Expected JS error observed\nReason: ${reason}\nReturned: ${returned}`
        : `Expected JS error, but returned a value\nReason: method returned a normal payload\nReturned: ${returned}`
    }
  }

  if (actualHasError) {
    const returned = _formatResultValue(result)
    const reason = _extractFailureReason(result.value)
    return {
      success: false,
      message: `Error\nReason: ${reason}\nReturned: ${returned}`
    }
  }

  if (JS_SPEC_NONE_RETURN_METHODS.has(test.method)) {
    return {
      success: true,
      message: `Call succeeded\nReturned: ${_formatResultValue(result)}`
    }
  }

  const validator = JS_SPEC_VALIDATORS[test.method]
  if (!validator) return null

  const validationError = validator(result.value)

  return {
    success: !validationError,
    message: !validationError
      ? `Returned: ${JSON.stringify(result.value)}`
      : `Spec validation failed\nReason: ${test.method} - ${validationError}\nReturned: ${JSON.stringify(result.value)}`
  }
}

// Gateway methods can sometimes return application-level error objects inside result
// instead of JSON-RPC top-level errors; treat these as test failures.
function _looksLikeGatewayErrorPayload(value) {
  if (!value || typeof value !== 'object' || Array.isArray(value)) return false

  if (value.error !== undefined && value.error !== null) return true
  if (value.success === false) return true
  if (value.ok === false) return true
  if (typeof value.status === 'string' && /error|failed|failure/i.test(value.status)) return true

  const hasCode = typeof value.code === 'number' || typeof value.code === 'string'
  const hasMessage = typeof value.message === 'string' && value.message.trim().length > 0
  return hasCode && hasMessage
}

// Setup device connection configuration before importing modules — runs once
function setupDeviceConnection() {
  if (_connectionSetup) return
  _connectionSetup = true
  connectionInfo = DeviceConnectionConfig.getConnectionInfo()

  // Priority 1: window.__firebolt.endpoint — injected at document start by the RDK BrowserLauncher
  //   as: window.__firebolt = { endpoint: 'ws://...' };
  // Priority 2: __firebolt_endpoint query param — legacy / dev fallback
  const rawEndpoint = (window.__firebolt && window.__firebolt.endpoint)
    || new URLSearchParams(window.location.search).get('__firebolt_endpoint')
  const fbEndpoint = rawEndpoint && _isValidFireboltEndpoint(rawEndpoint) ? rawEndpoint : null
  if (rawEndpoint && !fbEndpoint) {
    dbg('Ignoring invalid Firebolt endpoint (must be ws:// or wss://)')
  }
  if (fbEndpoint) {
    dbg(`Using Firebolt endpoint from ${window.__firebolt && window.__firebolt.endpoint ? 'window.__firebolt.endpoint' : '__firebolt_endpoint query param'}: ${fbEndpoint}`)
    connectionInfo = Object.assign({}, connectionInfo, { endpoint: fbEndpoint })
    window.__firebolt = window.__firebolt || {}
    window.__firebolt.endpoint = fbEndpoint
  }

  // Reference: https://github.com/rdkcentral/entservices-appgateway/blob/develop/docs/RDK8.md#compliant-json-rpc-detection
  if (!connectionInfo.endpoint.includes('RPCV2=true')) {
    connectionInfo.endpoint += (connectionInfo.endpoint.includes('?') ? '&' : '?') + 'RPCV2=true'
  }

  dbg(' Firebolt Device Connection:')
  dbg('   Endpoint: ' + connectionInfo.endpoint)
  dbg('   Type: ' + connectionInfo.display)

  if (window.__FIREBOLT_CONFIG__ === undefined) {
    window.__FIREBOLT_CONFIG__ = {
      endpoint: connectionInfo.endpoint,
      transport: 'ws'
    }
  }
}

// Attempt to load firebolt-js-client modules (primary backend) — runs once
function initializeFireboltModules() {
  if (_modulesInitPromise) return _modulesInitPromise
  _modulesInitPromise = _doInitializeFireboltModules()
  return _modulesInitPromise
}

async function _doInitializeFireboltModules() {
  setupDeviceConnection()

  try {
    const m = await import('@firebolt-js/core-client')
    const moduleMap = {
      Accessibility: m.Accessibility,
      Advertising: m.Advertising,
      Device: m.Device,
      Discovery: m.Discovery,
      Localization: m.Localization,
      Metrics: m.Metrics,
      Network: m.Network
    }
    Object.entries(moduleMap).forEach(([name, mod]) => {
      if (mod) FireboltModules[name] = mod
    })

    if (Object.keys(FireboltModules).length > 0) {
      useNpmPackage = true
      dbg('Firebolt JS Client initialized as primary backend')
      dbg(`   Connected to: ${connectionInfo.endpoint}`)
      return true
    }
  } catch (error) {
    dbg('Firebolt JS Client not available, falling back to window.Firebolt:' + error.message)
    useNpmPackage = false
  }
  return false
}

export default class FireboltAPI {
  constructor() {
    this.initialized = false
    this._mockFirebolt = this._createMockFirebolt()
    this.firebolt = window.Firebolt || this._mockFirebolt
    this.usingMock = !window.Firebolt
    this._lastCallBackend = 'unknown'
    this._testDefinitions = {}
    this._eventListeners = []   // { moduleObj, listenerId }
    this._eventSockets = []     // WebSocket refs for gateway-event subscriptions
    this._initializeDefaultTests()
  }

  // Initialize the API (call this after instantiation)
  async init() {
    if (!this.initialized) {
      await initializeFireboltModules()
      this.initialized = true
    }
  }

  // Subscribe persistently to all event-type and gateway-event-type tests at startup.
  // Fires a console.log whenever a subscribed event arrives.
  // Teardown is handled by unsubscribeAllEvents() (called from App._detach()).
  async subscribeAllEvents() {
    try {
      await this.init()
      const allDefs = Object.values(TEST_DEFINITIONS_BY_CATEGORY).flat()
      const eventDefs = allDefs.filter(d => d.type === 'event' || d.type === 'gateway-event')

      for (const def of eventDefs) {
        if (def.type === 'event') {
          const moduleName = def.method.split('.')[0]
          const moduleKey = Object.keys(FireboltModules).find(
            k => k.toLowerCase() === moduleName.toLowerCase()
          )
          const moduleObj = moduleKey ? FireboltModules[moduleKey] : null
          if (!moduleObj || typeof moduleObj.listen !== 'function') {
            dbg(`[Event Monitor] ${def.name}: module not available, skipping`)
            continue
          }
          try {
            const _errStr = (e) => e && e.message ? e.message : JSON.stringify(e)
            const listenPromise = moduleObj.listen(def.listenEvent, (value) => {
              const payloadStr = JSON.stringify(value)
              dbg(`[Event Monitor] ${def.name} fired`)
              dbg(`[Event Monitor] Full msg: ${payloadStr}`)
              if (this.onEventLog) this.onEventLog(def.name, payloadStr)
            })
            // Attach .catch() before await so any secondary internal SDK rejections
            // from this promise are silenced and don't become unhandled rejections.
            listenPromise.catch((e) => {
              dbg(`[Event Monitor] Internal rejection for ${def.name}:`+ _errStr(e))
            })
            const listenerId = await listenPromise
            this._eventListeners.push({ moduleObj, listenerId })
            dbg(`[Event Monitor] Subscribed: ${def.name}`)
          } catch (err) {
            const msg = err && err.message ? err.message : JSON.stringify(err)
            dbg(`[Event Monitor] Failed to subscribe ${def.name}:`+ msg)
          }
        } else if (def.type === 'gateway-event') {
          // Keep the WebSocket open persistently for gateway events
          this._subscribeGatewayEventPersistent(def.method, def.name)
        }
      }
    } catch (e) {
      console.error('subscribeAllEvents error: ' + (e && e.message ? e.message : JSON.stringify(e)))
    }
  }

  // Tear down all persistent event subscriptions registered by subscribeAllEvents().
  unsubscribeAllEvents() {
    for (const { moduleObj, listenerId } of this._eventListeners) {
      try { if (moduleObj.clear) moduleObj.clear(listenerId) } catch (_) {}
    }
    this._eventListeners = []

    for (const ws of this._eventSockets) {
      try { ws.close() } catch (_) {}
    }
    this._eventSockets = []
    dbg('[Event Monitor] All event subscriptions torn down')
  }

  // Opens a persistent WebSocket subscription for a gateway event and logs on each fire.
  _subscribeGatewayEventPersistent(method, label) {
    const doConnect = async () => {
      try {
        await this.init()
        const info = connectionInfo
        let ws
        try { ws = new WebSocket(info.endpoint) } catch (err) {
          dbg(`[Event Monitor] ${label}: cannot connect — ${err.message}`)
          return
        }
        const id = Math.floor(Math.random() * 900000) + 100000
        let subscribed = false
        this._eventSockets.push(ws)
        ws.onopen = () => {
          ws.send(JSON.stringify({ jsonrpc: '2.0', id, method, params: { listen: true } }))
        }
        ws.onmessage = (evt) => {
          try {
            const msg = JSON.parse(evt.data)
            if (!subscribed && msg.id === id) {
              if (msg.error) {
                const safeErrorMessage = String(msg.error.message ?? '').replace(/\r|\n/g, '')
                dbg(`[Event Monitor] ${label}: subscribe error — ${safeErrorMessage}`)
              } else {
                subscribed = true
                dbg(`[Event Monitor] Subscribed: ${label}`)
              }
            } else if (subscribed && !msg.id && msg.method === method) {
              const payloadStr = JSON.stringify(msg)
              dbg(`[Event Monitor] ${label} fired`)
              dbg(`[Event Monitor] Full msg: ${payloadStr}`)
              if (this.onEventLog) this.onEventLog(label, payloadStr)
            }
          } catch (_) {}
        }
        ws.onerror = () => dbg(`[Event Monitor] ${label}: WebSocket error`)
        ws.onclose = () => dbg(`[Event Monitor] ${label}: connection closed`)
      } catch (err) {
        dbg(`[Event Monitor] ${label}: init error — `+ err.message)
      }
    }
    doConnect()
  }

  // Call a spec-defined method that is not yet exposed by the SDK package.
  // Opens a short-lived WebSocket to the Firebolt JSON-RPC endpoint and makes
  // a single request, then closes the connection.
  async _callViaGateway(method, params = {}) {
    await this.init()

    const info = connectionInfo

    return new Promise((resolve, reject) => {
      let ws
      try {
        ws = new WebSocket(info.endpoint)
      } catch (err) {
        return reject(new Error(`Cannot open WebSocket to ${info.endpoint}: ${err.message}`))
      }

      const id = Math.floor(Math.random() * 900000) + 100000
      const timeout = setTimeout(() => {
        try { ws.close() } catch (_) {}
        reject(new Error(`Gateway timeout for ${method} (5s)`))
      }, 5000)

      ws.onopen = () => {
        ws.send(JSON.stringify({ jsonrpc: '2.0', id, method, params }))
      }

      ws.onmessage = (evt) => {
        try {
          const msg = JSON.parse(evt.data)
          if (msg.id === id) {
            clearTimeout(timeout)
            try { ws.close() } catch (_) {}
            if (msg.error) {
              reject(new Error(`[${msg.error.code}] ${msg.error.message}`))
            } else {
              this._lastCallBackend = 'gateway'
              resolve(msg.result)
            }
          }
        } catch (e) {
          clearTimeout(timeout)
          try { ws.close() } catch (_) {}
          reject(e)
        }
      }

      ws.onerror = () => {
        clearTimeout(timeout)
        try { ws.close() } catch (_) {}
        reject(new Error(`WebSocket error calling ${method}`))
      }
    })
  }

  // Subscribe to an event via the SDK module's listen() method.
  // One-shot: clears the listener when the event fires.
  // If timeoutMs > 0, the listener is also cleared when the timeout elapses (preventing leaks).
  _subscribeToEvent(moduleName, listenEvent, timeoutMs = 0) {
    const doSubscribe = async (resolve, reject) => {
      try {
        if (!this.initialized) await this.init()

        const moduleKey = Object.keys(FireboltModules).find(
          k => k.toLowerCase() === moduleName.toLowerCase()
        )
        const moduleObj = moduleKey ? FireboltModules[moduleKey] : null

        if (!moduleObj || typeof moduleObj.listen !== 'function') {
          return reject(new Error(`Module ${moduleName} not available or has no listen() method`))
        }

        this._lastCallBackend = 'core-client'
        let listenerId
        let timeoutHandle
        listenerId = await moduleObj.listen(listenEvent, (value) => {
          clearTimeout(timeoutHandle)
          dbg(`[Firebolt Event] ${moduleName}.${listenEvent} fired`)
          dbg(`[Firebolt Event] Full msg: ${JSON.stringify(value)}`)
          if (moduleObj.clear) moduleObj.clear(listenerId)
          resolve(value)
        })
        dbg(`[Firebolt Event] Subscribed to ${moduleName}.${listenEvent}, listener id:`+ listenerId)
        if (timeoutMs > 0) {
          timeoutHandle = setTimeout(() => {
            if (moduleObj.clear) moduleObj.clear(listenerId)
            resolve(null)
          }, timeoutMs)
        }
      } catch (err) {
        reject(err)
      }
    }
    return new Promise(doSubscribe)
  }

  // Subscribe to a spec-defined event that is not in the SDK, via raw WebSocket.
  // Sends {listen:true}, waits for ACK, then keeps WS open until the event fires.
  // If timeoutMs > 0, the WebSocket is closed and the promise resolves with null when
  // the timeout elapses, preventing socket leaks when no event arrives.
  _subscribeViaGateway(method, timeoutMs = 0) {
    return new Promise((resolve, reject) => {
      const doConnect = async () => {
        try {
          await this.init()
          const info = connectionInfo
          let ws
          try {
            ws = new WebSocket(info.endpoint)
          } catch (err) {
            return reject(new Error(`Cannot open WebSocket to ${info.endpoint}: ${err.message}`))
          }

          const id = Math.floor(Math.random() * 900000) + 100000
          let subscribed = false
          let timeoutHandle

          if (timeoutMs > 0) {
            timeoutHandle = setTimeout(() => {
              try { ws.close() } catch (_) {}
              resolve(null)
            }, timeoutMs)
          }

          ws.onopen = () => {
            ws.send(JSON.stringify({ jsonrpc: '2.0', id, method, params: { listen: true } }))
          }

          ws.onmessage = (evt) => {
            try {
              const msg = JSON.parse(evt.data)
              if (!subscribed && msg.id === id) {
                // Subscription ACK — now wait for the event notification
                if (msg.error) {
                  clearTimeout(timeoutHandle)
                  try { ws.close() } catch (_) {}
                  reject(new Error(`[${msg.error.code}] ${msg.error.message}`))
                } else {
                  subscribed = true
                  const safeListenerId = String(msg.result).replace(/[\r\n]/g, '')
                  dbg(`[Firebolt Gateway Event] Subscribed to ${method}, listener id:` + safeListenerId)
                }
              } else if (subscribed && !msg.id && msg.method === method) {
                // Event notification — method matches subscribed event
                clearTimeout(timeoutHandle)
                dbg(`[Firebolt Gateway Event] ${method} fired`)
                dbg(`[Firebolt Gateway Event] Full msg: ${JSON.stringify(msg)}`)
                try { ws.close() } catch (_) {}
                this._lastCallBackend = 'gateway'
                resolve(msg.params)
              }
            } catch (e) {
              clearTimeout(timeoutHandle)
              try { ws.close() } catch (_) {}
              reject(e)
            }
          }

          ws.onerror = () => {
            clearTimeout(timeoutHandle)
            try { ws.close() } catch (_) {}
            reject(new Error(`WebSocket error subscribing to ${method}`))
          }
        } catch (err) {
          reject(err)
        }
      }
      doConnect()
    })
  }

  // Safe method caller that works with both backends
  async _callMethod(methodPath, params = []) {
    // Ensure initialization on first call
    if (!this.initialized) {
      await this.init()
    }

    // account.* and mock.* are always served from the local mock regardless of window.Firebolt
    if (methodPath.startsWith('account.') || methodPath.startsWith('mock.')) {
      const [ns, methodName] = methodPath.split('.')
      const method = this._mockFirebolt[ns] && this._mockFirebolt[ns][methodName]
      if (typeof method === 'function') {
        this._lastCallBackend = 'mock'
        return await method.apply(this._mockFirebolt[ns], params)
      }
      throw new Error(`Mock method not found: ${methodPath}`)
    }

    try {
      // First, try firebolt-js-client if initialized
      if (useNpmPackage && methodPath.includes('.')) {
        const [moduleName, methodName] = methodPath.split('.')
        const moduleKey = Object.keys(FireboltModules).find(
          key => key.toLowerCase() === moduleName.toLowerCase()
        )
        const moduleObj = moduleKey ? FireboltModules[moduleKey] : null

        if (moduleObj) {
          const resolvedMethod = moduleObj[methodName]
            || moduleObj[methodName.toLowerCase()]
            || moduleObj[methodName.charAt(0).toLowerCase() + methodName.slice(1)]

          if (typeof resolvedMethod === 'function') {
            this._lastCallBackend = 'core-client'
            const result = await resolvedMethod(...params)
            return result
          }
        }

        // Retry once with module name in PascalCase for definitions like account.id
        const pascalModule = moduleName.charAt(0).toUpperCase() + moduleName.slice(1)
        if (FireboltModules[pascalModule]) {
          const resolvedMethod = FireboltModules[pascalModule][methodName]
            || FireboltModules[pascalModule][methodName.toLowerCase()]
            || FireboltModules[pascalModule][methodName.charAt(0).toLowerCase() + methodName.slice(1)]

          if (typeof resolvedMethod === 'function') {
            this._lastCallBackend = 'core-client'
            const result = await resolvedMethod(...params)
            return result
          }
        }
      }

      // Fallback to window.Firebolt (legacy or when npm package not available)
      const pathParts = methodPath.split('.')
      let obj = this.firebolt

      for (let i = 0; i < pathParts.length - 1; i++) {
        const part = pathParts[i]
        obj = obj[part] || obj[part.toLowerCase()] || obj[part.charAt(0).toLowerCase() + part.slice(1)]
        if (!obj) throw new Error(`API not available: ${pathParts.slice(0, i + 1).join('.')}`)
      }

      const methodName = pathParts[pathParts.length - 1].replace('()', '')
      const method = obj[methodName] || obj[methodName.toLowerCase()] || obj[methodName.charAt(0).toLowerCase() + methodName.slice(1)]

      if (!method) throw new Error(`Method not found: ${methodPath}`)

      this._lastCallBackend = this.usingMock ? 'mock' : 'window.firebolt'

      return await method.apply(obj, params)
    } catch (error) {
      console.error(`Error calling ${methodPath}: ${error.message}`)
      throw error
    }
  }

  _initializeDefaultTests() {
    // Fallback tests used if a category's JSON file fails to load.
    // All categories now have JSON test definitions, so this is rarely invoked.
    this._defaultTests = {
      account: [],
      accessibility: [],
      advertising: [],
      device: [],
      discovery: [],
      display: [],
      lifecycle: [],
      localization: [],
      metrics: [],
      network: [],
      presentation: [],
      stats: [],
      texttospeech: [],
      mockstress: []
    }
  }

  // Returns raw test definitions for a specific category (synchronous — for test introspection).
  _getTestsForCategory(categoryId) {
    return TEST_DEFINITIONS_BY_CATEGORY[categoryId] || []
  }

  // Returns all raw test definitions annotated with _category (synchronous — for test introspection).
  _getAllTestDefinitions() {
    return Object.entries(TEST_DEFINITIONS_BY_CATEGORY).flatMap(([cat, defs]) =>
      (defs || []).map(d => Object.assign({}, d, { _category: cat }))
    )
  }

  async loadTestsForCategory(categoryId) {
    if (this._testDefinitions[categoryId]) {
      return this._testDefinitions[categoryId]
    }

    try {
      const testDefs = TEST_DEFINITIONS_BY_CATEGORY[categoryId]
      if (!testDefs || !Array.isArray(testDefs)) {
        return this._defaultTests[categoryId] || []
      }

      const tests = testDefs.map(def => this._createTestFromDefinition(def))
      this._testDefinitions[categoryId] = tests
      return tests
    } catch (error) {
      dbg(`Using default tests for category: ${categoryId}`)
      return this._defaultTests[categoryId] || []
    }
  }

  _createTestFromDefinition(def) {
    return {
      name: def.name,
      method: def.method || def.name,
      description: def.description,
      type: def.type,
      expectedType: def.expectedType,
      params: def.params || [],
      gatewayParams: def.gatewayParams || {},
      execute: async () => {
        try {
          let response

          if (def.type === 'gateway') {
            // Spec-defined method not yet in SDK package — raw WebSocket JSON-RPC
            response = await this._callViaGateway(def.method, def.gatewayParams || {})
            return { value: response, type: typeof response, backend: 'gateway' }

          } else if (def.type === 'gateway-event') {
            // Spec-defined event not in SDK — subscribe via raw WebSocket, wait for event to fire
            const gatewayValue = await this._subscribeViaGateway(def.method, 7500)
            const eventFired = gatewayValue !== null
              ? { fired: true, value: gatewayValue }
              : { fired: false }
            if (eventFired.fired) {
              return {
                value: eventFired.value,
                type: typeof eventFired.value === 'object' ? 'object' : typeof eventFired.value,
                backend: 'gateway'
              }
            }
            return {
              value: 'Subscribed — no event fired in test window',
              type: 'error',
              backend: 'gateway'
            }

          } else if (def.type === 'event') {
            // Event test: subscribe and wait for the event to fire within the test window
            const moduleName = def.method.split('.')[0]
            const eventValue = await this._subscribeToEvent(moduleName, def.listenEvent, 7500)
            const eventFired = eventValue !== null
              ? { fired: true, value: eventValue }
              : { fired: false }
            if (eventFired.fired) {
              return {
                value: eventFired.value,
                type: typeof eventFired.value === 'object' ? 'object' : typeof eventFired.value,
                backend: this._lastCallBackend
              }
            }
            return {
              value: 'Subscribed — no event fired in test window',
              type: 'error',
              backend: this._lastCallBackend
            }

          } else {
            // Default: getter/method call through the SDK or mock fallback chain
            response = await this._callMethod(def.method, def.params || [])
            return { value: response, type: typeof response, backend: this._lastCallBackend }
          }

        } catch (error) {
          return { value: error.message, type: 'error', backend: this._lastCallBackend || 'unknown' }
        }
      }
    }
  }

  _createMockFirebolt() {
    // Mirrors the actual @firebolt-js/core-client module API surface.
    // Used when neither the SDK package nor window.Firebolt is available.
    return {
      // Account: NOT in firebolt-js-client OpenRPC spec — mock only
      account: {
        id: () => Promise.resolve('mock-account-id'),
        uid: () => Promise.resolve('mock-account-uid')
      },
      // Accessibility module (SDK: Accessibility.audioDescription, closedCaptionsSettings, highContrastUI, voiceGuidanceSettings)
      Accessibility: {
        audioDescription: () => Promise.resolve(true),
        closedCaptionsSettings: () => Promise.resolve({ enabled: true, preferredLanguages: ['eng'] }),
        highContrastUI: () => Promise.resolve(false),
        voiceGuidanceSettings: () => Promise.resolve({ enabled: false, rate: 1.0, navigationHints: true }),
        listen: () => Promise.resolve(1),
        clear: () => true
      },
      // Advertising module (SDK: Advertising.advertisingId)
      Advertising: {
        advertisingId: () => Promise.resolve({ ifa: 'mock-ifa-uuid', ifa_type: 'sessionid', lmt: '0' })
      },
      // Device module (SDK: Device.deviceClass, Device.hdr, Device.uid)
      Device: {
        deviceClass: () => Promise.resolve('stb'),
        hdr: () => Promise.resolve({ hdr10: true, hdr10Plus: false, dolbyVision: false, hlg: true }),
        uid: () => Promise.resolve('mock-device-uid-12345'),
        listen: () => Promise.resolve(1),
        clear: () => true
      },
      // Discovery module (SDK: Discovery.watched)
      Discovery: {
        watched: (entityId, progress, completed, watchedOn, agePolicy) => Promise.resolve(true)
      },
      // Localization module (SDK: Localization.country, preferredAudioLanguages, presentationLanguage)
      Localization: {
        country: () => Promise.resolve('US'),
        preferredAudioLanguages: () => Promise.resolve(['eng', 'spa']),
        presentationLanguage: () => Promise.resolve('en-US'),
        listen: () => Promise.resolve(1),
        clear: () => true
      },
      // Metrics module (SDK: all Metrics.* methods)
      Metrics: {
        ready: () => Promise.resolve(true),
        page: (pageId) => Promise.resolve(true),
        appInfo: (build) => Promise.resolve(null),
        startContent: (entityId) => Promise.resolve(true),
        stopContent: (entityId) => Promise.resolve(true),
        error: (type, code, desc, visible) => Promise.resolve(true),
        event: (schema, data) => Promise.resolve(true),
        mediaLoadStart: (entityId) => Promise.resolve(true),
        mediaPlay: (entityId) => Promise.resolve(true),
        mediaPlaying: (entityId) => Promise.resolve(true),
        mediaPause: (entityId) => Promise.resolve(true),
        mediaWaiting: (entityId) => Promise.resolve(true),
        mediaSeeking: (entityId, target) => Promise.resolve(true),
        mediaSeeked: (entityId, position) => Promise.resolve(true),
        mediaRateChanged: (entityId, rate) => Promise.resolve(true),
        mediaRenditionChanged: (entityId, bitrate, width, height, profile) => Promise.resolve(true),
        mediaEnded: (entityId) => Promise.resolve(true)
      },
      // Network module (SDK: Network.connected)
      Network: {
        connected: () => Promise.resolve(true),
        listen: () => Promise.resolve(1),
        clear: () => true
      },
      // Mock stress-test module — all 35 synthetic methods
      mock: {
        stringValue: () => Promise.resolve('mock-string'),
        numberValue: () => Promise.resolve(42),
        booleanTrue: () => Promise.resolve(true),
        booleanFalse: () => Promise.resolve(false),
        objectPayload: () => Promise.resolve({ key: 'value', count: 1 }),
        arrayPayload: () => Promise.resolve(['alpha', 'beta', 'gamma']),
        nullValue: () => Promise.resolve(null),
        slowResponse: () => new Promise(r => setTimeout(() => r('slow-ok'), 200)),
        fastResponse: () => Promise.resolve('fast-ok'),
        versionString: () => Promise.resolve('1.2.3'),
        deviceId: () => Promise.resolve('mock-device-uuid-abcd-1234'),
        sessionToken: () => Promise.resolve('mock-session-token-xyz'),
        countryCode: () => Promise.resolve('US'),
        languageCode: () => Promise.resolve('en-US'),
        resolutionWidth: () => Promise.resolve(1920),
        resolutionHeight: () => Promise.resolve(1080),
        frameRate: () => Promise.resolve(60),
        hdrProfile: () => Promise.resolve('HDR10'),
        audioChannels: () => Promise.resolve(2),
        audioCodec: () => Promise.resolve('AAC'),
        networkType: () => Promise.resolve('ethernet'),
        ipAddress: () => Promise.resolve('192.168.1.100'),
        macAddress: () => Promise.resolve('AA:BB:CC:DD:EE:FF'),
        storageUsed: () => Promise.resolve(4294967296),
        storageFree: () => Promise.resolve(12884901888),
        memoryUsed: () => Promise.resolve(512),
        memoryFree: () => Promise.resolve(1536),
        cpuLoad: () => Promise.resolve(23),
        temperature: () => Promise.resolve(55),
        firmwareVersion: () => Promise.resolve('mock-fw-2.0.1'),
        buildTimestamp: () => Promise.resolve('2026-01-01T00:00:00Z'),
        appId: () => Promise.resolve('com.mock.testapp'),
        appVersion: () => Promise.resolve('0.1.0'),
        partnerId: () => Promise.resolve('mock-partner-001'),
        platformName: () => Promise.resolve('MockOS')
      }
    }
  }

  getTestsForCategory(categoryId) {
    return this.loadTestsForCategory(categoryId)
  }

  async getVersionInfo() {
    let deviceUid = 'UID-Error'
    try {
      let _timeoutId
      const timeout = new Promise((_, reject) => {
        _timeoutId = setTimeout(() => reject(new Error('timeout')), 3000)
      })
      deviceUid = await Promise.race([this._callMethod('Device.uid').finally(() => clearTimeout(_timeoutId)), timeout])
    } catch (_) {}

    return {
      sdkVersion: '8.0.0',
      deviceUid,
      raw: null
    }
  }

  async getConnectionStatus() {
    await this.init()

    const info = connectionInfo

    const displayHost = _hostFromEndpoint(info.endpoint)
    // Derive status from whether the SDK modules loaded — no probe socket opened.
    if (useNpmPackage) {
      return { state: 'connected', label: `Connected (${displayHost})`, endpoint: info.endpoint, backend: 'core-client' }
    }

    if (this.usingMock) {
      return { state: 'mock', label: 'Mock Mode', endpoint: info.endpoint, backend: 'mock' }
    }

    // window.Firebolt present but npm SDK not loaded — calls still route through window.Firebolt
    return { state: 'connected-legacy', label: `Connected (${displayHost})`, endpoint: info.endpoint, backend: 'window.firebolt' }
  }

  async runTest(test) {
    let _timeoutId
    try {
      this._lastCallBackend = 'unknown'
      const startTime = Date.now()
      const timeoutPromise = new Promise((_, reject) => {
        _timeoutId = setTimeout(() => reject(new Error('Test timeout (8s)')), 8000)
      })
      const debugParams = test.type === 'gateway'
        ? (test.gatewayParams || {})
        : (test.params || [])
      dbg(`[Firebolt] >> ${test.method} params=${JSON.stringify(debugParams)}`)
      const result = await Promise.race([test.execute(), timeoutPromise])
      clearTimeout(_timeoutId)
      dbg(`[Firebolt] << ${test.method} result=${JSON.stringify(result)}`)
      const duration = Date.now() - startTime
      const backend = result?.backend || this._lastCallBackend || (this.usingMock ? 'mock' : 'window.firebolt')
      const actualType = result.value === null ? 'null' : result.value === undefined ? 'undefined' : typeof result.value
      const expectedType = test.expectedType
      const specValidation = _validateAgainstJsSpec(test, result)
      if (specValidation) {
        return {
          test,
          success: specValidation.success,
          result,
          backend,
          duration,
          message: specValidation.message
        }
      }

      // For event/gateway-event tests, success is determined by subscription (no error), not payload type
      const typeOk = !expectedType || test.type === 'event' || test.type === 'gateway-event' || actualType === expectedType
      const gatewayPayloadError = test.type === 'gateway' && _looksLikeGatewayErrorPayload(result.value)
      const returned = _formatResultValue(result)
      const reason = _extractFailureReason(result.value)

      return {
        test,
        success: result.type !== 'error' && typeOk && !gatewayPayloadError,
        result,
        backend,
        duration,
        message: result.type === 'error' || gatewayPayloadError
          ? `Error\nReason: ${reason}\nReturned: ${returned}`
          : typeOk
            ? `Returned: ${returned} (${actualType})`
            : `Type mismatch\nReason: expected ${expectedType}, got ${actualType}\nReturned: ${returned}`
      }
    } catch (error) {
      clearTimeout(_timeoutId)
      return {
        test,
        success: false,
        backend: this._lastCallBackend || (this.usingMock ? 'mock' : 'window.firebolt'),
        error: error.message,
        message: `Error\nReason: ${error.message}\nReturned: null`
      }
    }
  }
}
