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

export default class DimensionRuler extends Lightning.Component {
  static _template() {
    return {
      HorizontalRuler: {
        y: 0
      },
      VerticalRuler: {
        x: 0
      },
      SafeAreaMarkers: {}
    }
  }

  _init() {
    this._stageWidth = this.stage.w || 1920
    this._stageHeight = this.stage.h || 1080
    this._createRulers()
  }

  _createRulers() {
    this._createHorizontalRuler()
    this._createVerticalRuler()
  }

  setSafeArea(safeArea) {
    this._safeArea = safeArea
    this._createSafeAreaMarkers()
  }

  _createSafeAreaMarkers() {
    if (!this._safeArea) return

    const { offsetX, offsetY, width, height } = this._safeArea
    const lineWidth = 2
    const dashLength = 20
    const gapLength = 10
    const color = 0xffff0000

    const markers = []

    // Top horizontal line (dashed)
    const topDashes = Math.floor(width / (dashLength + gapLength))
    for (let i = 0; i < topDashes; i++) {
      markers.push({
        ref: `TopDash${i}`,
        x: offsetX + i * (dashLength + gapLength),
        y: offsetY,
        w: dashLength,
        h: lineWidth,
        rect: true,
        color: color
      })
    }

    // Bottom horizontal line (dashed)
    const bottomY = offsetY + height
    for (let i = 0; i < topDashes; i++) {
      markers.push({
        ref: `BottomDash${i}`,
        x: offsetX + i * (dashLength + gapLength),
        y: bottomY,
        w: dashLength,
        h: lineWidth,
        rect: true,
        color: color
      })
    }

    // Left vertical line (dashed)
    const leftDashes = Math.floor(height / (dashLength + gapLength))
    for (let i = 0; i < leftDashes; i++) {
      markers.push({
        ref: `LeftDash${i}`,
        x: offsetX,
        y: offsetY + i * (dashLength + gapLength),
        w: lineWidth,
        h: dashLength,
        rect: true,
        color: color
      })
    }

    // Right vertical line (dashed)
    const rightX = offsetX + width
    for (let i = 0; i < leftDashes; i++) {
      markers.push({
        ref: `RightDash${i}`,
        x: rightX,
        y: offsetY + i * (dashLength + gapLength),
        w: lineWidth,
        h: dashLength,
        rect: true,
        color: color
      })
    }

    // Corner markers (solid squares)
    const cornerSize = 10
    markers.push(
      {
        ref: 'TopLeftCorner',
        x: offsetX - cornerSize / 2,
        y: offsetY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      },
      {
        ref: 'TopRightCorner',
        x: rightX - cornerSize / 2,
        y: offsetY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      },
      {
        ref: 'BottomLeftCorner',
        x: offsetX - cornerSize / 2,
        y: bottomY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      },
      {
        ref: 'BottomRightCorner',
        x: rightX - cornerSize / 2,
        y: bottomY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      }
    )

    // Labels showing safe area dimensions
    markers.push(
      {
        ref: 'SafeAreaLabel',
        x: offsetX + width / 2 - 100,
        y: offsetY - 35,
        text: {
          text: `Safe Area: ${Math.round(width)}x${Math.round(height)}`,
          fontSize: 16,
          textColor: 0xffff0000
        }
      },
      {
        ref: 'OffsetLabel',
        x: offsetX + 5,
        y: offsetY + 5,
        text: {
          text: `Offset: (${Math.round(offsetX)}, ${Math.round(offsetY)})`,
          fontSize: 14,
          textColor: 0xffff0000
        }
      }
    )

    this.tag('SafeAreaMarkers').children = markers
  }

  _createHorizontalRuler() {
    const increment = 100
    const majorIncrement = 500
    const rulerHeight = 30
    const items = []

    // Background bar for horizontal ruler
    items.push({
      ref: 'Background',
      w: this._stageWidth,
      h: rulerHeight,
      rect: true,
      color: 0xcc000000
    })

    // Create tick marks and labels
    for (let x = 0; x <= this._stageWidth; x += increment) {
      const isMajor = x % majorIncrement === 0
      const tickHeight = isMajor ? 20 : 10

      items.push({
        ref: `Tick${x}`,
        x: x,
        y: rulerHeight - tickHeight,
        w: 1,
        h: tickHeight,
        rect: true,
        color: 0xffffffff
      })

      if (isMajor) {
        items.push({
          ref: `Label${x}`,
          x: x + 3,
          y: 2,
          text: {
            text: `${x}`,
            fontSize: 14,
            textColor: 0xffffffff
          }
        })
      }
    }

    this.tag('HorizontalRuler').children = items
  }

  _createVerticalRuler() {
    const increment = 100
    const majorIncrement = 500
    const rulerWidth = 30
    const items = []

    // Background bar for vertical ruler
    items.push({
      ref: 'Background',
      w: rulerWidth,
      h: this._stageHeight,
      rect: true,
      color: 0xcc000000
    })

    // Create tick marks and labels
    for (let y = 0; y <= this._stageHeight; y += increment) {
      const isMajor = y % majorIncrement === 0
      const tickWidth = isMajor ? 20 : 10

      items.push({
        ref: `Tick${y}`,
        x: rulerWidth - tickWidth,
        y: y,
        w: tickWidth,
        h: 1,
        rect: true,
        color: 0xffffffff
      })

      if (isMajor) {
        items.push({
          ref: `Label${y}`,
          x: 3,
          y: y + 3,
          text: {
            text: `${y}`,
            fontSize: 14,
            textColor: 0xffffffff
          }
        })
      }
    }

    this.tag('VerticalRuler').children = items
  }

  set enabled(value) {
    this.visible = value
  }
}
