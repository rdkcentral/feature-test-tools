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
 * Device Connection Configuration
 * Allows the app to connect to a remote RDK device instead of localhost
 *
 * Usage:
 * - Via query parameter: ?deviceIP=192.168.1.100
 * - Default: localhost (127.0.0.1)
 */

export class DeviceConnectionConfig {
  static DEFAULT_IP = '127.0.0.1'
  static DEFAULT_PORT = 3473
  static PROTOCOL = 'ws'

  static normalizeIP(ip) {
    return typeof ip === 'string' ? ip.trim() : ''
  }

  /**
   * Get the configured device IP
   * Priority: URL query param > default
   */
  static getDeviceIP() {
    const params = new URLSearchParams(window.location.search)
    const queryIP = this.normalizeIP(params.get('deviceIP'))
    if (queryIP) {
      if (this.isValidIP(queryIP)) {
        return queryIP
      }
      console.warn(`Invalid deviceIP query value "${queryIP}". Falling back to local device (${this.DEFAULT_IP}).`)
    }
    return this.DEFAULT_IP
  }

  /**
   * Get the full WebSocket endpoint URL
   * Format: ws://192.168.1.100:3473
   */
  static getEndpointURL() {
    const ip = this.getDeviceIP()
    return `${this.PROTOCOL}://${ip}:${this.DEFAULT_PORT}`
  }

  /**
   * Get connection info as display string
   */
  static getConnectionInfo() {
    const ip = this.getDeviceIP()
    const isLocal = ip === this.DEFAULT_IP || ip === 'localhost'
    return {
      ip,
      endpoint: this.getEndpointURL(),
      isLocal,
      display: isLocal ? 'Local Device (127.0.0.1)' : `Remote Device (${ip})`
    }
  }

  /**
   * Validate IP address format
   */
  static isValidIP(ip) {
    if (!ip) {
      return false
    }
    const normalized = this.normalizeIP(ip)
    const ipPattern = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
    return ipPattern.test(normalized) || normalized === 'localhost' || normalized === '127.0.0.1'
  }
}

export default DeviceConnectionConfig
