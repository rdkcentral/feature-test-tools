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

import settings from '../../app-settings.json'

class AppSettings {
  constructor() {
    this._settings = settings
    this._scaleX = 1
    this._scaleY = 1
    this._safeAreaOffsetX = 0
    this._safeAreaOffsetY = 0
  }

  // Initialize scale factors based on actual window size
  initScale(stageWidth, stageHeight) {
    this._scaleX = stageWidth / this._settings.screen.width
    this._scaleY = stageHeight / this._settings.screen.height

    // Calculate safe area offsets to center content
    const safeWidth = stageWidth * this._settings.safeArea.widthPercentage
    const safeHeight = stageHeight * this._settings.safeArea.heightPercentage
    this._safeAreaOffsetX = (stageWidth - safeWidth) / 2
    this._safeAreaOffsetY = (stageHeight - safeHeight) / 2
  }

  get safeArea() {
    return {
      offsetX: this._safeAreaOffsetX,
      offsetY: this._safeAreaOffsetY,
      width: (this._settings.screen.width * this._scaleX) * this._settings.safeArea.widthPercentage,
      height: (this._settings.screen.height * this._scaleY) * this._settings.safeArea.heightPercentage
    }
  }

  get debug() {
    return this._settings.debug
  }

  get execution() {
    return this._settings.execution
  }

  get screen() {
    return this._settings.screen
  }

  get layout() {
    return this._scaleLayout(this._settings.layout)
  }

  get components() {
    return this._scaleComponents(this._settings.components)
  }

  get colors() {
    return this._settings.colors
  }

  get radii() {
    return this._settings.radii
  }

  get emboss() {
    return this._settings.emboss
  }

  embossColors(baseColor) {
    const emboss = this._settings.emboss || {}
    const l = emboss.lightenAmount || 20
    const d = emboss.darkenAmount || 15
    const a = (baseColor >>> 24) & 0xff
    const r = (baseColor >>> 16) & 0xff
    const g = (baseColor >>> 8) & 0xff
    const b = baseColor & 0xff
    const clamp = v => Math.max(0, Math.min(255, v))
    return {
      top: ((a << 24) | (clamp(r + l) << 16) | (clamp(g + l) << 8) | clamp(b + l)) >>> 0,
      bottom: ((a << 24) | (clamp(r - d) << 16) | (clamp(g - d) << 8) | clamp(b - d)) >>> 0
    }
  }

  get typography() {
    return this._scaleTypography(this._settings.typography)
  }

  // The drawable area available to Content components (TestRunner, ResultsPanel, Menu).
  // Computed from safe area minus padding and header.
  // maxH is the usable height before the Legends panel starts (with a 20px gap).
  // All rounded-rect containers must size themselves to maxH so they never overlap Legends.
  get contentArea() {
    const sa = this.safeArea
    const layout = this.layout
    const contentH = Math.round(sa.height - layout.content.offsetY - layout.padding.bottom)
    const legendsRelY = Math.round(layout.legends.offsetY - layout.content.offsetY)
    return {
      width: Math.round(sa.width - layout.padding.left - layout.padding.right),
      height: contentH,
      maxH: Math.min(contentH, legendsRelY - 20)
    }
  }

  // Scale layout values
  _scaleLayout(layout) {
    return {
      screen: {
        width: Math.round(layout.screen.width * this._scaleX),
        height: Math.round(layout.screen.height * this._scaleY)
      },
      padding: {
        left: Math.round(layout.padding.left * this._scaleX),
        top: Math.round(layout.padding.top * this._scaleY),
        right: Math.round(layout.padding.right * this._scaleX),
        bottom: Math.round(layout.padding.bottom * this._scaleY)
      },
      header: {
        height: Math.round(layout.header.height * this._scaleY)
      },
      content: {
        offsetY: Math.round(layout.content.offsetY * this._scaleY),
        menuWidth: Math.round(layout.content.menuWidth * this._scaleX),
        testRunnerOffsetX: Math.round(layout.content.testRunnerOffsetX * this._scaleX)
      },
      legends: {
        offsetX: Math.round(layout.legends.offsetX * this._scaleX),
        offsetY: Math.round(layout.legends.offsetY * this._scaleY),
        width: Math.round(layout.legends.width * this._scaleX),
        height: Math.round(layout.legends.height * this._scaleY)
      },
      footer: {
        offsetY: Math.round(layout.footer.offsetY * this._scaleY)
      }
    }
  }

  // Scale component dimensions — only fixed UI values (heights, spacing, button sizes).
  // Dynamic layout values (buttonY, containerWidth/Height, etc.) are computed per-component
  // via AppSettings.contentArea so they adapt to any screen size.
  _scaleComponents(components) {
    return {
      menu: {
        itemWidth: Math.round(components.menu.itemWidth * this._scaleX),
        itemHeight: Math.round(components.menu.itemHeight * this._scaleY),
        itemSpacing: Math.round(components.menu.itemSpacing * this._scaleY),
        passRateGradientStep: components.menu.passRateGradientStep || 10,
        buttonWidth: Math.round(components.menu.buttonWidth * this._scaleX),
        buttonHeight: Math.round(components.menu.buttonHeight * this._scaleY)
      },
      testRunner: {
        testItemHeight: Math.round(components.testRunner.testItemHeight * this._scaleY),
        testItemSpacing: Math.round(components.testRunner.testItemSpacing * this._scaleY),
        buttonWidth: Math.round(components.testRunner.buttonWidth * this._scaleX),
        buttonHeight: Math.round(components.testRunner.buttonHeight * this._scaleY)
      },
      resultsPanel: {
        resultItemHeight: Math.round(components.resultsPanel.resultItemHeight * this._scaleY),
        resultItemSpacing: Math.round(components.resultsPanel.resultItemSpacing * this._scaleY)
      }
    }
  }

  // Scale typography
  _scaleTypography(typography) {
    // Use minimum to maintain aspect ratio
    const scale = Math.min(this._scaleX, this._scaleY)
    return {
      title: { fontSize: Math.round(typography.title.fontSize * scale) },
      subtitle: { fontSize: Math.round(typography.subtitle.fontSize * scale) },
      heading: { fontSize: Math.round(typography.heading.fontSize * scale) },
      body: { fontSize: Math.round(typography.body.fontSize * scale) },
      bodyLarge: { fontSize: Math.round(typography.bodyLarge.fontSize * scale) },
      bodySmall: { fontSize: Math.round(typography.bodySmall.fontSize * scale) },
      caption: { fontSize: Math.round(typography.caption.fontSize * scale) },
      icon: { fontSize: Math.round(typography.icon.fontSize * scale) }
    }
  }

  parseColor(colorString) {
    return parseInt(colorString, 16)
  }

  getColor(colorKey) {
    return this.parseColor(this._settings.colors[colorKey])
  }
}

export default new AppSettings()
