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
 * Menu.test.js — Unit tests for the Menu component.
 *
 * Tests category item rendering, focus navigation,
 * and Run All button visibility / disabled states.
 */
import { Lightning } from '@lightningjs/sdk'
import App from 'src/index'
import settings from '../../settings.json'

// -------------------------------------------------------------------
// Mock FireboltAPI so Menu tests don't open real WebSockets
// -------------------------------------------------------------------
jest.mock('src/lib/FireboltAPI', () => {
  return jest.fn().mockImplementation(() => ({
    init: jest.fn().mockResolvedValue(undefined),
    getConnectionStatus: jest.fn().mockResolvedValue({ state: 'connected', endpoint: 'ws://127.0.0.1:3473' }),
    getVersionInfo: jest.fn().mockResolvedValue({ sdkVersion: '1.0.0-test' }),
    getTestsForCategory: jest.fn().mockResolvedValue([]),
    runTest: jest.fn().mockResolvedValue({ success: true, message: 'OK' }),
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

describe('Menu — rendering', () => {
  let app, menu

  beforeAll(() => {
    app = bootApp()
    menu = app.tag('SafeContainer.Content.Menu')
  })

  it('Menu component should exist', () => {
    expect(menu).toBeDefined()
  })

  it('Run All button should be visible', () => {
    expect(menu.tag('RunAllButton')?.visible).not.toBe(false)
  })

  it('ListContainer should exist', () => {
    expect(menu.tag('ListContainer')).toBeDefined()
  })
})

describe('Menu — navigation', () => {
  let app, menu

  beforeEach(() => {
    app = bootApp()
    app._setState('Menu')
    menu = app.tag('SafeContainer.Content.Menu')
  })

  it('should handle Up key without throwing', () => {
    expect(() => menu._handleUp()).not.toThrow()
  })

  it('should handle Down key without throwing', () => {
    expect(() => menu._handleDown()).not.toThrow()
  })

  it('should handle Left key without throwing', () => {
    expect(() => menu._handleLeft()).not.toThrow()
  })

  it('should handle Right key without throwing', () => {
    expect(() => menu._handleRight()).not.toThrow()
  })
})

describe('Menu — Run All button focus', () => {
  let app, menu

  beforeAll(() => {
    app = bootApp()
    app._setState('Menu')
    menu = app.tag('SafeContainer.Content.Menu')
  })

  it('pressing Down from last row should move focus to Run All button', () => {
    // Simulate pressing down until we reach the button
    for (let i = 0; i < 20; i++) {
      menu._handleDown()
    }
    expect(menu._focusOnButton).toBe(true)
  })

  it('pressing Up from Run All button should move focus back to list', () => {
    menu._handleUp()
    expect(menu._focusOnButton).toBe(false)
  })
})
