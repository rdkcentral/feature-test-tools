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

import { Lightning } from '@lightningjs/sdk'
import Menu from './components/Menu'
import TestRunner from './components/TestRunner'
import ResultsPanel from './components/ResultsPanel'
import DimensionRuler from './components/DimensionRuler'
import AppSettings from './lib/AppSettings'
import FireboltAPI from './lib/FireboltAPI'
import settings from '../settings.json'

export default class App extends Lightning.Component {

  _construct() {
    const stageW = this.stage.w || window.innerWidth
    const stageH = this.stage.h || window.innerHeight
    AppSettings.initScale(stageW, stageH)
  }

  static _template() {
    return {
      Background: {
        w: w => w,
        h: h => h,
        rect: true,
        color: 0xff0f1419
      },
      SafeContainer: {
        Header: {
          x: 40,
          y: 40,
          HeaderBg: {
            x: -10,
            y: -10,
            w: 600,
            h: 100,
            rect: true,
            color: 0x20ffffff,
            shader: {
              type: Lightning.shaders.RoundedRectangle,
              radius: 12
            }
          },
          Title: {
            text: {
              text: ' Firebolt® API Test Tool',
              fontSize: 42,
              textColor: 0xffffffff,
              shadow: true,
              shadowColor: 0xff000000,
              shadowBlur: 10,
              shadowOffsetX: 2,
              shadowOffsetY: 2
            }
          },
          Subtitle: {
            x: 24,
            y: 60,
            text: {
              text: ' Loading...',
              fontSize: 20,
              textColor: 0xffa8b3cf
            }
          },
          ConnectionStatus: {
            x: 620,
            y: 10,
            Dot: {
              w: 14,
              h: 14,
              rect: true,
              color: 0xfff59e0b,
              shader: {
                type: Lightning.shaders.RoundedRectangle,
                radius: 7
              }
            },
            Label: {
              x: 24,
              y: -4,
              text: {
                text: ' Connecting...',
                fontSize: 20,
                textColor: 0xfff59e0b
              }
            }
          },
          Clock: {
            x: 620,
            y: 42,
            text: {
              text: '',
              fontSize: 18,
              textColor: 0xffa8b3cf
            }
          },
          HeaderGlow: {
            x: -12, y: -12, w: 624, h: 104, rect: true, color: 0x000097cc,
            shader: { type: Lightning.shaders.RoundedRectangle, radius: 14 }
          }
        },
        Legends: {
          x: 980,
          y: 680,
          w: 520,
          h: 150,
          LegendBg: {
          w: 520,
          h: 150,
          rect: true,
          color: 0x20ffffff,
          shader: {
            type: Lightning.shaders.RoundedRectangle,
            radius: 12
          }
        },
        NavTitle: {
          x: 20,
          y: 10,
          text: {
            text: 'Navigation:',
            fontSize: 18,
            textColor: 0xffa8b3cf
          }
        },
        NavInfo: {
          x: 20,
          y: 35,
          text: {
            text: 'Direction keys . Enter . Backspace',
            fontSize: 20,
            textColor: 0xffffffff
          }
        },
        ColorTitle: {
          x: 20,
          y: 80,
          text: {
            text: 'Pass Rate Graph:',
            fontSize: 18,
            textColor: 0xffa8b3cf
          }
        },
        GradientBar1: {
          x: 20,
          y: 110,
          w: 146,
          h: 30,
          rect: true,
          colorLeft: 0xff8b0000,
          colorRight: 0xffff4500,
          shader: {
            type: Lightning.shaders.RoundedRectangle,
            radius: [4, 0, 0, 4]
          }
        },
        GradientBar2: {
          x: 166,
          y: 110,
          w: 146,
          h: 30,
          rect: true,
          colorLeft: 0xffff4500,
          colorRight: 0xffffa500
        },
        GradientBar3: {
          x: 312,
          y: 110,
          w: 146,
          h: 30,
          rect: true,
          colorLeft: 0xffffa500,
          colorRight: 0xff006400,
          shader: {
            type: Lightning.shaders.RoundedRectangle,
            radius: [0, 4, 4, 0]
          }
        },
        Label0: {
          x: 20,
          y: 117,
          text: {
            text: '  0%',
            fontSize: 16,
            textColor: 0xffffffff
          }
        },
        Label100: {
          x: 460,
          y: 117,
          text: {
            text: '100%',
            fontSize: 16,
            textColor: 0xffffffff
          }
        }
      },
      APIConsole: {
        x: 0, y: 0, w: 100, h: 100,
        ConsoleBg: {
          w: 100, h: 100, rect: true, color: 0x15ffffff,
          shader: { type: Lightning.shaders.RoundedRectangle, radius: 12 }
        },
        ConsoleTitle: {
          x: 20, y: 12,
          text: { text: 'API Console', fontSize: 18, textColor: 0xffa8b3cf }
        },
        ConsoleViewport: {
          x: 0, y: 42, w: 100, h: 100, clipping: true, rect: true, color: 0x00000000,
          ConsoleLines: { x: 12, y: 0 }
        }
      },
      Content: {
        y: 250,
        x: 40,
        clipping: false,
        Menu: {
          type: Menu,
          signals: {
            onSelect: '_onMenuSelect',
            onStatus: '_onTestRunnerStatus'
          }
        },
        TestRunner: {
          x: 0,
          type: TestRunner,
          visible: false,
          signals: {
            onTestComplete: '_onTestComplete',
            onStatus: '_onTestRunnerStatus'
          }
        },
        ResultsPanel: {
          x: 0,
          type: ResultsPanel,
          visible: false
        }
      }
      },
      DimensionRuler: {
        type: DimensionRuler,
        visible: false
      },
      ExitDialog: {
        visible: false,
        zIndex: 100,
        x: 0, y: 0,
        Overlay: {
          w: 1920, h: 1080, rect: true, color: 0xaa000000
        },
        Box: {
          x: 660, y: 390,
          w: 600, h: 260,
          rect: true, color: 0xff1e2936,
          shader: { type: Lightning.shaders.RoundedRectangle, radius: 16 },
          Title: {
            mount: 0.5, x: 300, y: 52,
            text: { text: 'Exit App?', fontSize: 36, textColor: 0xffffffff }
          },
          Message: {
            mount: 0.5, x: 300, y: 104,
            text: { text: 'Are you sure you want to exit?', fontSize: 22, textColor: 0xffa8b3cf }
          },
          BtnYes: {
            x: 60, y: 158, w: 200, h: 60, rect: true,
            color: 0xffef4444,
            shader: { type: Lightning.shaders.RoundedRectangle, radius: 10 },
            Label: {
              mount: 0.5, x: 100, y: 30,
              text: { text: 'Yes, Exit', fontSize: 22, textColor: 0xffffffff }
            }
          },
          BtnNo: {
            x: 340, y: 158, w: 200, h: 60, rect: true,
            color: 0xff2a3f5f,
            shader: { type: Lightning.shaders.RoundedRectangle, radius: 10 },
            Label: {
              mount: 0.5, x: 100, y: 30,
              text: { text: 'No, Stay', fontSize: 22, textColor: 0xffffffff }
            }
          }
        }
      }
    }
  }

  _setup() {
    const { layout, colors, typography, safeArea } = AppSettings

    this.tag('Background').patch({
      color: parseInt(colors.background, 16)
    })

    this.tag('SafeContainer').patch({
      x: safeArea.offsetX,
      y: safeArea.offsetY
    })

    this.tag('SafeContainer.Header').patch({
      x: layout.padding.left,
      y: layout.padding.top
    })

    const headerW = Math.round(safeArea.width - layout.padding.left - layout.padding.right)
    this.tag('SafeContainer.Header.HeaderBg').patch({
      w: headerW + 20,
      color: parseInt(colors.titleBoxBgColor, 16),
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        radius: 12,
        stroke: 2,
        strokeColor: parseInt(colors.titleBoxBorderColor, 16)
      }
    })
    this.tag('SafeContainer.Header.HeaderGlow').patch({ w: headerW + 24 })
    this.tag('SafeContainer.Header.ConnectionStatus').patch({ x: headerW - 240 })
    this.tag('SafeContainer.Header.Clock').patch({ x: headerW - 240 })

    const warningColor = parseInt(colors.warning, 16)
    this.tag('SafeContainer.Header.ConnectionStatus.Dot').patch({ color: warningColor })
    this.tag('SafeContainer.Header.ConnectionStatus.Label').patch({
      text: { textColor: warningColor }
    })

    this._startHeaderGlow()

    this.tag('SafeContainer.Header.Title').patch({
      text: {
        text: ` Firebolt® API Test Tool`,
        fontSize: typography.title.fontSize,
        textColor: parseInt(colors.titleTextColor, 16)
      }
    })

    this.tag('SafeContainer.Header.Subtitle').patch({
      text: {
        fontSize: (typography.headerSubtitle || typography.subtitle).fontSize,
        textColor: parseInt(colors.subtitleTextColor, 16)
      }
    })

    this.tag('SafeContainer.Content').patch({
      y: layout.content.offsetY,
      x: layout.padding.left
    })

    this._fireboltAPI = new FireboltAPI()
    this._fireboltAPI.onEventLog = (label, payload) => this._addEventLogEntry(label, payload)
    this._connectionStatusTimer = null

    this._updateFireboltVersion()
    this._startConnectionMonitor()
    this._startClock()

    this.tag('SafeContainer.Legends').patch({
      zIndex: 10,
      x: layout.legends.offsetX,
      y: layout.legends.offsetY,
      w: layout.legends.width,
      h: layout.legends.height
    })
    const s0 = parseInt(colors.passRateStop0, 16)
    const s1 = parseInt(colors.passRateStop1, 16)
    const s2 = parseInt(colors.passRateStop2, 16)
    const s3 = parseInt(colors.passRateStop3, 16)
    this.tag('SafeContainer.Legends.GradientBar1').patch({ colorLeft: s0, colorRight: s1 })
    this.tag('SafeContainer.Legends.GradientBar2').patch({ colorLeft: s1, colorRight: s2 })
    this.tag('SafeContainer.Legends.GradientBar3').patch({ colorLeft: s2, colorRight: s3 })

    const consoleX = layout.padding.left
    const consoleY = layout.legends.offsetY
    const consoleW = layout.legends.offsetX - layout.padding.left - 16
    const consoleH = layout.legends.height
    this.tag('SafeContainer.APIConsole').patch({ x: consoleX, y: consoleY, w: consoleW, h: consoleH })
    this.tag('SafeContainer.APIConsole.ConsoleBg').patch({ w: consoleW, h: consoleH })

    const viewportH = consoleH - 44
    this.tag('SafeContainer.APIConsole.ConsoleViewport').patch({ w: consoleW, h: viewportH })
    this._consoleViewportH = viewportH
    this._consoleLineW = consoleW - 28
    this._consoleLineH = 48
    this._consoleEntries = []

    // Enable/disable dimension ruler based on settings
    if (AppSettings.debug.showDimensionRuler) {
      this.tag('DimensionRuler').visible = true
      this.tag('DimensionRuler').setSafeArea(safeArea)
    }

  }

  async _updateFireboltVersion() {
    try {
      if (!this._fireboltAPI) {
        this.tag('SafeContainer.Header.Subtitle').patch({
          text: { text: 'Firebolt SDK (Unavailable)' }
        })
        return
      }

      const status = await this._fireboltAPI.getConnectionStatus()
      const versionInfo = await this._fireboltAPI.getVersionInfo()
      const target = status.endpoint ? status.endpoint.replace('ws://', '') : 'unknown-target'

      if (versionInfo.sdkVersion) {
        this.tag('SafeContainer.Header.Subtitle').patch({
          text: { text: `Firebolt SDK v${versionInfo.sdkVersion} | ${target}` }
        })
      } else if (status.state === 'disconnected') {
        this.tag('SafeContainer.Header.Subtitle').patch({
          text: { text: `Firebolt SDK (Disconnected) | ${target}` }
        })
      } else if (status.state === 'mock') {
        this.tag('SafeContainer.Header.Subtitle').patch({
          text: { text: `Firebolt SDK (Mock Mode) | ${target}` }
        })
      } else {
        this.tag('SafeContainer.Header.Subtitle').patch({
          text: { text: `Firebolt SDK (Connected) | ${target}` }
        })
      }
    } catch (error) {
      this.tag('SafeContainer.Header.Subtitle').patch({
        text: { text: 'Firebolt SDK (Status Unknown)' }
      })
    }
  }

  _startHeaderGlow() {
    const { colors } = AppSettings
    const colorOff = parseInt(colors.headerGlowColorOff, 16)
    const colorOn = parseInt(colors.headerGlowColorOn, 16)
    const glow = this.tag('SafeContainer.Header.HeaderGlow')
    const pulse = () => {
      glow.patch({ smooth: { color: [colorOn, { duration: 1.8, timingFunction: 'ease-in-out' }] } })
      this._glowTimer = setTimeout(() => {
        glow.patch({ smooth: { color: [colorOff, { duration: 1.8, timingFunction: 'ease-in-out' }] } })
        this._glowTimer = setTimeout(pulse, 1900)
      }, 1900)
    }
    pulse()
  }

  _startClock() {
    const version = settings.appSettings && settings.appSettings.version ? ` v${settings.appSettings.version}` : ''
    const update = () => {
      const now = new Date()
      const h = String(now.getUTCHours()).padStart(2, '0')
      const m = String(now.getUTCMinutes()).padStart(2, '0')
      const s = String(now.getUTCSeconds()).padStart(2, '0')
      const ms = String(now.getUTCMilliseconds()).padStart(3, '0')
      this.tag('SafeContainer.Header.Clock').patch({
        text: { text: `${version}   ${h}:${m}:${s}.${ms} UTC` }
      })
    }
    update()
    this._clockTimer = setInterval(update, 50)
  }

  _startConnectionMonitor() {
    this._updateConnectionStatus()

    if (this._connectionStatusTimer) {
      clearInterval(this._connectionStatusTimer)
    }

    this._connectionStatusTimer = setInterval(() => {
      this._updateConnectionStatus()
    }, 5000)
  }

  _stopConnectionMonitor() {
    if (this._connectionStatusTimer) {
      clearInterval(this._connectionStatusTimer)
      this._connectionStatusTimer = null
    }
  }

  _detach() {
    this._stopConnectionMonitor()
    if (this._clockTimer) {
      clearInterval(this._clockTimer)
      this._clockTimer = null
    }
    if (this._glowTimer) {
      clearTimeout(this._glowTimer)
      this._glowTimer = null
    }
    if (this._fireboltAPI) {
      this._fireboltAPI.unsubscribeAllEvents()
    }
  }

  async _updateConnectionStatus() {
    if (!this._fireboltAPI) {
      return
    }

    try {
      const status = await this._fireboltAPI.getConnectionStatus()

      const { colors } = AppSettings
      let color = parseInt(colors.warning, 16)
      if (status.state === 'connected') {
        color = parseInt(colors.connectionStatusConnected, 16)
      } else if (status.state === 'disconnected') {
        color = parseInt(colors.error, 16)
      }

      this.tag('SafeContainer.Header.ConnectionStatus.Dot').patch({ color })
      this.tag('SafeContainer.Header.ConnectionStatus.Label').patch({
        text: {
          text: status.label,
          textColor: color
        }
      })
    } catch (error) {
      const color = parseInt(AppSettings.colors.error, 16)
      this.tag('SafeContainer.Header.ConnectionStatus.Dot').patch({ color })
      this.tag('SafeContainer.Header.ConnectionStatus.Label').patch({
        text: {
          text: 'Status Check Failed',
          textColor: color
        }
      })
    }
  }

  _init() {
    this._setState('Menu')
    if (this._fireboltAPI) {
      this._fireboltAPI.subscribeAllEvents().catch((e) => {
        console.error('subscribeAllEvents failed: ' + (e && e.message ? e.message : JSON.stringify(e)))
      })
    }
  }

  // Allow only directional keys, Enter, and Backspace. All other keys are consumed here.
  _captureKey(e) {
    const ALLOWED = new Set([
      37, 38, 39, 40,  // ArrowLeft, ArrowUp, ArrowRight, ArrowDown
      13,              // Enter
      8                // Backspace
    ])
    return !ALLOWED.has(e.keyCode)
  }

  _onMenuSelect(category) {
    this.tag('SafeContainer.Content.TestRunner').loadTests(category)
    this._setState('TestRunner')
  }

  _onTestComplete(results) {
    this.tag('SafeContainer.Content.ResultsPanel').showResults(results)
    this._setState('Results')
  }

  _onTestRunnerStatus(data) {
    if (data && typeof data === 'object' && data.isResponse) {
      this._addConsoleEntry(data)
    } else if (typeof data === 'string' && data) {
      this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = data
    }
  }

  _addConsoleEntry(data) {
    const { colors } = AppSettings
    const i = this._consoleEntries.length
    const lineH = this._consoleLineH
    const wrapW = this._consoleLineW
    const lines = this.tag('SafeContainer.APIConsole.ConsoleViewport.ConsoleLines')
    const child = lines.stage.c({
      ref: `CLEntry${i}`,
      y: i * lineH,
      w: wrapW,
      h: lineH,
      Req: {
        x: 0, y: 1,
        text: {
          text: `Request:  ${data.method}`,
          fontSize: 15,
          textColor: parseInt(colors.textSecondary, 16),
          wordWrapWidth: wrapW
        }
      },
      Res: {
        x: 0, y: 23,
        text: {
          text: `Response: ${data.response}`,
          fontSize: 15,
          textColor: data.success ? parseInt(colors.success, 16) : parseInt(colors.error, 16),
          wordWrapWidth: wrapW
        }
      }
    })
    lines.childList.add(child)
    this._consoleEntries.push(data)
    this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = 'API Console'
    const totalH = this._consoleEntries.length * lineH
    const maxScroll = Math.max(0, totalH - this._consoleViewportH)
    if (maxScroll > 0) {
      lines.setSmooth('y', -maxScroll, { duration: 0.2 })
    }
  }

  _addEventLogEntry(label, payload) {
    const { colors } = AppSettings
    const i = this._consoleEntries.length
    const lineH = this._consoleLineH
    const wrapW = this._consoleLineW
    const lines = this.tag('SafeContainer.APIConsole.ConsoleViewport.ConsoleLines')
    const child = lines.stage.c({
      ref: `CLEntry${i}`,
      y: i * lineH,
      w: wrapW,
      h: lineH,
      Req: {
        x: 0, y: 1,
        text: {
          text: `Event:   ${label}`,
          fontSize: 15,
          textColor: parseInt(colors.primary, 16),
          wordWrapWidth: wrapW
        }
      },
      Res: {
        x: 0, y: 23,
        text: {
          text: `Payload: ${payload}`,
          fontSize: 15,
          textColor: parseInt(colors.textSecondary, 16),
          wordWrapWidth: wrapW
        }
      }
    })
    lines.childList.add(child)
    this._consoleEntries.push({ label, payload, isEvent: true })
    this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = 'API Console'
    const totalH = this._consoleEntries.length * lineH
    const maxScroll = Math.max(0, totalH - this._consoleViewportH)
    if (maxScroll > 0) {
      lines.setSmooth('y', -maxScroll, { duration: 0.2 })
    }
  }

  _clearConsole() {
    if (!this._consoleEntries) { this._consoleEntries = []; return }
    this._consoleEntries = []
    const lines = this.tag('SafeContainer.APIConsole.ConsoleViewport.ConsoleLines')
    lines.children = []
    lines.patch({ y: 0 })
    this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = 'API Console'
  }

  static _states() {
    return [
      class Menu extends this {
        _getFocused() {
          return this.tag('SafeContainer.Content.Menu')
        }
        _handleBack() {
          this._setState('ExitConfirm')
          return true
        }
        $enter() {
          this.tag('SafeContainer.Content.Menu').visible = true
          this.tag('SafeContainer.Content.TestRunner').visible = false
          this.tag('SafeContainer.Content.ResultsPanel').visible = false
          this.tag('SafeContainer.Legends').visible = true
          this._clearConsole()
        }
      },
      class TestRunner extends this {
        _getFocused() {
          return this.tag('SafeContainer.Content.TestRunner')
        }
        _handleBack() {
          // Block back navigation if tests are running
          const testRunner = this.tag('SafeContainer.Content.TestRunner')
          if (testRunner._isRunning) {
            return true  // Consume key — do NOT propagate; returning false closes the app
          }
          this._setState('Menu')
          return true
        }
        $enter() {
          this.tag('SafeContainer.Content.Menu').visible = false
          this.tag('SafeContainer.Content.TestRunner').visible = true
          this.tag('SafeContainer.Content.ResultsPanel').visible = false
          this.tag('SafeContainer.Legends').visible = true
        }
      },
      class Results extends this {
        _getFocused() {
          return this.tag('SafeContainer.Content.ResultsPanel')
        }
        _handleBack() {
          this._setState('Menu')
          return true
        }
        $enter() {
          this.tag('SafeContainer.Content.Menu').visible = false
          this.tag('SafeContainer.Content.TestRunner').visible = false
          this.tag('SafeContainer.Content.ResultsPanel').visible = true
          this.tag('SafeContainer.Legends').visible = true
        }
      },
      class ExitConfirm extends this {
        _getFocused() { return this }
        $enter() {
          this._exitFocusYes = true
          this.tag('ExitDialog').visible = true
          this._highlightExitBtn()
        }
        $exit() {
          this.tag('ExitDialog').visible = false
        }
        _handleLeft() {
          this._exitFocusYes = true
          this._highlightExitBtn()
          return true
        }
        _handleRight() {
          this._exitFocusYes = false
          this._highlightExitBtn()
          return true
        }
        _handleEnter() {
          if (this._exitFocusYes) {
            this.application.closeApp()
            window.close()
          } else {
            this._setState('Menu')
          }
          return true
        }
        _handleBack() {
          this._setState('Menu')
          return true
        }
        _highlightExitBtn() {
          const yes = this._exitFocusYes
          this.tag('ExitDialog.Box.BtnYes').patch({ color: yes ? 0xffef4444 : 0xff2a3f5f })
          this.tag('ExitDialog.Box.BtnNo').patch({ color: yes ? 0xff2a3f5f : 0xff00aa55 })
        }
      }
    ]
  }
}
