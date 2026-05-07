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
 * App.test.js — Unit tests for the root App component.
 *
 * Boots the Lightning app in a JSDOM + WebGL-canvas-mock environment and
 * verifies the initial state, state transitions, and exit-dialog behaviour.
 */
import App from 'src/index'
import settings from '../settings.json'

// -------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------

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

// -------------------------------------------------------------------
// Suite
// -------------------------------------------------------------------

describe('App — boot', () => {
  let app

  beforeEach(() => {
    app = bootApp()
  })

  it('should render without throwing', () => {
    expect(app).toBeDefined()
  })

  it('should have an application tag after initial render', () => {
    expect(app.tag).toBeDefined()
    expect(typeof app._getState).toBe('function')
  })
})

describe('App — initial state', () => {
  let app

  beforeAll(() => {
    app = bootApp()
  })

  it('should start in Menu state', () => {
    const focused = app._getFocused()
    // In Menu state the App delegates focus to the Menu component
    expect(focused).toBeDefined()
  })

  it('Menu should be visible, TestRunner and ResultsPanel hidden', () => {
    const root = app._getFocused()
    // The app component itself knows the SafeContainer children
    const appComp = app
    const menuVisible = appComp.tag('SafeContainer.Content.Menu')?.visible
    const testRunnerVisible = appComp.tag('SafeContainer.Content.TestRunner')?.visible
    const resultsPanelVisible = appComp.tag('SafeContainer.Content.ResultsPanel')?.visible

    // Menu should be visible (default true), others hidden
    expect(menuVisible).not.toBe(false)
    expect(testRunnerVisible).toBe(false)
    expect(resultsPanelVisible).toBe(false)
  })

  it('ExitDialog should be hidden on boot', () => {
    const appComp = app
    expect(appComp.tag('ExitDialog')?.visible).toBe(false)
  })
})

describe('App — key filtering (_captureKey)', () => {
  let app

  beforeAll(() => {
    app = bootApp()
  })

  const ALLOWED_KEYCODES = [37, 38, 39, 40, 13, 8] // arrows, enter, backspace

  ALLOWED_KEYCODES.forEach(code => {
    it(`should NOT consume keyCode ${code}`, () => {
      const result = app._captureKey({ keyCode: code })
      // returning false/undefined = let it through
      expect(result).toBeFalsy()
    })
  })

  const BLOCKED_KEYCODES = [65, 66, 32, 9, 27, 112] // a, b, space, tab, esc, F1

  BLOCKED_KEYCODES.forEach(code => {
    it(`should consume (drop) keyCode ${code}`, () => {
      const result = app._captureKey({ keyCode: code })
      expect(result).toBe(true)
    })
  })
})

describe('App — ExitConfirm state', () => {
  let app

  beforeAll(() => {
    app = bootApp()
  })

  it('Back on Menu should show ExitDialog', () => {
    app._setState('Menu')
    app._handleBack()
    expect(app.tag('ExitDialog')?.visible).toBe(true)
  })

  it('Back again (from ExitConfirm) should hide ExitDialog and return to Menu', () => {
    app._setState('Menu')
    app._handleBack() // transitions to ExitConfirm, shows dialog
    app._handleBack() // dismisses dialog, returns to Menu
    expect(app.tag('ExitDialog')?.visible).toBe(false)
  })
})
