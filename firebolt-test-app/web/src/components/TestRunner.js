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
import FireboltAPI from '../lib/FireboltAPI'
import AppSettings from '../lib/AppSettings'

export default class TestRunner extends Lightning.Component {
  static _template() {
    return {
      CategoryTitle: {
        text: {
          fontSize: 36,
          textColor: 0xffffffff
        }
      },
      CategoryDescription: {
        y: 44,
        text: {
          fontSize: 20,
          textColor: 0xff6b7785,
          fontStyle: 'italic'
        }
      },
      ProgressBar: {
        visible: true,
        ProgressBg: {
          w: 1200,
          h: 8,
          rect: true,
          color: 0xff2a3f5f,
          shader: {
            type: Lightning.shaders.RoundedRectangle,
            radius: 4
          }
        },
        ProgressFill: {
          w: 0,
          h: 8,
          rect: true,
          color: 0xff2a3f5f,
          shader: {
            type: Lightning.shaders.RoundedRectangle,
            radius: 4
          }
        },
        ProgressText: {
          y: 20,
          text: {
            text: '',
            fontSize: 18,
            textColor: 0xffa8b3cf
          }
        }
      },
      TestListContainer: {
        y: 110,
        w: 100,
        h: 100,
        rect: true,
        color: 0x00000000,
        shader: {
          type: Lightning.shaders.RoundedRectangle,
          stroke: 2,
          strokeColor: 0xff2a3f5f,
          radius: 8
        },
        clipping: true,
        TestListContent: {
          x: 10,
          y: 10
        }
      },
      RunButton: {
        y: 0,
        w: 200,
        h: 50,
        rect: true,
        color: 0xff00aa00,
        Label: {
          mount: 0.5,
          x: 100,
          y: 25,
          text: {
            text: 'Run All Tests',
            fontSize: 24,
            textColor: 0xffffffff
          }
        },
        PressedOverlay: {
          x: 0, y: 0, w: 200, h: 50, rect: true, color: 0x55000000, alpha: 0,
          shader: { type: Lightning.shaders.RoundedRectangle, radius: 8 }
        },
        GlowRing: {
          x: 0, y: 0, w: 200, h: 50, rect: true, color: 0x00000000, alpha: 0,
          shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: 0xff00d9ff, radius: 8 }
        }
      }
    }
  }

  _setup() {
    const { testRunner } = AppSettings.components
    const { colors, typography } = AppSettings
    const { width: contentW, maxH } = AppSettings.contentArea

    // Header: title (left) + description (right, same row) + progress bar below
    const titleW = Math.floor(contentW * 0.48)
    const descX = titleW + 24
    const descW = contentW - descX
    const descY = Math.round((typography.heading.fontSize - typography.bodySmall.fontSize) / 2)
    const progressBarY = typography.heading.fontSize + 14
    // headerH = top of ProgressText + ProgressText height + gap
    const headerH = progressBarY + 20 + typography.bodySmall.fontSize + 12

    const buttonH = testRunner.buttonHeight
    const bottomH = buttonH + 20
    const containerH = maxH - headerH - bottomH
    const buttonY = headerH + containerH + 10

    // Store for use in run methods
    this._containerW = contentW
    this._containerH = containerH
    this._buttonY = buttonY
    this._columnWidth = Math.floor(contentW / 2) - 10

    this.tag('CategoryTitle').patch({
      text: {
        fontSize: typography.heading.fontSize,
        textColor: parseInt(colors.primary, 16),
        wordWrapWidth: titleW
      }
    })

    this.tag('CategoryDescription').patch({
      x: descX,
      y: descY,
      text: {
        fontSize: typography.bodySmall.fontSize,
        textColor: parseInt(colors.textTertiary, 16),
        wordWrapWidth: descW
      }
    })
    this.tag('ProgressBar').patch({
      y: progressBarY,
      ProgressBg: {
        w: contentW,
        color: parseInt(colors.focusedBackground, 16)
      },
      ProgressFill: {
        color: parseInt(colors.primary, 16)
      },
      ProgressText: {
        text: {
          fontSize: typography.bodySmall.fontSize,
          textColor: parseInt(colors.textSecondary, 16)
        }
      }
    })

    this.tag('TestListContainer').patch({
      y: headerH,
      w: contentW,
      h: containerH
    })
    const { top: btnTop, bottom: btnBottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('RunButton').patch({
      y: buttonY,
      w: testRunner.buttonWidth,
      h: buttonH,
      colorTop: btnTop,
      colorBottom: btnBottom,
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        radius: 8
      }
    })

    this.tag('RunButton.Label').patch({
      x: testRunner.buttonWidth / 2,
      y: buttonH / 2,
      text: {
        fontSize: typography.subtitle.fontSize,
        textColor: parseInt(colors.textPrimary, 16)
      }
    })

    this.tag('RunButton.PressedOverlay').patch({ w: testRunner.buttonWidth, h: buttonH, color: parseInt(colors.bevelPressedOverlay, 16) })
    this.tag('RunButton.GlowRing').patch({ w: testRunner.buttonWidth, h: buttonH, shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: parseInt(colors.focusGlowColor, 16), radius: 8 } })
  }

  _init() {
    this._tests = []
    this._selectedTestIndex = 0
    this._selectedColumn = 0
    this._selectedRow = 0
    this._fireboltAPI = new FireboltAPI()
    this._focusOnButton = false
    this._isRunning = false
    this._navigationTimeout = null
    this._loadRequestId = 0
    // NOTE: _containerW, _containerH, _buttonY, _columnWidth, _maxRows are
    // computed in _setup() which runs before _init() — do not reset them here.
  }

  _active() {
    this._focusOnButton = false
    this._isRunning = false
    this._selectedTestIndex = 0
    this._selectedColumn = 0
    this._selectedRow = 0
    this._unfocusButton()

    if (this._tests.length > 0) {
      this._focusItem(0)
    }
  }

  loadTests(category) {
    const requestId = ++this._loadRequestId
    this._category = category
    this._focusOnButton = false
    this._selectedTestIndex = 0
    this._isRunning = false

    if (this._navigationTimeout) {
      clearTimeout(this._navigationTimeout)
      this._navigationTimeout = null
    }

    if (this._progressResetTimer) {
      clearTimeout(this._progressResetTimer)
      this._progressResetTimer = null
    }

    // Reset progress bar to grey (inactive state)
    const { colors } = AppSettings
    this.tag('ProgressBar.ProgressFill').patch({
      w: 0,
      color: parseInt(colors.focusedBackground, 16)
    })
    this.tag('ProgressBar.ProgressText').text.text = ''

    this.tag('CategoryTitle').text.text = `${category.name} APIs`
    this.tag('CategoryDescription').text.text = category.description || ''

    this.signal('onStatus', 'Loading tests...')
    this._tests = []
    this._createTestList()

    this._fireboltAPI.getTestsForCategory(category.id).then(tests => {
      if (requestId !== this._loadRequestId || !this._category || this._category.id !== category.id) {
        return
      }
      this._tests = tests.filter(t => t.type !== 'event' && t.type !== 'gateway-event')
      this._createTestList()
      this.signal('onStatus', `${this._tests.length} tests available`)
      if (this._tests.length > 0) {
        this._selectedTestIndex = 0
        this._focusItem(0)
      }
    }).catch(error => {
      if (requestId !== this._loadRequestId || !this._category || this._category.id !== category.id) {
        return
      }
      console.error('Error loading tests:', error?.message || String(error))
      this._tests = []
      this._createTestList()
      this.signal('onStatus', 'No tests available')
    })

    this._clearAllResults()
  }

  _createTestList() {
    const spacing = AppSettings.components.testRunner.testItemSpacing
    const columnWidth = this._columnWidth || 740
    this._maxRows = Math.max(1, Math.floor((this._containerH - 20) / spacing))

    const items = []
    for (let i = 0; i < this._tests.length; i++) {
      const col = Math.floor(i / this._maxRows)
      const row = i % this._maxRows
      items.push({
        ref: `Test${i}`,
        x: col * columnWidth,
        y: row * spacing,
        TestItem: {
          type: TestItem,
          test: this._tests[i],
          isFocused: false
        }
      })
    }
    this.tag('TestListContainer.TestListContent').children = items

    this._selectedColumn = 0
    this._selectedRow = 0
    this._updateScroll()

    if (this._tests.length > 0) {
      this._selectedTestIndex = 0
      this._focusItem(0)
    }
  }

  _handleUp() {
    if (this._isRunning) return

    if (this._focusOnButton) {
      this._focusOnButton = false
      this._unfocusButton()
      const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows)
      this._selectedRow = colLen - 1
      this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow
      this._focusItem(this._selectedTestIndex)
    } else if (this._selectedRow > 0) {
      this._unfocusItem(this._selectedTestIndex)
      this._selectedRow--
      this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow
      this._focusItem(this._selectedTestIndex)
    }
  }

  _handleDown() {
    if (this._isRunning) return
    if (this._focusOnButton) return

    const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows)
    if (this._selectedRow < colLen - 1) {
      this._unfocusItem(this._selectedTestIndex)
      this._selectedRow++
      this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow
      this._focusItem(this._selectedTestIndex)
    } else if (this._selectedColumn === 0) {
      this._unfocusItem(this._selectedTestIndex)
      this._focusOnButton = true
      this._focusButton()
    }
  }

  _handleLeft() {
    if (this._isRunning || this._focusOnButton) return
    if (this._selectedColumn > 0) {
      this._unfocusItem(this._selectedTestIndex)
      this._selectedColumn--
      const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows)
      if (this._selectedRow > colLen - 1) this._selectedRow = colLen - 1
      this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow
      this._focusItem(this._selectedTestIndex)
      this._updateScroll()
    }
  }

  _handleRight() {
    if (this._isRunning || this._focusOnButton) return
    const totalColumns = Math.ceil(this._tests.length / this._maxRows)
    if (this._selectedColumn < totalColumns - 1) {
      this._unfocusItem(this._selectedTestIndex)
      this._selectedColumn++
      const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows)
      if (this._selectedRow > colLen - 1) this._selectedRow = colLen - 1
      this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow
      this._focusItem(this._selectedTestIndex)
      this._updateScroll()
    }
  }

  _updateScroll() {
    const visibleColumns = 2
    const columnWidth = this._columnWidth || 740
    let contentX = 10
    if (this._selectedColumn >= visibleColumns) {
      contentX = 10 - (this._selectedColumn - visibleColumns + 1) * columnWidth
    }
    this.tag('TestListContainer.TestListContent').setSmooth('x', contentX, { duration: 0.3 })
  }

  _handleEnter() {
    if (this._isRunning) return
    if (this._focusOnButton) {
      this._pressButtonAnimation(() => this._runAllTests())
    } else {
      this._runSingleTest(this._selectedTestIndex)
    }
  }

  async _runSingleTest(index) {
    if (this._isRunning) return
    this._isRunning = true

    if (this._progressResetTimer) {
      clearTimeout(this._progressResetTimer)
      this._progressResetTimer = null
    }

    const test = this._tests[index]
    const { colors } = AppSettings

    if (test && test.id === 'metrics_appInfo' && typeof window !== 'undefined' && typeof window.prompt === 'function') {
      const defaultValue = Array.isArray(test.params) && test.params.length > 0 ? String(test.params[0]) : ''
      const inputValue = window.prompt('Enter Metrics.appInfo build/version value:', defaultValue)
      if (inputValue === null) {
        this._isRunning = false
        this.signal('onStatus', 'Cancelled: Metrics.appInfo input not provided')
        return
      }
      test.runtimeParams = [inputValue]
    }

    this.tag('ProgressBar.ProgressFill').patch({ color: parseInt(colors.primary, 16), w: 0 })
    this.tag('ProgressBar.ProgressText').text.text = `Running: ${test.name}...`
    this.signal('onStatus', `Running: ${test.name}...`)

    const result = await this._fireboltAPI.runTest(test)

    // Fill progress bar only after result is received
    this.tag('ProgressBar.ProgressFill').patch({
      smooth: { w: [this._containerW, { duration: 0.3 }] }
    })

    const child = this.tag('TestListContainer.TestListContent').children[index]
    if (child) child.tag('TestItem').updateResult(result)

    this.signal('onStatus', { isResponse: true, method: test.method, name: test.name, response: result.message, success: result.success })
    this.tag('ProgressBar.ProgressText').text.text = result.success ? `\u25cf Passed` : `\u25cf Failed`

    this._isRunning = false

    // Reset progress bar after 2s
    if (this._progressResetTimer) clearTimeout(this._progressResetTimer)
    this._progressResetTimer = setTimeout(() => {
      this._progressResetTimer = null
      this.tag('ProgressBar.ProgressFill').patch({
        smooth: { w: [0, { duration: 0.3 }] },
        color: parseInt(colors.focusedBackground, 16)
      })
      this.tag('ProgressBar.ProgressText').text.text = ''
    }, 2000)
  }

  async _runAllTests() {
    if (this._isRunning) return
    this._isRunning = true
    const { colors } = AppSettings

    // Visually disable button during execution
    const { top: runTop, bottom: runBottom } = AppSettings.embossColors(parseInt(colors.focusedBackground, 16))
    this.tag('RunButton').patch({
      colorTop: runTop,
      colorBottom: runBottom,
      Label: { text: { text: 'Running...', textColor: parseInt(colors.textSecondary, 16) } }
    })

    // Change progress bar to cyan when active
    this.tag('ProgressBar.ProgressFill').patch({
      color: parseInt(colors.primary, 16)
    })

    this.signal('onStatus', 'Running all tests...')
    const results = []
    const totalTests = this._tests.length
    const progressBarWidth = this._containerW

    for (let i = 0; i < totalTests; i++) {
      const test = this._tests[i]
      const progress = ((i + 1) / totalTests) * 100

      this.signal('onStatus', `Running ${i + 1}/${totalTests}: ${test.name}...`)

      // Scroll to keep the running column visible
      const runningCol = Math.floor(i / this._maxRows)
      if (runningCol !== this._selectedColumn) {
        this._selectedColumn = runningCol
        this._updateScroll()
      }

      const result = await this._fireboltAPI.runTest(test)
      results.push(result)

      // Update progress bar only after result is received
      this.tag('ProgressBar.ProgressFill').patch({
        smooth: { w: [(progressBarWidth * (i + 1)) / totalTests, { duration: 0.3 }] }
      })
      this.tag('ProgressBar.ProgressText').text.text = `${Math.round(progress)}%`

      // Child index matches loop index since this._tests has no event tests
      const child = this.tag('TestListContainer.TestListContent').children[i]
      if (child) child.tag('TestItem').updateResult(result)
      this.signal('onStatus', { isResponse: true, method: test.method, name: test.name, response: result.message, success: result.success })
    }

    this._isRunning = false
    const passed = results.filter(r => r.success).length
    this.signal('onStatus', `Complete: ${passed}/${totalTests} tests passed`)

    // Re-enable button
    const { top: doneTop, bottom: doneBottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('RunButton').patch({
      colorTop: doneTop,
      colorBottom: doneBottom,
      Label: { text: { text: 'Run All Tests', textColor: parseInt(colors.textPrimary, 16) } }
    })

    // Only navigate to results if component is still visible
    this._navigationTimeout = setTimeout(() => {
      // Reset progress bar to grey after completion
      this.tag('ProgressBar.ProgressFill').patch({
        w: 0,
        color: parseInt(colors.focusedBackground, 16)
      })
      this.tag('ProgressBar.ProgressText').text.text = ''

      if (this.visible) {
        this.signal('onTestComplete', { category: this._category, results })
      }
      this._navigationTimeout = null
    }, 1000)
  }

  _focusItem(index) {
    const item = this.tag('TestListContainer.TestListContent').children[index]?.tag('TestItem')
    if (item) item.setFocus(true)
  }

  _unfocusItem(index) {
    const item = this.tag('TestListContainer.TestListContent').children[index]?.tag('TestItem')
    if (item) item.setFocus(false)
  }

  _focusButton() {
    const { colors } = AppSettings
    const { top, bottom } = AppSettings.embossColors(parseInt(colors.primary, 16))
    this.tag('RunButton').patch({
      smooth: {
        colorTop: [top, { duration: 0.2 }],
        colorBottom: [bottom, { duration: 0.2 }],
        scale: [1.05, { duration: 0.2 }]
      }
    })
    this.tag('RunButton.GlowRing').patch({ smooth: { alpha: [1, { duration: 0.2 }] } })
  }

  _unfocusButton() {
    const { colors } = AppSettings
    const { top, bottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('RunButton').patch({
      smooth: {
        colorTop: [top, { duration: 0.2 }],
        colorBottom: [bottom, { duration: 0.2 }],
        scale: [1, { duration: 0.2 }]
      }
    })
    this.tag('RunButton.GlowRing').patch({ smooth: { alpha: [0, { duration: 0.2 }] } })
  }

  _pressButtonAnimation(callback) {
    const btn = this.tag('RunButton')
    btn.patch({ smooth: { scale: [0.97, { duration: 0.08, timingFunction: 'ease-in' }] } })
    btn.tag('PressedOverlay').patch({ smooth: { alpha: [1, { duration: 0.08 }] } })
    setTimeout(() => {
      btn.patch({ smooth: { scale: [1.05, { duration: 0.15, timingFunction: 'ease-out' }] } })
      btn.tag('PressedOverlay').patch({ smooth: { alpha: [0, { duration: 0.15 }] } })
      if (callback) callback()
    }, 100)
  }

  _clearAllResults() {
    const content = this.tag('TestListContainer.TestListContent')
    if (content && content.children) {
      content.children.forEach(child => {
        const item = child.tag('TestItem')
        if (item && item.clearResult) {
          item.clearResult()
        }
      })
    }
  }

  _inactive() {
    // Cancel any running tests when component becomes inactive
    this._isRunning = false

    // Clear any pending navigation timeout
    if (this._navigationTimeout) {
      clearTimeout(this._navigationTimeout)
      this._navigationTimeout = null
    }

    if (this._progressResetTimer) {
      clearTimeout(this._progressResetTimer)
      this._progressResetTimer = null
    }

    // Reset progress bar to grey
    const { colors } = AppSettings
    this.tag('ProgressBar.ProgressFill').patch({
      w: 0,
      color: parseInt(colors.focusedBackground, 16)
    })
    this.tag('ProgressBar.ProgressText').text.text = ''
  }

  _getFocused() {
    return this
  }
}

class TestItem extends Lightning.Component {
  static _template() {
    return {
      w: 580,
      h: 60,
      Background: {
        w: 580,
        h: 60,
        rect: true,
        color: 0xff1a1a1a
      },
      GlowRing: {
        x: 0, y: 0, w: 580, h: 60, rect: true, color: 0x00000000, alpha: 0,
        shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: 0xff00d9ff, radius: 6 }
      },
      Status: {
        x: 10,
        text: {
          text: '○',
          fontSize: 20,
          textColor: 0xff888888
        }
      },
      Name: {
        x: 40,
        text: {
          fontSize: 20,
          textColor: 0xffffffff,
          wordWrapWidth: 530
        }
      },
      Method: {
        x: 40,
        text: {
          fontSize: 16,
          textColor: 0xff888888,
          wordWrapWidth: 530
        }
      }
    }
  }

  _setup() {
    const { testRunner } = AppSettings.components
    const { colors, typography } = AppSettings
    const { width: contentW } = AppSettings.contentArea
    const itemWidth = Math.floor(contentW / 2) - 20
    const itemHeight = testRunner.testItemHeight

    // Calculate vertical positions relative to item height
    const nameHeight = typography.body.fontSize
    const methodHeight = typography.caption.fontSize
    const totalTextHeight = nameHeight + methodHeight + 4 // 4px gap between texts
    const startY = Math.round((itemHeight - totalTextHeight) / 2) - 2

    this.patch({
      w: itemWidth,
      h: itemHeight,
      clipping: false
    })

    const { top: bgTop, bottom: bgBottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('Background').patch({
      w: itemWidth,
      h: itemHeight,
      colorTop: bgTop,
      colorBottom: bgBottom,
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        radius: 6
      }
    })

    const ri = AppSettings.radii.card
    this.tag('GlowRing').patch({ w: itemWidth, h: itemHeight, shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: parseInt(colors.focusGlowColor, 16), radius: ri } })

    this.tag('Status').patch({
      y: startY,
      text: {
        fontSize: typography.body.fontSize,
        textColor: parseInt(colors.textTertiary, 16)
      }
    })

    this.tag('Name').patch({
      y: startY,
      text: {
        fontSize: typography.body.fontSize,
        textColor: parseInt(colors.textPrimary, 16),
        wordWrapWidth: itemWidth - 50
      }
    })

    this.tag('Method').patch({
      y: startY + nameHeight + 4,
      text: {
        fontSize: typography.caption.fontSize,
        textColor: parseInt(colors.textSecondary, 16),
        wordWrapWidth: itemWidth - 50
      }
    })
  }

  set test(value) {
    this._test = value
    this.tag('Name').text.text = value.name
    this.tag('Method').text.text = value.method
  }

  set isFocused(value) {
    this.setFocus(value)
  }

  setFocus(focused) {
    const { colors } = AppSettings
    const baseColor = parseInt(focused ? colors.focusedBackground : colors.cardBackground, 16)
    const { top, bottom } = AppSettings.embossColors(baseColor)
    this.tag('Background').patch({
      smooth: {
        colorTop: [top, { duration: 0.2 }],
        colorBottom: [bottom, { duration: 0.2 }]
      },
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        radius: 6
      }
    })
    this.tag('GlowRing').patch({ smooth: { alpha: [focused ? 1 : 0, { duration: 0.2 }] } })
  }

  clearResult() {
    const { colors } = AppSettings
    this.tag('Status').patch({
      text: { text: '○', textColor: parseInt(colors.textTertiary, 16) }
    })
    const { top, bottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('Background').patch({ colorTop: top, colorBottom: bottom })
  }

  updateResult(result) {
    const { colors } = AppSettings
    if (result.success) {
      this.tag('Status').patch({
        text: { text: '\u25cf', textColor: parseInt(colors.success, 16) }
      })
      const { top, bottom } = AppSettings.embossColors(parseInt(colors.cardBackgroundPass, 16))
      this.tag('Background').patch({
        smooth: {
          colorTop: [top, { duration: 0.3 }],
          colorBottom: [bottom, { duration: 0.3 }]
        }
      })
    } else {
      this.tag('Status').patch({
        text: { text: '\u25cf', textColor: parseInt(colors.error, 16) }
      })
      const { top, bottom } = AppSettings.embossColors(parseInt(colors.cardBackgroundFail, 16))
      this.tag('Background').patch({
        smooth: {
          colorTop: [top, { duration: 0.3 }],
          colorBottom: [bottom, { duration: 0.3 }]
        }
      })
    }
  }
}
