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

export default class ResultsPanel extends Lightning.Component {
  static _template() {
    return {
      Title: {
        text: {
          text: 'Test Results',
          fontSize: 36,
          textColor: 0xffffffff
        }
      },
      Summary: {
        y: 60,
        text: {
          fontSize: 24,
          textColor: 0xffffffff
        }
      },
      ResultsList: {
        y: 120,
        w: 1300,
        h: 700,
        rect: true,
        color: 0x00000000,
        shader: {
          type: Lightning.shaders.RoundedRectangle,
          stroke: 2,
          strokeColor: 0xff2a3f5f,
          radius: 8
        },
        clipping: true,
        ResultsContent: {
          x: 10,
          y: 10
        }
      }
    }
  }

  _setup() {
    const { colors, typography } = AppSettings
    const { width: contentW, maxH } = AppSettings.contentArea

    const titleH = typography.heading.fontSize + 10
    const summaryH = typography.bodyLarge.fontSize + 10
    const listY = titleH + summaryH + 10
    const listH = maxH - listY - 10
    const listW = contentW

    // Store for use in _createResultsList and ResultItem
    this._listW = listW
    this._listH = listH

    this.tag('Title').patch({
      text: {
        fontSize: typography.heading.fontSize,
        textColor: parseInt(colors.primary, 16)
      }
    })

    this.tag('Summary').patch({
      y: titleH,
      text: {
        fontSize: typography.bodyLarge.fontSize,
        textColor: parseInt(colors.textPrimary, 16)
      }
    })

    this.tag('ResultsList').patch({
      y: listY,
      w: listW,
      h: listH
    })
  }

  showResults(data) {
    const { category, results } = data
    const passed = results.filter(r => r.success).length
    const failed = results.length - passed

    this.tag('Title').text.text = `${category.name} - Test Results`
    this.tag('Summary').text.text = `Total: ${results.length} | Passed: ${passed} | Failed: ${failed}`

    this._selectedColumn = 0
    this._createResultsList(results)
  }

  _createResultsList(results) {
    const spacing = AppSettings.components.resultsPanel.resultItemSpacing
    const listH = this._listH || 600
    const listW = this._listW || AppSettings.contentArea.width
    const maxRows = Math.max(1, Math.floor((listH - 20) / spacing))
    const columnWidth = Math.floor(listW / 2)

    // Store for scroll navigation
    this._maxRows = maxRows
    this._columnWidth = columnWidth
    this._totalColumns = Math.ceil(results.length / maxRows)

    const items = results.map((result, index) => {
      const col = Math.floor(index / maxRows)
      const row = index % maxRows
      return {
        ref: `Result${index}`,
        x: col * columnWidth,
        y: row * spacing,
        ResultItem: {
          type: ResultItem,
          result: result
        }
      }
    })

    this.tag('ResultsList.ResultsContent').children = items
    this._updateScroll()
  }

  _updateScroll() {
    const visibleColumns = 2
    const columnWidth = this._columnWidth || 860
    let contentX = 10
    if (this._selectedColumn >= visibleColumns) {
      contentX = 10 - (this._selectedColumn - visibleColumns + 1) * columnWidth
    }
    this.tag('ResultsList.ResultsContent').setSmooth('x', contentX, { duration: 0.3 })
  }

  _handleLeft() {
    if (this._selectedColumn > 0) {
      this._selectedColumn--
      this._updateScroll()
    }
  }

  _handleRight() {
    if (this._selectedColumn < (this._totalColumns || 1) - 1) {
      this._selectedColumn++
      this._updateScroll()
    }
  }

  _getFocused() {
    return this
  }
}

class ResultItem extends Lightning.Component {
  static _template() {
    return {
      w: 1300,
      h: 60,
      Background: {
        w: 1300,
        h: 60,
        rect: true,
        color: 0xff1a1a1a
      },
      Status: {
        x: 20,
        text: {
          fontSize: 28
        }
      },
      Name: {
        x: 70,
        text: {
          fontSize: 22,
          textColor: 0xffffffff
        }
      },
      Source: {
        x: 420,
        text: {
          fontSize: 16,
          textColor: 0xffa8b3cf
        }
      }
    }
  }

  _setup() {
    const { resultsPanel } = AppSettings.components
    const { colors, typography } = AppSettings
    const { width: contentW } = AppSettings.contentArea

    const itemWidth = Math.floor(contentW / 2) - 20
    const itemHeight = resultsPanel.resultItemHeight

    // Single row: Status + Name + Source are all on the same Y — center on one line height
    const nameHeight = typography.body.fontSize
    const startY = Math.round((itemHeight - nameHeight) / 2) - 3

    this.patch({
      w: itemWidth,
      h: itemHeight
    })

    const { top: bgTop, bottom: bgBottom } = AppSettings.embossColors(parseInt(colors.cardBackground, 16))
    this.tag('Background').patch({
      w: itemWidth,
      h: itemHeight,
      colorTop: bgTop,
      colorBottom: bgBottom,
      shader: {
        type: Lightning.shaders.RoundedRectangle,
        radius: AppSettings.radii.card
      }
    })

    // Re-apply result color: _setup() runs after set result() in the Lightning lifecycle,
    // so Background would otherwise revert to cardBackground.
    if (this._result) {
      const bgKey = this._result.success ? 'cardBackgroundPass' : 'cardBackgroundFail'
      const { top: rTop, bottom: rBottom } = AppSettings.embossColors(parseInt(colors[bgKey], 16))
      this.tag('Background').patch({ colorTop: rTop, colorBottom: rBottom })
    }

    this.tag('Status').patch({
      y: startY,
      text: {
        fontSize: typography.icon.fontSize
      }
    })

    this.tag('Name').patch({
      y: startY,
      text: {
        fontSize: typography.body.fontSize,
        textColor: parseInt(colors.textPrimary, 16),
        wordWrapWidth: itemWidth - 250
      }
    })

    this.tag('Source').patch({
      x: itemWidth - 170,
      y: startY + 2,
      text: {
        fontSize: typography.bodySmall.fontSize,
        textColor: parseInt(colors.textSecondary, 16)
      }
    })
  }

  set result(value) {
    const { colors } = AppSettings
    this._result = value

    if (value.success) {
      this.tag('Status').patch({
        text: { text: '\u25cf', textColor: parseInt(colors.success, 16) }
      })
      const { top, bottom } = AppSettings.embossColors(parseInt(colors.cardBackgroundPass, 16))
      this.tag('Background').patch({ colorTop: top, colorBottom: bottom })
    } else {
      this.tag('Status').patch({
        text: { text: '\u25cf', textColor: parseInt(colors.error, 16) }
      })
      const { top, bottom } = AppSettings.embossColors(parseInt(colors.cardBackgroundFail, 16))
      this.tag('Background').patch({ colorTop: top, colorBottom: bottom })
    }

    this.tag('Name').text.text = value.test.name
    this.tag('Source').text.text = `[${value.backend || 'unknown'}]`
  }
}
