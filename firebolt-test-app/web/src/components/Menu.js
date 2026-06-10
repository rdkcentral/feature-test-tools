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
import AppSettings from '../lib/AppSettings'
import FireboltAPI from '../lib/FireboltAPI'
import categories from '../data/categories.json'

export default class Menu extends Lightning.Component {
  static _template() {
    return {
      ListContainer: {
        x: 0,
        y: 0,
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
        ColumnContainer: {
          y: 10,
          x: 25
        }
      },
      RunAllButton: {
        x: 0,
        y: 100,
        w: 300,
        h: 60,
        rect: true,
        color: 0xff1e2936,
        visible: true,
        shader: {
          type: Lightning.shaders.RoundedRectangle,
          radius: 8
        },
        Label: {
          mount: 0.5,
          x: 150,
          y: 30,
          text: {
            text: 'Run All Tests',
            fontSize: 24,
            textColor: 0xffffffff
          }
        },
        PressedOverlay: {
          x: 0, y: 0, w: 300, h: 60, rect: true, color: 0x55000000, alpha: 0,
          shader: { type: Lightning.shaders.RoundedRectangle, radius: 8 }
        },
        GlowRing: {
          x: 0, y: 0, w: 300, h: 60, rect: true, color: 0x00000000, alpha: 0,
          shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: 0xff00d9ff, radius: 8 }
        }
      },
      ProgressBar: {
        x: 0,
        y: 200,
        visible: false,
        ProgressBg: {
          w: 920,
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
          color: 0xff00d9ff,
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
      }
    }
  }

  _init() {
    this._index = 0
    this._columnIndex = 0
    this._focusOnButton = false
    this._isRunning = false
    this._fireboltAPI = new FireboltAPI()
    this._categories = AppSettings.debug.mockEnabled
      ? categories
      : categories.filter(c => c.id !== 'mockstress' && !(AppSettings.firebolt8Mode && c.firebolt9Only))
    this._totalColumns = 0

    this._createListAsync()
  }

  _isCategoryInteractive(category) {
    if (!category) {
      return false
    }

    const isDummyById = /^test\d+$/i.test(category.id || '')
    const isDummyByName = /^test\s+category/i.test((category.name || '').trim())
    return !(isDummyById || isDummyByName)
  }

  _setup() {
    const { menu } = AppSettings.components
    const { width: contentW, maxH } = AppSettings.contentArea

    // List container fills available height, bounded above the Legends panel
    const listW = contentW
    const buttonH = menu.buttonHeight
    const progressAreaH = 40             // 8px bar + 20px text + 12px gap
    const buttonY = maxH - progressAreaH - buttonH - 10
    const listH = buttonY - 10

    this.tag('ListContainer').patch({ w: listW, h: listH })
    const r = AppSettings.radii.container
    this.tag('RunAllButton').patch({
      y: buttonY,
      w: menu.buttonWidth,
      h: buttonH
    })

    this.tag('RunAllButton.Label').patch({
      x: menu.buttonWidth / 2,
      y: buttonH / 2
    })

    const { colors } = AppSettings
    const { top: btnTop, bottom: btnBottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('RunAllButton').patch({ colorTop: btnTop, colorBottom: btnBottom })
    this.tag('RunAllButton.PressedOverlay').patch({ w: menu.buttonWidth, h: buttonH, color: parseInt(colors.bevelPressedOverlay, 16) })
    this.tag('RunAllButton.GlowRing').patch({ w: menu.buttonWidth, h: buttonH, shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: parseInt(colors.focusGlowColor, 16), radius: r } })

    this.tag('ProgressBar').patch({
      y: buttonY + buttonH + 10,
      ProgressBg: { w: listW }
    })

    // store for reuse in _runAllTests, _createListAsync, navigation
    this._listW = listW
    this._listH = listH
    this._buttonY = buttonY
    this._columnWidth = menu.itemWidth + 20
    this._maxRows = Math.max(1, Math.floor((listH - 20) / menu.itemSpacing))
  }

  _createListAsync() {
    const spacing = AppSettings.components.menu.itemSpacing
    const maxRows = this._maxRows || 10
    const columnWidth = this._columnWidth || AppSettings.components.menu.itemWidth + 20

    // Calculate total columns needed
    const totalColumns = Math.ceil(this._categories.length / maxRows)
    this._totalColumns = totalColumns

    const columns = []

    for (let col = 0; col < totalColumns; col++) {
      const columnItems = []
      for (let row = 0; row < maxRows; row++) {
        const index = col * maxRows + row
        if (index >= this._categories.length) break
        const category = this._categories[index]
        columnItems.push({
          ref: `Item${col}_${row}`,
          x: col * columnWidth,
          y: row * spacing,
          CategoryItem: {
            type: CategoryItem,
            category: {
              ...category,
              name: category.name,
              isDisabled: !this._isCategoryInteractive(category),
              isRunAllExcluded: !!category.runAllExcluded
            },
            isFocused: this._columnIndex === col && row === this._index
          }
        })
      }
      columns.push(...columnItems)
    }

    this.tag('ListContainer.ColumnContainer').children = columns
  }

  _active() {
    this._focusOnButton = false
    this._isRunning = false
    const { colors: _ac } = AppSettings
    const { top: _acTop, bottom: _acBottom } = AppSettings.embossColors(parseInt(_ac.cardBackground, 16))
    this.tag('RunAllButton').patch({
      colorTop: _acTop,
      colorBottom: _acBottom,
      Label: {
        text: { textColor: parseInt(_ac.textPrimary, 16) }
      },
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        radius: 8
      }
    })
    this.tag('ProgressBar').visible = false
    this.tag('ProgressBar.ProgressFill').w = 0
    this._resetCategoryProgress()

    // Restore focus to last selected item if available
    if (typeof this._lastColumnIndex === 'number' && typeof this._lastIndex === 'number') {
      this._columnIndex = this._lastColumnIndex
      this._index = this._lastIndex
    } else {
      this._columnIndex = 0
      this._index = 0
    }
    this._focusItem(this._columnIndex, this._index)
  }

  _handleUp() {
    if (this._isRunning) return

    if (this._focusOnButton) {
      this._focusOnButton = false
      const { colors: _uc } = AppSettings
      const { top: _ucTop, bottom: _ucBottom } = AppSettings.embossColors(parseInt(_uc.cardBackground, 16))
      this.tag('RunAllButton').patch({
        colorTop: _ucTop,
        colorBottom: _ucBottom,
        Label: { text: { textColor: parseInt(_uc.textPrimary, 16) } },
        shader: { type: Lightning.shaders.RoundedRectangle, radius: 8 }
      })
      this.tag('RunAllButton.GlowRing').patch({ smooth: { alpha: [0, { duration: 0.2 }] } })
      // Move to last item in current column
      const maxRows = this._maxRows || 10
      const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows)
      this._index = colLength - 1
      this._focusItem(this._columnIndex, this._index)
    } else if (this._index > 0) {
      this._unfocusItem(this._columnIndex, this._index)
      this._index--
      this._focusItem(this._columnIndex, this._index)
    }
  }

  _handleDown() {
    if (this._isRunning) return

    const maxRows = this._maxRows || 10
    const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows)
    const canMoveToButton = this._columnIndex === 0 && this._index === (colLength - 1)

    if (this._index < colLength - 1) {
      this._unfocusItem(this._columnIndex, this._index)
      this._index++
      this._focusItem(this._columnIndex, this._index)
    } else if (canMoveToButton && !this._focusOnButton) {
      this._unfocusItem(this._columnIndex, this._index)
      this._focusOnButton = true
      const { colors: _dc } = AppSettings
      const { top: _dcTop, bottom: _dcBottom } = AppSettings.embossColors(parseInt(_dc.primary, 16))
      this.tag('RunAllButton').patch({
        colorTop: _dcTop,
        colorBottom: _dcBottom,
        Label: { text: { textColor: parseInt(_dc.background, 16) } },
        shader: { type: Lightning.shaders.RoundedRectangle, radius: 8 }
      })
      this.tag('RunAllButton.GlowRing').patch({ smooth: { alpha: [1, { duration: 0.2 }] } })
    }
  }

  _handleLeft() {
    if (this._isRunning || this._focusOnButton) return

    if (this._columnIndex > 0) {
      this._unfocusItem(this._columnIndex, this._index)
      this._columnIndex--
      const maxRows = this._maxRows || 10
      const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows)
      if (this._index > colLength - 1) this._index = colLength - 1
      this._focusItem(this._columnIndex, this._index)
      this._scrollToColumn()
    }
  }

  _handleRight() {
    if (this._isRunning || this._focusOnButton) return

    if (this._columnIndex < this._totalColumns - 1) {
      this._unfocusItem(this._columnIndex, this._index)
      this._columnIndex++
      const maxRows = this._maxRows || 10
      const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows)
      if (this._index > colLength - 1) this._index = colLength - 1
      this._focusItem(this._columnIndex, this._index)
      this._scrollToColumn()
    }
  }

  _handleEnter() {
    if (this._isRunning) return

    if (this._focusOnButton) {
      if (this.tag('ProgressBar').visible) {
        this.tag('ProgressBar').visible = false
        this.tag('ProgressBar.ProgressFill').w = 0
        this._resetCategoryProgress()
      } else {
        // Guard immediately so rapid re-presses are blocked during animation
        this._isRunning = true
        this._setButtonRunningState(true)
        this._pressButtonAnimation(() => this._runAllTests())
      }
    } else {
      this._lastColumnIndex = this._columnIndex
      this._lastIndex = this._index
      const globalIndex = this._columnIndex * (this._maxRows || 10) + this._index
      const selectedCategory = this._categories[globalIndex]

      if (!this._isCategoryInteractive(selectedCategory)) {
        return
      }

      const ref = `Item${this._columnIndex}_${this._index}`
      const listItem = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref)
      const catItem = listItem && listItem.tag('CategoryItem')
      if (catItem) {
        catItem.press().then(() => this.signal('onSelect', selectedCategory))
      } else {
        this.signal('onSelect', selectedCategory)
      }
    }
  }

  _setButtonRunningState(isRunning) {
    const { colors } = AppSettings
    const btn = this.tag('RunAllButton')
    if (isRunning) {
      const { top, bottom } = AppSettings.embossColors(parseInt(colors.focusedBackground, 16))
      btn.patch({
        colorTop: top,
        colorBottom: bottom,
        Label: { text: { text: 'Running...', textColor: parseInt(colors.textSecondary, 16) } }
      })
    } else {
      const { top, bottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
      btn.patch({
        colorTop: top,
        colorBottom: bottom,
        Label: { text: { text: 'Run All Tests', textColor: parseInt(colors.textPrimary, 16) } }
      })
    }
  }

  _pressButtonAnimation(callback) {
    const btn = this.tag('RunAllButton')
    btn.patch({ smooth: { scale: [0.97, { duration: 0.08, timingFunction: 'ease-in' }] } })
    btn.tag('PressedOverlay').patch({ smooth: { alpha: [1, { duration: 0.08 }] } })
    setTimeout(() => {
      btn.patch({ smooth: { scale: [1, { duration: 0.15, timingFunction: 'ease-out' }] } })
      btn.tag('PressedOverlay').patch({ smooth: { alpha: [0, { duration: 0.15 }] } })
      if (callback) callback()
    }, 100)
  }

  _focusItem(columnIndex, index) {
    const ref = `Item${columnIndex}_${index}`
    const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref)
    if (item && item.tag('CategoryItem')) {
      item.tag('CategoryItem').setFocus(true)
    }
  }

  _unfocusItem(columnIndex, index) {
    const ref = `Item${columnIndex}_${index}`
    const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref)
    if (item && item.tag('CategoryItem')) {
      item.tag('CategoryItem').setFocus(false)
    }
  }
  _scrollToColumn() {
    const columnWidth = this._columnWidth || AppSettings.components.menu.itemWidth + 20
    const visibleColumns = 3
    let scrollX = 0
    if (this._columnIndex >= visibleColumns) {
      scrollX = (this._columnIndex - visibleColumns + 1) * columnWidth
    }
    this.tag('ListContainer.ColumnContainer').x = 25 - scrollX
  }

  async _runAllTests() {
    this.tag('ProgressBar').visible = true
    const startTime = Date.now()

    const { categoryBatchSize: rawCategoryBatchSize, testBatchSize: rawTestBatchSize } = AppSettings.execution
    const parsedCategoryBatchSize = Math.floor(Number(rawCategoryBatchSize))
    const parsedTestBatchSize = Math.floor(Number(rawTestBatchSize))
    const categoryBatchSize = parsedCategoryBatchSize > 0 ? parsedCategoryBatchSize : 1
    const testBatchSize = parsedTestBatchSize > 0 ? parsedTestBatchSize : 1

    if (categoryBatchSize !== rawCategoryBatchSize || testBatchSize !== rawTestBatchSize) {
      const configMessage = `Invalid execution batch size config; using categoryBatchSize=${categoryBatchSize}, testBatchSize=${testBatchSize}`
      this.signal('onStatus', configMessage)
      this.tag('ProgressBar.ProgressText').text.text = configMessage
    }

    const runnableCategories = this._categories.filter(category => this._isCategoryInteractive(category) && !category.runAllExcluded)
    const totalCategories = runnableCategories.length
    const categoryResults = []

    if (totalCategories === 0) {
      this.tag('ProgressBar.ProgressFill').w = 0
      this.tag('ProgressBar.ProgressText').text.text = 'No runnable categories available.'
      this._isRunning = false
      this._setButtonRunningState(false)
      return
    }

    let completedCount = 0
    // Process categories sequentially to avoid races on shared FireboltAPI instance state
    const processCategoryBatch = async (categories) => {
      const results = []
      for (const { category, columnIndex, rowIndex } of categories) {
        this._setCategoryProgress(columnIndex, rowIndex, 'running')

        const tests = await this._fireboltAPI.getTestsForCategory(category.id)
        const runnableTests = tests.filter(t => t.type !== 'event' && t.type !== 'gateway-event')
        let passedTests = 0
        const totalTests = runnableTests.length

        // Process tests sequentially within the category
        const processTestBatch = async (testBatch) => {
          let batchPassedTests = 0
          for (const test of testBatch) {
            try {
              const result = await this._fireboltAPI.runTest(test)
              this.signal('onStatus', { isResponse: true, method: test.method, name: test.name, response: result.message, success: result.success })
              batchPassedTests += result.success ? 1 : 0
            } catch (error) {
              this.signal('onStatus', { isResponse: true, method: test.method || '?', name: test.name || '?', response: `Error: ${error.message}`, success: false })
            }
          }
          return batchPassedTests
        }

        // Split tests into batches
        for (let i = 0; i < runnableTests.length; i += testBatchSize) {
          const batch = runnableTests.slice(i, i + testBatchSize)
          passedTests += await processTestBatch(batch)
          await new Promise(resolve => setTimeout(resolve, 50))
        }

        // Calculate pass rate
        const passRate = totalTests > 0 ? Math.round((passedTests / totalTests) * 100) : 0
        this._setCategoryProgress(columnIndex, rowIndex, 'complete', passRate)

        completedCount++
        const progress = (completedCount / totalCategories) * 100
        this.tag('ProgressBar.ProgressFill').setSmooth('w', (this._listW * progress) / 100, { duration: 0.3 })
        this.tag('ProgressBar.ProgressText').text.text = `Running tests... (${completedCount}/${totalCategories} categories)`

        results.push({ category: category.name, passRate, passed: passedTests, total: totalTests })
      }
      return results
    }

    if (AppSettings.debug.verbose) console.log(`[RunAll] _maxRows=${this._maxRows}, runnableCategories=${runnableCategories.map(c => c.id).join(', ')}`)

    try {
      // Process categories in sequential batches to limit concurrency against shared API state
      for (let i = 0; i < runnableCategories.length; i += categoryBatchSize) {
        const maxRows = this._maxRows || 10
        const batch = runnableCategories.slice(i, i + categoryBatchSize).map((category) => {
          const actualIndex = this._categories.findIndex(c => c.id === category.id)
          const columnIndex = Math.floor(actualIndex / maxRows)
          const rowIndex = actualIndex % maxRows
          if (AppSettings.debug.verbose) console.log(`[RunAll] category=${category.id} actualIndex=${actualIndex} -> ref=Item${columnIndex}_${rowIndex}`)
          return {
            category,
            columnIndex,
            rowIndex
          }
        })

        const batchResults = await processCategoryBatch(batch)
        categoryResults.push(...batchResults)
      }

      // All tests complete
      const endTime = Date.now()
      const totalTime = ((endTime - startTime) / 1000).toFixed(2)
      const totalPassed = categoryResults.reduce((sum, r) => sum + r.passed, 0)
      const totalTests = categoryResults.reduce((sum, r) => sum + r.total, 0)
      const summaryText = totalTests === 0
        ? `All tests completed in ${totalTime}s! ${totalPassed}/${totalTests} passed (no tests executed)`
        : `All tests completed in ${totalTime}s! ${totalPassed}/${totalTests} passed (${Math.round((totalPassed / totalTests) * 100)}%)`
      this.tag('ProgressBar.ProgressText').patch({ text: { text: summaryText, fontStyle: 'bold' } })
    } finally {
      this._isRunning = false
      this._setButtonRunningState(false)
    }
  }

  _setCategoryProgress(columnIndex, rowIndex, state, passRate) {
    const ref = `Item${columnIndex}_${rowIndex}`
    const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref)

    if (item && item.tag('CategoryItem')) {
      const { colors } = AppSettings
      let color

      if (state === 'running') {
        color = parseInt(colors.categoryRunningBackground, 16)
      } else if (state === 'complete' && passRate !== null && passRate !== undefined) {
        const gradientStep = AppSettings.components.menu.passRateGradientStep || 10
        const steppedPassRate = Math.round(passRate / gradientStep) * gradientStep

        const ratio = steppedPassRate / 100

        if (ratio <= 0.33) {
          const colorRed = parseInt(colors.passRateColorDarkRed, 16)
          const colorOrangeRed = parseInt(colors.passRateColorOrangeRed, 16)
          const localRatio = ratio / 0.33
          color = Lightning.StageUtils.mergeColors(colorOrangeRed, colorRed, localRatio)
        } else if (ratio <= 0.66) {
          const colorOrangeRed = parseInt(colors.passRateColorOrangeRed, 16)
          const colorOrange = parseInt(colors.passRateColorOrange, 16)
          const localRatio = (ratio - 0.33) / 0.33
          color = Lightning.StageUtils.mergeColors(colorOrange, colorOrangeRed, localRatio)
        } else {
          const colorOrange = parseInt(colors.passRateColorOrange, 16)
          const colorGreen = parseInt(colors.passRateColorDarkGreen, 16)
          const localRatio = (ratio - 0.66) / 0.34
          color = Lightning.StageUtils.mergeColors(colorGreen, colorOrange, localRatio)
        }
      } else {
        color = parseInt(colors.cardBackground, 16)
      }
      const { top: ec_top, bottom: ec_bottom } = AppSettings.embossColors(color)
      item.tag('CategoryItem.Background').patch({
        colorTop: ec_top,
        colorBottom: ec_bottom,
        shader: {
          type: Lightning.shaders.RoundedRectangle,
          radius: 8
        }
      })

      if (state === 'complete') {
        item.tag('CategoryItem').setTestResult()
      }

      if (passRate !== null && passRate !== undefined && state === 'complete') {
        const cat = this._categories[columnIndex * (this._maxRows || 10) + rowIndex]
        const baseName = cat ? cat.name : ''
        item.tag('CategoryItem.Name').text.text = `${baseName} (${passRate}%)`
      }
    }
  }

  _resetCategoryProgress() {
    const maxRows = this._maxRows || 10
    for (let i = 0; i < this._categories.length; i++) {
      const columnIndex = Math.floor(i / maxRows)
      const rowIndex = i % maxRows
      const ref = `Item${columnIndex}_${rowIndex}`
      const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref)

      if (item && item.tag('CategoryItem')) {
        item.tag('CategoryItem').clearTestResult()
        const { colors } = AppSettings
        const { top: rcp_top, bottom: rcp_bottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
        item.tag('CategoryItem.Background').patch({
          colorTop: rcp_top,
          colorBottom: rcp_bottom,
          shader: {
            type: Lightning.shaders.RoundedRectangle,
            radius: 8
          }
        })
        item.tag('CategoryItem.Name').patch({
          text: { textColor: parseInt(colors.textPrimary, 16) }
        })
        item.tag('CategoryItem.Name').text.text = this._categories[i] ? this._categories[i].name : ''
      }
    }
  }

  _getFocused() {
    return this
  }
}

class CategoryItem extends Lightning.Component {
  _construct() {
    // Build template dynamically to get scaled values
    const { menu } = AppSettings.components

    this._itemWidth = menu.itemWidth
    this._itemHeight = menu.itemHeight
  }

  static _template() {
    return {
      w: 400,
      h: 50,
      Background: {
        w: 400,
        h: 50,
        rect: true,
        color: 0xff1e2936,
        shader: {
          type: Lightning.shaders.RoundedRectangle,
          radius: 8
        }
      },
      HatchOverlay: {
        x: 0, y: 0, w: 400, h: 50,
        visible: false,
        shader: { type: Lightning.shaders.RoundedRectangle, radius: 8 }
      },
      Name: {
        x: 20,
        text: {
          fontSize: 24,
          textColor: 0xffffffff
        }
      },
      PressedOverlay: {
        x: 0, y: 0, w: 400, h: 50, rect: true, color: 0x55000000, alpha: 0,
        shader: { type: Lightning.shaders.RoundedRectangle, radius: 8 }
      },
      Border: {
        w: 400,
        h: 50,
        rect: true,
        color: 0x00000000,
        shader: {
          type: Lightning.shaders.RoundedRectangle,
          stroke: 4,
          strokeColor: 0xff00d9ff,
          radius: 8
        },
        alpha: 0
      },
      GlowRing: {
        x: 0, y: 0, w: 400, h: 50, rect: true, color: 0x00000000, alpha: 0,
        shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: 0xff00d9ff, radius: 8 }
      }
    }
  }

  _setup() {
    // Apply scaled values after template is built
    const { menu } = AppSettings.components
    const { colors, typography } = AppSettings

    const itemH = menu.itemHeight
    const startY = Math.round((itemH - typography.subtitle.fontSize) / 2) - 3

    this.patch({
      w: menu.itemWidth,
      h: itemH
    })

    const { top: bgTop, bottom: bgBottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('Background').patch({
      w: menu.itemWidth,
      h: itemH,
      colorTop: bgTop,
      colorBottom: bgBottom,
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        radius: 8
      }
    })

    this.tag('HatchOverlay').patch({ w: menu.itemWidth, h: itemH })

    this.tag('Border').patch({
      w: menu.itemWidth,
      h: itemH,
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        stroke: 3,
        strokeColor: parseInt(colors.primary, 16),
        radius: 8
      }
    })

    this.tag('Name').patch({
      y: startY,
      text: {
        fontSize: typography.subtitle.fontSize,
        textColor: parseInt(colors.textPrimary, 16)
      }
    })

    const r = AppSettings.radii.container
    this.tag('PressedOverlay').patch({ w: menu.itemWidth, h: itemH, color: parseInt(colors.bevelPressedOverlay, 16) })
    this.tag('GlowRing').patch({ w: menu.itemWidth, h: itemH, shader: { type: Lightning.shaders.RoundedRectangle, stroke: 3, strokeColor: parseInt(colors.focusGlowColor, 16), radius: r } })
  }

  set category(value) {
    this._category = value
    this._isDisabled = !!value.isDisabled
    this._isRunAllExcluded = !!value.isRunAllExcluded
    this._hasTestResult = false
    this.tag('Name').text.text = value.name

    if (this._isDisabled) {
      this.patch({ alpha: 0.55 })
      this.tag('HatchOverlay').visible = false
    } else if (this._isRunAllExcluded) {
      this.patch({ alpha: 1 })
      const w = this._itemWidth
      const h = this._itemHeight
      this.tag('HatchOverlay').patch({
        visible: true,
        texture: {
          type: Lightning.textures.StaticCanvasTexture,
          content: {
            w, h,
            draw: (ctx, canvas) => {
              // Dark translucent base so the card reads as muted
              ctx.fillStyle = 'rgba(0,0,0,0.45)'
              ctx.fillRect(0, 0, canvas.width, canvas.height)
              // Bold diagonal stripes
              ctx.strokeStyle = 'rgba(255,255,255,0.35)'
              ctx.lineWidth = 2.5
              const step = 10
              for (let x = -canvas.height; x < canvas.width + canvas.height; x += step) {
                ctx.beginPath()
                ctx.moveTo(x, 0)
                ctx.lineTo(x + canvas.height, canvas.height)
                ctx.stroke()
              }
            }
          }
        }
      })
    } else {
      this.patch({ alpha: 1 })
      this.tag('HatchOverlay').visible = false
    }
  }

  set isFocused(value) {
    this.setFocus(value)
  }

  setTestResult() {
    this._hasTestResult = true
  }

  clearTestResult() {
    this._hasTestResult = false
  }

  setFocus(focused) {
    const { colors } = AppSettings;
    const canFocus = focused && !this._isDisabled

    // Run-all-excluded items (e.g. Lifecycle): fade out the hatch overlay on focus so
    // the card visually "opens up" and the user can clearly see it is selected/active.
    if (this._isRunAllExcluded) {
      this.tag('HatchOverlay').patch({ smooth: { alpha: [focused ? 0 : 1, { duration: 0.2 }] } })
    }

    // Show teal border + glow if focused, else hide
    this.tag('Border').patch({
      smooth: { alpha: [canFocus ? 1 : 0, { duration: 0.2 }] }
    });
    this.tag('GlowRing').patch({
      smooth: { alpha: [canFocus ? 1 : 0, { duration: 0.2 }] }
    });

    if (this._isDisabled) {
      this.tag('Name').patch({
        text: { textColor: parseInt(colors.textSecondary, 16) }
      })
      return
    }

    if (!this._hasTestResult) {
      const { top: bgTop, bottom: bgBottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
      this.tag('Background').patch({
        smooth: {
          colorTop: [bgTop, { duration: 0.2 }],
          colorBottom: [bgBottom, { duration: 0.2 }]
        }
      })
      this.tag('Name').patch({
        text: { textColor: parseInt(colors.textPrimary, 16) }
      })
    } else {
      this.tag('Name').patch({
        text: { textColor: parseInt(colors.textPrimary, 16) }
      });
    }
  }

  press() {
    return new Promise(resolve => {
      this.patch({ smooth: { scale: [0.96, { duration: 0.08, timingFunction: 'ease-in' }] } })
      this.tag('PressedOverlay').patch({ smooth: { alpha: [1, { duration: 0.08 }] } })
      setTimeout(() => {
        this.patch({ smooth: { scale: [1, { duration: 0.15, timingFunction: 'ease-out' }] } })
        this.tag('PressedOverlay').patch({ smooth: { alpha: [0, { duration: 0.15 }] } })
        setTimeout(resolve, 150)
      }, 100)
    })
  }
}
