/**
 * App version: 1.0.0
 * SDK version: 5.5.6
 * CLI version: 2.14.2
 * 
 * Generated: Wed, 22 Apr 2026 16:14:07 GMT
 */

var APP_com_rdkcentral_fbttest = (function () {
  'use strict';

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  const settings$3 = {};
  const subscribers$1 = {};
  const initSettings$1 = (appSettings, platformSettings) => {
    settings$3['app'] = appSettings;
    settings$3['platform'] = platformSettings;
    settings$3['user'] = {};
  };
  const publish$1 = (key, value) => {
    subscribers$1[key] && subscribers$1[key].forEach(subscriber => subscriber(value));
  };
  const dotGrab$2 = function () {
    let obj = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};
    let key = arguments.length > 1 ? arguments[1] : undefined;
    if (obj === null) return undefined;
    const keys = key.split('.');
    for (let i = 0; i < keys.length; i++) {
      obj = obj[keys[i]] = obj[keys[i]] !== undefined ? obj[keys[i]] : {};
    }
    return typeof obj === 'object' && obj !== null ? Object.keys(obj).length ? obj : undefined : obj;
  };
  var Settings$2 = {
    get(type, key) {
      let fallback = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : undefined;
      const val = dotGrab$2(settings$3[type], key);
      return val !== undefined ? val : fallback;
    },
    has(type, key) {
      return !!this.get(type, key);
    },
    set(key, value) {
      settings$3['user'][key] = value;
      publish$1(key, value);
    },
    subscribe(key, callback) {
      subscribers$1[key] = subscribers$1[key] || [];
      subscribers$1[key].push(callback);
    },
    unsubscribe(key, callback) {
      if (callback) {
        const index = subscribers$1[key] && subscribers$1[key].findIndex(cb => cb === callback);
        index > -1 && subscribers$1[key].splice(index, 1);
      } else {
        if (key in subscribers$1) {
          subscribers$1[key] = [];
        }
      }
    },
    clearSubscribers() {
      for (const key of Object.getOwnPropertyNames(subscribers$1)) {
        delete subscribers$1[key];
      }
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const prepLog$1 = (type, args) => {
    const colors = {
      Info: 'green',
      Debug: 'gray',
      Warn: 'orange',
      Error: 'red'
    };
    args = Array.from(args);
    return ['%c' + (args.length > 1 && typeof args[0] === 'string' ? args.shift() : type), 'background-color: ' + colors[type] + '; color: white; padding: 2px 4px; border-radius: 2px', args];
  };
  var Log$1 = {
    info() {
      Settings$2.get('platform', 'log') && console.log.apply(console, prepLog$1('Info', arguments));
    },
    debug() {
      Settings$2.get('platform', 'log') && console.debug.apply(console, prepLog$1('Debug', arguments));
    },
    error() {
      Settings$2.get('platform', 'log') && console.error.apply(console, prepLog$1('Error', arguments));
    },
    warn() {
      Settings$2.get('platform', 'log') && console.warn.apply(console, prepLog$1('Warn', arguments));
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  var Lightning$1 = window.lng;

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class ColorShift extends Lightning$1.shaders.WebGLDefaultShader {
    set brightness(v) {
      this._brightness = (v - 50) / 100;
      this.redraw();
    }
    set contrast(v) {
      this._contrast = (v + 50) / 100;
      this.redraw();
    }
    set gamma(v) {
      this._gamma = (v + 50) / 100;
      this.redraw();
    }
    setupUniforms(operation) {
      super.setupUniforms(operation);
      const gl = this.gl;
      this._setUniform('colorAdjust', [this._brightness || 0.0, this._contrast || 1.0, this._gamma || 1.0], gl.uniform3fv);
    }
  }
  ColorShift.before = "\n    #ifdef GL_ES\n    # ifdef GL_FRAGMENT_PRECISION_HIGH\n    precision highp float;\n    # else\n    precision lowp float;\n    # endif\n    #endif\n        \n    varying vec2 vTextureCoord;\n    varying vec4 vColor;\n    uniform sampler2D uSampler;\n    uniform vec3 colorAdjust;\n    \n    const mat3 RGBtoOpponentMat = mat3(0.2814, -0.0971, -0.0930, 0.6938, 0.1458,-0.2529, 0.0638, -0.0250, 0.4665);\n    const mat3 OpponentToRGBMat = mat3(1.1677, 0.9014, 0.7214, -6.4315, 2.5970, 0.1257, -0.5044, 0.0159, 2.0517);    \n";
  ColorShift.after = "    \n    vec3 brightnessContrast(vec3 value, float brightness, float contrast)\n    {\n        return (value - 0.5) * contrast + 0.5 + brightness;\n    }   \n    \n    vec3 updateGamma(vec3 value, float param)\n    {\n        return vec3(pow(abs(value.r), param),pow(abs(value.g), param),pow(abs(value.b), param));\n    } \n       \n    void main(void){\n        vec4 fragColor = texture2D(uSampler, vTextureCoord);        \n        vec4 color = filter(fragColor) * vColor;       \n        \n        vec3 bc = brightnessContrast(color.rgb,colorAdjust[0],colorAdjust[1]);        \n        vec3 ga = updateGamma(bc.rgb, colorAdjust[2]);  \n              \n        gl_FragColor = vec4(ga.rgb, color.a);          \n    }    \n";

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class ProtanopiaShader extends ColorShift {}
  ProtanopiaShader.fragmentShaderSource = "\n    ".concat(ColorShift.before, "    \n    vec4 vision(vec4 color)\n    {\n        vec4 r = vec4( 0.20,  0.99, -0.19, 0.0);\n        vec4 g = vec4( 0.16,  0.79,  0.04, 0.0);\n        vec4 b = vec4( 0.01, -0.01,  1.00, 0.0);\n       \n        return vec4(dot(color, r), dot(color, g), dot(color, b), color.a);\t\n    }\n    \n    vec4 filter( vec4 color )\n    {   \n        vec3 opponentColor = RGBtoOpponentMat * vec3(color.r, color.g, color.b);\n        opponentColor.x -= opponentColor.y * 1.5; \n        vec3 rgbColor = OpponentToRGBMat * opponentColor;\n        return vision(vec4(rgbColor.r, rgbColor.g, rgbColor.b, color.a));      \n    }    \n    ").concat(ColorShift.after, " \n");

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class DeuteranopiaShader extends ColorShift {}
  DeuteranopiaShader.fragmentShaderSource = "\n    ".concat(ColorShift.before, "\n    vec4 vision(vec4 color)\n    {\n        vec4 r = vec4( 0.43,  0.72, -0.15, 0.0 );\n        vec4 g = vec4( 0.34,  0.57,  0.09, 0.0 );\n        vec4 b = vec4(-0.02,  0.03,  1.00, 0.0 );\n       \n        return vec4(dot(color, r), dot(color, g), dot(color, b), color.a);\t\n    }\n       \n    vec4 filter( vec4 color )\n    {   \n        vec3 opponentColor = RGBtoOpponentMat * vec3(color.r, color.g, color.b);\n        opponentColor.x -= opponentColor.y * 1.5; \n        vec3 rgbColor = OpponentToRGBMat * opponentColor;\n        return vision(vec4(rgbColor.r, rgbColor.g, rgbColor.b, color.a));    \n    }\n    ").concat(ColorShift.after, "    \n");

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class TritanopiaShader extends ColorShift {}
  TritanopiaShader.fragmentShaderSource = "\n    ".concat(ColorShift.before, "    \n    vec4 vision(vec4 color)\n    {\n        vec4 r = vec4( 0.97,  0.11, -0.08, 0.0 );\n        vec4 g = vec4( 0.02,  0.82,  0.16, 0.0 );\n        vec4 b = vec4(-0.06,  0.88,  0.18, 0.0 );\n       \n        return vec4(dot(color, r), dot(color, g), dot(color, b), color.a);\t\n    }   \n    \n    vec4 filter( vec4 color )\n    {   \n        vec3 opponentColor = RGBtoOpponentMat * vec3(color.r, color.g, color.b);\n        opponentColor.x -= ((3.0 * opponentColor.z) - opponentColor.y) * 0.25;\n        vec3 rgbColor = OpponentToRGBMat * opponentColor;\n        return vision(vec4(rgbColor.r, rgbColor.g, rgbColor.b, color.a));\n    }   \n    ").concat(ColorShift.after, " \n");

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class NeutralShader extends ColorShift {}
  NeutralShader.fragmentShaderSource = "\n    ".concat(ColorShift.before, "\n    vec4 filter( vec4 color )\n    {\n        return color;\n    }\n    ").concat(ColorShift.after, "\n");

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class MonochromacyShader extends ColorShift {}
  MonochromacyShader.fragmentShaderSource = "\n    ".concat(ColorShift.before, "\n    vec4 filter( vec4 color )\n    {   \n        float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));\n        return vec4(vec3(grey, grey, grey), 1.0 ); \n    }\n    ").concat(ColorShift.after, "\n");

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const colorshiftShader = type => {
    const shadersMap = {
      normal: NeutralShader,
      monochromacy: MonochromacyShader,
      deuteranopia: DeuteranopiaShader,
      tritanopia: TritanopiaShader,
      protanopia: ProtanopiaShader
    };
    type = typeof type === 'string' && type.toLowerCase() || null;
    return Object.keys(shadersMap).indexOf(type) > -1 ? shadersMap[type] : false;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /* global SpeechSynthesisErrorEvent */
  function flattenStrings() {
    let series = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : [];
    const flattenedSeries = [];
    for (var i = 0; i < series.length; i++) {
      if (typeof series[i] === 'string' && !series[i].includes('PAUSE-')) {
        flattenedSeries.push(series[i]);
      } else {
        break;
      }
    }
    // add a "word boundary" to ensure the Announcer doesn't automatically try to
    // interpret strings that look like dates but are not actually dates
    // for example, if "Rising Sun" and "1993" are meant to be two separate lines,
    // when read together, "Sun 1993" is interpretted as "Sunday 1993"
    return [flattenedSeries.join(',\b ')].concat(series.slice(i));
  }
  function delay(pause) {
    return new Promise(resolve => {
      setTimeout(resolve, pause);
    });
  }

  /**
   * Speak a string
   *
   * @param {string} phrase Phrase to speak
   * @param {SpeechSynthesisUtterance[]} utterances An array which the new SpeechSynthesisUtterance instance representing this utterance will be appended
   * @return {Promise<void>} Promise resolved when the utterance has finished speaking, and rejected if there's an error
   */
  function speak(phrase, utterances) {
    let lang = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 'en-US';
    const synth = window.speechSynthesis;
    return new Promise((resolve, reject) => {
      const utterance = new SpeechSynthesisUtterance(phrase);
      utterance.lang = lang;
      utterance.onend = () => {
        resolve();
      };
      utterance.onerror = e => {
        reject(e);
      };
      utterances.push(utterance);
      synth.speak(utterance);
    });
  }
  function speakSeries(series, lang) {
    let root = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : true;
    const synth = window.speechSynthesis;
    const remainingPhrases = flattenStrings(Array.isArray(series) ? series : [series]);
    const nestedSeriesResults = [];
    /*
      We hold this array of SpeechSynthesisUtterances in order to prevent them from being
      garbage collected prematurely on STB hardware which can cause the 'onend' events of
      utterances to not fire consistently.
    */
    const utterances = [];
    let active = true;
    const seriesChain = (async () => {
      try {
        while (active && remainingPhrases.length) {
          const phrase = await Promise.resolve(remainingPhrases.shift());
          if (!active) {
            // Exit
            // Need to check this after the await in case it was cancelled in between
            break;
          } else if (typeof phrase === 'string' && phrase.includes('PAUSE-')) {
            // Pause it
            let pause = phrase.split('PAUSE-')[1] * 1000;
            if (isNaN(pause)) {
              pause = 0;
            }
            await delay(pause);
          } else if (typeof phrase === 'string' && phrase.length) {
            // Speak it
            const totalRetries = 3;
            let retriesLeft = totalRetries;
            while (active && retriesLeft > 0) {
              try {
                await speak(phrase, utterances, lang);
                retriesLeft = 0;
              } catch (e) {
                // eslint-disable-next-line no-undef
                if (e instanceof SpeechSynthesisErrorEvent) {
                  if (e.error === 'network') {
                    retriesLeft--;
                    console.warn("Speech synthesis network error. Retries left: ".concat(retriesLeft));
                    await delay(500 * (totalRetries - retriesLeft));
                  } else if (e.error === 'canceled' || e.error === 'interrupted') {
                    // Cancel or interrupt error (ignore)
                    retriesLeft = 0;
                  } else {
                    throw new Error("SpeechSynthesisErrorEvent: ".concat(e.error));
                  }
                } else {
                  throw e;
                }
              }
            }
          } else if (typeof phrase === 'function') {
            const seriesResult = speakSeries(phrase(), lang, false);
            nestedSeriesResults.push(seriesResult);
            await seriesResult.series;
          } else if (Array.isArray(phrase)) {
            // Speak it (recursively)
            const seriesResult = speakSeries(phrase, lang, false);
            nestedSeriesResults.push(seriesResult);
            await seriesResult.series;
          }
        }
      } finally {
        active = false;
      }
    })();
    return {
      series: seriesChain,
      get active() {
        return active;
      },
      append: toSpeak => {
        remainingPhrases.push(toSpeak);
      },
      cancel: () => {
        if (!active) {
          return;
        }
        if (root) {
          synth.cancel();
        }
        nestedSeriesResults.forEach(nestedSeriesResults => {
          nestedSeriesResults.cancel();
        });
        active = false;
      }
    };
  }
  let currentSeries;
  function SpeechEngine (toSpeak, lang) {
    currentSeries && currentSeries.cancel();
    currentSeries = speakSeries(toSpeak, lang);
    return currentSeries;
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   *
   * Code from: https://github.com/jashkenas/underscore is
   * Copyright (c) 2009-2022 Jeremy Ashkenas, Julian Gonggrijp, and DocumentCloud and Investigative Reporters & Editors
   * Licensed under the MIT License based off:
   * http://unscriptable.com/2009/03/20/debouncing-javascript-methods/ which is:
   * Copyright (c) 2007-2009 unscriptable.com and John M. Hann
   * Licensed under the MIT License (with X11 advertising exception)
   */

  function getElmName(elm) {
    return elm.ref || elm.constructor.name;
  }

  /**
   * Returns a function, that, as long as it continues to be invoked, will not
   * be triggered. The function will be called after it stops being called for
   * N milliseconds. If `immediate` is passed, trigger the function on the
   * leading edge, instead of the trailing. The function also has a property 'clear'
   * that is a function which will clear the timer to prevent previously scheduled executions.
   *
   * @source underscore.js
   * @see http://unscriptable.com/2009/03/20/debouncing-javascript-methods/
   * @param {Function} function to wrap
   * @param {Number} timeout in ms (`100`)
   * @param {Boolean} whether to execute at the beginning (`false`)
   * @api public
   */
  function debounce(func, wait, immediate) {
    var timeout, args, context, timestamp, result;
    if (null == wait) wait = 100;
    function later() {
      var last = Date.now() - timestamp;
      if (last < wait && last >= 0) {
        timeout = setTimeout(later, wait - last);
      } else {
        timeout = null;
        if (!immediate) {
          result = func.apply(context, args);
          context = args = null;
        }
      }
    }
    var debounced = function () {
      context = this;
      args = arguments;
      timestamp = Date.now();
      var callNow = immediate && !timeout;
      if (!timeout) timeout = setTimeout(later, wait);
      if (callNow) {
        result = func.apply(context, args);
        context = args = null;
      }
      return result;
    };
    debounced.clear = function () {
      if (timeout) {
        clearTimeout(timeout);
        timeout = null;
      }
    };
    debounced.flush = function () {
      if (timeout) {
        result = func.apply(context, args);
        context = args = null;
        clearTimeout(timeout);
        timeout = null;
      }
    };
    return debounced;
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let resetFocusPathTimer;
  let prevFocusPath = [];
  let currentlySpeaking;
  let voiceOutDisabled = false;
  const fiveMinutes = 300000;
  function onFocusChangeCore() {
    let focusPath = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : [];
    if (!Announcer.enabled) {
      return;
    }
    const loaded = focusPath.every(elm => !elm.loading);
    const focusDiff = focusPath.filter(elm => !prevFocusPath.includes(elm));
    resetFocusPathTimer();
    if (!loaded) {
      Announcer.onFocusChange();
      return;
    }
    prevFocusPath = focusPath.slice(0);
    let toAnnounceText = [];
    let toAnnounce = focusDiff.reduce((acc, elm) => {
      if (elm.announce) {
        acc.push([getElmName(elm), 'Announce', elm.announce]);
        toAnnounceText.push(elm.announce);
      } else if (elm.title) {
        acc.push([getElmName(elm), 'Title', elm.title]);
        toAnnounceText.push(elm.title);
      }
      return acc;
    }, []);
    focusDiff.reverse().reduce((acc, elm) => {
      if (elm.announceContext) {
        acc.push([getElmName(elm), 'Context', elm.announceContext]);
        toAnnounceText.push(elm.announceContext);
      } else {
        acc.push([getElmName(elm), 'No Context', '']);
      }
      return acc;
    }, toAnnounce);
    if (Announcer.debug) {
      console.table(toAnnounce);
    }
    if (toAnnounceText.length) {
      return Announcer.speak(toAnnounceText.reduce((acc, val) => acc.concat(val), []));
    }
  }
  function textToSpeech(toSpeak) {
    if (voiceOutDisabled) {
      return;
    }
    return currentlySpeaking = SpeechEngine(toSpeak);
  }
  const Announcer = {
    enabled: true,
    debug: false,
    cancel: function () {
      currentlySpeaking && currentlySpeaking.cancel();
    },
    clearPrevFocus: function () {
      let depth = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
      prevFocusPath = prevFocusPath.slice(0, depth);
      resetFocusPathTimer();
    },
    speak: function (text) {
      let {
        append = false,
        notification = false
      } = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
      if (Announcer.enabled) {
        Announcer.onFocusChange.flush();
        if (append && currentlySpeaking && currentlySpeaking.active) {
          currentlySpeaking.append(text);
        } else {
          Announcer.cancel();
          textToSpeech(text);
        }
        if (notification) {
          voiceOutDisabled = true;
          currentlySpeaking.series.finally(() => {
            voiceOutDisabled = false;
            Announcer.refresh();
          });
        }
      }
      return currentlySpeaking;
    },
    setupTimers: function () {
      let {
        focusDebounce = 400,
        focusChangeTimeout = fiveMinutes
      } = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};
      Announcer.onFocusChange = debounce(onFocusChangeCore, focusDebounce);
      resetFocusPathTimer = debounce(() => {
        // Reset focus path for full announce
        prevFocusPath = [];
      }, focusChangeTimeout);
    }
  };
  Announcer.setupTimers();

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  var Accessibility = {
    Announcer,
    colorshift(component) {
      let type = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : false;
      let config = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {
        brightness: 50,
        contrast: 50,
        gamma: 50
      };
      config = {
        ...{
          brightness: 50,
          contrast: 50,
          gamma: 50
        },
        ...config
      };
      const shader = type && colorshiftShader(type);
      if (shader) {
        Log$1.info('Accessibility Colorshift', type, config);
        component.rtt = true;
        component.shader = {
          type: shader,
          ...config
        };
      } else {
        Log$1.info('Accessibility Colorshift', 'Disabled');
        component.rtt = false;
        component.shader = null;
      }
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  let Log;
  let Settings$1;
  let ApplicationInstance$1;
  let Ads$1;
  let Lightning;
  const initLightningSdkPlugin = {
    set log(v) {
      Log = v;
    },
    set settings(v) {
      Settings$1 = v;
    },
    set ads(v) {
      Ads$1 = v;
    },
    set lightning(v) {
      Lightning = v;
    },
    set appInstance(v) {
      ApplicationInstance$1 = v;
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const initMetrics = config => {
    sendMetric = config.sendMetric;
  };
  let sendMetric = (type, event, params) => {
    Log.info('Sending metric', type, event, params);
  };

  // available metric per category
  const metrics$1 = {
    app: ['launch', 'loaded', 'ready', 'close'],
    page: ['view', 'leave'],
    user: ['click', 'input'],
    media: ['abort', 'canplay', 'ended', 'pause', 'play',
    // with some videos there occur almost constant suspend events ... should investigate
    // 'suspend',
    'volumechange', 'waiting', 'seeking', 'seeked']
  };

  // error metric function (added to each category)
  const errorMetric = function (type, message, code, visible) {
    let params = arguments.length > 4 && arguments[4] !== undefined ? arguments[4] : {};
    params = {
      params,
      ...{
        message,
        code,
        visible
      }
    };
    sendMetric(type, 'error', params);
  };
  const Metric = function (type, events) {
    let options = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {};
    return events.reduce((obj, event) => {
      obj[event] = function (name) {
        let params = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
        params = {
          ...options,
          ...(name ? {
            name
          } : {}),
          ...params
        };
        sendMetric(type, event, params);
      };
      return obj;
    }, {
      error(message, code, params) {
        errorMetric(type, message, code, params);
      },
      event(name, params) {
        sendMetric(type, name, params);
      }
    });
  };
  const Metrics = types => {
    return Object.keys(types).reduce((obj, type) => {
      // media metric works a bit different!
      // it's a function that accepts a url and returns an object with the available metrics
      // url is automatically passed as a param in every metric
      type === 'media' ? obj[type] = url => Metric(type, types[type], {
        url
      }) : obj[type] = Metric(type, types[type]);
      return obj;
    }, {
      error: errorMetric,
      event: sendMetric
    });
  };
  var Metrics$1 = Metrics(metrics$1);

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const formatLocale = locale => {
    if (locale && locale.length === 2) {
      return "".concat(locale.toLowerCase(), "-").concat(locale.toUpperCase());
    } else {
      return locale;
    }
  };
  const getLocale = defaultValue => {
    if ('language' in navigator) {
      const locale = formatLocale(navigator.language);
      return Promise.resolve(locale);
    } else {
      return Promise.resolve(defaultValue);
    }
  };
  const getLanguage = defaultValue => {
    if ('language' in navigator) {
      const language = formatLocale(navigator.language).slice(0, 2);
      return Promise.resolve(language);
    } else {
      return Promise.resolve(defaultValue);
    }
  };
  const getCountryCode = defaultValue => {
    if ('language' in navigator) {
      const countryCode = formatLocale(navigator.language).slice(3, 5);
      return Promise.resolve(countryCode);
    } else {
      return Promise.resolve(defaultValue);
    }
  };
  const hasOrAskForGeoLocationPermission = () => {
    return new Promise(resolve => {
      // force to prompt for location permission
      if (Settings$1.get('platform', 'forceBrowserGeolocation') === true) resolve(true);
      if ('permissions' in navigator && typeof navigator.permissions.query === 'function') {
        navigator.permissions.query({
          name: 'geolocation'
        }).then(status => {
          resolve(status.state === 'granted' || status.status === 'granted');
        });
      } else {
        resolve(false);
      }
    });
  };
  const getLatLon = defaultValue => {
    return new Promise(resolve => {
      hasOrAskForGeoLocationPermission().then(granted => {
        if (granted === true) {
          if ('geolocation' in navigator) {
            navigator.geolocation.getCurrentPosition(
            // success
            result => result && result.coords && resolve([result.coords.latitude, result.coords.longitude]),
            // error
            () => resolve(defaultValue),
            // options
            {
              enableHighAccuracy: true,
              timeout: 5000,
              maximumAge: 0
            });
          } else {
            return queryForLatLon().then(result => resolve(result || defaultValue));
          }
        } else {
          return queryForLatLon().then(result => resolve(result || defaultValue));
        }
      });
    });
  };
  const queryForLatLon = () => {
    return new Promise(resolve => {
      fetch('https://geolocation-db.com/json/').then(response => response.json()).then(_ref => {
        let {
          latitude,
          longitude
        } = _ref;
        return latitude && longitude ? resolve([latitude, longitude]) : resolve(false);
      }).catch(() => resolve(false));
    });
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const defaultProfile = {
    ageRating: 'adult',
    city: 'New York',
    zipCode: '27505',
    countryCode: () => getCountryCode('US'),
    ip: '127.0.0.1',
    household: 'b2244e9d4c04826ccd5a7b2c2a50e7d4',
    language: () => getLanguage('en'),
    latlon: () => getLatLon([40.7128, 74.006]),
    locale: () => getLocale('en-US'),
    mac: '00:00:00:00:00:00',
    operator: 'metrological',
    platform: 'metrological',
    packages: [],
    uid: 'ee6723b8-7ab3-462c-8d93-dbf61227998e',
    stbType: 'metrological'
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let getInfo = key => {
    const profile = {
      ...defaultProfile,
      ...Settings$1.get('platform', 'profile')
    };
    return Promise.resolve(typeof profile[key] === 'function' ? profile[key]() : profile[key]);
  };
  let setInfo = (key, params) => {
    if (key in defaultProfile) return defaultProfile[key] = params;
  };
  const initProfile = config => {
    getInfo = config.getInfo ? config.getInfo : getInfo;
    setInfo = config.setInfo ? config.setInfo : setInfo;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const initPurchase = config => {
    if (config.billingUrl) config.billingUrl;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const defaultChannels = [{
    number: 1,
    name: 'Metro News 1',
    description: 'New York Cable News Channel',
    entitled: true,
    program: {
      title: 'The Morning Show',
      description: "New York's best morning show",
      startTime: new Date(new Date() - 60 * 5 * 1000).toUTCString(),
      // started 5 minutes ago
      duration: 60 * 30,
      // 30 minutes
      ageRating: 0
    }
  }, {
    number: 2,
    name: 'MTV',
    description: 'Music Television',
    entitled: true,
    program: {
      title: 'Beavis and Butthead',
      description: 'American adult animated sitcom created by Mike Judge',
      startTime: new Date(new Date() - 60 * 20 * 1000).toUTCString(),
      // started 20 minutes ago
      duration: 60 * 45,
      // 45 minutes
      ageRating: 18
    }
  }, {
    number: 3,
    name: 'NBC',
    description: 'NBC TV Network',
    entitled: false,
    program: {
      title: 'The Tonight Show Starring Jimmy Fallon',
      description: 'Late-night talk show hosted by Jimmy Fallon on NBC',
      startTime: new Date(new Date() - 60 * 10 * 1000).toUTCString(),
      // started 10 minutes ago
      duration: 60 * 60,
      // 1 hour
      ageRating: 10
    }
  }];
  const channels = () => Settings$1.get('platform', 'tv', defaultChannels);
  const randomChannel = () => channels()[~~(channels.length * Math.random())];

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let currentChannel;
  const callbacks = {};
  const emit$1 = function (event) {
    for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
      args[_key - 1] = arguments[_key];
    }
    callbacks[event] && callbacks[event].forEach(cb => {
      cb.apply(null, args);
    });
  };

  // local mock methods
  let methods = {
    getChannel() {
      if (!currentChannel) currentChannel = randomChannel();
      return new Promise((resolve, reject) => {
        if (currentChannel) {
          const channel = {
            ...currentChannel
          };
          delete channel.program;
          resolve(channel);
        } else {
          reject('No channel found');
        }
      });
    },
    getProgram() {
      if (!currentChannel) currentChannel = randomChannel();
      return new Promise((resolve, reject) => {
        currentChannel.program ? resolve(currentChannel.program) : reject('No program found');
      });
    },
    setChannel(number) {
      return new Promise((resolve, reject) => {
        if (number) {
          const newChannel = channels().find(c => c.number === number);
          if (newChannel) {
            currentChannel = newChannel;
            const channel = {
              ...currentChannel
            };
            delete channel.program;
            emit$1('channelChange', channel);
            resolve(channel);
          } else {
            reject('Channel not found');
          }
        } else {
          reject('No channel number supplied');
        }
      });
    }
  };
  const initTV = config => {
    methods = {};
    if (config.getChannel && typeof config.getChannel === 'function') {
      methods.getChannel = config.getChannel;
    }
    if (config.getProgram && typeof config.getProgram === 'function') {
      methods.getProgram = config.getProgram;
    }
    if (config.setChannel && typeof config.setChannel === 'function') {
      methods.setChannel = config.setChannel;
    }
    if (config.emit && typeof config.emit === 'function') {
      config.emit(emit$1);
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const initPin = config => {
    if (config.submit && typeof config.submit === 'function') {
      config.submit;
    }
    if (config.check && typeof config.check === 'function') {
      config.check;
    }
  };

  var executeAsPromise = (function (method) {
    let args = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : null;
    let context = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : null;
    let result;
    if (method && typeof method === 'function') {
      try {
        result = method.apply(context, args);
      } catch (e) {
        result = e;
      }
    } else {
      result = method;
    }

    // if it looks like a duck .. ehm ... promise and talks like a promise, let's assume it's a promise
    if (result !== null && typeof result === 'object' && result.then && typeof result.then === 'function') {
      return result;
    }
    // otherwise make it into a promise
    else {
      return new Promise((resolve, reject) => {
        if (result instanceof Error) {
          reject(result);
        } else {
          resolve(result);
        }
      });
    }
  });

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  var events$1 = {
    abort: 'Abort',
    canplay: 'CanPlay',
    canplaythrough: 'CanPlayThrough',
    durationchange: 'DurationChange',
    emptied: 'Emptied',
    encrypted: 'Encrypted',
    ended: 'Ended',
    error: 'Error',
    interruptbegin: 'InterruptBegin',
    interruptend: 'InterruptEnd',
    loadeddata: 'LoadedData',
    loadedmetadata: 'LoadedMetadata',
    loadstart: 'LoadStart',
    pause: 'Pause',
    play: 'Play',
    playing: 'Playing',
    progress: 'Progress',
    ratechange: 'Ratechange',
    seeked: 'Seeked',
    seeking: 'Seeking',
    stalled: 'Stalled',
    // suspend: 'Suspend', // this one is called a looooot for some videos
    timeupdate: 'TimeUpdate',
    volumechange: 'VolumeChange',
    waiting: 'Waiting'
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  var autoSetupMixin = (function (sourceObject) {
    let setup = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : () => {};
    let ready = false;
    const doSetup = () => {
      if (ready === false) {
        setup();
        ready = true;
      }
    };
    return Object.keys(sourceObject).reduce((obj, key) => {
      if (typeof sourceObject[key] === 'function') {
        obj[key] = function () {
          doSetup();
          return sourceObject[key].apply(sourceObject, arguments);
        };
      } else if (typeof Object.getOwnPropertyDescriptor(sourceObject, key).get === 'function') {
        obj.__defineGetter__(key, function () {
          doSetup();
          return Object.getOwnPropertyDescriptor(sourceObject, key).get.apply(sourceObject);
        });
      } else if (typeof Object.getOwnPropertyDescriptor(sourceObject, key).set === 'function') {
        obj.__defineSetter__(key, function () {
          doSetup();
          return Object.getOwnPropertyDescriptor(sourceObject, key).set.sourceObject[key].apply(sourceObject, arguments);
        });
      } else {
        obj[key] = sourceObject[key];
      }
      return obj;
    }, {});
  });

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  let timeout = null;
  var easeExecution = (cb, delay) => {
    clearTimeout(timeout);
    timeout = setTimeout(() => {
      cb();
    }, delay);
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  var VideoTexture = () => {
    return class VideoTexture extends Lightning.Component {
      static _template() {
        return {
          Video: {
            alpha: 1,
            visible: false,
            pivot: 0.5,
            texture: {
              type: Lightning.textures.StaticTexture,
              options: {}
            }
          }
        };
      }
      set videoEl(v) {
        this._videoEl = v;
      }
      get videoEl() {
        return this._videoEl;
      }
      get videoView() {
        return this.tag('Video');
      }
      get videoTexture() {
        return this.videoView.texture;
      }
      get isVisible() {
        return this.videoView.alpha === 1 && this.videoView.visible === true;
      }
      _init() {
        this._createVideoTexture();
      }
      _createVideoTexture() {
        const stage = this.stage;
        const gl = stage.gl;
        const glTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, glTexture);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        this.videoTexture.options = {
          source: glTexture,
          w: this.videoEl.width,
          h: this.videoEl.height
        };
        this.videoView.w = this.videoEl.width / this.stage.getRenderPrecision();
        this.videoView.h = this.videoEl.height / this.stage.getRenderPrecision();
      }
      start() {
        const stage = this.stage;
        this._lastTime = 0;
        if (!this._updateVideoTexture) {
          this._updateVideoTexture = () => {
            if (this.videoTexture.options.source && this.videoEl.videoWidth && this.active) {
              const gl = stage.gl;
              const currentTime = new Date().getTime();
              const getVideoPlaybackQuality = this.videoEl.getVideoPlaybackQuality();

              // When BR2_PACKAGE_GST1_PLUGINS_BAD_PLUGIN_DEBUGUTILS is not set in WPE, webkitDecodedFrameCount will not be available.
              // We'll fallback to fixed 30fps in this case.
              // As 'webkitDecodedFrameCount' is about to deprecate, check for the 'totalVideoFrames'
              const frameCount = getVideoPlaybackQuality ? getVideoPlaybackQuality.totalVideoFrames : this.videoEl.webkitDecodedFrameCount;
              const mustUpdate = frameCount ? this._lastFrame !== frameCount : this._lastTime < currentTime - 30;
              if (mustUpdate) {
                this._lastTime = currentTime;
                this._lastFrame = frameCount;
                try {
                  gl.bindTexture(gl.TEXTURE_2D, this.videoTexture.options.source);
                  gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
                  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.videoEl);
                  this._lastFrame = this.videoEl.webkitDecodedFrameCount;
                  this.videoView.visible = true;
                  this.videoTexture.options.w = this.videoEl.width;
                  this.videoTexture.options.h = this.videoEl.height;
                  const expectedAspectRatio = this.videoView.w / this.videoView.h;
                  const realAspectRatio = this.videoEl.width / this.videoEl.height;
                  if (expectedAspectRatio > realAspectRatio) {
                    this.videoView.scaleX = realAspectRatio / expectedAspectRatio;
                    this.videoView.scaleY = 1;
                  } else {
                    this.videoView.scaleY = expectedAspectRatio / realAspectRatio;
                    this.videoView.scaleX = 1;
                  }
                } catch (e) {
                  Log.error('texImage2d video', e);
                  this.stop();
                }
                this.videoTexture.source.forceRenderUpdate();
              }
            }
          };
        }
        if (!this._updatingVideoTexture) {
          stage.on('frameStart', this._updateVideoTexture);
          this._updatingVideoTexture = true;
        }
      }
      stop() {
        const stage = this.stage;
        stage.removeListener('frameStart', this._updateVideoTexture);
        this._updatingVideoTexture = false;
        this.videoView.visible = false;
        if (this.videoTexture.options.source) {
          const gl = stage.gl;
          gl.bindTexture(gl.TEXTURE_2D, this.videoTexture.options.source);
          gl.clearColor(0, 0, 0, 1);
          gl.clear(gl.COLOR_BUFFER_BIT);
        }
      }
      position(top, left) {
        this.videoView.patch({
          x: left,
          y: top
        });
      }
      size(width, height) {
        this.videoView.patch({
          w: width,
          h: height
        });
      }
      show() {
        this.videoView.alpha = 1;
      }
      hide() {
        this.videoView.alpha = 0;
      }
    };
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let mediaUrl$1 = url => url;
  let videoEl;
  let videoTexture;
  let metrics;
  let consumer$1;
  let precision = 1;
  let textureMode = false;
  const initVideoPlayer = config => {
    if (config.mediaUrl) {
      mediaUrl$1 = config.mediaUrl;
    }
  };
  // todo: add this in a 'Registry' plugin
  // to be able to always clean this up on app close
  let eventHandlers = {};
  const state$1 = {
    adsEnabled: false,
    playing: false,
    _playingAds: false,
    get playingAds() {
      return this._playingAds;
    },
    set playingAds(val) {
      if (this._playingAds !== val) {
        this._playingAds = val;
        fireOnConsumer$1(val === true ? 'AdStart' : 'AdEnd');
      }
    },
    skipTime: false,
    playAfterSeek: null
  };
  const hooks = {
    play() {
      state$1.playing = true;
    },
    pause() {
      state$1.playing = false;
    },
    seeked() {
      state$1.playAfterSeek === true && videoPlayerPlugin.play();
      state$1.playAfterSeek = null;
    },
    abort() {
      deregisterEventListeners();
    }
  };
  const withPrecision = val => Math.round(precision * val) + 'px';
  const fireOnConsumer$1 = (event, args) => {
    if (consumer$1) {
      consumer$1.fire('$videoPlayer' + event, args, videoEl.currentTime);
      consumer$1.fire('$videoPlayerEvent', event, args, videoEl.currentTime);
    }
  };
  const fireHook = (event, args) => {
    hooks[event] && typeof hooks[event] === 'function' && hooks[event].call(null, event, args);
  };
  let customLoader = null;
  let customUnloader = null;
  const loader$1 = (url, videoEl, config) => {
    return customLoader && typeof customLoader === 'function' ? customLoader(url, videoEl, config) : new Promise(resolve => {
      url = mediaUrl$1(url);
      videoEl.setAttribute('src', url);
      videoEl.load();
      resolve();
    });
  };
  const unloader = videoEl => {
    return customUnloader && typeof customUnloader === 'function' ? customUnloader(videoEl) : new Promise(resolve => {
      videoEl.removeAttribute('src');
      videoEl.load();
      resolve();
    });
  };
  const setupVideoTag = () => {
    const videoEls = document.getElementsByTagName('video');
    if (videoEls && videoEls.length) {
      return videoEls[0];
    } else {
      const videoEl = document.createElement('video');
      const platformSettingsWidth = Settings$1.get('platform', 'width') ? Settings$1.get('platform', 'width') : 1920;
      const platformSettingsHeight = Settings$1.get('platform', 'height') ? Settings$1.get('platform', 'height') : 1080;
      videoEl.setAttribute('id', 'video-player');
      videoEl.setAttribute('width', withPrecision(platformSettingsWidth));
      videoEl.setAttribute('height', withPrecision(platformSettingsHeight));
      videoEl.style.position = 'absolute';
      videoEl.style.zIndex = '1';
      videoEl.style.display = 'none';
      videoEl.style.visibility = 'hidden';
      videoEl.style.top = withPrecision(0);
      videoEl.style.left = withPrecision(0);
      videoEl.style.width = withPrecision(platformSettingsWidth);
      videoEl.style.height = withPrecision(platformSettingsHeight);
      document.body.appendChild(videoEl);
      return videoEl;
    }
  };
  const setUpVideoTexture = () => {
    if (!ApplicationInstance$1.tag('VideoTexture')) {
      const el = ApplicationInstance$1.stage.c({
        type: VideoTexture(),
        ref: 'VideoTexture',
        zIndex: 0,
        videoEl
      });
      ApplicationInstance$1.childList.addAt(el, 0);
    }
    return ApplicationInstance$1.tag('VideoTexture');
  };
  const registerEventListeners = () => {
    Log.info('VideoPlayer', 'Registering event listeners');
    Object.keys(events$1).forEach(event => {
      const handler = e => {
        // Fire a metric for each event (if it exists on the metrics object)
        if (metrics && metrics[event] && typeof metrics[event] === 'function') {
          metrics[event]({
            currentTime: videoEl.currentTime
          });
        }
        // fire an internal hook
        fireHook(event, {
          videoElement: videoEl,
          event: e
        });

        // fire the event (with human friendly event name) to the consumer of the VideoPlayer
        fireOnConsumer$1(events$1[event], {
          videoElement: videoEl,
          event: e
        });
      };
      eventHandlers[event] = handler;
      videoEl.addEventListener(event, handler);
    });
  };
  const deregisterEventListeners = () => {
    Log.info('VideoPlayer', 'Deregistering event listeners');
    Object.keys(eventHandlers).forEach(event => {
      videoEl.removeEventListener(event, eventHandlers[event]);
    });
    eventHandlers = {};
  };
  const videoPlayerPlugin = {
    consumer(component) {
      consumer$1 = component;
    },
    loader(loaderFn) {
      customLoader = loaderFn;
    },
    unloader(unloaderFn) {
      customUnloader = unloaderFn;
    },
    position() {
      let top = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
      let left = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
      videoEl.style.left = withPrecision(left);
      videoEl.style.top = withPrecision(top);
      if (textureMode === true) {
        videoTexture.position(top, left);
      }
    },
    size() {
      let width = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 1920;
      let height = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 1080;
      videoEl.style.width = withPrecision(width);
      videoEl.style.height = withPrecision(height);
      videoEl.width = parseFloat(videoEl.style.width);
      videoEl.height = parseFloat(videoEl.style.height);
      if (textureMode === true) {
        videoTexture.size(width, height);
      }
    },
    area() {
      let top = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
      let right = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 1920;
      let bottom = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 1080;
      let left = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : 0;
      this.position(top, left);
      this.size(right - left, bottom - top);
    },
    open(url) {
      let config = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
      if (!this.canInteract) return;
      metrics = Metrics$1.media(url);
      this.hide();
      deregisterEventListeners();
      if (this.src == url) {
        this.clear().then(this.open(url, config));
      } else {
        const adConfig = {
          enabled: state$1.adsEnabled,
          duration: 300
        };
        if (config.videoId) {
          adConfig.caid = config.videoId;
        }
        Ads$1.get(adConfig, consumer$1).then(ads => {
          state$1.playingAds = true;
          ads.prerolls().then(() => {
            state$1.playingAds = false;
            loader$1(url, videoEl, config).then(() => {
              registerEventListeners();
              this.show();
              this.play();
            }).catch(e => {
              fireOnConsumer$1('Error', {
                videoElement: videoEl,
                event: e
              });

              // This is not API-compliant, as it results in firing "$videoPlayererror" rather than "$videoPlayerError".
              // See docs here for API-compliant events -> https://github.com/Metrological/metrological-sdk/blob/master/docs/plugins/videoplayer.md#event-overview
              // It has been kept for backwards compatability for library consumers who may have already written handler functions to match it.
              fireOnConsumer$1('error', {
                videoElement: videoEl,
                event: e
              });
            });
          });
        });
      }
    },
    reload() {
      if (!this.canInteract) return;
      const url = videoEl.getAttribute('src');
      this.close();
      this.open(url);
    },
    close() {
      Ads$1.cancel();
      if (state$1.playingAds) {
        state$1.playingAds = false;
        Ads$1.stop();
        // call self in next tick
        setTimeout(() => {
          this.close();
        });
      }
      if (!this.canInteract) return;
      this.clear();
      this.hide();
      deregisterEventListeners();
    },
    clear() {
      if (!this.canInteract) return;
      // pause the video first to disable sound
      this.pause();
      if (textureMode === true) videoTexture.stop();
      return unloader(videoEl).then(() => {
        fireOnConsumer$1('Clear', {
          videoElement: videoEl
        });
      });
    },
    play() {
      if (!this.canInteract) return;
      if (textureMode === true) videoTexture.start();
      executeAsPromise(videoEl.play, null, videoEl).catch(e => {
        fireOnConsumer$1('Error', {
          videoElement: videoEl,
          event: e
        });

        // This is not API-compliant, as it results in firing "$videoPlayererror" rather than "$videoPlayerError".
        // See docs here for API-compliant events -> https://github.com/Metrological/metrological-sdk/blob/master/docs/plugins/videoplayer.md#event-overview
        // It has been kept for backwards compatability for library consumers who may have already written handler functions to match it.
        fireOnConsumer$1('error', {
          videoElement: videoEl,
          event: e
        });
      });
    },
    pause() {
      if (!this.canInteract) return;
      videoEl.pause();
    },
    playPause() {
      if (!this.canInteract) return;
      this.playing === true ? this.pause() : this.play();
    },
    mute() {
      let muted = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : true;
      if (!this.canInteract) return;
      videoEl.muted = muted;
    },
    loop() {
      let looped = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : true;
      videoEl.loop = looped;
    },
    seek(time) {
      if (!this.canInteract) return;
      if (!this.src) return;
      // define whether should continue to play after seek is complete (in seeked hook)
      if (state$1.playAfterSeek === null) {
        state$1.playAfterSeek = !!state$1.playing;
      }
      // pause before actually seeking
      this.pause();
      // currentTime always between 0 and the duration of the video (minus 0.1s to not set to the final frame and stall the video)
      videoEl.currentTime = Math.max(0, Math.min(time, this.duration - 0.1));
    },
    skip(seconds) {
      if (!this.canInteract) return;
      if (!this.src) return;
      state$1.skipTime = (state$1.skipTime || videoEl.currentTime) + seconds;
      easeExecution(() => {
        this.seek(state$1.skipTime);
        state$1.skipTime = false;
      }, 300);
    },
    show() {
      if (!this.canInteract) return;
      if (textureMode === true) {
        videoTexture.show();
      } else {
        videoEl.style.display = 'block';
        videoEl.style.visibility = 'visible';
      }
    },
    hide() {
      if (!this.canInteract) return;
      if (textureMode === true) {
        videoTexture.hide();
      } else {
        videoEl.style.display = 'none';
        videoEl.style.visibility = 'hidden';
      }
    },
    enableAds() {
      let enabled = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : true;
      state$1.adsEnabled = enabled;
    },
    /* Public getters */
    get duration() {
      return videoEl && (isNaN(videoEl.duration) ? Infinity : videoEl.duration);
    },
    get currentTime() {
      return videoEl && videoEl.currentTime;
    },
    get muted() {
      return videoEl && videoEl.muted;
    },
    get looped() {
      return videoEl && videoEl.loop;
    },
    get src() {
      return videoEl && videoEl.getAttribute('src');
    },
    get playing() {
      return state$1.playing;
    },
    get playingAds() {
      return state$1.playingAds;
    },
    get canInteract() {
      // todo: perhaps add an extra flag wether we allow interactions (i.e. pauze, mute, etc.) during ad playback
      return state$1.playingAds === false;
    },
    get top() {
      return videoEl && parseFloat(videoEl.style.top);
    },
    get left() {
      return videoEl && parseFloat(videoEl.style.left);
    },
    get bottom() {
      return videoEl && parseFloat(videoEl.style.top - videoEl.style.height);
    },
    get right() {
      return videoEl && parseFloat(videoEl.style.left - videoEl.style.width);
    },
    get width() {
      return videoEl && parseFloat(videoEl.style.width);
    },
    get height() {
      return videoEl && parseFloat(videoEl.style.height);
    },
    get visible() {
      if (textureMode === true) {
        return videoTexture.isVisible;
      } else {
        return videoEl && videoEl.style.display === 'block';
      }
    },
    get adsEnabled() {
      return state$1.adsEnabled;
    },
    // prefixed with underscore to indicate 'semi-private'
    // because it's not recommended to interact directly with the video element
    get _videoEl() {
      return videoEl;
    },
    get _consumer() {
      return consumer$1;
    }
  };
  autoSetupMixin(videoPlayerPlugin, () => {
    precision = ApplicationInstance$1 && ApplicationInstance$1.stage && ApplicationInstance$1.stage.getRenderPrecision() || precision;
    videoEl = setupVideoTag();
    textureMode = Settings$1.get('platform', 'textureMode', false);
    if (textureMode === true) {
      videoEl.setAttribute('crossorigin', 'anonymous');
      videoTexture = setUpVideoTexture();
    }
  });

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let consumer;
  let getAds = () => {
    // todo: enable some default ads during development, maybe from the settings.json
    return Promise.resolve({
      prerolls: [],
      midrolls: [],
      postrolls: []
    });
  };
  const initAds = config => {
    if (config.getAds) {
      getAds = config.getAds;
    }
  };
  const state = {
    active: false
  };
  const playSlot = function () {
    let slot = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : [];
    return slot.reduce((promise, ad) => {
      return promise.then(() => {
        return playAd(ad);
      });
    }, Promise.resolve(null));
  };
  const playAd = ad => {
    return new Promise(resolve => {
      if (state.active === false) {
        Log$1.info('Ad', 'Skipping add due to inactive state');
        return resolve();
      }
      // is it safe to rely on videoplayer plugin already created the video tag?
      const videoEl = document.getElementsByTagName('video')[0];
      videoEl.style.display = 'block';
      videoEl.style.visibility = 'visible';
      videoEl.src = mediaUrl$1(ad.url);
      videoEl.load();
      let timeEvents = null;
      let timeout;
      const cleanup = () => {
        // remove all listeners
        Object.keys(handlers).forEach(handler => videoEl.removeEventListener(handler, handlers[handler]));
        resolve();
      };
      const handlers = {
        play() {
          Log$1.info('Ad', 'Play ad', ad.url);
          fireOnConsumer('Play', ad);
          sendBeacon(ad.callbacks, 'defaultImpression');
        },
        ended() {
          fireOnConsumer('Ended', ad);
          sendBeacon(ad.callbacks, 'complete');
          cleanup();
        },
        timeupdate() {
          if (!timeEvents && videoEl.duration) {
            // calculate when to fire the time based events (now that duration is known)
            timeEvents = {
              firstQuartile: videoEl.duration / 4,
              midPoint: videoEl.duration / 2,
              thirdQuartile: videoEl.duration / 4 * 3
            };
            Log$1.info('Ad', 'Calculated quartiles times', {
              timeEvents
            });
          }
          if (timeEvents && timeEvents.firstQuartile && videoEl.currentTime >= timeEvents.firstQuartile) {
            fireOnConsumer('FirstQuartile', ad);
            delete timeEvents.firstQuartile;
            sendBeacon(ad.callbacks, 'firstQuartile');
          }
          if (timeEvents && timeEvents.midPoint && videoEl.currentTime >= timeEvents.midPoint) {
            fireOnConsumer('MidPoint', ad);
            delete timeEvents.midPoint;
            sendBeacon(ad.callbacks, 'midPoint');
          }
          if (timeEvents && timeEvents.thirdQuartile && videoEl.currentTime >= timeEvents.thirdQuartile) {
            fireOnConsumer('ThirdQuartile', ad);
            delete timeEvents.thirdQuartile;
            sendBeacon(ad.callbacks, 'thirdQuartile');
          }
        },
        stalled() {
          fireOnConsumer('Stalled', ad);
          timeout = setTimeout(() => {
            cleanup();
          }, 5000); // make timeout configurable
        },
        canplay() {
          timeout && clearTimeout(timeout);
        },
        error() {
          fireOnConsumer('Error', ad);
          cleanup();
        },
        // this doesn't work reliably on sky box, moved logic to timeUpdate event
        // loadedmetadata() {
        //   // calculate when to fire the time based events (now that duration is known)
        //   timeEvents = {
        //     firstQuartile: videoEl.duration / 4,
        //     midPoint: videoEl.duration / 2,
        //     thirdQuartile: (videoEl.duration / 4) * 3,
        //   }
        // },
        abort() {
          cleanup();
        }
        // todo: pause, resume, mute, unmute beacons
      };
      // add all listeners
      Object.keys(handlers).forEach(handler => videoEl.addEventListener(handler, handlers[handler]));
      videoEl.play();
    });
  };
  const sendBeacon = (callbacks, event) => {
    if (callbacks && callbacks[event]) {
      Log$1.info('Ad', 'Sending beacon', event, callbacks[event]);
      return callbacks[event].reduce((promise, url) => {
        return promise.then(() => fetch(url)
        // always resolve, also in case of a fetch error (so we don't block firing the rest of the beacons for this event)
        // note: for fetch failed http responses don't throw an Error :)
        .then(response => {
          if (response.status === 200) {
            fireOnConsumer('Beacon' + event + 'Sent');
          } else {
            fireOnConsumer('Beacon' + event + 'Failed' + response.status);
          }
          Promise.resolve(null);
        }).catch(() => {
          Promise.resolve(null);
        }));
      }, Promise.resolve(null));
    } else {
      Log$1.info('Ad', 'No callback found for ' + event);
    }
  };
  const fireOnConsumer = (event, args) => {
    if (consumer) {
      consumer.fire('$ad' + event, args);
      consumer.fire('$adEvent', event, args);
    }
  };
  var Ads = {
    get(config, videoPlayerConsumer) {
      if (config.enabled === false) {
        return Promise.resolve({
          prerolls() {
            return Promise.resolve();
          }
        });
      }
      consumer = videoPlayerConsumer;
      return new Promise(resolve => {
        Log$1.info('Ad', 'Starting session');
        getAds(config).then(ads => {
          Log$1.info('Ad', 'API result', ads);
          resolve({
            prerolls() {
              if (ads.preroll) {
                state.active = true;
                fireOnConsumer('PrerollSlotImpression', ads);
                sendBeacon(ads.preroll.callbacks, 'slotImpression');
                return playSlot(ads.preroll.ads).then(() => {
                  fireOnConsumer('PrerollSlotEnd', ads);
                  sendBeacon(ads.preroll.callbacks, 'slotEnd');
                  state.active = false;
                });
              }
              return Promise.resolve();
            },
            midrolls() {
              return Promise.resolve();
            },
            postrolls() {
              return Promise.resolve();
            }
          });
        });
      });
    },
    cancel() {
      Log$1.info('Ad', 'Cancel Ad');
      state.active = false;
    },
    stop() {
      Log$1.info('Ad', 'Stop Ad');
      state.active = false;
      // fixme: duplication
      const videoEl = document.getElementsByTagName('video')[0];
      videoEl.pause();
      videoEl.removeAttribute('src');
    }
  };

  var isMergeableObject = function isMergeableObject(value) {
    return isNonNullObject(value) && !isSpecial(value);
  };
  function isNonNullObject(value) {
    return !!value && typeof value === 'object';
  }
  function isSpecial(value) {
    var stringValue = Object.prototype.toString.call(value);
    return stringValue === '[object RegExp]' || stringValue === '[object Date]' || isReactElement(value);
  }

  // see https://github.com/facebook/react/blob/b5ac963fb791d1298e7f396236383bc955f916c1/src/isomorphic/classic/element/ReactElement.js#L21-L25
  var canUseSymbol = typeof Symbol === 'function' && Symbol.for;
  var REACT_ELEMENT_TYPE = canUseSymbol ? Symbol.for('react.element') : 0xeac7;
  function isReactElement(value) {
    return value.$$typeof === REACT_ELEMENT_TYPE;
  }
  function emptyTarget(val) {
    return Array.isArray(val) ? [] : {};
  }
  function cloneUnlessOtherwiseSpecified(value, options) {
    return options.clone !== false && options.isMergeableObject(value) ? deepmerge(emptyTarget(value), value, options) : value;
  }
  function defaultArrayMerge(target, source, options) {
    return target.concat(source).map(function (element) {
      return cloneUnlessOtherwiseSpecified(element, options);
    });
  }
  function getMergeFunction(key, options) {
    if (!options.customMerge) {
      return deepmerge;
    }
    var customMerge = options.customMerge(key);
    return typeof customMerge === 'function' ? customMerge : deepmerge;
  }
  function getEnumerableOwnPropertySymbols(target) {
    return Object.getOwnPropertySymbols ? Object.getOwnPropertySymbols(target).filter(function (symbol) {
      return Object.propertyIsEnumerable.call(target, symbol);
    }) : [];
  }
  function getKeys(target) {
    return Object.keys(target).concat(getEnumerableOwnPropertySymbols(target));
  }
  function propertyIsOnObject(object, property) {
    try {
      return property in object;
    } catch (_) {
      return false;
    }
  }

  // Protects from prototype poisoning and unexpected merging up the prototype chain.
  function propertyIsUnsafe(target, key) {
    return propertyIsOnObject(target, key) // Properties are safe to merge if they don't exist in the target yet,
    && !(Object.hasOwnProperty.call(target, key) // unsafe if they exist up the prototype chain,
    && Object.propertyIsEnumerable.call(target, key)); // and also unsafe if they're nonenumerable.
  }
  function mergeObject(target, source, options) {
    var destination = {};
    if (options.isMergeableObject(target)) {
      getKeys(target).forEach(function (key) {
        destination[key] = cloneUnlessOtherwiseSpecified(target[key], options);
      });
    }
    getKeys(source).forEach(function (key) {
      if (propertyIsUnsafe(target, key)) {
        return;
      }
      if (propertyIsOnObject(target, key) && options.isMergeableObject(source[key])) {
        destination[key] = getMergeFunction(key, options)(target[key], source[key], options);
      } else {
        destination[key] = cloneUnlessOtherwiseSpecified(source[key], options);
      }
    });
    return destination;
  }
  function deepmerge(target, source, options) {
    options = options || {};
    options.arrayMerge = options.arrayMerge || defaultArrayMerge;
    options.isMergeableObject = options.isMergeableObject || isMergeableObject;
    // cloneUnlessOtherwiseSpecified is added to `options` so that custom arrayMerge()
    // implementations can use it. The caller may not replace it.
    options.cloneUnlessOtherwiseSpecified = cloneUnlessOtherwiseSpecified;
    var sourceIsArray = Array.isArray(source);
    var targetIsArray = Array.isArray(target);
    var sourceAndTargetTypesMatch = sourceIsArray === targetIsArray;
    if (!sourceAndTargetTypesMatch) {
      return cloneUnlessOtherwiseSpecified(source, options);
    } else if (sourceIsArray) {
      return options.arrayMerge(target, source, options);
    } else {
      return mergeObject(target, source, options);
    }
  }
  deepmerge.all = function deepmergeAll(array, options) {
    if (!Array.isArray(array)) {
      throw new Error('first argument should be an array');
    }
    return array.reduce(function (prev, next) {
      return deepmerge(prev, next, options);
    }, {});
  };
  var deepmerge_1 = deepmerge;
  var cjs = deepmerge_1;

  /*!
   * is-plain-object <https://github.com/jonschlinkert/is-plain-object>
   *
   * Copyright (c) 2014-2017, Jon Schlinkert.
   * Released under the MIT License.
   */

  function isObject$2(o) {
    return Object.prototype.toString.call(o) === '[object Object]';
  }

  function isPlainObject(o) {
    var ctor,prot;

    if (isObject$2(o) === false) return false;

    // If has modified constructor
    ctor = o.constructor;
    if (ctor === undefined) return true;

    // If has modified prototype
    prot = ctor.prototype;
    if (isObject$2(prot) === false) return false;

    // If constructor does not have an Object-specific method
    if (prot.hasOwnProperty('isPrototypeOf') === false) {
      return false;
    }

    // Most likely a plain Object
    return true;
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let warned = false;
  const deprecated$1 = function () {
    let force = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;
    if (force === true || warned === false) {
      console.warn(["The 'Locale'-plugin in the Lightning-SDK is deprecated and will be removed in future releases.", "Please consider using the new 'Language'-plugin instead.", 'https://rdkcentral.github.io/Lightning-SDK/#/plugins/language'].join('\n\n'));
    }
    warned = true;
  };
  class Locale {
    constructor() {
      this.__enabled = false;
    }

    /**
     * Loads translation object from external json file.
     *
     * @param {String} path Path to resource.
     * @return {Promise}
     */
    async load(path) {
      if (!this.__enabled) {
        return;
      }
      await fetch(path).then(resp => resp.json()).then(resp => {
        this.loadFromObject(resp);
      });
    }

    /**
     * Sets language used by module.
     *
     * @param {String} lang
     */
    setLanguage(lang) {
      deprecated$1();
      this.__enabled = true;
      this.language = lang;
    }

    /**
     * Returns reference to translation object for current language.
     *
     * @return {Object}
     */
    get tr() {
      deprecated$1(true);
      return this.__trObj[this.language];
    }

    /**
     * Loads translation object from existing object (binds existing object).
     *
     * @param {Object} trObj
     */
    loadFromObject(trObj) {
      deprecated$1();
      const fallbackLanguage = 'en';
      if (Object.keys(trObj).indexOf(this.language) === -1) {
        Log$1.warn('No translations found for: ' + this.language);
        if (Object.keys(trObj).indexOf(fallbackLanguage) > -1) {
          Log$1.warn('Using fallback language: ' + fallbackLanguage);
          this.language = fallbackLanguage;
        } else {
          const error = 'No translations found for fallback language: ' + fallbackLanguage;
          Log$1.error(error);
          throw Error(error);
        }
      }
      this.__trObj = trObj;
      for (const lang of Object.values(this.__trObj)) {
        for (const str of Object.keys(lang)) {
          lang[str] = new LocalizedString(lang[str]);
        }
      }
    }
  }

  /**
   * Extended string class used for localization.
   */
  class LocalizedString extends String {
    /**
     * Returns formatted LocalizedString.
     * Replaces each placeholder value (e.g. {0}, {1}) with corresponding argument.
     *
     * E.g.:
     * > new LocalizedString('{0} and {1} and {0}').format('A', 'B');
     * A and B and A
     *
     * @param  {...any} args List of arguments for placeholders.
     */
    format() {
      for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
        args[_key] = arguments[_key];
      }
      const sub = args.reduce((string, arg, index) => string.split("{".concat(index, "}")).join(arg), this);
      return new LocalizedString(sub);
    }
  }
  var Locale$1 = new Locale();

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class VersionLabel extends Lightning$1.Component {
    static _template() {
      return {
        rect: true,
        color: 0xbb0078ac,
        h: 40,
        w: 100,
        x: w => w - 50,
        y: h => h - 50,
        mount: 1,
        Text: {
          w: w => w,
          h: h => h,
          y: 5,
          x: 20,
          text: {
            fontSize: 22,
            lineHeight: 26
          }
        }
      };
    }
    _firstActive() {
      this.tag('Text').text = "APP - v".concat(this.version, "\nSDK - v").concat(this.sdkVersion);
      this.tag('Text').loadTexture();
      this.w = this.tag('Text').renderWidth + 40;
      this.h = this.tag('Text').renderHeight + 5;
    }
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class FpsIndicator extends Lightning$1.Component {
    static _template() {
      return {
        rect: true,
        color: 0xffffffff,
        texture: Lightning$1.Tools.getRoundRect(80, 80, 40),
        h: 80,
        w: 80,
        x: 100,
        y: 100,
        mount: 1,
        Background: {
          x: 3,
          y: 3,
          texture: Lightning$1.Tools.getRoundRect(72, 72, 36),
          color: 0xff008000
        },
        Counter: {
          w: w => w,
          h: h => h,
          y: 10,
          text: {
            fontSize: 32,
            textAlign: 'center'
          }
        },
        Text: {
          w: w => w,
          h: h => h,
          y: 48,
          text: {
            fontSize: 15,
            textAlign: 'center',
            text: 'FPS'
          }
        }
      };
    }
    _setup() {
      this.config = {
        ...{
          log: false,
          interval: 500,
          threshold: 1
        },
        ...Settings$2.get('platform', 'showFps')
      };
      this.fps = 0;
      this.lastFps = this.fps - this.config.threshold;
      const fpsCalculator = () => {
        this.fps = ~~(1 / this.stage.dt);
      };
      this.stage.on('frameStart', fpsCalculator);
      this.stage.off('framestart', fpsCalculator);
      this.interval = setInterval(this.showFps.bind(this), this.config.interval);
    }
    _firstActive() {
      this.showFps();
    }
    _detach() {
      clearInterval(this.interval);
    }
    showFps() {
      if (Math.abs(this.lastFps - this.fps) <= this.config.threshold) return;
      this.lastFps = this.fps;
      // green
      let bgColor = 0xff008000;
      // orange
      if (this.fps <= 40 && this.fps > 20) bgColor = 0xffffa500;
      // red
      else if (this.fps <= 20) bgColor = 0xffff0000;
      this.tag('Background').setSmooth('color', bgColor);
      this.tag('Counter').text = "".concat(this.fps);
      this.config.log && Log$1.info('FPS', this.fps);
    }
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  var fetchJson = file => {
    return new Promise((resolve, reject) => {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function () {
        if (xhr.readyState == XMLHttpRequest.DONE) {
          // file protocol returns 0
          // http(s) protocol returns 200
          if (xhr.status === 0 || xhr.status === 200) resolve(JSON.parse(xhr.responseText));else reject(xhr.statusText);
        }
      };
      xhr.open('GET', file);
      xhr.send(null);
    });
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  let basePath;
  let proxyUrl;
  const initUtils = config => {
    basePath = ensureUrlWithProtocol(makeFullStaticPath(window.location.pathname, config.path || '/'));
    if (config.proxyUrl) {
      proxyUrl = ensureUrlWithProtocol(config.proxyUrl);
    }
  };
  var Utils = {
    asset(relPath) {
      return basePath + relPath;
    },
    proxyUrl(url) {
      let options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
      return proxyUrl ? proxyUrl + '?' + makeQueryString(url, options) : url;
    },
    makeQueryString() {
      return makeQueryString(...arguments);
    },
    // since imageworkers don't work without protocol
    ensureUrlWithProtocol() {
      return ensureUrlWithProtocol(...arguments);
    }
  };
  const ensureUrlWithProtocol = url => {
    if (/^\/[^/]/i.test(url) && /^(?:file:)/i.test(window.location.protocol)) {
      return window.location.protocol + '//' + url;
    }
    if (/^\/\//.test(url)) {
      return window.location.protocol + url;
    }
    if (!/^(?:https?:)/i.test(url)) {
      return window.location.origin + url;
    }
    return url;
  };
  const makeFullStaticPath = function () {
    let pathname = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : '/';
    let path = arguments.length > 1 ? arguments[1] : undefined;
    // ensure path has traling slash
    path = path.charAt(path.length - 1) !== '/' ? path + '/' : path;

    // if path is URL, we assume it's already the full static path, so we just return it
    if (/^(?:https?:)?(?:\/\/)/.test(path)) {
      return path;
    }
    if (path.charAt(0) === '/') {
      return path;
    } else {
      // cleanup the pathname (i.e. remove possible index.html)
      pathname = cleanUpPathName(pathname);

      // remove possible leading dot from path
      path = path.charAt(0) === '.' ? path.substr(1) : path;
      // ensure path has leading slash
      path = path.charAt(0) !== '/' ? '/' + path : path;
      return pathname + path;
    }
  };
  const cleanUpPathName = pathname => {
    if (pathname.slice(-1) === '/') return pathname.slice(0, -1);
    const parts = pathname.split('/');
    if (parts[parts.length - 1].indexOf('.') > -1) parts.pop();
    return parts.join('/');
  };
  const makeQueryString = function (url) {
    let options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
    let type = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 'url';
    // add operator as an option
    options.operator = 'metrological'; // Todo: make this configurable (via url?)
    // add type (= url or qr) as an option, with url as the value
    options[type] = url;
    return Object.keys(options).map(key => {
      return encodeURIComponent(key) + '=' + encodeURIComponent('' + options[key]);
    }).join('&');
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let meta = {};
  let translations = {};
  let language = null;
  const initLanguage = function (file) {
    let language = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : null;
    return new Promise((resolve, reject) => {
      fetchJson(file).then(json => {
        setTranslations(json);
        // set language (directly or in a promise)
        typeof language === 'object' && 'then' in language && typeof language.then === 'function' ? language.then(lang => setLanguage(lang).then(resolve).catch(reject)).catch(e => {
          Log$1.error(e);
          reject(e);
        }) : setLanguage(language).then(resolve).catch(reject);
      }).catch(() => {
        const error = 'Language file ' + file + ' not found';
        Log$1.error(error);
        reject(error);
      });
    });
  };
  const setTranslations = obj => {
    if ('meta' in obj) {
      meta = {
        ...obj.meta
      };
      delete obj.meta;
    }
    translations = obj;
  };
  const setLanguage = lng => {
    language = null;
    return new Promise((resolve, reject) => {
      if (lng in translations) {
        language = lng;
      } else {
        if ('map' in meta && lng in meta.map && meta.map[lng] in translations) {
          language = meta.map[lng];
        } else if ('default' in meta && meta.default in translations) {
          const error = 'Translations for Language ' + language + ' not found. Using default language ' + meta.default;
          Log$1.warn(error);
          language = meta.default;
        } else {
          const error = 'Translations for Language ' + language + ' not found.';
          Log$1.error(error);
          reject(error);
        }
      }
      if (language) {
        Log$1.info('Setting language to', language);
        const translationsObj = translations[language];
        if (typeof translationsObj === 'object') {
          resolve();
        } else if (typeof translationsObj === 'string') {
          const url = Utils.asset(translationsObj);
          fetchJson(url).then(json => {
            // save the translations for this language (to prevent loading twice)
            translations[language] = json;
            resolve();
          }).catch(e => {
            const error = 'Error while fetching ' + url;
            Log$1.error(error, e);
            reject(error);
          });
        }
      }
    });
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const registry = {
    eventListeners: [],
    timeouts: [],
    intervals: [],
    targets: []
  };
  var Registry = {
    // Timeouts
    setTimeout(cb, timeout) {
      for (var _len = arguments.length, params = new Array(_len > 2 ? _len - 2 : 0), _key = 2; _key < _len; _key++) {
        params[_key - 2] = arguments[_key];
      }
      const timeoutId = setTimeout(() => {
        registry.timeouts = registry.timeouts.filter(id => id !== timeoutId);
        cb.apply(null, params);
      }, timeout, params);
      Log$1.info('Set Timeout', 'ID: ' + timeoutId);
      registry.timeouts.push(timeoutId);
      return timeoutId;
    },
    clearTimeout(timeoutId) {
      if (registry.timeouts.indexOf(timeoutId) > -1) {
        registry.timeouts = registry.timeouts.filter(id => id !== timeoutId);
        Log$1.info('Clear Timeout', 'ID: ' + timeoutId);
        clearTimeout(timeoutId);
      } else {
        Log$1.error('Clear Timeout', 'ID ' + timeoutId + ' not found');
      }
    },
    clearTimeouts() {
      registry.timeouts.forEach(timeoutId => {
        this.clearTimeout(timeoutId);
      });
    },
    // Intervals
    setInterval(cb, interval) {
      for (var _len2 = arguments.length, params = new Array(_len2 > 2 ? _len2 - 2 : 0), _key2 = 2; _key2 < _len2; _key2++) {
        params[_key2 - 2] = arguments[_key2];
      }
      const intervalId = setInterval(() => {
        registry.intervals.filter(id => id !== intervalId);
        cb.apply(null, params);
      }, interval, params);
      Log$1.info('Set Interval', 'ID: ' + intervalId);
      registry.intervals.push(intervalId);
      return intervalId;
    },
    clearInterval(intervalId) {
      if (registry.intervals.indexOf(intervalId) > -1) {
        registry.intervals = registry.intervals.filter(id => id !== intervalId);
        Log$1.info('Clear Interval', 'ID: ' + intervalId);
        clearInterval(intervalId);
      } else {
        Log$1.error('Clear Interval', 'ID ' + intervalId + ' not found');
      }
    },
    clearIntervals() {
      registry.intervals.forEach(intervalId => {
        this.clearInterval(intervalId);
      });
    },
    // Event listeners
    addEventListener(target, event, handler) {
      target.addEventListener(event, handler);
      const targetIndex = registry.targets.indexOf(target) > -1 ? registry.targets.indexOf(target) : registry.targets.push(target) - 1;
      registry.eventListeners[targetIndex] = registry.eventListeners[targetIndex] || {};
      registry.eventListeners[targetIndex][event] = registry.eventListeners[targetIndex][event] || [];
      registry.eventListeners[targetIndex][event].push(handler);
      Log$1.info('Add eventListener', 'Target:', target, 'Event: ' + event, 'Handler:', handler.toString());
    },
    removeEventListener(target, event, handler) {
      const targetIndex = registry.targets.indexOf(target);
      if (targetIndex > -1 && registry.eventListeners[targetIndex] && registry.eventListeners[targetIndex][event] && registry.eventListeners[targetIndex][event].indexOf(handler) > -1) {
        registry.eventListeners[targetIndex][event] = registry.eventListeners[targetIndex][event].filter(fn => fn !== handler);
        Log$1.info('Remove eventListener', 'Target:', target, 'Event: ' + event, 'Handler:', handler.toString());
        target.removeEventListener(event, handler);
        // remove key from event listeners object when no events are registered for that event
        Object.keys(registry.eventListeners[targetIndex]).forEach(event => {
          if (registry.eventListeners[targetIndex][event].length === 0) {
            delete registry.eventListeners[targetIndex][event];
          }
        });
        // remove reference to the target when target has no event listeners registered
        if (Object.keys(registry.eventListeners[targetIndex]).length === 0) {
          registry.targets.splice(targetIndex, 1);
          registry.eventListeners.splice(targetIndex, 1);
        }
      } else {
        Log$1.error('Remove eventListener', 'Not found', 'Target', target, 'Event: ' + event, 'Handler', handler.toString());
      }
    },
    // if `event` is omitted, removes all registered event listeners for target
    // if `target` is also omitted, removes all registered event listeners
    removeEventListeners(target, event) {
      if (target && event) {
        const targetIndex = registry.targets.indexOf(target);
        if (targetIndex > -1) {
          registry.eventListeners[targetIndex][event].forEach(handler => {
            this.removeEventListener(target, event, handler);
          });
        }
      } else if (target) {
        const targetIndex = registry.targets.indexOf(target);
        if (targetIndex > -1) {
          Object.keys(registry.eventListeners[targetIndex]).forEach(_event => {
            this.removeEventListeners(target, _event);
          });
        }
      } else {
        Object.keys(registry.eventListeners).forEach(targetIndex => {
          this.removeEventListeners(registry.targets[targetIndex]);
        });
      }
    },
    // Clear everything (to be called upon app close for proper cleanup)
    clear() {
      this.clearTimeouts();
      this.clearIntervals();
      this.removeEventListeners();
      registry.eventListeners = [];
      registry.timeouts = [];
      registry.intervals = [];
      registry.targets = [];
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  const isObject$1 = v => {
    return typeof v === 'object' && v !== null;
  };
  const isString$1 = v => {
    return typeof v === 'string';
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let colors$1 = {
    white: '#ffffff',
    black: '#000000',
    red: '#ff0000',
    green: '#00ff00',
    blue: '#0000ff',
    yellow: '#feff00',
    cyan: '#00feff',
    magenta: '#ff00ff'
  };
  const normalizedColors = {
    //store for normalized colors
  };
  const addColors = (colorsToAdd, value) => {
    if (isObject$1(colorsToAdd)) {
      // clean up normalizedColors if they exist in the to be added colors
      Object.keys(colorsToAdd).forEach(color => cleanUpNormalizedColors(color));
      colors$1 = Object.assign({}, colors$1, colorsToAdd);
    } else if (isString$1(colorsToAdd) && value) {
      cleanUpNormalizedColors(colorsToAdd);
      colors$1[colorsToAdd] = value;
    }
  };
  const cleanUpNormalizedColors = color => {
    for (let c in normalizedColors) {
      if (c.indexOf(color) > -1) {
        delete normalizedColors[c];
      }
    }
  };
  const initColors = file => {
    return new Promise((resolve, reject) => {
      if (typeof file === 'object') {
        addColors(file);
        return resolve();
      }
      fetchJson(file).then(json => {
        addColors(json);
        return resolve();
      }).catch(() => {
        const error = 'Colors file ' + file + ' not found';
        Log$1.error(error);
        return reject(error);
      });
    });
  };

  var name = "@lightningjs/sdk";
  var version = "5.5.6";
  var license = "Apache-2.0";
  var types = "index.d.ts";
  var scripts = {
  	postinstall: "node ./scripts/postinstall.js",
  	lint: "eslint '**/*.js'",
  	release: "npm publish --access public",
  	typedoc: "typedoc --tsconfig tsconfig.typedoc.json",
  	tsd: "tsd"
  };
  var husky = {
  	hooks: {
  		"pre-commit": "lint-staged"
  	}
  };
  var dependencies = {
  	"@babel/polyfill": "^7.11.5",
  	"@lightningjs/core": "^2.16.1",
  	"@metrological/sdk": "^1.0.2",
  	"@michieljs/execute-as-promise": "^1.0.0",
  	deepmerge: "^4.2.2",
  	"is-plain-object": "^5.0.0",
  	localcookies: "^2.0.0",
  	shelljs: "^0.8.5",
  	"url-polyfill": "^1.1.10",
  	"whatwg-fetch": "^3.0.0"
  };
  var devDependencies = {
  	"@babel/core": "^7.11.6",
  	"@babel/plugin-transform-parameters": "^7.10.5 ",
  	"@babel/plugin-transform-spread": "^7.11.0",
  	"@babel/preset-env": "^7.11.5",
  	"babel-eslint": "^10.1.0",
  	eslint: "^7.10.0",
  	"eslint-config-prettier": "^6.12.0",
  	"eslint-plugin-prettier": "^3.1.4",
  	husky: "^4.3.0",
  	"lint-staged": "^10.4.0",
  	prettier: "^1.19.1",
  	rollup: "^1.32.1",
  	"rollup-plugin-babel": "^4.4.0",
  	tsd: "^0.22.0",
  	typedoc: "^0.23.9"
  };
  var repository = {
  	type: "git",
  	url: "git@github.com:rdkcentral/Lightning-SDK.git"
  };
  var bugs = {
  	url: "https://github.com/rdkcentral/Lightning-SDK/issues"
  };
  var packageInfo = {
  	name: name,
  	version: version,
  	license: license,
  	types: types,
  	scripts: scripts,
  	"lint-staged": {
  	"*.js": [
  		"eslint --fix"
  	],
  	"src/startApp.js": [
  		"rollup -c ./rollup.config.js"
  	]
  },
  	husky: husky,
  	dependencies: dependencies,
  	devDependencies: devDependencies,
  	repository: repository,
  	bugs: bugs
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let AppInstance;
  const defaultOptions = {
    stage: {
      w: 1920,
      h: 1080,
      precision: 1,
      clearColor: 0x00000000,
      canvas2d: false
    },
    debug: false,
    defaultFontFace: 'RobotoRegular',
    keys: {
      8: 'Back',
      13: 'Enter',
      27: 'Menu',
      37: 'Left',
      38: 'Up',
      39: 'Right',
      40: 'Down',
      174: 'ChannelDown',
      175: 'ChannelUp',
      178: 'Stop',
      250: 'PlayPause',
      191: 'Search',
      // Use "/" for keyboard
      409: 'Search'
    }
  };
  const customFontFaces = [];
  const fontLoader = (fonts, store) => new Promise((resolve, reject) => {
    fonts.map(_ref => {
      let {
        family,
        url,
        urls,
        descriptors
      } = _ref;
      return () => {
        const src = urls ? urls.map(url => {
          return 'url(' + url + ')';
        }) : 'url(' + url + ')';
        const fontFace = new FontFace(family, src, descriptors || {});
        store.push(fontFace);
        Log$1.info('Loading font', family);
        document.fonts.add(fontFace);
        return fontFace.load();
      };
    }).reduce((promise, method) => {
      return promise.then(() => method());
    }, Promise.resolve(null)).then(resolve).catch(reject);
  });
  function Application (App, appData, platformSettings) {
    const {
      width,
      height
    } = platformSettings;
    if (width && height) {
      defaultOptions.stage['w'] = width;
      defaultOptions.stage['h'] = height;
      defaultOptions.stage['precision'] = width / 1920;
    }

    // support for 720p browser
    if (!width && !height && window.innerHeight === 720) {
      defaultOptions.stage['w'] = 1280;
      defaultOptions.stage['h'] = 720;
      defaultOptions.stage['precision'] = 1280 / 1920;
    }
    return class Application extends Lightning$1.Application {
      constructor(options) {
        const config = cjs(defaultOptions, options, {
          isMergeableObject: isPlainObject
        });
        super(config);
        this.config = config;
      }
      static _template() {
        return {
          w: 1920,
          h: 1080
        };
      }
      colorshift() {
        let type = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;
        let config = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
        Accessibility.colorshift(this, type, config);
      }
      get keymapping() {
        return this.stage.application.config.keys;
      }

      /**
       * This function overrides the default keymap with the latest keymap.
       * @param customKeyMap
       * @param keepDuplicates
       */
      overrideKeyMap(customKeyMap) {
        let keepDuplicates = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : false;
        const baseKeyMap = this.stage.application.config.keys;
        Object.keys(customKeyMap).reduce((keymapping, key) => {
          // prevent duplicate values to exist in final keymapping (i.e. 2 keys triggering 'Back')
          if (!keepDuplicates) {
            Object.keys(baseKeyMap).forEach(baseKey => {
              if (baseKey != key && baseKeyMap[baseKey] == customKeyMap[key]) {
                delete keymapping[baseKey];
              }
            });
          }
          keymapping[key] = customKeyMap[key];
          return keymapping;
        }, baseKeyMap);
        return baseKeyMap;
      }
      _setup() {
        Promise.all([this.loadFonts(App.config && App.config.fonts || App.getFonts && App.getFonts() || []),
        // to be deprecated
        Locale$1.load(App.config && App.config.locale || App.getLocale && App.getLocale()), App.language && this.loadLanguage(App.language()), App.colors && this.loadColors(App.colors())]).then(() => {
          Metrics$1.app.loaded();
          this.w = this.config.stage.w / this.config.stage.precision;
          this.h = this.config.stage.h / this.config.stage.precision;
          AppInstance = this.stage.c({
            ref: 'App',
            type: App,
            zIndex: 1,
            forceZIndexContext: !!platformSettings.showVersion || !!platformSettings.showFps
          });
          this.childList.a(AppInstance);
          this._refocus();
          Log$1.info('App version', this.config.version);
          Log$1.info('SDK version', packageInfo.version);
          if (platformSettings.showVersion) {
            this.childList.a({
              ref: 'VersionLabel',
              type: VersionLabel,
              version: this.config.version,
              sdkVersion: packageInfo.version,
              zIndex: 1
            });
          }
          if (platformSettings.showFps) {
            this.childList.a({
              ref: 'FpsCounter',
              type: FpsIndicator,
              zIndex: 1
            });
          }
          super._setup();
        }).catch(console.error);
      }
      _handleBack() {
        this.closeApp();
      }
      _handleExit() {
        this.closeApp();
      }
      closeApp() {
        Log$1.info('Signaling App Close');
        if (platformSettings.onClose && typeof platformSettings.onClose === 'function') {
          platformSettings.onClose(...arguments);
        } else {
          this.close();
        }
      }
      close() {
        Log$1.info('Closing App');
        Settings$2.clearSubscribers();
        Registry.clear();
        this.childList.remove(this.tag('App'));
        this.cleanupFonts();
        // force texture garbage collect
        this.stage.gc();
        this.destroy();
      }
      loadFonts(fonts) {
        return platformSettings.fontLoader && typeof platformSettings.fontLoader === 'function' ? platformSettings.fontLoader(fonts, customFontFaces) : fontLoader(fonts, customFontFaces);
      }
      cleanupFonts() {
        if ('delete' in document.fonts) {
          customFontFaces.forEach(fontFace => {
            Log$1.info('Removing font', fontFace.family);
            document.fonts.delete(fontFace);
          });
        } else {
          Log$1.info('No support for removing manually-added fonts');
        }
      }
      loadLanguage(config) {
        let file = Utils.asset('translations.json');
        let language = config;
        if (typeof language === 'object') {
          language = config.language || null;
          file = config.file || file;
        }
        return initLanguage(file, language);
      }
      loadColors(config) {
        let file = Utils.asset('colors.json');
        if (config && (typeof config === 'string' || typeof config === 'object')) {
          file = config;
        }
        return initColors(file);
      }
      set focus(v) {
        this._focussed = v;
        this._refocus();
      }
      _getFocused() {
        return this._focussed || this.tag('App');
      }
    };
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class ScaledImageTexture extends Lightning$1.textures.ImageTexture {
    constructor(stage) {
      super(stage);
      this._scalingOptions = undefined;
    }
    set options(options) {
      this.resizeMode = this._scalingOptions = options;
    }
    _getLookupId() {
      return "".concat(this._src, "-").concat(this._scalingOptions.type, "-").concat(this._scalingOptions.w, "-").concat(this._scalingOptions.h);
    }
    getNonDefaults() {
      const obj = super.getNonDefaults();
      if (this._src) {
        obj.src = this._src;
      }
      return obj;
    }
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const events = ['timeupdate', 'error', 'ended', 'loadeddata', 'canplay', 'play', 'playing', 'pause', 'loadstart', 'seeking', 'seeked', 'encrypted'];
  let mediaUrl = url => url;
  const initMediaPlayer = config => {
    if (config.mediaUrl) {
      mediaUrl = config.mediaUrl;
    }
  };
  class Mediaplayer extends Lightning$1.Component {
    _construct() {
      this._skipRenderToTexture = false;
      this._metrics = null;
      this._textureMode = Settings$2.get('platform', 'textureMode') || false;
      Log$1.info('Texture mode: ' + this._textureMode);
      console.warn(["The 'MediaPlayer'-plugin in the Lightning-SDK is deprecated and will be removed in future releases.", "Please consider using the new 'VideoPlayer'-plugin instead.", 'https://rdkcentral.github.io/Lightning-SDK/#/plugins/videoplayer'].join('\n\n'));
    }
    static _template() {
      return {
        Video: {
          VideoWrap: {
            VideoTexture: {
              visible: false,
              pivot: 0.5,
              texture: {
                type: Lightning$1.textures.StaticTexture,
                options: {}
              }
            }
          }
        }
      };
    }
    set skipRenderToTexture(v) {
      this._skipRenderToTexture = v;
    }
    get textureMode() {
      return this._textureMode;
    }
    get videoView() {
      return this.tag('Video');
    }
    _init() {
      //re-use videotag if already there
      const videoEls = document.getElementsByTagName('video');
      if (videoEls && videoEls.length > 0) this.videoEl = videoEls[0];else {
        this.videoEl = document.createElement('video');
        this.videoEl.setAttribute('id', 'video-player');
        this.videoEl.style.position = 'absolute';
        this.videoEl.style.zIndex = '1';
        this.videoEl.style.display = 'none';
        this.videoEl.setAttribute('width', '100%');
        this.videoEl.setAttribute('height', '100%');
        this.videoEl.style.visibility = this.textureMode ? 'hidden' : 'visible';
        document.body.appendChild(this.videoEl);
      }
      if (this.textureMode && !this._skipRenderToTexture) {
        this._createVideoTexture();
      }
      this.eventHandlers = [];
    }
    _registerListeners() {
      events.forEach(event => {
        const handler = e => {
          if (this._metrics && this._metrics[event] && typeof this._metrics[event] === 'function') {
            this._metrics[event]({
              currentTime: this.videoEl.currentTime
            });
          }
          this.fire(event, {
            videoElement: this.videoEl,
            event: e
          });
        };
        this.eventHandlers.push(handler);
        this.videoEl.addEventListener(event, handler);
      });
    }
    _deregisterListeners() {
      Log$1.info('Deregistering event listeners MediaPlayer');
      events.forEach((event, index) => {
        this.videoEl.removeEventListener(event, this.eventHandlers[index]);
      });
      this.eventHandlers = [];
    }
    _attach() {
      this._registerListeners();
    }
    _detach() {
      this._deregisterListeners();
      this.close();
    }
    _createVideoTexture() {
      const stage = this.stage;
      const gl = stage.gl;
      const glTexture = gl.createTexture();
      gl.bindTexture(gl.TEXTURE_2D, glTexture);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      this.videoTexture.options = {
        source: glTexture,
        w: this.videoEl.width,
        h: this.videoEl.height
      };
    }
    _startUpdatingVideoTexture() {
      if (this.textureMode && !this._skipRenderToTexture) {
        const stage = this.stage;
        if (!this._updateVideoTexture) {
          this._updateVideoTexture = () => {
            if (this.videoTexture.options.source && this.videoEl.videoWidth && this.active) {
              const gl = stage.gl;
              const currentTime = new Date().getTime();

              // When BR2_PACKAGE_GST1_PLUGINS_BAD_PLUGIN_DEBUGUTILS is not set in WPE, webkitDecodedFrameCount will not be available.
              // We'll fallback to fixed 30fps in this case.
              const frameCount = this.videoEl.webkitDecodedFrameCount;
              const mustUpdate = frameCount ? this._lastFrame !== frameCount : this._lastTime < currentTime - 30;
              if (mustUpdate) {
                this._lastTime = currentTime;
                this._lastFrame = frameCount;
                try {
                  gl.bindTexture(gl.TEXTURE_2D, this.videoTexture.options.source);
                  gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
                  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.videoEl);
                  this._lastFrame = this.videoEl.webkitDecodedFrameCount;
                  this.videoTextureView.visible = true;
                  this.videoTexture.options.w = this.videoEl.videoWidth;
                  this.videoTexture.options.h = this.videoEl.videoHeight;
                  const expectedAspectRatio = this.videoTextureView.w / this.videoTextureView.h;
                  const realAspectRatio = this.videoEl.videoWidth / this.videoEl.videoHeight;
                  if (expectedAspectRatio > realAspectRatio) {
                    this.videoTextureView.scaleX = realAspectRatio / expectedAspectRatio;
                    this.videoTextureView.scaleY = 1;
                  } else {
                    this.videoTextureView.scaleY = expectedAspectRatio / realAspectRatio;
                    this.videoTextureView.scaleX = 1;
                  }
                } catch (e) {
                  Log$1.error('texImage2d video', e);
                  this._stopUpdatingVideoTexture();
                  this.videoTextureView.visible = false;
                }
                this.videoTexture.source.forceRenderUpdate();
              }
            }
          };
        }
        if (!this._updatingVideoTexture) {
          stage.on('frameStart', this._updateVideoTexture);
          this._updatingVideoTexture = true;
        }
      }
    }
    _stopUpdatingVideoTexture() {
      if (this.textureMode) {
        const stage = this.stage;
        stage.removeListener('frameStart', this._updateVideoTexture);
        this._updatingVideoTexture = false;
        this.videoTextureView.visible = false;
        if (this.videoTexture.options.source) {
          const gl = stage.gl;
          gl.bindTexture(gl.TEXTURE_2D, this.videoTexture.options.source);
          gl.clearColor(0, 0, 0, 1);
          gl.clear(gl.COLOR_BUFFER_BIT);
        }
      }
    }
    updateSettings() {
      let settings = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};
      // The Component that 'consumes' the media player.
      this._consumer = settings.consumer;
      if (this._consumer && this._consumer.getMediaplayerSettings) {
        // Allow consumer to add settings.
        settings = Object.assign(settings, this._consumer.getMediaplayerSettings());
      }
      if (!Lightning$1.Utils.equalValues(this._stream, settings.stream)) {
        if (settings.stream && settings.stream.keySystem) {
          navigator.requestMediaKeySystemAccess(settings.stream.keySystem.id, settings.stream.keySystem.config).then(keySystemAccess => {
            return keySystemAccess.createMediaKeys();
          }).then(createdMediaKeys => {
            return this.videoEl.setMediaKeys(createdMediaKeys);
          }).then(() => {
            if (settings.stream && settings.stream.src) this.open(settings.stream.src);
          }).catch(() => {
            console.error('Failed to set up MediaKeys');
          });
        } else if (settings.stream && settings.stream.src) {
          // This is here to be backwards compatible, will be removed
          // in future sdk release
          if (Settings$2.get('app', 'hls')) {
            if (!window.Hls) {
              window.Hls = class Hls {
                static isSupported() {
                  console.warn('hls-light not included');
                  return false;
                }
              };
            }
            if (window.Hls.isSupported()) {
              if (!this._hls) this._hls = new window.Hls({
                liveDurationInfinity: true
              });
              this._hls.loadSource(settings.stream.src);
              this._hls.attachMedia(this.videoEl);
              this.videoEl.style.display = 'block';
            }
          } else {
            this.open(settings.stream.src);
          }
        } else {
          this.close();
        }
        this._stream = settings.stream;
      }
      this._setHide(settings.hide);
      this._setVideoArea(settings.videoPos);
    }
    _setHide(hide) {
      if (this.textureMode) {
        this.tag('Video').setSmooth('alpha', hide ? 0 : 1);
      } else {
        this.videoEl.style.visibility = hide ? 'hidden' : 'visible';
      }
    }
    open(url) {
      let settings = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {
        hide: false,
        videoPosition: null
      };
      // prep the media url to play depending on platform (mediaPlayerplugin)
      url = mediaUrl(url);
      this._metrics = Metrics$1.media(url);
      Log$1.info('Playing stream', url);
      if (this.application.noVideo) {
        Log$1.info('noVideo option set, so ignoring: ' + url);
        return;
      }
      // close the video when opening same url as current (effectively reloading)
      if (this.videoEl.getAttribute('src') === url) {
        this.close();
      }
      this.videoEl.setAttribute('src', url);

      // force hide, then force show (in next tick!)
      // (fixes comcast playback rollover issue)
      this.videoEl.style.visibility = 'hidden';
      this.videoEl.style.display = 'none';
      setTimeout(() => {
        this.videoEl.style.display = 'block';
        this.videoEl.style.visibility = 'visible';
      });
      this._setHide(settings.hide);
      this._setVideoArea(settings.videoPosition || [0, 0, 1920, 1080]);
    }
    close() {
      // We need to pause first in order to stop sound.
      this.videoEl.pause();
      this.videoEl.removeAttribute('src');

      // force load to reset everything without errors
      this.videoEl.load();
      this._clearSrc();
      this.videoEl.style.display = 'none';
    }
    playPause() {
      if (this.isPlaying()) {
        this.doPause();
      } else {
        this.doPlay();
      }
    }
    get muted() {
      return this.videoEl.muted;
    }
    set muted(v) {
      this.videoEl.muted = v;
    }
    get loop() {
      return this.videoEl.loop;
    }
    set loop(v) {
      this.videoEl.loop = v;
    }
    isPlaying() {
      return this._getState() === 'Playing';
    }
    doPlay() {
      this.videoEl.play();
    }
    doPause() {
      this.videoEl.pause();
    }
    reload() {
      var url = this.videoEl.getAttribute('src');
      this.close();
      this.videoEl.src = url;
    }
    getPosition() {
      return Promise.resolve(this.videoEl.currentTime);
    }
    setPosition(pos) {
      this.videoEl.currentTime = pos;
    }
    getDuration() {
      return Promise.resolve(this.videoEl.duration);
    }
    seek(time) {
      let absolute = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : false;
      if (absolute) {
        this.videoEl.currentTime = time;
      } else {
        this.videoEl.currentTime += time;
      }
    }
    get videoTextureView() {
      return this.tag('Video').tag('VideoTexture');
    }
    get videoTexture() {
      return this.videoTextureView.texture;
    }
    _setVideoArea(videoPos) {
      if (Lightning$1.Utils.equalValues(this._videoPos, videoPos)) {
        return;
      }
      this._videoPos = videoPos;
      if (this.textureMode) {
        this.videoTextureView.patch({
          smooth: {
            x: videoPos[0],
            y: videoPos[1],
            w: videoPos[2] - videoPos[0],
            h: videoPos[3] - videoPos[1]
          }
        });
      } else {
        const precision = this.stage.getRenderPrecision();
        this.videoEl.style.left = Math.round(videoPos[0] * precision) + 'px';
        this.videoEl.style.top = Math.round(videoPos[1] * precision) + 'px';
        this.videoEl.style.width = Math.round((videoPos[2] - videoPos[0]) * precision) + 'px';
        this.videoEl.style.height = Math.round((videoPos[3] - videoPos[1]) * precision) + 'px';
      }
    }
    _fireConsumer(event, args) {
      if (this._consumer) {
        this._consumer.fire(event, args);
      }
    }
    _equalInitData(buf1, buf2) {
      if (!buf1 || !buf2) return false;
      if (buf1.byteLength != buf2.byteLength) return false;
      const dv1 = new Int8Array(buf1);
      const dv2 = new Int8Array(buf2);
      for (let i = 0; i != buf1.byteLength; i++) if (dv1[i] != dv2[i]) return false;
      return true;
    }
    error(args) {
      this._fireConsumer('$mediaplayerError', args);
      this._setState('');
      return '';
    }
    loadeddata(args) {
      this._fireConsumer('$mediaplayerLoadedData', args);
    }
    play(args) {
      this._fireConsumer('$mediaplayerPlay', args);
    }
    playing(args) {
      this._fireConsumer('$mediaplayerPlaying', args);
      this._setState('Playing');
    }
    canplay(args) {
      this.videoEl.play();
      this._fireConsumer('$mediaplayerStart', args);
    }
    loadstart(args) {
      this._fireConsumer('$mediaplayerLoad', args);
    }
    seeked() {
      this._fireConsumer('$mediaplayerSeeked', {
        currentTime: this.videoEl.currentTime,
        duration: this.videoEl.duration || 1
      });
    }
    seeking() {
      this._fireConsumer('$mediaplayerSeeking', {
        currentTime: this.videoEl.currentTime,
        duration: this.videoEl.duration || 1
      });
    }
    durationchange(args) {
      this._fireConsumer('$mediaplayerDurationChange', args);
    }
    encrypted(args) {
      const video = args.videoElement;
      const event = args.event;
      // FIXME: Double encrypted events need to be properly filtered by Gstreamer
      if (video.mediaKeys && !this._equalInitData(this._previousInitData, event.initData)) {
        this._previousInitData = event.initData;
        this._fireConsumer('$mediaplayerEncrypted', args);
      }
    }
    static _states() {
      return [class Playing extends this {
        $enter() {
          this._startUpdatingVideoTexture();
        }
        $exit() {
          this._stopUpdatingVideoTexture();
        }
        timeupdate() {
          this._fireConsumer('$mediaplayerProgress', {
            currentTime: this.videoEl.currentTime,
            duration: this.videoEl.duration || 1
          });
        }
        ended(args) {
          this._fireConsumer('$mediaplayerEnded', args);
          this._setState('');
        }
        pause(args) {
          this._fireConsumer('$mediaplayerPause', args);
          this._setState('Playing.Paused');
        }
        _clearSrc() {
          this._fireConsumer('$mediaplayerStop', {});
          this._setState('');
        }
        static _states() {
          return [class Paused extends this {}];
        }
      }];
    }
  }

  class localCookie {
    constructor(e) {
      return e = e || {}, this.forceCookies = e.forceCookies || !1, !0 === this._checkIfLocalStorageWorks() && !0 !== e.forceCookies ? {
        getItem: this._getItemLocalStorage,
        setItem: this._setItemLocalStorage,
        removeItem: this._removeItemLocalStorage,
        clear: this._clearLocalStorage,
        keys: this._getLocalStorageKeys
      } : {
        getItem: this._getItemCookie,
        setItem: this._setItemCookie,
        removeItem: this._removeItemCookie,
        clear: this._clearCookies,
        keys: this._getCookieKeys
      };
    }
    _checkIfLocalStorageWorks() {
      if ("undefined" == typeof localStorage) return !1;
      try {
        return localStorage.setItem("feature_test", "yes"), "yes" === localStorage.getItem("feature_test") && (localStorage.removeItem("feature_test"), !0);
      } catch (e) {
        return !1;
      }
    }
    _getItemLocalStorage(e) {
      return window.localStorage.getItem(e);
    }
    _setItemLocalStorage(e, t) {
      return window.localStorage.setItem(e, t);
    }
    _removeItemLocalStorage(e) {
      return window.localStorage.removeItem(e);
    }
    _clearLocalStorage() {
      return window.localStorage.clear();
    }
    _getLocalStorageKeys() {
      return Object.keys(window.localStorage);
    }
    _getItemCookie(e) {
      var t = document.cookie.match(RegExp("(?:^|;\\s*)" + function (e) {
        return e.replace(/([.*+?\^${}()|\[\]\/\\])/g, "\\$1");
      }(e) + "=([^;]*)"));
      return t && "" === t[1] && (t[1] = null), t ? t[1] : null;
    }
    _setItemCookie(e, t) {
      var o = new Date(),
        r = new Date(o.getTime() + 15768e7);
      document.cookie = "".concat(e, "=").concat(t, "; expires=").concat(r.toUTCString(), ";");
    }
    _removeItemCookie(e) {
      document.cookie = "".concat(e, "=;Max-Age=-99999999;");
    }
    _clearCookies() {
      document.cookie.split(";").forEach(e => {
        document.cookie = e.replace(/^ +/, "").replace(/=.*/, "=;expires=Max-Age=-99999999");
      });
    }
    _getCookieKeys() {
      return document.cookie.split(";").map(e => e.split("=")[0]);
    }
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const initStorage = () => {
    Settings$2.get('platform', 'id');
    // todo: pass options (for example to force the use of cookies)
    new localCookie();
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  const hasRegex = /\{\/(.*?)\/([igm]{0,3})\}/g;
  const isWildcard = /^[!*$]$/;
  const hasLookupId = /\/:\w+?@@([0-9]+?)@@/;
  const isNamedGroup = /^\/:/;

  /**
   * Test if a route is part regular expressed
   * and replace it for a simple character
   * @param route
   * @returns {*}
   */
  const stripRegex = function (route) {
    let char = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'R';
    // if route is part regular expressed we replace
    // the regular expression for a character to
    // simplify floor calculation and backtracking
    if (hasRegex.test(route)) {
      route = route.replace(hasRegex, char);
    }
    return route;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /**
   * Create a local request register
   * @param flags
   * @returns {Map<any, any>}
   */
  const createRegister = flags => {
    const reg = new Map()
    // store user defined and router
    // defined flags in register
  ;
    [...Object.keys(flags), ...Object.getOwnPropertySymbols(flags)].forEach(key => {
      reg.set(key, flags[key]);
    });
    return reg;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class Request {
    constructor() {
      let hash = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : '';
      let navArgs = arguments.length > 1 ? arguments[1] : undefined;
      let storeCaller = arguments.length > 2 ? arguments[2] : undefined;
      /**
       * Hash we navigate to
       * @type {string}
       * @private
       */
      this._hash = hash;

      /**
       * Do we store previous hash in history
       * @type {boolean}
       * @private
       */
      this._storeCaller = storeCaller;

      /**
       * Request and navigate data
       * @type {Map}
       * @private
       */
      this._register = new Map();

      /**
       * Flag if the instance is created due to
       * this request
       * @type {boolean}
       * @private
       */
      this._isCreated = false;

      /**
       * Flag if the instance is shared between
       * previous and current request
       * @type {boolean}
       * @private
       */
      this._isSharedInstance = false;

      /**
       * Flag if the request has been cancelled
       * @type {boolean}
       * @private
       */
      this._cancelled = false;

      /**
       * if instance is shared between requests we copy state object
       * from instance before the new request overrides state
       * @type {null}
       * @private
       */
      this._copiedHistoryState = null;

      // if there are arguments attached to navigate()
      // we store them in new request
      if (isObject(navArgs)) {
        this._register = createRegister(navArgs);
      } else if (isBoolean(navArgs)) {
        // if second navigate() argument is explicitly
        // set to false we prevent the calling page
        // from ending up in history
        this._storeCaller = navArgs;
      }
      // @todo: remove because we can simply check
      // ._storeCaller property
      this._register.set(symbols.store, this._storeCaller);
    }
    cancel() {
      Log$1.debug('[router]:', "cancelled ".concat(this._hash));
      this._cancelled = true;
    }
    get url() {
      return this._hash;
    }
    get register() {
      return this._register;
    }
    get hash() {
      return this._hash;
    }
    set hash(args) {
      this._hash = args;
    }
    get route() {
      return this._route;
    }
    set route(args) {
      this._route = args;
    }
    get provider() {
      return this._provider;
    }
    set provider(args) {
      this._provider = args;
    }
    get providerType() {
      return this._providerType;
    }
    set providerType(args) {
      this._providerType = args;
    }
    set page(args) {
      this._page = args;
    }
    get page() {
      return this._page;
    }
    set isCreated(args) {
      this._isCreated = args;
    }
    get isCreated() {
      return this._isCreated;
    }
    get isSharedInstance() {
      return this._isSharedInstance;
    }
    set isSharedInstance(args) {
      this._isSharedInstance = args;
    }
    get isCancelled() {
      return this._cancelled;
    }
    set copiedHistoryState(v) {
      this._copiedHistoryState = v;
    }
    get copiedHistoryState() {
      return this._copiedHistoryState;
    }
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class Route {
    constructor() {
      let config = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};
      // keep backwards compatible
      let type = ['on', 'before', 'after'].reduce((acc, type) => {
        return isFunction(config[type]) ? type : acc;
      }, undefined);
      this._cfg = config;
      if (type) {
        this._provider = {
          type,
          request: config[type]
        };
      }
    }
    get path() {
      return this._cfg.path;
    }
    get name() {
      return this._cfg.name;
    }
    get component() {
      return this._cfg.component;
    }
    get options() {
      return this._cfg.options;
    }
    get widgets() {
      return this._cfg.widgets;
    }
    get cache() {
      return this._cfg.cache;
    }
    get hook() {
      return this._cfg.hook;
    }
    get beforeNavigate() {
      return this._cfg.beforeNavigate;
    }
    get provider() {
      return this._provider;
    }
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /**
   * Simple route length calculation
   * @param route {string}
   * @returns {number} - floor
   */
  const getFloor = route => {
    return stripRegex(route).split('/').length;
  };

  /**
   * return all stored routes that live on the same floor
   * @param floor
   * @returns {Array}
   */
  const getRoutesByFloor = floor => {
    const matches = [];
    // simple filter of level candidates
    for (let [route] of routes.entries()) {
      if (getFloor(route) === floor) {
        matches.push(route);
      }
    }
    return matches;
  };

  /**
   * return a matching route by provided hash
   * hash: home/browse/12 will match:
   * route: home/browse/:categoryId
   * @param hash {string}
   * @returns {boolean|{}} - route
   */
  const getRouteByHash = hash => {
    // @todo: clean up on handleHash
    hash = hash.replace(/^#/, '');
    const getUrlParts = /(\/?:?[^/]+)/g;
    // grab possible candidates from stored routes
    const candidates = getRoutesByFloor(getFloor(hash));
    // break hash down in chunks
    const hashParts = hash.match(getUrlParts) || [];

    // to simplify the route matching and prevent look around
    // in our getUrlParts regex we get the regex part from
    // route candidate and store them so that we can reference
    // them when we perform the actual regex against hash
    let regexStore = [];
    let matches = candidates.filter(route => {
      let isMatching = true;
      // replace regex in route with lookup id => @@{storeId}@@
      if (hasRegex.test(route)) {
        const regMatches = route.match(hasRegex);
        if (regMatches && regMatches.length) {
          route = regMatches.reduce((fullRoute, regex) => {
            const lookupId = regexStore.length;
            fullRoute = fullRoute.replace(regex, "@@".concat(lookupId, "@@"));
            regexStore.push(regex.substring(1, regex.length - 1));
            return fullRoute;
          }, route);
        }
      }
      const routeParts = route.match(getUrlParts) || [];
      for (let i = 0, j = routeParts.length; i < j; i++) {
        const routePart = routeParts[i];
        const hashPart = hashParts[i];

        // Since we support catch-all and regex driven name groups
        // we first test for regex lookup id and see if the regex
        // matches the value from the hash
        if (hasLookupId.test(routePart)) {
          const routeMatches = hasLookupId.exec(routePart);
          const storeId = routeMatches[1];
          const routeRegex = regexStore[storeId];

          // split regex and modifiers so we can use both
          // to create a new RegExp
          // eslint-disable-next-line
          const regMatches = /\/([^\/]+)\/([igm]{0,3})/.exec(routeRegex);
          if (regMatches && regMatches.length) {
            const expression = regMatches[1];
            const modifiers = regMatches[2];
            const regex = new RegExp("^/".concat(expression, "$"), modifiers);
            if (!regex.test(hashPart)) {
              isMatching = false;
            }
          }
        } else if (isNamedGroup.test(routePart)) {
          // we kindly skip namedGroups because this is dynamic
          // we only need to the static and regex drive parts
          continue;
        } else if (hashPart && routePart.toLowerCase() !== hashPart.toLowerCase()) {
          isMatching = false;
        }
      }
      return isMatching;
    });
    if (matches.length) {
      if (matches.indexOf(hash) !== -1) {
        const match = matches[matches.indexOf(hash)];
        return routes.get(match);
      } else {
        // we give prio to static routes over dynamic
        matches = matches.sort(a => {
          return isNamedGroup.test(a) ? -1 : 1;
        });
        // would be strange if this fails
        // but still we test
        if (routeExists(matches[0])) {
          return routes.get(matches[0]);
        }
      }
    }
    return false;
  };
  const getValuesFromHash = function () {
    let hash = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : '';
    let path = arguments.length > 1 ? arguments[1] : undefined;
    // replace the regex definition from the route because
    // we already did the matching part
    path = stripRegex(path, '');
    const getUrlParts = /(\/?:?[\w%\s:.-]+)/g;
    const hashParts = hash.match(getUrlParts) || [];
    const routeParts = path.match(getUrlParts) || [];
    const getNamedGroup = /^\/:([\w-]+)\/?/;
    return routeParts.reduce((storage, value, index) => {
      const match = getNamedGroup.exec(value);
      if (match && match.length) {
        storage.set(match[1], decodeURIComponent(hashParts[index].replace(/^\//, '')));
      }
      return storage;
    }, new Map());
  };
  const getOption = (stack, prop) => {
    // eslint-disable-next-line
    if (stack && stack.hasOwnProperty(prop)) {
      return stack[prop];
    }
    // we explicitly return undefined since we're testing
    // for explicit test values
  };

  /**
   * create and return new Route instance
   * @param config
   */
  const createRoute = config => {
    // we need to provide a bit of additional logic
    // for the bootComponent
    if (config.path === '$') {
      let options = {
        preventStorage: true
      };
      if (isObject(config.options)) {
        options = {
          ...config.options,
          ...options
        };
      }
      config.options = options;
      // if configured add reference to bootRequest
      // as router after provider
      if (bootRequest) {
        config.after = bootRequest;
      }
    }
    return new Route(config);
  };

  /**
   * Create a new Router request object
   * @param url
   * @param args
   * @param store
   * @returns {*}
   */
  const createRequest = (url, args, store) => {
    return new Request(url, args, store);
  };
  const getHashByName = obj => {
    if (!obj.to && !obj.name) {
      return false;
    }
    const route = getRouteByName(obj.to || obj.name);
    const hasDynamicGroup = /\/:([\w-]+)\/?/;
    let hash = route;

    // if route contains dynamic group
    // we replace them with the provided params
    if (hasDynamicGroup.test(route)) {
      if (obj.params) {
        const keys = Object.keys(obj.params);
        hash = keys.reduce((acc, key) => {
          return acc.replace(":".concat(key), obj.params[key]);
        }, route);
      }
      if (obj.query) {
        return "".concat(hash).concat(objectToQueryString(obj.query));
      }
    }
    return hash;
  };
  const getRouteByName = name => {
    for (let [path, route] of routes.entries()) {
      if (route.name === name) {
        return path;
      }
    }
    return false;
  };
  const keepActivePageAlive = (route, request) => {
    if (isString(route)) {
      const routes = getRoutes();
      if (routes.has(route)) {
        route = routes.get(route);
      } else {
        return false;
      }
    }
    const register = request.register;
    const routeOptions = route.options;
    if (register.has('keepAlive')) {
      return register.get('keepAlive');
    } else if (routeOptions && routeOptions.keepAlive) {
      return routeOptions.keepAlive;
    }
    return false;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  var emit = (function (page) {
    let events = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : [];
    let params = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {};
    if (!isArray(events)) {
      events = [events];
    }
    events.forEach(e => {
      const event = "_on".concat(ucfirst(e));
      if (isFunction(page[event])) {
        page[event](params);
      }
    });
  });

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let activeWidget = null;
  const getReferences = () => {
    if (!widgetsHost) {
      return;
    }
    return widgetsHost.get().reduce((storage, widget) => {
      const key = widget.ref.toLowerCase();
      storage[key] = widget;
      return storage;
    }, {});
  };

  /**
   * update the visibility of the available widgets
   * for the current page / route
   * @param page
   */
  const updateWidgets = (widgets, page) => {
    // force lowercase lookup
    const configured = (widgets || []).map(ref => ref.toLowerCase());
    widgetsHost.forEach(widget => {
      widget.visible = configured.indexOf(widget.ref.toLowerCase()) !== -1;
      if (widget.visible) {
        emit(widget, ['activated'], page);
      }
    });
    if (app.state === 'Widgets' && activeWidget && !activeWidget.visible) {
      app._setState('');
    }
  };
  const getWidgetByName = name => {
    name = ucfirst(name);
    return widgetsHost.getByRef(name) || false;
  };

  /**
   * delegate app focus to a on-screen widget
   * @param name - {string}
   */
  const focusWidget = name => {
    const widget = getWidgetByName(name);
    if (widget) {
      setActiveWidget(widget);

      // if app is already in 'Widgets' state we can assume that
      // focus has been delegated from one widget to another so
      // we need to set the new widget reference and trigger a
      // new focus calculation of Lightning's focuspath
      if (app.state === 'Widgets') {
        app.reload(activeWidget);
      } else {
        app._setState('Widgets', [activeWidget]);
      }
    }
  };
  const restoreFocus = () => {
    activeWidget = null;
    app._setState('');
  };
  const getActiveWidget = () => {
    return activeWidget;
  };
  const setActiveWidget = instance => {
    activeWidget = instance;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const createComponent = (stage, type) => {
    return stage.c({
      type,
      visible: false,
      widgets: getReferences()
    });
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /**
   * Simple flat array that holds the visited hashes + state Object
   * so the router can navigate back to them
   * @type {Array}
   */
  let history = [];
  const updateHistory = request => {
    const hash = getActiveHash();
    if (!hash) {
      return;
    }

    // navigate storage flag
    const register = request.register;
    const forceNavigateStore = register.get(symbols.store);

    // test preventStorage on route configuration
    const activeRoute = getRouteByHash(hash);
    const preventStorage = getOption(activeRoute.options, 'preventStorage');

    // we give prio to navigate storage flag
    let store = isBoolean(forceNavigateStore) ? forceNavigateStore : !preventStorage;
    if (store) {
      const toStore = hash.replace(/^\//, '');
      const location = locationInHistory(toStore);
      const stateObject = getStateObject(getActivePage(), request);
      const routerConfig = getRouterConfig();

      // store hash if it's not a part of history or flag for
      // storage of same hash is true
      if (location === -1 || routerConfig.get('storeSameHash')) {
        history.push({
          hash: toStore,
          state: stateObject
        });
      } else {
        // if we visit the same route we want to sync history
        const prev = history.splice(location, 1)[0];
        history.push({
          hash: prev.hash,
          state: stateObject
        });
      }
    }
  };
  const locationInHistory = hash => {
    for (let i = 0; i < history.length; i++) {
      if (history[i].hash === hash) {
        return i;
      }
    }
    return -1;
  };
  const getHistoryState = hash => {
    let state = null;
    if (history.length) {
      // if no hash is provided we get the last
      // pushed history record
      if (!hash) {
        const record = history[history.length - 1];
        // could be null
        state = record.state;
      } else {
        if (locationInHistory(hash) !== -1) {
          const record = history[locationInHistory(hash)];
          state = record.state;
        }
      }
    }
    return state;
  };
  const replaceHistoryState = function () {
    let state = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : null;
    let hash = arguments.length > 1 ? arguments[1] : undefined;
    if (!history.length) {
      return;
    }
    const location = hash ? locationInHistory(hash) : history.length - 1;
    if (location !== -1 && isObject(state)) {
      history[location].state = state;
    }
  };
  const getStateObject = (page, request) => {
    // if the new request shared instance with the
    // previous request we used the copied state object
    if (request.isSharedInstance) {
      if (request.copiedHistoryState) {
        return request.copiedHistoryState;
      }
    } else if (page && isFunction(page.historyState)) {
      return page.historyState();
    }
    return null;
  };
  const getHistory = () => {
    return history.slice(0);
  };
  const setHistory = function () {
    let arr = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : [];
    if (isArray(arr)) {
      history = arr;
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /**
   * @type {Lightning.Application}
   */
  let application;

  /**
   * Actual instance of the app
   * @type {Lightning.Component}
   */
  let app;

  /**
   * Component that hosts all routed pages
   * @type {Lightning.Component}
   */
  let pagesHost;

  /**
   * @type {Lightning.Stage}
   */
  let stage;

  /**
   * Platform driven Router configuration
   * @type {Map<string>}
   */
  let routerConfig;

  /**
   * Component that hosts all attached widgets
   * @type {Lightning.Component}
   */
  let widgetsHost;

  /**
   * Hash we point the browser to when we boot the app
   * and there is no deep-link provided
   * @type {string|Function}
   */
  let rootHash;

  /**
   * Boot request will fire before app start
   * can be used to execute some global logic
   * and can be configured
   */
  let bootRequest;

  /**
   * Flag if we need to update the browser location hash.
   * Router can work without.
   * @type {boolean}
   */
  let updateHash = true;

  /**
   * Will be called before a route starts, can be overridden
   * via routes config
   * @param from - route we came from
   * @param to - route we navigate to
   * @returns {Promise<*>}
   */
  // eslint-disable-next-line
  let beforeEachRoute = async (from, to) => {
    return true;
  };

  /**
   *  * Will be called after a navigate successfully resolved,
   * can be overridden via routes config
   */
  let afterEachRoute = () => {};

  /**
   * All configured routes
   * @type {Map<string, object>}
   */
  let routes = new Map();

  /**
   * Store all page components per route
   * @type {Map<string, object>}
   */
  let components$1 = new Map();

  /**
   * Flag if router has been initialised
   * @type {boolean}
   */
  let initialised = false;

  /**
   * Current page being rendered on screen
   * @type {null}
   */
  let activePage = null;
  let activeHash;
  let activeRoute;

  /**
   *  During the process of a navigation request a new
   *  request can start, to prevent unwanted behaviour
   *  the navigate()-method stores the last accepted hash
   *  so we can invalidate any prior requests
   */
  let lastAcceptedHash;

  /**
   * With on()-data providing behaviour the Router forced the App
   * in a Loading state. When the data-provider resolves we want to
   * change the state back to where we came from
   */
  let previousState;
  const mixin = app => {
    // by default the Router Baseclass provides the component
    // reference in which we store our pages
    if (app.pages) {
      pagesHost = app.pages.childList;
    }
    // if the app is using widgets we grab refs
    // and hide all the widgets
    if (app.widgets && app.widgets.children) {
      widgetsHost = app.widgets.childList;
      // hide all widgets on boot
      widgetsHost.forEach(w => w.visible = false);
    }
    app._handleBack = e => {
      step(-1);
      e.preventDefault();
    };
  };
  const bootRouter = (config, instance) => {
    let {
      appInstance,
      routes
    } = config;

    // if instance is provided and it's and Lightning Component instance
    if (instance && isPage(instance)) {
      app = instance;
    }
    if (!app) {
      app = appInstance || AppInstance;
    }
    application = app.application;
    pagesHost = application.childList;
    stage = app.stage;
    routerConfig = getConfigMap();
    mixin(app);
    if (isArray(routes)) {
      setup(config);
    } else if (isFunction(routes)) {
      console.warn('[Router]: Calling Router.route() directly is deprecated.');
      console.warn('Use object config: https://rdkcentral.github.io/Lightning-SDK/#/plugins/router/configuration');
    }
  };
  const setup = config => {
    if (!initialised) {
      init(config);
    }
    config.routes.forEach(r => {
      const path = cleanHash(r.path);
      if (!routeExists(path)) {
        const route = createRoute(r);
        routes.set(path, route);
        // if route has a configured component property
        // we store it in a different map to simplify
        // the creating and destroying per route
        if (route.component) {
          let type = route.component;
          if (isComponentConstructor(type)) {
            if (!routerConfig.get('lazyCreate')) {
              type = createComponent(stage, type);
              pagesHost.a(type);
            }
          }
          components$1.set(path, type);
        }
      } else {
        console.error("".concat(path, " already exists in routes configuration"));
      }
    });
  };
  const init = config => {
    rootHash = config.root;
    if (isFunction(config.boot)) {
      bootRequest = config.boot;
    }
    if (isBoolean(config.updateHash)) {
      updateHash = config.updateHash;
    }
    if (isFunction(config.beforeEachRoute)) {
      beforeEachRoute = config.beforeEachRoute;
    }
    if (isFunction(config.afterEachRoute)) {
      afterEachRoute = config.afterEachRoute;
    }
    if (config.bootComponent) {
      console.warn('[Router]: Boot Component is now available as a special router: https://rdkcentral.github.io/Lightning-SDK/#/plugins/router/configuration?id=special-routes');
      console.warn('[Router]: setting { bootComponent } property will be deprecated in a future release');
      if (isPage(config.bootComponent)) {
        config.routes.push({
          path: '$',
          component: config.bootComponent,
          // we try to assign the bootRequest as after data-provider
          // so it will behave as any other component
          after: bootRequest || null,
          options: {
            preventStorage: true
          }
        });
      } else {
        console.error("[Router]: ".concat(config.bootComponent, " is not a valid boot component"));
      }
    }
    config.routes.forEach(item => {
      // replacing regexes with 'R' to avoid issues with pattern matching below
      const strippedPath = stripRegex(item.path);

      // Pattern to identify the last path of the route
      // It should start with "/:" + any word  and ends with "?"
      // It should be the last path of the route
      // valid => /player/:asset/:assetId? (:assetId is optional)
      // invalid => /player/:asset/:assetId?/test (:assetId? is not an optional path)
      // invalid => /player/:asset?/:assetId? (second path is not considered as an optional path)
      const pattern = /.*\/:.*?\?$/u;
      if (pattern.test(strippedPath)) {
        const optionalPath = item.path.substring(0, item.path.lastIndexOf('/'));
        const originalPath = item.path.substring(0, item.path.lastIndexOf('?'));
        item.path = originalPath;
        //Create another entry with the optional path
        let optionalItem = {
          ...item
        };
        optionalItem.path = optionalPath;
        config.routes.push(optionalItem);
      }
    });
    initialised = true;
  };
  const storeComponent = (route, type) => {
    if (components$1.has(route)) {
      components$1.set(route, type);
    }
  };
  const getComponent = route => {
    if (components$1.has(route)) {
      return components$1.get(route);
    }
    return null;
  };

  // delete existing route instance from memory
  const deleteCurrentInstance = route => {
    if (components$1.has(route) && pagesHost.getIndex(components$1.get(route)) !== -1) {
      pagesHost.remove(components$1.get(route));
      storeComponent(route, components$1.get(route)._routedType || components$1.get(route).constructor);
    }
  };

  /**
   * Test if router needs to update browser location hash
   * @returns {boolean}
   */
  const mustUpdateLocationHash = () => {
    if (!routerConfig || !routerConfig.size) {
      return false;
    }
    // we need support to either turn change hash off
    // per platform or per app
    const updateConfig = routerConfig.get('updateHash');
    return !(isBoolean(updateConfig) && !updateConfig || isBoolean(updateHash) && !updateHash);
  };

  /**
   * Will be called when a new navigate() request has completed
   * and has not been expired due to it's async nature
   * @param request
   */
  const onRequestResolved = request => {
    const hash = request.hash;
    const route = request.route;
    const register = request.register;
    const page = request.page;

    // clean up history if modifier is set
    if (getOption(route.options, 'clearHistory')) {
      setHistory([]);
    } else if (hash && !isWildcard.test(route.path)) {
      updateHistory(request);
    }

    // we only update the stackLocation if a route
    // is not expired before it resolves
    storeComponent(route.path, page);
    if (request.isSharedInstance || !request.isCreated) {
      emit(page, 'changed');
    } else if (request.isCreated) {
      emit(page, 'mounted');
    }

    // only update widgets if we have a host
    if (widgetsHost) {
      updateWidgets(route.widgets, page);
    }

    // we want to clean up if there is an
    // active page that is not being shared
    // between current and previous route
    if (getActivePage() && !request.isSharedInstance) {
      cleanUp(activePage, request);
    }

    // provide history object to active page
    if (register.get(symbols.historyState) && isFunction(page.historyState)) {
      page.historyState(register.get(symbols.historyState));
    }
    setActivePage(page);
    activeHash = request.hash;
    activeRoute = route.path;

    // cleanup all cancelled requests
    for (let request of navigateQueue.values()) {
      if (request.isCancelled && request.hash) {
        navigateQueue.delete(request.hash);
      }
    }
    afterEachRoute(request);
    Log$1.info('[route]:', route.path);
    Log$1.info('[hash]:', hash);
  };
  const cleanUp = (page, request) => {
    const route = activeRoute;
    const register = request.register;
    const lazyDestroy = routerConfig.get('lazyDestroy');
    const destroyOnBack = routerConfig.get('destroyOnHistoryBack');
    const keepAlive = register.get('keepAlive');
    const isFromHistory = register.get(symbols.backtrack);
    let doCleanup = false;

    // if this request is executed due to a step back in history
    // and we have configured to destroy active page when we go back
    // in history or lazyDestory is enabled
    if (isFromHistory && (destroyOnBack || lazyDestroy)) {
      doCleanup = true;
    }

    // clean up if lazyDestroy is enabled and the keepAlive flag
    // in navigation register is false
    if (lazyDestroy && !keepAlive) {
      doCleanup = true;
    }

    // if the current and new request share the same route blueprint
    if (activeRoute === request.route.path) {
      doCleanup = true;
    }
    if (doCleanup) {
      // grab original class constructor if
      // statemachine routed else store constructor
      storeComponent(route, page._routedType || page.constructor);

      // actual remove of page from memory
      pagesHost.remove(page);

      // force texture gc() if configured
      // so we can cleanup textures in the same tick
      if (routerConfig.get('gcOnUnload')) {
        stage.gc();
      }
    } else {
      // If we're not removing the page we need to
      // reset it's properties
      page.patch({
        x: 0,
        y: 0,
        scale: 1,
        visible: false,
        alpha: 1
      });
    }
  };
  const getActiveHash = () => {
    return activeHash;
  };
  const setActivePage = page => {
    activePage = page;
  };
  const getActivePage = () => {
    return activePage;
  };
  const getActiveRoute = () => {
    return activeRoute;
  };
  const getLastHash = () => {
    return lastAcceptedHash;
  };
  const setLastHash = hash => {
    lastAcceptedHash = hash;
  };
  const setPreviousState = state => {
    previousState = state;
  };
  const getPreviousState = () => {
    return previousState;
  };
  const routeExists = key => {
    return routes.has(key);
  };
  const getRootHash = () => {
    return rootHash;
  };
  const getBootRequest = () => {
    return bootRequest;
  };
  const getRouterConfig = () => {
    return routerConfig;
  };
  const getRoutes = () => {
    return routes;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const isFunction = v => {
    return typeof v === 'function';
  };
  const isObject = v => {
    return typeof v === 'object' && v !== null;
  };
  const isBoolean = v => {
    return typeof v === 'boolean';
  };
  const isPage = v => {
    if (v instanceof Lightning$1.Element || isComponentConstructor(v)) {
      return true;
    }
    return false;
  };
  const isComponentConstructor = type => {
    return type.prototype && 'isComponent' in type.prototype;
  };
  const isArray = v => {
    return Array.isArray(v);
  };
  const ucfirst = v => {
    return "".concat(v.charAt(0).toUpperCase()).concat(v.slice(1));
  };
  const isString = v => {
    return typeof v === 'string';
  };
  const isPromise = method => {
    let result;
    if (isFunction(method)) {
      try {
        result = method.apply(null);
      } catch (e) {
        result = e;
      }
    } else {
      result = method;
    }
    return isObject(result) && isFunction(result.then);
  };
  const cleanHash = function () {
    let hash = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : '';
    return hash.replace(/^#/, '').replace(/\/+$/, '');
  };
  const getConfigMap = () => {
    const routerSettings = Settings$2.get('platform', 'router');
    const isObj = isObject(routerSettings);
    return ['backtrack', 'gcOnUnload', 'destroyOnHistoryBack', 'lazyCreate', 'lazyDestroy', 'reuseInstance', 'autoRestoreRemote', 'numberNavigation', 'updateHash', 'storeSameHash'].reduce((config, key) => {
      config.set(key, isObj ? routerSettings[key] : Settings$2.get('platform', key));
      return config;
    }, new Map());
  };
  const getQueryStringParams = function () {
    let hash = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : getActiveHash();
    const resumeHash = getResumeHash();
    if ((hash === '$' || !hash) && resumeHash) {
      if (isString(resumeHash)) {
        hash = resumeHash;
      }
    }
    let parse = '';
    const getQuery = /([?&].*)/;
    const matches = getQuery.exec(hash);
    const params = {};
    if (document.location && document.location.search) {
      parse = document.location.search;
    }
    if (matches && matches.length) {
      let hashParams = matches[1];
      if (parse) {
        // if location.search is not empty we
        // remove the leading ? to create a
        // valid string
        hashParams = hashParams.replace(/^\?/, '');
        // we parse hash params last so they we can always
        // override search params with hash params
        parse = "".concat(parse, "&").concat(hashParams);
      } else {
        parse = hashParams;
      }
    }
    if (parse) {
      const urlParams = new URLSearchParams(parse);
      for (const [key, value] of urlParams.entries()) {
        params[key] = value;
      }
      return params;
    } else {
      return false;
    }
  };
  const objectToQueryString = obj => {
    if (!isObject(obj)) {
      return '';
    }
    return '?' + Object.keys(obj).map(key => {
      return "".concat(key, "=").concat(obj[key]);
    }).join('&');
  };
  const symbols = {
    route: Symbol('route'),
    hash: Symbol('hash'),
    store: Symbol('store'),
    fromHistory: Symbol('fromHistory'),
    expires: Symbol('expires'),
    resume: Symbol('resume'),
    backtrack: Symbol('backtrack'),
    historyState: Symbol('historyState'),
    queryParams: Symbol('queryParams')
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  const dataHooks = {
    on: request => {
      setPreviousState(app.state || '');
      app._setState('Loading');
      return execProvider(request);
    },
    before: request => {
      return execProvider(request);
    },
    after: request => {
      try {
        execProvider(request, true);
      } catch (e) {
        // for now we fail silently
      }
      return Promise.resolve();
    }
  };
  const execProvider = (request, emitProvided) => {
    const route = request.route;
    const provider = route.provider;
    const expires = route.cache ? route.cache * 1000 : 0;
    const params = addPersistData(request);
    return provider.request(request.page, {
      ...params
    }).then(() => {
      request.page[symbols.expires] = Date.now() + expires;
      if (emitProvided) {
        emit(request.page, 'dataProvided');
      }
    }).catch(e => {
      request.page[symbols.expires] = Date.now();
      throw e;
    });
  };
  const addPersistData = _ref => {
    let {
      page,
      route,
      hash,
      register = new Map()
    } = _ref;
    const urlValues = getValuesFromHash(hash, route.path);
    const queryParams = getQueryStringParams(hash);
    const pageData = new Map([...urlValues, ...register]);
    const params = {};

    // make dynamic url data available to the page
    // as instance properties
    for (let [name, value] of pageData) {
      params[name] = value;
    }
    if (queryParams) {
      params[symbols.queryParams] = queryParams;
    }

    // check navigation register for persistent data
    if (register.size) {
      const obj = {};
      for (let [k, v] of register) {
        obj[k] = v;
      }
      page.persist = obj;
    }

    // make url data and persist data available
    // via params property
    page.params = params;
    emit(page, ['urlParams'], params);
    return params;
  };

  /**
   * Test if page passed cache-time
   * @param page
   * @returns {boolean}
   */
  const isPageExpired = page => {
    if (!page[symbols.expires]) {
      return false;
    }
    const expires = page[symbols.expires];
    const now = Date.now();
    return now >= expires;
  };
  const hasProvider = path => {
    if (routeExists(path)) {
      const record = routes.get(path);
      return !!record.provider;
    }
    return false;
  };
  const getProvider = route => {
    // @todo: fix, route already is passed in
    if (routeExists(route.path)) {
      const {
        provider
      } = routes.get(route.path);
      return {
        type: provider.type,
        provider: provider.request
      };
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  const fade = (i, o) => {
    return new Promise(resolve => {
      i.patch({
        alpha: 0,
        visible: true,
        smooth: {
          alpha: [1, {
            duration: 0.5,
            delay: 0.1
          }]
        }
      });
      // resolve on y finish
      i.transition('alpha').on('finish', () => {
        if (o) {
          o.visible = false;
        }
        resolve();
      });
    });
  };
  const crossFade = (i, o) => {
    return new Promise(resolve => {
      i.patch({
        alpha: 0,
        visible: true,
        smooth: {
          alpha: [1, {
            duration: 0.5,
            delay: 0.1
          }]
        }
      });
      if (o) {
        o.patch({
          smooth: {
            alpha: [0, {
              duration: 0.5,
              delay: 0.3
            }]
          }
        });
      }
      // resolve on y finish
      i.transition('alpha').on('finish', () => {
        resolve();
      });
    });
  };
  const moveOnAxes = (axis, direction, i, o) => {
    const bounds = axis === 'x' ? 1920 : 1080;
    return new Promise(resolve => {
      i.patch({
        ["".concat(axis)]: direction ? bounds * -1 : bounds,
        visible: true,
        smooth: {
          ["".concat(axis)]: [0, {
            duration: 0.4,
            delay: 0.2
          }]
        }
      });
      // out is optional
      if (o) {
        o.patch({
          ["".concat(axis)]: 0,
          smooth: {
            ["".concat(axis)]: [direction ? bounds : bounds * -1, {
              duration: 0.4,
              delay: 0.2
            }]
          }
        });
      }
      // resolve on y finish
      i.transition(axis).on('finish', () => {
        resolve();
      });
    });
  };
  const up = (i, o) => {
    return moveOnAxes('y', 0, i, o);
  };
  const down = (i, o) => {
    return moveOnAxes('y', 1, i, o);
  };
  const left = (i, o) => {
    return moveOnAxes('x', 0, i, o);
  };
  const right = (i, o) => {
    return moveOnAxes('x', 1, i, o);
  };
  var Transitions = {
    fade,
    crossFade,
    up,
    down,
    left,
    right
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /**
   * execute transition between new / old page and
   * toggle the defined widgets
   * @todo: platform override default transition
   * @param pageIn
   * @param pageOut
   */
  const executeTransition = function (pageIn) {
    let pageOut = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : null;
    const transition = pageIn.pageTransition || pageIn.easing;
    const hasCustomTransitions = !!(pageIn.smoothIn || pageIn.smoothInOut || transition);
    const transitionsDisabled = getRouterConfig().get('disableTransitions');
    if (pageIn.easing) {
      console.warn('easing() method is deprecated and will be removed. Use pageTransition()');
    }

    // default behaviour is a visibility toggle
    if (!hasCustomTransitions || transitionsDisabled) {
      pageIn.visible = true;
      if (pageOut) {
        pageOut.visible = false;
      }
      return Promise.resolve();
    }
    if (transition) {
      let type;
      try {
        type = transition.call(pageIn, pageIn, pageOut);
      } catch (e) {
        type = 'crossFade';
      }
      if (isPromise(type)) {
        return type;
      }
      if (isString(type)) {
        const fn = Transitions[type];
        if (fn) {
          return fn(pageIn, pageOut);
        }
      }

      // keep backwards compatible for now
      if (pageIn.smoothIn) {
        // provide a smooth function that resolves itself
        // on transition finish
        const smooth = function (p, v) {
          let args = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {};
          return new Promise(resolve => {
            pageIn.visible = true;
            pageIn.setSmooth(p, v, args);
            pageIn.transition(p).on('finish', () => {
              resolve();
            });
          });
        };
        return pageIn.smoothIn({
          pageIn,
          smooth
        });
      }
    }
    return Transitions.crossFade(pageIn, pageOut);
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /**
   * The actual loading of the component
   * */
  const load = async request => {
    let expired = false;
    try {
      request = await loader(request);
      if (request && !request.isCancelled) {
        // in case of on() providing we need to reset
        // app state;
        if (app.state === 'Loading') {
          if (getPreviousState() === 'Widgets') {
            app._setState('Widgets', [getActiveWidget()]);
          } else {
            app._setState('');
          }
        }
        // Do page transition if instance
        // is not shared between the routes
        if (!request.isSharedInstance && !request.isCancelled) {
          await executeTransition(request.page, getActivePage());
        }
      } else {
        expired = true;
      }
      // on expired we only cleanup
      if (expired || request.isCancelled) {
        Log$1.debug('[router]:', "Rejected ".concat(request.hash, " because route to ").concat(getLastHash(), " started"));
        if (request.isCreated && !request.isSharedInstance) {
          // remove from render-tree
          pagesHost.remove(request.page);
        }
      } else {
        onRequestResolved(request);
        // resolve promise
        return request.page;
      }
    } catch (request) {
      if (!request.route) {
        console.error(request);
      } else if (!expired) {
        // @todo: revisit
        const {
          route
        } = request;
        // clean up history if modifier is set
        if (getOption(route.options, 'clearHistory')) {
          setHistory([]);
        } else if (!isWildcard.test(route.path)) {
          updateHistory(request);
        }
        if (request.isCreated && !request.isSharedInstance) {
          // remove from render-tree
          pagesHost.remove(request.page);
        }
        handleError(request);
      }
    }
  };
  const loader = async request => {
    const route = request.route;
    const hash = request.hash;
    const register = request.register;

    // todo: grab from Route instance
    let type = getComponent(route.path);
    let isConstruct = isComponentConstructor(type);
    let provide = false;

    // if it's an instance bt we're not coming back from
    // history we test if we can re-use this instance
    if (!isConstruct && !register.get(symbols.backtrack)) {
      if (!mustReuse(route)) {
        type = type.constructor;
        isConstruct = true;
      }
    }

    // If page is Lightning Component instance
    if (!isConstruct) {
      request.page = type;
      // if we have have a data route for current page
      if (hasProvider(route.path)) {
        if (isPageExpired(type) || type[symbols.hash] !== hash) {
          provide = true;
        }
      }
      let currentRoute = getActivePage() && getActivePage()[symbols.route];
      // if the new route is equal to the current route it means that both
      // route share the Component instance and stack location / since this case
      // is conflicting with the way before() and after() loading works we flag it,
      // and check platform settings in we want to re-use instance
      if (route.path === currentRoute) {
        request.isSharedInstance = true;
        // since we're re-using the instance we must attach
        // historyState to the request to prevent it from
        // being overridden.
        if (isFunction(request.page.historyState)) {
          request.copiedHistoryState = request.page.historyState();
        }
      }
    } else {
      request.page = createComponent(stage, type);
      pagesHost.a(request.page);
      // test if need to request data provider
      if (hasProvider(route.path)) {
        provide = true;
      }
      request.isCreated = true;
    }

    // we store hash and route as properties on the page instance
    // that way we can easily calculate new behaviour on page reload
    request.page[symbols.hash] = hash;
    request.page[symbols.route] = route.path;
    try {
      if (provide) {
        // extract attached data-provider for route
        // we're processing
        const {
          type: loadType,
          provider
        } = getProvider(route);

        // update running request
        request.provider = provider;
        request.providerType = loadType;
        await dataHooks[loadType](request);

        // we early exit if the current request is expired
        if (hash !== getLastHash()) {
          return false;
        } else {
          if (request.providerType !== 'after') {
            emit(request.page, 'dataProvided');
          }
          // resolve promise
          return request;
        }
      } else {
        addPersistData(request);
        return request;
      }
    } catch (e) {
      request.error = e;
      return Promise.reject(request);
    }
  };
  const handleError = request => {
    if (request && request.error) {
      console.error(request.error);
    } else if (request) {
      Log$1.error(request);
    }
    if (request.page && routeExists('!')) {
      navigate('!', {
        request
      }, false);
    }
  };
  const mustReuse = route => {
    const opt = getOption(route.options, 'reuseInstance');
    const config = routerConfig.get('reuseInstance');

    // route always has final decision
    if (isBoolean(opt)) {
      return opt;
    }
    return !(isBoolean(config) && config === false);
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class RoutedApp extends Lightning$1.Component {
    static _template() {
      return {
        Pages: {
          forceZIndexContext: true
        },
        /**
         * This is a default Loading page that will be made visible
         * during data-provider on() you CAN override in child-class
         */
        Loading: {
          rect: true,
          w: 1920,
          h: 1080,
          color: 0xff000000,
          visible: false,
          zIndex: 99,
          Label: {
            mount: 0.5,
            x: 960,
            y: 540,
            text: {
              text: 'Loading..'
            }
          }
        }
      };
    }
    static _states() {
      return [class Loading extends this {
        $enter() {
          this.tag('Loading').visible = true;
        }
        $exit() {
          this.tag('Loading').visible = false;
        }
      }, class Widgets extends this {
        $enter(args, widget) {
          // store widget reference
          this._widget = widget;

          // since it's possible that this behaviour
          // is non-remote driven we force a recalculation
          // of the focuspath
          this._refocus();
        }
        _getFocused() {
          // we delegate focus to selected widget
          // so it can consume remotecontrol presses
          return this._widget;
        }

        // if we want to widget to widget focus delegation
        reload(widget) {
          this._widget = widget;
          this._refocus();
        }
        _handleKey() {
          const restoreFocus = routerConfig.get('autoRestoreRemote');
          /**
           * The Router used to delegate focus back to the page instance on
           * every unhandled key. This is barely usefull in any situation
           * so for now we offer the option to explicity turn that behaviour off
           * so we don't don't introduce a breaking change.
           */
          if (!isBoolean(restoreFocus) || restoreFocus === true) {
            Router.focusPage();
          }
        }
      }];
    }

    /**
     * Return location where pages need to be stored
     */
    get pages() {
      return this.tag('Pages');
    }

    /**
     * Tell router where widgets are stored
     */
    get widgets() {
      return this.tag('Widgets');
    }

    /**
     * we MUST register _handleBack method so the Router
     * can override it
     * @private
     */
    _handleBack() {}

    /**
     * We MUST return Router.activePage() so the new Page
     * can listen to the remote-control.
     */
    _getFocused() {
      return Router.getActivePage();
    }
  }

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */

  /*
  rouThor ==[x]
   */
  let navigateQueue = new Map();
  let forcedHash = '';
  let resumeHash = '';

  /**
   * Start routing the app
   * @param config - route config object
   * @param instance - instance of the app
   */
  const startRouter = (config, instance) => {
    bootRouter(config, instance);
    registerListener();
    start();
  };

  // start translating url
  const start = () => {
    let hash = (getHash() || '').replace(/^#/, '');
    const bootKey = '$';
    const params = getQueryStringParams(hash);
    const bootRequest = getBootRequest();
    const rootHash = getRootHash();
    const isDirectLoad = hash.indexOf(bootKey) !== -1;

    // prevent direct reload of wildcard routes
    // expect bootComponent
    if (isWildcard.test(hash) && hash !== bootKey) {
      hash = '';
    }

    // store resume point for manual resume
    resumeHash = isDirectLoad ? rootHash : hash || rootHash;
    const ready = () => {
      if (!hash && rootHash) {
        if (isString(rootHash)) {
          navigate(rootHash);
        } else if (isFunction(rootHash)) {
          rootHash().then(res => {
            if (isObject(res)) {
              navigate(res.path, res.params);
            } else {
              navigate(res);
            }
          });
        }
      } else {
        queue(hash);
        handleHashChange().then(() => {
          app._refocus();
        }).catch(e => {
          console.error(e);
        });
      }
    };
    if (routeExists(bootKey)) {
      if (hash && !isDirectLoad) {
        if (!getRouteByHash(hash)) {
          navigate('*', {
            failedHash: hash
          });
          return;
        }
      }
      navigate(bootKey, {
        resume: resumeHash,
        reload: bootKey === hash
      }, false);
    } else if (isFunction(bootRequest)) {
      bootRequest(params).then(() => {
        ready();
      }).catch(e => {
        handleBootError(e);
      });
    } else {
      ready();
    }
  };
  const handleBootError = e => {
    if (routeExists('!')) {
      navigate('!', {
        request: {
          error: e
        }
      });
    } else {
      console.error(e);
    }
  };

  /**
   * start a new request
   * @param url
   * @param args
   * @param store
   */
  const navigate = function (url) {
    let args = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
    let store = arguments.length > 2 ? arguments[2] : undefined;
    if (isObject(url)) {
      url = getHashByName(url);
      if (!url) {
        return;
      }
    }
    let hash = getHash();
    if (!mustUpdateLocationHash() && forcedHash) {
      hash = forcedHash;
    }
    if (hash.replace(/^#/, '') !== url) {
      // push request in the queue
      queue(url, args, store);
      if (mustUpdateLocationHash()) {
        setHash(url);
      } else {
        forcedHash = url;
        handleHashChange(url).then(() => {
          app._refocus();
        }).catch(e => {
          console.error(e);
        });
      }
    } else if (args.reload) {
      // push request in the queue
      queue(url, args, store);
      handleHashChange(url).then(() => {
        app._refocus();
      }).catch(e => {
        console.error(e);
      });
    }
  };
  const queue = function (hash) {
    let args = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
    let store = arguments.length > 2 ? arguments[2] : undefined;
    hash = cleanHash(hash);
    if (!navigateQueue.has(hash)) {
      for (let request of navigateQueue.values()) {
        request.cancel();
      }
      const request = createRequest(hash, args, store);
      navigateQueue.set(decodeURIComponent(hash), request);
      return request;
    }
    return false;
  };

  /**
   * Handle change of hash
   * @param override
   * @returns {Promise<void>}
   */
  const handleHashChange = async override => {
    const hash = cleanHash(override || getHash());
    const queueId = decodeURIComponent(hash);
    let request = navigateQueue.get(queueId);

    // handle hash updated manually
    if (!request && !navigateQueue.size) {
      request = queue(hash);
    }
    const route = getRouteByHash(hash);
    if (!route) {
      if (routeExists('*')) {
        navigate('*', {
          failedHash: hash
        });
      } else {
        console.error("Unable to navigate to: ".concat(hash));
      }
      return;
    }

    // update current processed request
    request.hash = hash;
    request.route = route;
    let result = await beforeEachRoute(getActiveHash(), request);

    // test if a local hook is configured for the route
    if (result && route.beforeNavigate) {
      result = await route.beforeNavigate(getActiveHash(), request);
    }
    if (isBoolean(result)) {
      // only if resolve value is explicitly true
      // we continue the current route request
      if (result) {
        return resolveHashChange(request);
      }
    } else {
      // if navigation guard didn't return true
      // we cancel the current request
      request.cancel();
      navigateQueue.delete(queueId);
      if (isString(result)) {
        navigate(result);
      } else if (isObject(result)) {
        let store = true;
        if (isBoolean(result.store)) {
          store = result.store;
        }
        navigate(result.path, result.params, store);
      }
    }
  };

  /**
   * Continue processing the hash change if not blocked
   * by global or local hook
   * @param request - {}
   */
  const resolveHashChange = request => {
    const hash = request.hash;
    const route = request.route;
    const queueId = decodeURIComponent(hash);
    // store last requested hash so we can
    // prevent a route that resolved later
    // from displaying itself
    setLastHash(hash);
    if (route.path) {
      const component = getComponent(route.path);
      // if a hook is provided for the current route
      if (isFunction(route.hook)) {
        const urlParams = getValuesFromHash(hash, route.path);
        const params = {};
        for (const key of urlParams.keys()) {
          params[key] = urlParams.get(key);
        }
        route.hook(app, {
          ...params
        });
      }
      // if there is a component attached to the route
      if (component) {
        // force page to root state to prevent shared state issues
        const activePage = getActivePage();
        if (activePage) {
          const keepAlive = keepActivePageAlive(getActiveRoute(), request);
          if (activePage && route.path === getActiveRoute() && !keepAlive) {
            activePage._setState('');
          }
        }
        if (isPage(component)) {
          load(request).then(() => {
            app._refocus();
            navigateQueue.delete(queueId);
          });
        } else {
          // of the component is not a constructor
          // or a Component instance we can assume
          // that it's a dynamic import
          component().then(contents => {
            return contents.default;
          }).then(module => {
            storeComponent(route.path, module);
            return load(request);
          }).then(() => {
            app._refocus();
            navigateQueue.delete(queueId);
          });
        }
      } else {
        navigateQueue.delete(queueId);
      }
    }
  };

  /**
   * Directional step in history
   * @param level
   */
  const step = function () {
    let level = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
    if (!level || isNaN(level)) {
      return false;
    }
    const history = getHistory();
    // for now we only support negative numbers
    level = Math.abs(level);

    //Check whether we have any history avaialble or not
    if (history.length) {
      // for now we only support history back
      const route = history.splice(history.length - level, level)[0];
      // store changed history
      setHistory(history);
      return navigate(route.hash, {
        [symbols.backtrack]: true,
        [symbols.historyState]: route.state
      }, false);
    } else if (routerConfig.get('backtrack')) {
      const hashLastPart = /(\/:?[\w%\s-]+)$/;
      let hash = stripRegex(getHash());
      let floor = getFloor(hash);

      // test if we got deep-linked
      if (floor > 1) {
        while (floor--) {
          // strip of last part
          hash = hash.replace(hashLastPart, '');
          // if we have a configured route
          // we navigate to it
          if (getRouteByHash(hash)) {
            return navigate(hash, {
              [symbols.backtrack]: true
            }, false);
          }
        }
      }
    }

    // we can't step back past the amount
    // of history entries
    if (level > history.length) {
      if (isFunction(app._handleAppClose)) {
        return app._handleAppClose();
      }
      return app.application.closeApp();
    }
    return false;
  };

  /**
   * Resume Router's page loading process after
   * the BootComponent became visible;
   */
  const resume = () => {
    if (isString(resumeHash)) {
      navigate(resumeHash, false);
      resumeHash = '';
    } else if (isFunction(resumeHash)) {
      resumeHash().then(res => {
        resumeHash = '';
        if (isObject(res)) {
          navigate(res.path, res.params);
        } else {
          navigate(res);
        }
      });
    } else {
      console.warn('[Router]: resume() called but no hash found');
    }
  };

  /**
   * Force reload active hash
   */
  const reload = () => {
    if (!isNavigating()) {
      const hash = getActiveHash();
      navigate(hash, {
        reload: true
      }, false);
    }
  };

  /**
   * Query if the Router is still processing a Request
   * @returns {boolean}
   */
  const isNavigating = () => {
    if (navigateQueue.size) {
      let isProcessing = false;
      for (let request of navigateQueue.values()) {
        if (!request.isCancelled) {
          isProcessing = true;
        }
      }
      return isProcessing;
    }
    return false;
  };
  const getResumeHash = () => {
    return resumeHash;
  };

  /**
   * By default we return the location hash
   * @returns {string}
   */
  let getHash = () => {
    return document.location.hash;
  };

  /**
   * Update location hash
   * @param url
   */
  let setHash = url => {
    document.location.hash = url;
  };

  /**
   * This can be called from the platform / bootstrapper to override
   * the default getting and setting of the hash
   * @param config
   */
  const initRouter = config => {
    if (config.getHash) {
      getHash = config.getHash;
    }
    if (config.setHash) {
      setHash = config.setHash;
    }
  };

  /**
   * On hash change we start processing
   */
  const registerListener = () => {
    Registry.addEventListener(window, 'hashchange', async () => {
      if (mustUpdateLocationHash()) {
        try {
          await handleHashChange();
        } catch (e) {
          console.error(e);
        }
      }
    });
  };

  /**
   * Navigate to root hash
   */
  const root = () => {
    const rootHash = getRootHash();
    if (isString(rootHash)) {
      navigate(rootHash);
    } else if (isFunction(rootHash)) {
      rootHash().then(res => {
        if (isObject(res)) {
          navigate(res.path, res.params);
        } else {
          navigate(res);
        }
      });
    }
  };
  const deletePage = param => {
    deleteCurrentInstance(param);
  };

  // export API
  var Router = {
    startRouter,
    navigate,
    resume,
    step,
    go: step,
    back: step.bind(null, -1),
    activePage: getActivePage,
    getActivePage() {
      // warning
      return getActivePage();
    },
    deletePage,
    getActiveRoute,
    getActiveHash,
    focusWidget,
    getActiveWidget,
    restoreFocus,
    isNavigating,
    getHistory,
    setHistory,
    getHistoryState,
    replaceHistoryState,
    getQueryStringParams,
    reload,
    symbols,
    App: RoutedApp,
    // keep backwards compatible
    focusPage: restoreFocus,
    root: root,
    /**
     * Deprecated api methods
     */
    setupRoutes() {
      console.warn('Router: setupRoutes is deprecated, consolidate your configuration');
      console.warn('https://rdkcentral.github.io/Lightning-SDK/#/plugins/router/configuration');
    },
    on() {
      console.warn('Router.on() is deprecated, consolidate your configuration');
      console.warn('https://rdkcentral.github.io/Lightning-SDK/#/plugins/router/configuration');
    },
    before() {
      console.warn('Router.before() is deprecated, consolidate your configuration');
      console.warn('https://rdkcentral.github.io/Lightning-SDK/#/plugins/router/configuration');
    },
    after() {
      console.warn('Router.after() is deprecated, consolidate your configuration');
      console.warn('https://rdkcentral.github.io/Lightning-SDK/#/plugins/router/configuration');
    }
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  let ApplicationInstance;
  var Launch = (App, appSettings, platformSettings, appData) => {
    initSettings$1(appSettings, platformSettings);
    initUtils(platformSettings);
    initStorage();
    // Initialize plugins
    if (platformSettings.plugins) {
      platformSettings.plugins.profile && initProfile(platformSettings.plugins.profile);
      platformSettings.plugins.metrics && initMetrics(platformSettings.plugins.metrics);
      platformSettings.plugins.mediaPlayer && initMediaPlayer(platformSettings.plugins.mediaPlayer);
      platformSettings.plugins.mediaPlayer && initVideoPlayer(platformSettings.plugins.mediaPlayer);
      platformSettings.plugins.ads && initAds(platformSettings.plugins.ads);
      platformSettings.plugins.router && initRouter(platformSettings.plugins.router);
      platformSettings.plugins.tv && initTV(platformSettings.plugins.tv);
      platformSettings.plugins.purchase && initPurchase(platformSettings.plugins.purchase);
      platformSettings.plugins.pin && initPin(platformSettings.plugins.pin);
    }
    const app = Application(App, appData, platformSettings);
    initLightningSdkPlugin.log = Log$1;
    initLightningSdkPlugin.settings = Settings$2;
    initLightningSdkPlugin.ads = Ads;
    initLightningSdkPlugin.lightning = Lightning$1;
    ApplicationInstance = new app(appSettings);
    initLightningSdkPlugin.appInstance = ApplicationInstance;
    return ApplicationInstance;
  };

  /*
   * If not stated otherwise in this file or this component's LICENSE file the
   * following copyright and licenses apply:
   *
   * Copyright 2020 Metrological
   *
   * Licensed under the Apache License, Version 2.0 (the License);
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
   */
  class SubtitleComponent extends Lightning$1.Component {
    static _template() {
      return {
        visible: false,
        rect: true,
        color: 0x90000000,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: 5
        },
        Text: {
          y: 5,
          x: 20,
          text: {
            textColor: 0xffffffff,
            fontSize: 38,
            lineHeight: 38 * 1.4,
            textAlign: 'center',
            wordWrap: true,
            maxLines: 3,
            shadow: true,
            shadowColor: 0xff333333
          }
        }
      };
    }
    _init() {
      this._textTextureDefaults = new Lightning$1.textures.TextTexture(this.stage).cloneArgs();
      this.tag('Text').on('txLoaded', _ref => {
        let {
          _source
        } = _ref;
        this.w = _source.w + this.tag('Text').x * 2;
        this.h = _source.h;
        this.position();
      });
    }
    get textFormat() {
      const textTag = this.tag('Text').text;
      return {
        fontFace: textTag.fontFace || 'sans-serif',
        fontSize: textTag.fontSize,
        lineHeight: textTag.lineHeight,
        textAlign: textTag.textAlign,
        wordWrap: true,
        maxLines: textTag.maxLines
      };
    }
    show() {
      this.visible = true;
    }
    hide() {
      this.visible = false;
    }
    position() {
      this.x = this._calculateX(this.xPos);
      this.y = this._calculateY(this.yPos);
    }
    set viewportW(v) {
      this._viewportW = v;
      this.x = this._calculateX(this.xPos);
    }
    get viewportW() {
      return this._viewportW || this.application.finalW;
    }
    set viewportH(v) {
      this._viewportH = v;
      this.y = this._calculateY(this.yPos);
    }
    get viewportH() {
      return this._viewportH || this.application.finalH;
    }
    _calculateX(x) {
      if (x === 'center') {
        x = (this.viewportW - this.finalW) / 2;
      } else if (x === 'left') {
        x = 60;
      } else if (x === 'right') {
        x = this.viewportW - this.finalW - 60;
      }
      return x;
    }
    set xPos(v) {
      this._x = v;
      this.x = this._calculateX(v);
    }
    get xPos() {
      return this._x || 'center';
    }
    _calculateY(y) {
      if (y === 'center') {
        return (this.viewportH - this.finalH) / 2;
      } else if (y === 'top') {
        return 60;
      } else if (y === 'bottom') {
        return this.viewportH - this.finalH - 60;
      }
      return y;
    }
    set yPos(v) {
      this._y = v;
      this.y = this._calculateY(v);
    }
    get yPos() {
      return this._y || 'bottom';
    }
    set fontFamily(v) {
      this.tag('Text').text.fontFace = v;
    }
    set fontSize(v) {
      this.tag('Text').text.fontSize = v;
      this.tag('Text').text.lineHeight = v * 1.3;
    }
    set fontColor(v) {
      this.tag('Text').color = v;
    }
    set backgroundColor(v) {
      this.color = v;
    }
    _defineBreakpoint(text, breakpoint) {
      if (breakpoint >= this.maxWidth) return this.maxWidth;
      const info = Lightning$1.textures.TextTexture.renderer(this.stage, this.stage.platform.getDrawingCanvas(), {
        ...this._textTextureDefaults,
        ...this.textFormat,
        ...{
          wordWrapWidth: breakpoint
        },
        text
      })._calculateRenderInfo();
      if (info.width <= breakpoint && info.lines.length <= 2) {
        return breakpoint;
      } else {
        return this._defineBreakpoint(text, breakpoint * 1.25);
      }
    }
    set text(v) {
      this.alpha = 0;
      if (v && v.length) {
        const breakpoint = this._defineBreakpoint(v, 640);
        this.tag('Text').text.wordWrapWidth = breakpoint;
        this.tag('Text').text = v;
        this.alpha = 1;
      }
    }
    set textAlign(v) {
      this._textAlign = v;
      this.tag('Text').text.textAlign = v;
    }
    set maxWidth(v) {
      this._maxWidth = v;
    }
    get maxWidth() {
      return (this._maxWidth || 1200) - this.tag('Text').x * 2;
    }
    set maxLines(v) {
      this.tag('Text').text.maxLines = v;
    }
  }

  var screen = {
  	width: 1920,
  	height: 1080
  };
  var safeArea = {
  	widthPercentage: 0.95,
  	heightPercentage: 0.95
  };
  var debug = {
  	showDimensionRuler: false,
  	mockEnabled: false,
  	verbose: false
  };
  var execution = {
  	categoryBatchSize: 5,
  	testBatchSize: 3
  };
  var layout = {
  	screen: {
  		width: 1920,
  		height: 1080
  	},
  	padding: {
  		left: 40,
  		top: 40,
  		right: 40,
  		bottom: 80
  	},
  	header: {
  		height: 120
  	},
  	content: {
  		offsetY: 160,
  		menuWidth: 400,
  		testRunnerOffsetX: 500
  	},
  	legends: {
  		offsetX: 1264,
  		offsetY: 860,
  		width: 520,
  		height: 150
  	},
  	footer: {
  		offsetY: 1000
  	}
  };
  var components = {
  	menu: {
  		itemWidth: 400,
  		itemHeight: 50,
  		itemSpacing: 54,
  		passRateGradientStep: 10,
  		buttonWidth: 300,
  		buttonHeight: 46
  	},
  	testRunner: {
  		testItemHeight: 50,
  		testItemSpacing: 54,
  		buttonWidth: 200,
  		buttonHeight: 42
  	},
  	resultsPanel: {
  		resultItemHeight: 50,
  		resultItemSpacing: 54
  	}
  };
  var colors = {
  	background: "0xff0f1419",
  	backgroundPattern: "0xff1a2332",
  	cardBackground: "0xff1e2936",
  	focusedBackground: "0xff2a3f5f",
  	primary: "0xff00d9ff",
  	primaryDark: "0xff0099cc",
  	secondary: "0xff7c3aed",
  	success: "0xff10b981",
  	error: "0xffef4444",
  	warning: "0xfff59e0b",
  	textPrimary: "0xffffffff",
  	textSecondary: "0xffa8b3cf",
  	textTertiary: "0xff6b7785",
  	accent: "0xffff6b6b",
  	accentGreen: "0xff4ecdc4",
  	titleBoxBgColor: "0x20ffffff",
  	titleBoxBorderColor: "0xff00d9ff",
  	titleTextColor: "0xff26c6da",
  	subtitleTextColor: "0xff0097cc",
  	headerGlowColorOff: "0x000097cc",
  	headerGlowColorOn: "0x220097cc",
  	connectionStatusConnected: "0xff22c55e",
  	passRateStop0: "0xffee1111",
  	passRateStop1: "0xffff8800",
  	passRateStop2: "0xffffdd00",
  	passRateStop3: "0xff22c55e",
  	bevelHighlight: "0x35ffffff",
  	bevelShadow: "0x45000000",
  	bevelPressedOverlay: "0x55000000",
  	focusGlowColor: "0x6600d9ff",
  	cardBackgroundPass: "0xff1a3a1a",
  	cardBackgroundFail: "0xff3a1a1a",
  	categoryRunningBackground: "0xff2d4a6d",
  	passRateColorDarkRed: "0xff8b0000",
  	passRateColorOrangeRed: "0xffff4500",
  	passRateColorOrange: "0xffffa500",
  	passRateColorDarkGreen: "0xff006400"
  };
  var typography = {
  	title: {
  		fontSize: 48
  	},
  	subtitle: {
  		fontSize: 24
  	},
  	headerSubtitle: {
  		fontSize: 14
  	},
  	heading: {
  		fontSize: 36
  	},
  	body: {
  		fontSize: 20
  	},
  	bodyLarge: {
  		fontSize: 24
  	},
  	bodySmall: {
  		fontSize: 18
  	},
  	caption: {
  		fontSize: 16
  	},
  	icon: {
  		fontSize: 28
  	}
  };
  var radii = {
  	card: 6,
  	container: 8,
  	panel: 12
  };
  var settings$2 = {
  	screen: screen,
  	safeArea: safeArea,
  	debug: debug,
  	execution: execution,
  	layout: layout,
  	components: components,
  	colors: colors,
  	typography: typography,
  	radii: radii
  };

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
  class AppSettings {
    constructor() {
      this._settings = settings$2;
      this._scaleX = 1;
      this._scaleY = 1;
      this._safeAreaOffsetX = 0;
      this._safeAreaOffsetY = 0;
    }

    // Initialize scale factors based on actual window size
    initScale(stageWidth, stageHeight) {
      this._scaleX = stageWidth / this._settings.screen.width;
      this._scaleY = stageHeight / this._settings.screen.height;

      // Calculate safe area offsets to center content
      const safeWidth = stageWidth * this._settings.safeArea.widthPercentage;
      const safeHeight = stageHeight * this._settings.safeArea.heightPercentage;
      this._safeAreaOffsetX = (stageWidth - safeWidth) / 2;
      this._safeAreaOffsetY = (stageHeight - safeHeight) / 2;
    }
    get safeArea() {
      return {
        offsetX: this._safeAreaOffsetX,
        offsetY: this._safeAreaOffsetY,
        width: this._settings.screen.width * this._scaleX * this._settings.safeArea.widthPercentage,
        height: this._settings.screen.height * this._scaleY * this._settings.safeArea.heightPercentage
      };
    }
    get debug() {
      return this._settings.debug;
    }
    get execution() {
      return this._settings.execution;
    }
    get screen() {
      return this._settings.screen;
    }
    get layout() {
      return this._scaleLayout(this._settings.layout);
    }
    get components() {
      return this._scaleComponents(this._settings.components);
    }
    get colors() {
      return this._settings.colors;
    }
    get radii() {
      return this._settings.radii;
    }
    get emboss() {
      return this._settings.emboss;
    }
    embossColors(baseColor) {
      const emboss = this._settings.emboss || {};
      const l = emboss.lightenAmount || 20;
      const d = emboss.darkenAmount || 15;
      const a = baseColor >>> 24 & 0xff;
      const r = baseColor >>> 16 & 0xff;
      const g = baseColor >>> 8 & 0xff;
      const b = baseColor & 0xff;
      const clamp = v => Math.max(0, Math.min(255, v));
      return {
        top: (a << 24 | clamp(r + l) << 16 | clamp(g + l) << 8 | clamp(b + l)) >>> 0,
        bottom: (a << 24 | clamp(r - d) << 16 | clamp(g - d) << 8 | clamp(b - d)) >>> 0
      };
    }
    get typography() {
      return this._scaleTypography(this._settings.typography);
    }

    // The drawable area available to Content components (TestRunner, ResultsPanel, Menu).
    // Computed from safe area minus padding and header.
    // maxH is the usable height before the Legends panel starts (with a 20px gap).
    // All rounded-rect containers must size themselves to maxH so they never overlap Legends.
    get contentArea() {
      const sa = this.safeArea;
      const layout = this.layout;
      const contentH = Math.round(sa.height - layout.content.offsetY - layout.padding.bottom);
      const legendsRelY = Math.round(layout.legends.offsetY - layout.content.offsetY);
      return {
        width: Math.round(sa.width - layout.padding.left - layout.padding.right),
        height: contentH,
        maxH: Math.min(contentH, legendsRelY - 20)
      };
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
      };
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
      };
    }

    // Scale typography
    _scaleTypography(typography) {
      // Use minimum to maintain aspect ratio
      const scale = Math.min(this._scaleX, this._scaleY);
      return {
        title: {
          fontSize: Math.round(typography.title.fontSize * scale)
        },
        subtitle: {
          fontSize: Math.round(typography.subtitle.fontSize * scale)
        },
        heading: {
          fontSize: Math.round(typography.heading.fontSize * scale)
        },
        body: {
          fontSize: Math.round(typography.body.fontSize * scale)
        },
        bodyLarge: {
          fontSize: Math.round(typography.bodyLarge.fontSize * scale)
        },
        bodySmall: {
          fontSize: Math.round(typography.bodySmall.fontSize * scale)
        },
        caption: {
          fontSize: Math.round(typography.caption.fontSize * scale)
        },
        icon: {
          fontSize: Math.round(typography.icon.fontSize * scale)
        }
      };
    }
    parseColor(colorString) {
      return parseInt(colorString, 16);
    }
    getColor(colorKey) {
      return this.parseColor(this._settings.colors[colorKey]);
    }
  }
  var AppSettings$1 = new AppSettings();

  function _defineProperty(e, r, t) {
    return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, {
      value: t,
      enumerable: !0,
      configurable: !0,
      writable: !0
    }) : e[r] = t, e;
  }
  function _toPrimitive(t, r) {
    if ("object" != typeof t || !t) return t;
    var e = t[Symbol.toPrimitive];
    if (void 0 !== e) {
      var i = e.call(t, r || "default");
      if ("object" != typeof i) return i;
      throw new TypeError("@@toPrimitive must return a primitive value.");
    }
    return ("string" === r ? String : Number)(t);
  }
  function _toPropertyKey(t) {
    var i = _toPrimitive(t, "string");
    return "symbol" == typeof i ? i : i + "";
  }

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

  class DeviceConnectionConfig {
    static normalizeIP(ip) {
      return typeof ip === 'string' ? ip.trim() : '';
    }

    /**
     * Get the configured device IP
     * Priority: URL query param > default
     */
    static getDeviceIP() {
      const params = new URLSearchParams(window.location.search);
      const queryIP = this.normalizeIP(params.get('deviceIP'));
      if (queryIP) {
        if (this.isValidIP(queryIP)) {
          return queryIP;
        }
        console.warn("Invalid deviceIP query value \"".concat(queryIP, "\". Falling back to local device (").concat(this.DEFAULT_IP, ")."));
      }
      return this.DEFAULT_IP;
    }

    /**
     * Get the full WebSocket endpoint URL
     * Format: ws://192.168.1.100:3473
     */
    static getEndpointURL() {
      const ip = this.getDeviceIP();
      return "".concat(this.PROTOCOL, "://").concat(ip, ":").concat(this.DEFAULT_PORT);
    }

    /**
     * Get connection info as display string
     */
    static getConnectionInfo() {
      const ip = this.getDeviceIP();
      const isLocal = ip === this.DEFAULT_IP || ip === 'localhost';
      return {
        ip,
        endpoint: this.getEndpointURL(),
        isLocal,
        display: isLocal ? 'Local Device (127.0.0.1)' : "Remote Device (".concat(ip, ")")
      };
    }

    /**
     * Validate IP address format
     */
    static isValidIP(ip) {
      if (!ip) {
        return false;
      }
      const normalized = this.normalizeIP(ip);
      const ipPattern = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
      return ipPattern.test(normalized) || normalized === 'localhost' || normalized === '127.0.0.1';
    }
  }
  _defineProperty(DeviceConnectionConfig, "DEFAULT_IP", '127.0.0.1');
  _defineProperty(DeviceConnectionConfig, "DEFAULT_PORT", 3473);
  _defineProperty(DeviceConnectionConfig, "PROTOCOL", 'ws');

  var accountTests = [
  	{
  		id: "account_id",
  		name: "account.id() [mock-only]",
  		description: "Platform back-office account identifier — Account is NOT in the @firebolt-js/core-client OpenRPC spec. Always returns mock data.",
  		method: "account.id",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "account_uid",
  		name: "account.uid() [mock-only]",
  		description: "Platform back-office user identifier — Account is NOT in the @firebolt-js/core-client OpenRPC spec. Always returns mock data.",
  		method: "account.uid",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	}
  ];

  var accessibilityTests = [
  	{
  		id: "accessibility_audioDescription",
  		name: "Accessibility.audioDescription()",
  		description: "Returns the audio description device setting",
  		method: "Accessibility.audioDescription",
  		type: "getter",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "accessibility_closedCaptionsSettings",
  		name: "Accessibility.closedCaptionsSettings()",
  		description: "Returns closed-captions settings: enabled flag and list of preferred languages",
  		method: "Accessibility.closedCaptionsSettings",
  		type: "getter",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "accessibility_highContrastUI",
  		name: "Accessibility.highContrastUI()",
  		description: "Returns the high contrast UI device setting",
  		method: "Accessibility.highContrastUI",
  		type: "getter",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "accessibility_voiceGuidanceSettings",
  		name: "Accessibility.voiceGuidanceSettings()",
  		description: "Returns voice guidance settings: enabled, rate and navigationHints",
  		method: "Accessibility.voiceGuidanceSettings",
  		type: "getter",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "accessibility_onAudioDescriptionChanged",
  		name: "Accessibility.onAudioDescriptionChanged [event]",
  		description: "Subscribe to audio description setting changes",
  		method: "Accessibility.audioDescription",
  		type: "event",
  		listenEvent: "onAudioDescriptionChanged",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "accessibility_onClosedCaptionsSettingsChanged",
  		name: "Accessibility.onClosedCaptionsSettingsChanged [event]",
  		description: "Subscribe to closed-captions settings changes",
  		method: "Accessibility.closedCaptionsSettings",
  		type: "event",
  		listenEvent: "onClosedCaptionsSettingsChanged",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "accessibility_onHighContrastUIChanged",
  		name: "Accessibility.onHighContrastUIChanged [event]",
  		description: "Subscribe to high contrast UI setting changes",
  		method: "Accessibility.highContrastUI",
  		type: "event",
  		listenEvent: "onHighContrastUIChanged",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "accessibility_onVoiceGuidanceSettingsChanged",
  		name: "Accessibility.onVoiceGuidanceSettingsChanged [event]",
  		description: "Subscribe to voice guidance settings changes",
  		method: "Accessibility.voiceGuidanceSettings",
  		type: "event",
  		listenEvent: "onVoiceGuidanceSettingsChanged",
  		params: [
  		],
  		expectedType: "object"
  	}
  ];

  var advertisingTests = [
  	{
  		id: "advertising_advertisingId",
  		name: "Advertising.advertisingId()",
  		description: "Returns the IFA (Identifier For Advertising): ifa UUID, ifa_type source, and lmt limit-ad-tracking flag",
  		method: "Advertising.advertisingId",
  		type: "getter",
  		params: [
  		],
  		expectedType: "object"
  	}
  ];

  var deviceTests = [
  	{
  		id: "device_deviceClass",
  		name: "Device.deviceClass()",
  		description: "Returns the class of the device: ott | stb | tv",
  		method: "Device.deviceClass",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "device_hdr",
  		name: "Device.hdr()",
  		description: "Returns HDR formats supported by the attached display: hdr10, hdr10Plus, dolbyVision, hlg",
  		method: "Device.hdr",
  		type: "getter",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "device_uid",
  		name: "Device.uid()",
  		description: "Returns a persistent unique UUID for the current app and device (reset on app/device reset)",
  		method: "Device.uid",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "device_chipsetId",
  		name: "Device.chipsetId() [gateway]",
  		description: "Returns the chipset identifier — spec-defined; called via Gateway JSON-RPC",
  		method: "Device.chipsetId",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "string"
  	},
  	{
  		id: "device_uptime",
  		name: "Device.uptime() [gateway]",
  		description: "Returns device uptime in seconds — spec-defined; called via Gateway JSON-RPC",
  		method: "Device.uptime",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "number"
  	},
  	{
  		id: "device_timeInActiveState",
  		name: "Device.timeInActiveState() [gateway]",
  		description: "Returns total time device has been in active state — spec-defined; called via Gateway JSON-RPC",
  		method: "Device.timeInActiveState",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "number"
  	},
  	{
  		id: "device_onHdrChanged",
  		name: "Device.onHdrChanged [event]",
  		description: "Subscribe to HDR format changes on the attached display",
  		method: "Device.hdr",
  		type: "event",
  		listenEvent: "onHdrChanged",
  		params: [
  		],
  		expectedType: "object"
  	}
  ];

  var discoveryTests = [
  	{
  		id: "discovery_watched",
  		name: "Discovery.watched()",
  		description: "Notify the platform that content was partially or fully watched. Returns true on success.",
  		method: "Discovery.watched",
  		type: "getter",
  		params: [
  			"test-entity-001",
  			0.95,
  			true,
  			"2021-04-23T18:25:43.511Z"
  		],
  		expectedType: "boolean"
  	}
  ];

  var displayTests = [
  	{
  		id: "display_edid",
  		name: "Display.edid() [gateway]",
  		description: "Returns the raw EDID data from the attached display — spec-defined; called via Gateway JSON-RPC",
  		method: "Display.edid",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "string"
  	},
  	{
  		id: "display_size",
  		name: "Display.size() [gateway]",
  		description: "Returns the physical display size in centimetres — spec-defined; called via Gateway JSON-RPC",
  		method: "Display.size",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "object"
  	},
  	{
  		id: "display_maxResolution",
  		name: "Display.maxResolution() [gateway]",
  		description: "Returns the maximum supported display resolution — spec-defined; called via Gateway JSON-RPC",
  		method: "Display.maxResolution",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "object"
  	}
  ];

  var lifecycleTests = [
  	{
  		id: "lifecycle_state",
  		name: "Lifecycle2.state() [gateway]",
  		description: "Returns the current lifecycle state of the application — spec-defined; called via Gateway JSON-RPC",
  		method: "Lifecycle2.state",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "string"
  	},
  	{
  		id: "lifecycle_close",
  		name: "Lifecycle2.close() [gateway]",
  		description: "Request the platform to deactivate the app with reason type=unload — spec-defined; called via Gateway JSON-RPC",
  		method: "Lifecycle2.close",
  		type: "gateway",
  		gatewayParams: {
  			type: "unload"
  		},
  		expectedType: "null"
  	}
  ];

  var localizationTests = [
  	{
  		id: "localization_country",
  		name: "Localization.country()",
  		description: "Returns the ISO 3166-1 alpha-2 country code for the device location",
  		method: "Localization.country",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "localization_preferredAudioLanguages",
  		name: "Localization.preferredAudioLanguages()",
  		description: "Returns list of ISO 639-2/B language codes for preferred audio in order of preference",
  		method: "Localization.preferredAudioLanguages",
  		type: "getter",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "localization_presentationLanguage",
  		name: "Localization.presentationLanguage()",
  		description: "Returns the full BCP 47 code (including script, region, variant) for the preferred locale",
  		method: "Localization.presentationLanguage",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "localization_onCountryChanged",
  		name: "Localization.onCountryChanged [event]",
  		description: "Subscribe to country code changes",
  		method: "Localization.onCountryChanged",
  		type: "event",
  		listenEvent: "onCountryChanged",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "localization_onPreferredAudioLanguagesChanged",
  		name: "Localization.onPreferredAudioLanguagesChanged [event]",
  		description: "Subscribe to preferred audio language changes",
  		method: "Localization.onPreferredAudioLanguagesChanged",
  		type: "event",
  		listenEvent: "onPreferredAudioLanguagesChanged",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "localization_onPresentationLanguageChanged",
  		name: "Localization.onPresentationLanguageChanged [event]",
  		description: "Subscribe to presentation language/locale changes",
  		method: "Localization.onPresentationLanguageChanged",
  		type: "event",
  		listenEvent: "onPresentationLanguageChanged",
  		params: [
  		],
  		expectedType: "string"
  	}
  ];

  var metricsTests = [
  	{
  		id: "metrics_ready",
  		name: "Metrics.ready()",
  		description: "Inform platform the app is minimally usable (called automatically by Lifecycle.ready)",
  		method: "Metrics.ready",
  		type: "getter",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_page",
  		name: "Metrics.page()",
  		description: "Inform the platform the user navigated to a page or view",
  		method: "Metrics.page",
  		type: "getter",
  		params: [
  			"test-page-home"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_appInfo",
  		name: "Metrics.appInfo()",
  		description: "Inform the platform about the app build/version string",
  		method: "Metrics.appInfo",
  		type: "getter",
  		params: [
  			"1.0.0-test"
  		],
  		expectedType: "null"
  	},
  	{
  		id: "metrics_startContent",
  		name: "Metrics.startContent()",
  		description: "Inform the platform the user started content playback",
  		method: "Metrics.startContent",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_stopContent",
  		name: "Metrics.stopContent()",
  		description: "Inform the platform the user stopped content playback",
  		method: "Metrics.stopContent",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_error",
  		name: "Metrics.error()",
  		description: "Inform the platform of an error that has occurred in the app",
  		method: "Metrics.error",
  		type: "getter",
  		params: [
  			"network",
  			"ERR_TEST_001",
  			"Test error from Firebolt test app",
  			false
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_event",
  		name: "Metrics.event()",
  		description: "Inform the platform of 1st party distributor metrics via schema/data JSON",
  		method: "Metrics.event",
  		type: "getter",
  		params: [
  			"http://firebolt.io/schema/test",
  			"{\"test\":true}"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaLoadStart",
  		name: "Metrics.mediaLoadStart()",
  		description: "Called when setting media asset URL to play, to infer load time",
  		method: "Metrics.mediaLoadStart",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaPlay",
  		name: "Metrics.mediaPlay()",
  		description: "Called when media playback should start (autoplay, user-play, unpause)",
  		method: "Metrics.mediaPlay",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaPlaying",
  		name: "Metrics.mediaPlaying()",
  		description: "Called when media playback actually starts",
  		method: "Metrics.mediaPlaying",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaPause",
  		name: "Metrics.mediaPause()",
  		description: "Called when media playback pauses intentionally",
  		method: "Metrics.mediaPause",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaWaiting",
  		name: "Metrics.mediaWaiting()",
  		description: "Called when playback halts due to network, buffer or other unintentional constraint",
  		method: "Metrics.mediaWaiting",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaSeeking",
  		name: "Metrics.mediaSeeking()",
  		description: "Called when a seek is initiated during media playback",
  		method: "Metrics.mediaSeeking",
  		type: "getter",
  		params: [
  			"test-entity-001",
  			0.25
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaSeeked",
  		name: "Metrics.mediaSeeked()",
  		description: "Called when a seek completes during media playback",
  		method: "Metrics.mediaSeeked",
  		type: "getter",
  		params: [
  			"test-entity-001",
  			0.25
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaRateChanged",
  		name: "Metrics.mediaRateChanged()",
  		description: "Called when playback rate changes (e.g. 2x fast-forward)",
  		method: "Metrics.mediaRateChanged",
  		type: "getter",
  		params: [
  			"test-entity-001",
  			1.5
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaRenditionChanged",
  		name: "Metrics.mediaRenditionChanged()",
  		description: "Called when playback rendition changes (bitrate/dimensions/profile)",
  		method: "Metrics.mediaRenditionChanged",
  		type: "getter",
  		params: [
  			"test-entity-001",
  			3000,
  			1920,
  			1080,
  			"HDR"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_mediaEnded",
  		name: "Metrics.mediaEnded()",
  		description: "Called when end-of-media is reached",
  		method: "Metrics.mediaEnded",
  		type: "getter",
  		params: [
  			"test-entity-001"
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_signIn",
  		name: "Metrics.signIn() [gateway]",
  		description: "Inform platform the user signed in — spec-defined; called via Gateway JSON-RPC",
  		method: "Metrics.signIn",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "boolean"
  	},
  	{
  		id: "metrics_signOut",
  		name: "Metrics.signOut() [gateway]",
  		description: "Inform platform the user signed out — spec-defined; called via Gateway JSON-RPC",
  		method: "Metrics.signOut",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "boolean"
  	}
  ];

  var networkTests = [
  	{
  		id: "network_connected",
  		name: "Network.connected()",
  		description: "Returns whether the device currently has a usable network connection",
  		method: "Network.connected",
  		type: "getter",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "network_onConnectedChanged",
  		name: "Network.onConnectedChanged [event]",
  		description: "Subscribe to network connectivity changes",
  		method: "Network.onConnectedChanged",
  		type: "event",
  		listenEvent: "onConnectedChanged",
  		params: [
  		],
  		expectedType: "boolean"
  	}
  ];

  var presentationTests = [
  	{
  		id: "presentation_focused",
  		name: "Presentation.focused() [gateway]",
  		description: "Returns whether the app is currently in a focused/presentation state — spec-defined; called via Gateway JSON-RPC",
  		method: "Presentation.focused",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "boolean"
  	},
  	{
  		id: "presentation_onFocusedChanged",
  		name: "Presentation.onFocusedChanged [gateway-event]",
  		description: "Subscribe to presentation focus changes via Gateway JSON-RPC — spec-defined",
  		method: "Presentation.onFocusedChanged",
  		type: "gateway-event",
  		expectedType: "boolean"
  	}
  ];

  var statsTests = [
  	{
  		id: "stats_memoryUsage",
  		name: "Stats.memoryUsage() [gateway]",
  		description: "Returns application-level memory usage statistics — spec-defined; called via Gateway JSON-RPC",
  		method: "Stats.memoryUsage",
  		type: "gateway",
  		gatewayParams: {
  		},
  		expectedType: "object"
  	}
  ];

  var texttospeechTests = [
  	{
  		id: "tts_listvoices",
  		name: "TextToSpeech.listvoices() [gateway]",
  		description: "List available TTS voices for the given language — spec-defined; called via Gateway JSON-RPC",
  		method: "TextToSpeech.listvoices",
  		type: "gateway",
  		gatewayParams: {
  			language: "en"
  		},
  		expectedType: "object"
  	},
  	{
  		id: "tts_getspeechstate",
  		name: "TextToSpeech.getspeechstate() [gateway]",
  		description: "Get current state of a TTS speech request (use speechId=0 for system default) — spec-defined",
  		method: "TextToSpeech.getspeechstate",
  		type: "gateway",
  		gatewayParams: {
  			speechid: 0
  		},
  		expectedType: "object"
  	},
  	{
  		id: "tts_speak",
  		name: "TextToSpeech.speak() [gateway]",
  		description: "Synthesize and speak the given text via the platform TTS engine — spec-defined",
  		method: "TextToSpeech.speak",
  		type: "gateway",
  		gatewayParams: {
  			text: "Hello from Firebolt Test App"
  		},
  		expectedType: "object"
  	},
  	{
  		id: "tts_pause",
  		name: "TextToSpeech.pause() [gateway]",
  		description: "Pause an in-progress TTS speech request — spec-defined",
  		method: "TextToSpeech.pause",
  		type: "gateway",
  		gatewayParams: {
  			speechid: 0
  		},
  		expectedType: "object"
  	},
  	{
  		id: "tts_resume",
  		name: "TextToSpeech.resume() [gateway]",
  		description: "Resume a paused TTS speech request — spec-defined",
  		method: "TextToSpeech.resume",
  		type: "gateway",
  		gatewayParams: {
  			speechid: 0
  		},
  		expectedType: "object"
  	},
  	{
  		id: "tts_cancel",
  		name: "TextToSpeech.cancel() [gateway]",
  		description: "Cancel an in-progress TTS speech request — spec-defined",
  		method: "TextToSpeech.cancel",
  		type: "gateway",
  		gatewayParams: {
  			speechid: 0
  		},
  		expectedType: "object"
  	}
  ];

  var mockstressTests = [
  	{
  		id: "mock_001",
  		name: "Mock.stringValue()",
  		description: "Returns a mock string value",
  		method: "mock.stringValue",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_002",
  		name: "Mock.numberValue()",
  		description: "Returns a mock numeric value",
  		method: "mock.numberValue",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_003",
  		name: "Mock.booleanTrue()",
  		description: "Returns boolean true",
  		method: "mock.booleanTrue",
  		type: "getter",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "mock_004",
  		name: "Mock.booleanFalse()",
  		description: "Returns boolean false",
  		method: "mock.booleanFalse",
  		type: "getter",
  		params: [
  		],
  		expectedType: "boolean"
  	},
  	{
  		id: "mock_005",
  		name: "Mock.objectPayload()",
  		description: "Returns a mock object with multiple fields",
  		method: "mock.objectPayload",
  		type: "getter",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "mock_006",
  		name: "Mock.arrayPayload()",
  		description: "Returns a mock array of strings",
  		method: "mock.arrayPayload",
  		type: "getter",
  		params: [
  		],
  		expectedType: "object"
  	},
  	{
  		id: "mock_007",
  		name: "Mock.nullValue()",
  		description: "Returns null (valid response indicator)",
  		method: "mock.nullValue",
  		type: "getter",
  		params: [
  		],
  		expectedType: "null"
  	},
  	{
  		id: "mock_008",
  		name: "Mock.slowResponse()",
  		description: "Simulates a 200ms delayed response",
  		method: "mock.slowResponse",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_009",
  		name: "Mock.fastResponse()",
  		description: "Simulates an immediate response",
  		method: "mock.fastResponse",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_010",
  		name: "Mock.versionString()",
  		description: "Returns a mock semantic version string",
  		method: "mock.versionString",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_011",
  		name: "Mock.deviceId()",
  		description: "Returns a mock device UUID",
  		method: "mock.deviceId",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_012",
  		name: "Mock.sessionToken()",
  		description: "Returns a mock session token",
  		method: "mock.sessionToken",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_013",
  		name: "Mock.countryCode()",
  		description: "Returns a mock ISO 3166 country code",
  		method: "mock.countryCode",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_014",
  		name: "Mock.languageCode()",
  		description: "Returns a mock BCP-47 language tag",
  		method: "mock.languageCode",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_015",
  		name: "Mock.resolutionWidth()",
  		description: "Returns mock horizontal resolution in pixels",
  		method: "mock.resolutionWidth",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_016",
  		name: "Mock.resolutionHeight()",
  		description: "Returns mock vertical resolution in pixels",
  		method: "mock.resolutionHeight",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_017",
  		name: "Mock.frameRate()",
  		description: "Returns mock frame rate in Hz",
  		method: "mock.frameRate",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_018",
  		name: "Mock.hdrProfile()",
  		description: "Returns mock HDR profile string",
  		method: "mock.hdrProfile",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_019",
  		name: "Mock.audioChannels()",
  		description: "Returns mock number of audio channels",
  		method: "mock.audioChannels",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_020",
  		name: "Mock.audioCodec()",
  		description: "Returns mock audio codec string",
  		method: "mock.audioCodec",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_021",
  		name: "Mock.networkType()",
  		description: "Returns mock network interface type",
  		method: "mock.networkType",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_022",
  		name: "Mock.ipAddress()",
  		description: "Returns mock IPv4 address string",
  		method: "mock.ipAddress",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_023",
  		name: "Mock.macAddress()",
  		description: "Returns mock MAC address string",
  		method: "mock.macAddress",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_024",
  		name: "Mock.storageUsed()",
  		description: "Returns mock used storage in bytes",
  		method: "mock.storageUsed",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_025",
  		name: "Mock.storageFree()",
  		description: "Returns mock free storage in bytes",
  		method: "mock.storageFree",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_026",
  		name: "Mock.memoryUsed()",
  		description: "Returns mock used RAM in MB",
  		method: "mock.memoryUsed",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_027",
  		name: "Mock.memoryFree()",
  		description: "Returns mock free RAM in MB",
  		method: "mock.memoryFree",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_028",
  		name: "Mock.cpuLoad()",
  		description: "Returns mock CPU load percentage",
  		method: "mock.cpuLoad",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_029",
  		name: "Mock.temperature()",
  		description: "Returns mock CPU temperature in Celsius",
  		method: "mock.temperature",
  		type: "getter",
  		params: [
  		],
  		expectedType: "number"
  	},
  	{
  		id: "mock_030",
  		name: "Mock.firmwareVersion()",
  		description: "Returns mock firmware version string",
  		method: "mock.firmwareVersion",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_031",
  		name: "Mock.buildTimestamp()",
  		description: "Returns mock ISO 8601 build timestamp",
  		method: "mock.buildTimestamp",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_032",
  		name: "Mock.appId()",
  		description: "Returns mock application identifier",
  		method: "mock.appId",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_033",
  		name: "Mock.appVersion()",
  		description: "Returns mock application version",
  		method: "mock.appVersion",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_034",
  		name: "Mock.partnerId()",
  		description: "Returns mock partner/operator identifier",
  		method: "mock.partnerId",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	},
  	{
  		id: "mock_035",
  		name: "Mock.platformName()",
  		description: "Returns mock platform name string",
  		method: "mock.platformName",
  		type: "getter",
  		params: [
  		],
  		expectedType: "string"
  	}
  ];

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

  // Verbose debug logger — only emits when app-settings.json debug.verbose is true
  const dbg = msg => AppSettings$1.debug.verbose && console.log(msg);
  let FireboltModules = {};
  let useNpmPackage = false;
  let connectionInfo = null;
  let _connectionSetup = false;
  let _modulesInitPromise = null;

  // Maps category IDs to their statically-imported JSON test definitions.
  // All 13 Firebolt OpenRPC modules are represented here.
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
  };

  // Validate that a Firebolt endpoint URL uses the ws:// or wss:// scheme.
  // This guards against non-WebSocket URL schemes; it does not restrict the target host.
  function _isValidFireboltEndpoint(url) {
    try {
      const parsed = new URL(url);
      return parsed.protocol === 'ws:' || parsed.protocol === 'wss:';
    } catch (_) {
      return false;
    }
  }

  // Extract the display hostname from a WebSocket endpoint URL.
  function _hostFromEndpoint(endpoint) {
    try {
      return new URL(endpoint).hostname;
    } catch (_) {
      return endpoint;
    }
  }

  // Setup device connection configuration before importing modules — runs once
  function setupDeviceConnection() {
    if (_connectionSetup) return;
    _connectionSetup = true;
    connectionInfo = DeviceConnectionConfig.getConnectionInfo();

    // Priority 1: window.__firebolt.endpoint — injected at document start by the RDK BrowserLauncher
    //   as: window.__firebolt = { endpoint: 'ws://...' };
    // Priority 2: __firebolt_endpoint query param — legacy / dev fallback
    const rawEndpoint = window.__firebolt && window.__firebolt.endpoint || new URLSearchParams(window.location.search).get('__firebolt_endpoint');
    const fbEndpoint = rawEndpoint && _isValidFireboltEndpoint(rawEndpoint) ? rawEndpoint : null;
    if (rawEndpoint && !fbEndpoint) {
      dbg('Ignoring invalid Firebolt endpoint (must be ws:// or wss://)');
    }
    if (fbEndpoint) {
      dbg("Using Firebolt endpoint from ".concat(window.__firebolt && window.__firebolt.endpoint ? 'window.__firebolt.endpoint' : '__firebolt_endpoint query param', ": ").concat(fbEndpoint));
      connectionInfo = Object.assign({}, connectionInfo, {
        endpoint: fbEndpoint
      });
      window.__firebolt = window.__firebolt || {};
      window.__firebolt.endpoint = fbEndpoint;
    }

    // Reference: https://github.com/rdkcentral/entservices-appgateway/blob/develop/docs/RDK8.md#compliant-json-rpc-detection
    if (!connectionInfo.endpoint.includes('RPCV2=true')) {
      connectionInfo.endpoint += (connectionInfo.endpoint.includes('?') ? '&' : '?') + 'RPCV2=true';
    }
    dbg(' Firebolt Device Connection:');
    dbg('   Endpoint: ' + connectionInfo.endpoint);
    dbg('   Type: ' + connectionInfo.display);
    if (window.__FIREBOLT_CONFIG__ === undefined) {
      window.__FIREBOLT_CONFIG__ = {
        endpoint: connectionInfo.endpoint,
        transport: 'ws'
      };
    }
  }

  // Attempt to load firebolt-js-client modules (primary backend) — runs once
  function initializeFireboltModules() {
    if (_modulesInitPromise) return _modulesInitPromise;
    _modulesInitPromise = _doInitializeFireboltModules();
    return _modulesInitPromise;
  }
  async function _doInitializeFireboltModules() {
    setupDeviceConnection();
    try {
      const m = await Promise.resolve().then(function () { return firebolt; });
      const moduleMap = {
        Accessibility: m.Accessibility,
        Advertising: m.Advertising,
        Device: m.Device,
        Discovery: m.Discovery,
        Localization: m.Localization,
        Metrics: m.Metrics,
        Network: m.Network
      };
      Object.entries(moduleMap).forEach(_ref => {
        let [name, mod] = _ref;
        if (mod) FireboltModules[name] = mod;
      });
      if (Object.keys(FireboltModules).length > 0) {
        useNpmPackage = true;
        dbg('Firebolt JS Client initialized as primary backend');
        dbg("   Connected to: ".concat(connectionInfo.endpoint));
        return true;
      }
    } catch (error) {
      dbg('Firebolt JS Client not available, falling back to window.Firebolt:' + error.message);
      useNpmPackage = false;
    }
    return false;
  }
  class FireboltAPI {
    constructor() {
      this.initialized = false;
      this._mockFirebolt = this._createMockFirebolt();
      this.firebolt = window.Firebolt || this._mockFirebolt;
      this.usingMock = !window.Firebolt;
      this._lastCallBackend = 'unknown';
      this._testDefinitions = {};
      this._eventListeners = []; // { moduleObj, listenerId }
      this._eventSockets = []; // WebSocket refs for gateway-event subscriptions
      this._initializeDefaultTests();
    }

    // Initialize the API (call this after instantiation)
    async init() {
      if (!this.initialized) {
        await initializeFireboltModules();
        this.initialized = true;
      }
    }

    // Subscribe persistently to all event-type and gateway-event-type tests at startup.
    // Fires a console.log whenever a subscribed event arrives — no teardown.
    async subscribeAllEvents() {
      try {
        await this.init();
        const allDefs = Object.values(TEST_DEFINITIONS_BY_CATEGORY).flat();
        const eventDefs = allDefs.filter(d => d.type === 'event' || d.type === 'gateway-event');
        for (const def of eventDefs) {
          if (def.type === 'event') {
            const moduleName = def.method.split('.')[0];
            const moduleKey = Object.keys(FireboltModules).find(k => k.toLowerCase() === moduleName.toLowerCase());
            const moduleObj = moduleKey ? FireboltModules[moduleKey] : null;
            if (!moduleObj || typeof moduleObj.listen !== 'function') {
              dbg("[Event Monitor] ".concat(def.name, ": module not available, skipping"));
              continue;
            }
            try {
              const _errStr = e => e && e.message ? e.message : JSON.stringify(e);
              const listenPromise = moduleObj.listen(def.listenEvent, value => {
                const payloadStr = JSON.stringify(value);
                dbg("[Event Monitor] ".concat(def.name, " fired"));
                dbg("[Event Monitor] Full msg: ".concat(payloadStr));
                if (this.onEventLog) this.onEventLog(def.name, payloadStr);
              });
              // Attach .catch() before await so any secondary internal SDK rejections
              // from this promise are silenced and don't become unhandled rejections.
              listenPromise.catch(e => {
                dbg("[Event Monitor] Internal rejection for ".concat(def.name, ":") + _errStr(e));
              });
              const listenerId = await listenPromise;
              this._eventListeners.push({
                moduleObj,
                listenerId
              });
              dbg("[Event Monitor] Subscribed: ".concat(def.name));
            } catch (err) {
              const msg = err && err.message ? err.message : JSON.stringify(err);
              dbg("[Event Monitor] Failed to subscribe ".concat(def.name, ":") + msg);
            }
          } else if (def.type === 'gateway-event') {
            // Keep the WebSocket open persistently for gateway events
            this._subscribeGatewayEventPersistent(def.method, def.name);
          }
        }
      } catch (e) {
        console.error('subscribeAllEvents error: ' + (e && e.message ? e.message : JSON.stringify(e)));
      }
    }

    // Tear down all persistent event subscriptions registered by subscribeAllEvents().
    unsubscribeAllEvents() {
      for (const {
        moduleObj,
        listenerId
      } of this._eventListeners) {
        try {
          if (moduleObj.clear) moduleObj.clear(listenerId);
        } catch (_) {}
      }
      this._eventListeners = [];
      for (const ws of this._eventSockets) {
        try {
          ws.close();
        } catch (_) {}
      }
      this._eventSockets = [];
      dbg('[Event Monitor] All event subscriptions torn down');
    }

    // Opens a persistent WebSocket subscription for a gateway event and logs on each fire.
    _subscribeGatewayEventPersistent(method, label) {
      const doConnect = async () => {
        try {
          await this.init();
          const info = connectionInfo;
          let ws;
          try {
            ws = new WebSocket(info.endpoint);
          } catch (err) {
            dbg("[Event Monitor] ".concat(label, ": cannot connect \u2014 ").concat(err.message));
            return;
          }
          this._eventSockets.push(ws);
          const id = Math.floor(Math.random() * 900000) + 100000;
          let subscribed = false;
          ws.onopen = () => {
            ws.send(JSON.stringify({
              jsonrpc: '2.0',
              id,
              method,
              params: {
                listen: true
              }
            }));
          };
          ws.onmessage = evt => {
            try {
              const msg = JSON.parse(evt.data);
              if (!subscribed && msg.id === id) {
                if (msg.error) {
                  var _msg$error$message;
                  const safeErrorMessage = String((_msg$error$message = msg.error.message) !== null && _msg$error$message !== void 0 ? _msg$error$message : '').replace(/\r|\n/g, '');
                  dbg("[Event Monitor] ".concat(label, ": subscribe error \u2014 ").concat(safeErrorMessage));
                } else {
                  subscribed = true;
                  dbg("[Event Monitor] Subscribed: ".concat(label));
                }
              } else if (subscribed && !msg.id && msg.method) {
                const payloadStr = JSON.stringify(msg);
                dbg("[Event Monitor] ".concat(label, " fired"));
                dbg("[Event Monitor] Full msg: ".concat(payloadStr));
                if (this.onEventLog) this.onEventLog(label, payloadStr);
              }
            } catch (_) {}
          };
          ws.onerror = () => dbg("[Event Monitor] ".concat(label, ": WebSocket error"));
          ws.onclose = () => dbg("[Event Monitor] ".concat(label, ": connection closed"));
        } catch (err) {
          dbg("[Event Monitor] ".concat(label, ": init error \u2014 ") + err.message);
        }
      };
      doConnect();
    }

    // Call a spec-defined method that is not yet exposed by the SDK package.
    // Opens a short-lived WebSocket to the Firebolt JSON-RPC endpoint and makes
    // a single request, then closes the connection.
    async _callViaGateway(method) {
      let params = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
      await this.init();
      const info = connectionInfo;
      return new Promise((resolve, reject) => {
        let ws;
        try {
          ws = new WebSocket(info.endpoint);
        } catch (err) {
          return reject(new Error("Cannot open WebSocket to ".concat(info.endpoint, ": ").concat(err.message)));
        }
        const id = Math.floor(Math.random() * 900000) + 100000;
        const timeout = setTimeout(() => {
          try {
            ws.close();
          } catch (_) {}
          reject(new Error("Gateway timeout for ".concat(method, " (5s)")));
        }, 5000);
        ws.onopen = () => {
          ws.send(JSON.stringify({
            jsonrpc: '2.0',
            id,
            method,
            params
          }));
        };
        ws.onmessage = evt => {
          try {
            const msg = JSON.parse(evt.data);
            if (msg.id === id) {
              clearTimeout(timeout);
              try {
                ws.close();
              } catch (_) {}
              if (msg.error) {
                reject(new Error("[".concat(msg.error.code, "] ").concat(msg.error.message)));
              } else {
                this._lastCallBackend = 'gateway';
                resolve(msg.result);
              }
            }
          } catch (e) {
            clearTimeout(timeout);
            try {
              ws.close();
            } catch (_) {}
            reject(e);
          }
        };
        ws.onerror = () => {
          clearTimeout(timeout);
          try {
            ws.close();
          } catch (_) {}
          reject(new Error("WebSocket error calling ".concat(method)));
        };
      });
    }

    // Subscribe to an event via the SDK module's listen() method.
    // One-shot: clears the listener when the event fires.
    // If timeoutMs > 0, the listener is also cleared when the timeout elapses (preventing leaks).
    _subscribeToEvent(moduleName, listenEvent) {
      let timeoutMs = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 0;
      const doSubscribe = async (resolve, reject) => {
        try {
          if (!this.initialized) await this.init();
          const moduleKey = Object.keys(FireboltModules).find(k => k.toLowerCase() === moduleName.toLowerCase());
          const moduleObj = moduleKey ? FireboltModules[moduleKey] : null;
          if (!moduleObj || typeof moduleObj.listen !== 'function') {
            return reject(new Error("Module ".concat(moduleName, " not available or has no listen() method")));
          }
          this._lastCallBackend = 'core-client';
          let listenerId;
          let timeoutHandle;
          listenerId = await moduleObj.listen(listenEvent, value => {
            clearTimeout(timeoutHandle);
            dbg("[Firebolt Event] ".concat(moduleName, ".").concat(listenEvent, " fired"));
            dbg("[Firebolt Event] Full msg: ".concat(JSON.stringify(value)));
            if (moduleObj.clear) moduleObj.clear(listenerId);
            resolve(value);
          });
          dbg("[Firebolt Event] Subscribed to ".concat(moduleName, ".").concat(listenEvent, ", listener id:") + listenerId);
          if (timeoutMs > 0) {
            timeoutHandle = setTimeout(() => {
              if (moduleObj.clear) moduleObj.clear(listenerId);
              resolve(null);
            }, timeoutMs);
          }
        } catch (err) {
          reject(err);
        }
      };
      return new Promise(doSubscribe);
    }

    // Subscribe to a spec-defined event that is not in the SDK, via raw WebSocket.
    // Sends {listen:true}, waits for ACK, then keeps WS open until the event fires.
    // If timeoutMs > 0, the WebSocket is closed and the promise resolves with null when
    // the timeout elapses, preventing socket leaks when no event arrives.
    _subscribeViaGateway(method) {
      let timeoutMs = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
      return new Promise((resolve, reject) => {
        const doConnect = async () => {
          try {
            await this.init();
            const info = connectionInfo;
            let ws;
            try {
              ws = new WebSocket(info.endpoint);
            } catch (err) {
              return reject(new Error("Cannot open WebSocket to ".concat(info.endpoint, ": ").concat(err.message)));
            }
            const id = Math.floor(Math.random() * 900000) + 100000;
            let subscribed = false;
            let timeoutHandle;
            if (timeoutMs > 0) {
              timeoutHandle = setTimeout(() => {
                try {
                  ws.close();
                } catch (_) {}
                resolve(null);
              }, timeoutMs);
            }
            ws.onopen = () => {
              ws.send(JSON.stringify({
                jsonrpc: '2.0',
                id,
                method,
                params: {
                  listen: true
                }
              }));
            };
            ws.onmessage = evt => {
              try {
                const msg = JSON.parse(evt.data);
                if (!subscribed && msg.id === id) {
                  // Subscription ACK — now wait for the event notification
                  if (msg.error) {
                    clearTimeout(timeoutHandle);
                    try {
                      ws.close();
                    } catch (_) {}
                    reject(new Error("[".concat(msg.error.code, "] ").concat(msg.error.message)));
                  } else {
                    subscribed = true;
                    const safeListenerId = String(msg.result).replace(/[\r\n]/g, '');
                    dbg("[Firebolt Gateway Event] Subscribed to ".concat(method, ", listener id:") + safeListenerId);
                  }
                } else if (subscribed && !msg.id && msg.method) {
                  // Event notification — no id field, method matches
                  clearTimeout(timeoutHandle);
                  dbg("[Firebolt Gateway Event] ".concat(method, " fired"));
                  dbg("[Firebolt Gateway Event] Full msg: ".concat(JSON.stringify(msg)));
                  try {
                    ws.close();
                  } catch (_) {}
                  this._lastCallBackend = 'gateway';
                  resolve(msg.params);
                }
              } catch (e) {
                clearTimeout(timeoutHandle);
                try {
                  ws.close();
                } catch (_) {}
                reject(e);
              }
            };
            ws.onerror = () => {
              clearTimeout(timeoutHandle);
              try {
                ws.close();
              } catch (_) {}
              reject(new Error("WebSocket error subscribing to ".concat(method)));
            };
          } catch (err) {
            reject(err);
          }
        };
        doConnect();
      });
    }

    // Safe method caller that works with both backends
    async _callMethod(methodPath) {
      let params = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : [];
      // Ensure initialization on first call
      if (!this.initialized) {
        await this.init();
      }

      // account.* and mock.* are always served from the local mock regardless of window.Firebolt
      if (methodPath.startsWith('account.') || methodPath.startsWith('mock.')) {
        const [ns, methodName] = methodPath.split('.');
        const method = this._mockFirebolt[ns] && this._mockFirebolt[ns][methodName];
        if (typeof method === 'function') {
          this._lastCallBackend = 'mock';
          return await method.apply(this._mockFirebolt[ns], params);
        }
        throw new Error("Mock method not found: ".concat(methodPath));
      }
      try {
        // First, try firebolt-js-client if initialized
        if (useNpmPackage && methodPath.includes('.')) {
          const [moduleName, methodName] = methodPath.split('.');
          const moduleKey = Object.keys(FireboltModules).find(key => key.toLowerCase() === moduleName.toLowerCase());
          const moduleObj = moduleKey ? FireboltModules[moduleKey] : null;
          if (moduleObj) {
            const resolvedMethod = moduleObj[methodName] || moduleObj[methodName.toLowerCase()] || moduleObj[methodName.charAt(0).toLowerCase() + methodName.slice(1)];
            if (typeof resolvedMethod === 'function') {
              this._lastCallBackend = 'core-client';
              const result = await resolvedMethod(...params);
              return result;
            }
          }

          // Retry once with module name in PascalCase for definitions like account.id
          const pascalModule = moduleName.charAt(0).toUpperCase() + moduleName.slice(1);
          if (FireboltModules[pascalModule]) {
            const resolvedMethod = FireboltModules[pascalModule][methodName] || FireboltModules[pascalModule][methodName.toLowerCase()] || FireboltModules[pascalModule][methodName.charAt(0).toLowerCase() + methodName.slice(1)];
            if (typeof resolvedMethod === 'function') {
              this._lastCallBackend = 'core-client';
              const result = await resolvedMethod(...params);
              return result;
            }
          }
        }

        // Fallback to window.Firebolt (legacy or when npm package not available)
        const pathParts = methodPath.split('.');
        let obj = this.firebolt;
        for (let i = 0; i < pathParts.length - 1; i++) {
          const part = pathParts[i];
          obj = obj[part] || obj[part.toLowerCase()] || obj[part.charAt(0).toLowerCase() + part.slice(1)];
          if (!obj) throw new Error("API not available: ".concat(pathParts.slice(0, i + 1).join('.')));
        }
        const methodName = pathParts[pathParts.length - 1].replace('()', '');
        const method = obj[methodName] || obj[methodName.toLowerCase()] || obj[methodName.charAt(0).toLowerCase() + methodName.slice(1)];
        if (!method) throw new Error("Method not found: ".concat(methodPath));
        this._lastCallBackend = this.usingMock ? 'mock' : 'window.firebolt';
        return await method.apply(obj, params);
      } catch (error) {
        console.error("Error calling ".concat(methodPath, ": ").concat(error.message));
        throw error;
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
      };
    }

    // Returns raw test definitions for a specific category (synchronous — for test introspection).
    _getTestsForCategory(categoryId) {
      return TEST_DEFINITIONS_BY_CATEGORY[categoryId] || [];
    }

    // Returns all raw test definitions annotated with _category (synchronous — for test introspection).
    _getAllTestDefinitions() {
      return Object.entries(TEST_DEFINITIONS_BY_CATEGORY).flatMap(_ref2 => {
        let [cat, defs] = _ref2;
        return (defs || []).map(d => Object.assign({}, d, {
          _category: cat
        }));
      });
    }
    async loadTestsForCategory(categoryId) {
      if (this._testDefinitions[categoryId]) {
        return this._testDefinitions[categoryId];
      }
      try {
        const testDefs = TEST_DEFINITIONS_BY_CATEGORY[categoryId];
        if (!testDefs || !Array.isArray(testDefs)) {
          return this._defaultTests[categoryId] || [];
        }
        const tests = testDefs.map(def => this._createTestFromDefinition(def));
        this._testDefinitions[categoryId] = tests;
        return tests;
      } catch (error) {
        dbg("Using default tests for category: ".concat(categoryId));
        return this._defaultTests[categoryId] || [];
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
            let response;
            if (def.type === 'gateway') {
              // Spec-defined method not yet in SDK package — raw WebSocket JSON-RPC
              response = await this._callViaGateway(def.method, def.gatewayParams || {});
              return {
                value: response,
                type: typeof response,
                backend: 'gateway'
              };
            } else if (def.type === 'gateway-event') {
              // Spec-defined event not in SDK — subscribe via raw WebSocket, wait for event to fire
              const eventFired = await Promise.race([this._subscribeViaGateway(def.method).then(v => ({
                fired: true,
                value: v
              })), new Promise(resolve => setTimeout(() => resolve({
                fired: false
              }), 7500))]);
              if (eventFired.fired) {
                return {
                  value: eventFired.value,
                  type: typeof eventFired.value === 'object' ? 'object' : typeof eventFired.value,
                  backend: 'gateway'
                };
              }
              return {
                value: 'Subscribed — no event fired in test window',
                type: 'string',
                backend: 'gateway'
              };
            } else if (def.type === 'event') {
              // Event test: subscribe and wait for the event to fire within the test window
              const moduleName = def.method.split('.')[0];
              const eventFired = await Promise.race([this._subscribeToEvent(moduleName, def.listenEvent).then(v => ({
                fired: true,
                value: v
              })), new Promise(resolve => setTimeout(() => resolve({
                fired: false
              }), 7500))]);
              if (eventFired.fired) {
                return {
                  value: eventFired.value,
                  type: typeof eventFired.value === 'object' ? 'object' : typeof eventFired.value,
                  backend: this._lastCallBackend
                };
              }
              return {
                value: 'Subscribed \u2014 no event fired in test window',
                type: 'string',
                backend: this._lastCallBackend
              };
            } else {
              // Default: getter/method call through the SDK or mock fallback chain
              response = await this._callMethod(def.method, def.params || []);
              return {
                value: response,
                type: typeof response,
                backend: this._lastCallBackend
              };
            }
          } catch (error) {
            return {
              value: error.message,
              type: 'error',
              backend: this._lastCallBackend || 'unknown'
            };
          }
        }
      };
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
          closedCaptionsSettings: () => Promise.resolve({
            enabled: true,
            preferredLanguages: ['eng']
          }),
          highContrastUI: () => Promise.resolve(false),
          voiceGuidanceSettings: () => Promise.resolve({
            enabled: false,
            rate: 1.0,
            navigationHints: true
          }),
          listen: () => Promise.resolve(1),
          clear: () => true
        },
        // Advertising module (SDK: Advertising.advertisingId)
        Advertising: {
          advertisingId: () => Promise.resolve({
            ifa: 'mock-ifa-uuid',
            ifa_type: 'sessionid',
            lmt: '0'
          })
        },
        // Device module (SDK: Device.deviceClass, Device.hdr, Device.uid)
        Device: {
          deviceClass: () => Promise.resolve('stb'),
          hdr: () => Promise.resolve({
            hdr10: true,
            hdr10Plus: false,
            dolbyVision: false,
            hlg: true
          }),
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
          page: pageId => Promise.resolve(true),
          appInfo: build => Promise.resolve(null),
          startContent: entityId => Promise.resolve(true),
          stopContent: entityId => Promise.resolve(true),
          error: (type, code, desc, visible) => Promise.resolve(true),
          event: (schema, data) => Promise.resolve(true),
          mediaLoadStart: entityId => Promise.resolve(true),
          mediaPlay: entityId => Promise.resolve(true),
          mediaPlaying: entityId => Promise.resolve(true),
          mediaPause: entityId => Promise.resolve(true),
          mediaWaiting: entityId => Promise.resolve(true),
          mediaSeeking: (entityId, target) => Promise.resolve(true),
          mediaSeeked: (entityId, position) => Promise.resolve(true),
          mediaRateChanged: (entityId, rate) => Promise.resolve(true),
          mediaRenditionChanged: (entityId, bitrate, width, height, profile) => Promise.resolve(true),
          mediaEnded: entityId => Promise.resolve(true)
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
          objectPayload: () => Promise.resolve({
            key: 'value',
            count: 1
          }),
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
      };
    }
    getTestsForCategory(categoryId) {
      return this.loadTestsForCategory(categoryId);
    }
    async getVersionInfo() {
      let deviceUid = 'UID-Error';
      try {
        const timeout = new Promise((_, reject) => setTimeout(() => reject(new Error('timeout')), 3000));
        deviceUid = await Promise.race([this._callMethod('Device.uid'), timeout]);
      } catch (_) {}
      return {
        sdkVersion: '8.0.0',
        deviceUid,
        raw: null
      };
    }
    async getConnectionStatus() {
      await this.init();
      const info = connectionInfo;
      const displayHost = _hostFromEndpoint(info.endpoint);
      // Derive status from whether the SDK modules loaded — no probe socket opened.
      if (useNpmPackage) {
        return {
          state: 'connected',
          label: "Connected (".concat(displayHost, ")"),
          endpoint: info.endpoint,
          backend: 'core-client'
        };
      }
      if (this.usingMock) {
        return {
          state: 'mock',
          label: 'Mock Mode',
          endpoint: info.endpoint,
          backend: 'mock'
        };
      }

      // window.Firebolt present but npm SDK not loaded — calls still route through window.Firebolt
      return {
        state: 'connected-legacy',
        label: "Connected (".concat(displayHost, ")"),
        endpoint: info.endpoint,
        backend: 'window.firebolt'
      };
    }
    async runTest(test) {
      let _timeoutId;
      try {
        this._lastCallBackend = 'unknown';
        const startTime = Date.now();
        const timeoutPromise = new Promise((_, reject) => {
          _timeoutId = setTimeout(() => reject(new Error('Test timeout (8s)')), 8000);
        });
        dbg("[Firebolt] >> ".concat(test.method, " params=").concat(JSON.stringify(test.params || test.gatewayParams || [])));
        const result = await Promise.race([test.execute(), timeoutPromise]);
        clearTimeout(_timeoutId);
        dbg("[Firebolt] << ".concat(test.method, " result=").concat(JSON.stringify(result)));
        const duration = Date.now() - startTime;
        const backend = (result === null || result === void 0 ? void 0 : result.backend) || this._lastCallBackend || (this.usingMock ? 'mock' : 'window.firebolt');
        const actualType = result.value === null ? 'null' : result.value === undefined ? 'undefined' : typeof result.value;
        const expectedType = test.expectedType;
        // For event/gateway-event tests, success is determined by subscription (no error), not payload type
        const typeOk = !expectedType || test.type === 'event' || test.type === 'gateway-event' || actualType === expectedType;
        return {
          test,
          success: result.type !== 'error' && typeOk,
          result,
          backend,
          duration,
          message: result.type === 'error' ? "Error: ".concat(result.value) : typeOk ? "Returned: ".concat(JSON.stringify(result.value), " (").concat(actualType, ")") : "Type mismatch: expected ".concat(expectedType, ", got ").concat(actualType, " \u2014 ").concat(JSON.stringify(result.value))
        };
      } catch (error) {
        clearTimeout(_timeoutId);
        return {
          test,
          success: false,
          backend: this._lastCallBackend || (this.usingMock ? 'mock' : 'window.firebolt'),
          error: error.message,
          message: "Error: ".concat(error.message)
        };
      }
    }
  }

  var categories = [
  	{
  		id: "accessibility",
  		name: "Accessibility",
  		description: "Closed captions, voice guidance and audio description settings"
  	},
  	{
  		id: "advertising",
  		name: "Advertising",
  		description: "Platform advertising identifier (IFA)"
  	},
  	{
  		id: "device",
  		name: "Device",
  		description: "Device class, HDR capabilities and identifiers"
  	},
  	{
  		id: "discovery",
  		name: "Discovery",
  		description: "Content watch history and discovery"
  	},
  	{
  		id: "display",
  		name: "Display",
  		description: "Display EDID, size and max resolution [spec-defined, via Gateway]"
  	},
  	{
  		id: "lifecycle",
  		name: "Lifecycle",
  		description: "Application lifecycle state and close [spec-defined, via Gateway]",
  		runAllExcluded: true
  	},
  	{
  		id: "localization",
  		name: "Localization",
  		description: "Country, preferred audio languages and presentation locale"
  	},
  	{
  		id: "metrics",
  		name: "Metrics",
  		description: "App readiness, page, media playback and custom metrics"
  	},
  	{
  		id: "network",
  		name: "Network",
  		description: "Network connectivity status and events"
  	},
  	{
  		id: "presentation",
  		name: "Presentation",
  		description: "App focus/presentation state [spec-defined, via Gateway]"
  	},
  	{
  		id: "stats",
  		name: "Stats",
  		description: "Application memory usage statistics [spec-defined, via Gateway]"
  	},
  	{
  		id: "texttospeech",
  		name: "TextToSpeech",
  		description: "TTS synthesis, control and speech state [spec-defined, via Gateway]"
  	},
  	{
  		id: "account",
  		name: "Account (mock)",
  		description: "Account ID — not in firebolt-js-client OpenRPC spec, falls back to mock"
  	},
  	{
  		id: "mockstress",
  		name: "Mock Stress Test",
  		description: "35 synthetic mock tests to validate multi-column scroll layout under load"
  	}
  ];

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
  class Menu extends Lightning$1.Component {
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
            type: Lightning$1.shaders.RoundedRectangle,
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
            type: Lightning$1.shaders.RoundedRectangle,
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
            x: 0,
            y: 0,
            w: 300,
            h: 60,
            rect: true,
            color: 0x55000000,
            alpha: 0,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
              radius: 8
            }
          },
          GlowRing: {
            x: 0,
            y: 0,
            w: 300,
            h: 60,
            rect: true,
            color: 0x00000000,
            alpha: 0,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
              stroke: 3,
              strokeColor: 0xff00d9ff,
              radius: 8
            }
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
              type: Lightning$1.shaders.RoundedRectangle,
              radius: 4
            }
          },
          ProgressFill: {
            w: 0,
            h: 8,
            rect: true,
            color: 0xff00d9ff,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
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
      };
    }
    _init() {
      this._index = 0;
      this._columnIndex = 0;
      this._focusOnButton = false;
      this._isRunning = false;
      this._fireboltAPI = new FireboltAPI();
      this._categories = AppSettings$1.debug.mockEnabled ? categories : categories.filter(c => c.id !== 'mockstress');
      this._totalColumns = 0;
      this._scrollOffset = 0;
      this._createListAsync();
    }
    _isCategoryInteractive(category) {
      if (!category) {
        return false;
      }
      const isDummyById = /^test\d+$/i.test(category.id || '');
      const isDummyByName = /^test\s+category/i.test((category.name || '').trim());
      return !(isDummyById || isDummyByName);
    }
    _setup() {
      const {
        menu
      } = AppSettings$1.components;
      const {
        width: contentW,
        maxH
      } = AppSettings$1.contentArea;

      // List container fills available height, bounded above the Legends panel
      const listW = contentW;
      const buttonH = menu.buttonHeight;
      const progressAreaH = 40; // 8px bar + 20px text + 12px gap
      const buttonY = maxH - progressAreaH - buttonH - 10;
      const listH = buttonY - 10;
      this.tag('ListContainer').patch({
        w: listW,
        h: listH
      });
      const r = AppSettings$1.radii.container;
      this.tag('RunAllButton').patch({
        y: buttonY,
        w: menu.buttonWidth,
        h: buttonH
      });
      this.tag('RunAllButton.Label').patch({
        x: menu.buttonWidth / 2,
        y: buttonH / 2
      });
      const {
        colors
      } = AppSettings$1;
      const {
        top: btnTop,
        bottom: btnBottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('RunAllButton').patch({
        colorTop: btnTop,
        colorBottom: btnBottom
      });
      this.tag('RunAllButton.PressedOverlay').patch({
        w: menu.buttonWidth,
        h: buttonH,
        color: parseInt(colors.bevelPressedOverlay, 16)
      });
      this.tag('RunAllButton.GlowRing').patch({
        w: menu.buttonWidth,
        h: buttonH,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          stroke: 3,
          strokeColor: parseInt(colors.focusGlowColor, 16),
          radius: r
        }
      });
      this.tag('ProgressBar').patch({
        y: buttonY + buttonH + 10,
        ProgressBg: {
          w: listW
        }
      });

      // store for reuse in _runAllTests, _createListAsync, navigation
      this._listW = listW;
      this._listH = listH;
      this._buttonY = buttonY;
      this._columnWidth = menu.itemWidth + 20;
      this._maxRows = Math.max(1, Math.floor((listH - 20) / menu.itemSpacing));
    }
    _createListAsync() {
      const spacing = AppSettings$1.components.menu.itemSpacing;
      const maxRows = this._maxRows || 10;
      const columnWidth = this._columnWidth || AppSettings$1.components.menu.itemWidth + 20;

      // Calculate total columns needed
      const totalColumns = Math.ceil(this._categories.length / maxRows);
      this._totalColumns = totalColumns;
      const columns = [];
      for (let col = 0; col < totalColumns; col++) {
        const columnItems = [];
        for (let row = 0; row < maxRows; row++) {
          const index = col * maxRows + row;
          if (index >= this._categories.length) break;
          const category = this._categories[index];
          columnItems.push({
            ref: "Item".concat(col, "_").concat(row),
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
          });
        }
        columns.push(...columnItems);
      }
      this.tag('ListContainer.ColumnContainer').children = columns;
    }
    _active() {
      this._focusOnButton = false;
      this._isRunning = false;
      const {
        colors: _ac
      } = AppSettings$1;
      const {
        top: _acTop,
        bottom: _acBottom
      } = AppSettings$1.embossColors(parseInt(_ac.cardBackground, 16));
      this.tag('RunAllButton').patch({
        colorTop: _acTop,
        colorBottom: _acBottom,
        Label: {
          text: {
            textColor: parseInt(_ac.textPrimary, 16)
          }
        },
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: 8
        }
      });
      this.tag('ProgressBar').visible = false;
      this.tag('ProgressBar.ProgressFill').w = 0;
      this._resetCategoryProgress();

      // Restore focus to last selected item if available
      if (typeof this._lastColumnIndex === 'number' && typeof this._lastIndex === 'number') {
        this._columnIndex = this._lastColumnIndex;
        this._index = this._lastIndex;
      } else {
        this._columnIndex = 0;
        this._index = 0;
      }
      this._focusItem(this._columnIndex, this._index);
    }
    _handleUp() {
      if (this._isRunning) return;
      if (this._focusOnButton) {
        this._focusOnButton = false;
        const {
          colors: _uc
        } = AppSettings$1;
        const {
          top: _ucTop,
          bottom: _ucBottom
        } = AppSettings$1.embossColors(parseInt(_uc.cardBackground, 16));
        this.tag('RunAllButton').patch({
          colorTop: _ucTop,
          colorBottom: _ucBottom,
          Label: {
            text: {
              textColor: parseInt(_uc.textPrimary, 16)
            }
          },
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            radius: 8
          }
        });
        this.tag('RunAllButton.GlowRing').patch({
          smooth: {
            alpha: [0, {
              duration: 0.2
            }]
          }
        });
        // Move to last item in current column
        const maxRows = this._maxRows || 10;
        const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows);
        this._index = colLength - 1;
        this._focusItem(this._columnIndex, this._index);
      } else if (this._index > 0) {
        this._unfocusItem(this._columnIndex, this._index);
        this._index--;
        this._focusItem(this._columnIndex, this._index);
      }
    }
    _handleDown() {
      if (this._isRunning) return;
      const maxRows = this._maxRows || 10;
      const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows);
      const canMoveToButton = this._columnIndex === 0 && this._index === colLength - 1;
      if (this._index < colLength - 1) {
        this._unfocusItem(this._columnIndex, this._index);
        this._index++;
        this._focusItem(this._columnIndex, this._index);
      } else if (canMoveToButton && !this._focusOnButton) {
        this._unfocusItem(this._columnIndex, this._index);
        this._focusOnButton = true;
        const {
          colors: _dc
        } = AppSettings$1;
        const {
          top: _dcTop,
          bottom: _dcBottom
        } = AppSettings$1.embossColors(parseInt(_dc.primary, 16));
        this.tag('RunAllButton').patch({
          colorTop: _dcTop,
          colorBottom: _dcBottom,
          Label: {
            text: {
              textColor: parseInt(_dc.background, 16)
            }
          },
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            radius: 8
          }
        });
        this.tag('RunAllButton.GlowRing').patch({
          smooth: {
            alpha: [1, {
              duration: 0.2
            }]
          }
        });
      }
    }
    _handleLeft() {
      if (this._isRunning || this._focusOnButton) return;
      if (this._columnIndex > 0) {
        this._unfocusItem(this._columnIndex, this._index);
        this._columnIndex--;
        const maxRows = this._maxRows || 10;
        const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows);
        if (this._index > colLength - 1) this._index = colLength - 1;
        this._focusItem(this._columnIndex, this._index);
        this._scrollToColumn();
      }
    }
    _handleRight() {
      if (this._isRunning || this._focusOnButton) return;
      if (this._columnIndex < this._totalColumns - 1) {
        this._unfocusItem(this._columnIndex, this._index);
        this._columnIndex++;
        const maxRows = this._maxRows || 10;
        const colLength = Math.min(maxRows, this._categories.length - this._columnIndex * maxRows);
        if (this._index > colLength - 1) this._index = colLength - 1;
        this._focusItem(this._columnIndex, this._index);
        this._scrollToColumn();
      }
    }
    _handleEnter() {
      if (this._isRunning) return;
      if (this._focusOnButton) {
        if (this.tag('ProgressBar').visible) {
          this.tag('ProgressBar').visible = false;
          this.tag('ProgressBar.ProgressFill').w = 0;
          this._resetCategoryProgress();
        } else {
          // Guard immediately so rapid re-presses are blocked during animation
          this._isRunning = true;
          this._setButtonRunningState(true);
          this._pressButtonAnimation(() => this._runAllTests());
        }
      } else {
        this._lastColumnIndex = this._columnIndex;
        this._lastIndex = this._index;
        const globalIndex = this._columnIndex * (this._maxRows || 10) + this._index;
        const selectedCategory = this._categories[globalIndex];
        if (!this._isCategoryInteractive(selectedCategory)) {
          return;
        }
        const ref = "Item".concat(this._columnIndex, "_").concat(this._index);
        const listItem = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref);
        const catItem = listItem && listItem.tag('CategoryItem');
        if (catItem) {
          catItem.press().then(() => this.signal('onSelect', selectedCategory));
        } else {
          this.signal('onSelect', selectedCategory);
        }
      }
    }
    _setButtonRunningState(isRunning) {
      const {
        colors
      } = AppSettings$1;
      const btn = this.tag('RunAllButton');
      if (isRunning) {
        const {
          top,
          bottom
        } = AppSettings$1.embossColors(parseInt(colors.focusedBackground, 16));
        btn.patch({
          colorTop: top,
          colorBottom: bottom,
          Label: {
            text: {
              text: 'Running...',
              textColor: parseInt(colors.textSecondary, 16)
            }
          }
        });
      } else {
        const {
          top,
          bottom
        } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
        btn.patch({
          colorTop: top,
          colorBottom: bottom,
          Label: {
            text: {
              text: 'Run All Tests',
              textColor: parseInt(colors.textPrimary, 16)
            }
          }
        });
      }
    }
    _pressButtonAnimation(callback) {
      const btn = this.tag('RunAllButton');
      btn.patch({
        smooth: {
          scale: [0.97, {
            duration: 0.08,
            timingFunction: 'ease-in'
          }]
        }
      });
      btn.tag('PressedOverlay').patch({
        smooth: {
          alpha: [1, {
            duration: 0.08
          }]
        }
      });
      setTimeout(() => {
        btn.patch({
          smooth: {
            scale: [1, {
              duration: 0.15,
              timingFunction: 'ease-out'
            }]
          }
        });
        btn.tag('PressedOverlay').patch({
          smooth: {
            alpha: [0, {
              duration: 0.15
            }]
          }
        });
        if (callback) callback();
      }, 100);
    }
    _focusItem(columnIndex, index) {
      const ref = "Item".concat(columnIndex, "_").concat(index);
      const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref);
      if (item && item.tag('CategoryItem')) {
        item.tag('CategoryItem').setFocus(true);
      }
    }
    _unfocusItem(columnIndex, index) {
      const ref = "Item".concat(columnIndex, "_").concat(index);
      const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref);
      if (item && item.tag('CategoryItem')) {
        item.tag('CategoryItem').setFocus(false);
      }
    }
    _scrollToColumn() {
      const columnWidth = this._columnWidth || AppSettings$1.components.menu.itemWidth + 20;
      const visibleColumns = 3;
      let scrollX = 0;
      if (this._columnIndex >= visibleColumns) {
        scrollX = (this._columnIndex - visibleColumns + 1) * columnWidth;
      }
      this.tag('ListContainer.ColumnContainer').x = 25 - scrollX;
    }
    async _runAllTests() {
      this.tag('ProgressBar').visible = true;
      const startTime = Date.now();
      const {
        categoryBatchSize,
        testBatchSize
      } = AppSettings$1.execution;
      const runnableCategories = this._categories.filter(category => this._isCategoryInteractive(category) && !category.runAllExcluded);
      const totalCategories = runnableCategories.length;
      const categoryResults = [];
      if (totalCategories === 0) {
        this.tag('ProgressBar.ProgressFill').w = 0;
        this.tag('ProgressBar.ProgressText').text.text = 'No runnable categories available.';
        this._isRunning = false;
        this._setButtonRunningState(false);
        return;
      }
      let completedCount = 0;
      // Process categories sequentially to avoid races on shared FireboltAPI instance state
      const processCategoryBatch = async categories => {
        const results = [];
        for (const {
          category,
          columnIndex,
          rowIndex
        } of categories) {
          this._setCategoryProgress(columnIndex, rowIndex, 'running');
          const tests = await this._fireboltAPI.getTestsForCategory(category.id);
          const runnableTests = tests.filter(t => t.type !== 'event' && t.type !== 'gateway-event');
          let passedTests = 0;
          const totalTests = runnableTests.length;

          // Process tests sequentially within the category
          const processTestBatch = async testBatch => {
            let batchPassedTests = 0;
            for (const test of testBatch) {
              try {
                const result = await this._fireboltAPI.runTest(test);
                this.signal('onStatus', {
                  isResponse: true,
                  method: test.method,
                  name: test.name,
                  response: result.message,
                  success: result.success
                });
                batchPassedTests += result.success ? 1 : 0;
              } catch (error) {
                this.signal('onStatus', {
                  isResponse: true,
                  method: test.method || '?',
                  name: test.name || '?',
                  response: "Error: ".concat(error.message),
                  success: false
                });
              }
            }
            return batchPassedTests;
          };

          // Split tests into batches
          for (let i = 0; i < runnableTests.length; i += testBatchSize) {
            const batch = runnableTests.slice(i, i + testBatchSize);
            passedTests += await processTestBatch(batch);
            await new Promise(resolve => setTimeout(resolve, 50));
          }

          // Calculate pass rate
          const passRate = totalTests > 0 ? Math.round(passedTests / totalTests * 100) : 0;
          this._setCategoryProgress(columnIndex, rowIndex, 'complete', passRate);
          completedCount++;
          const progress = completedCount / totalCategories * 100;
          this.tag('ProgressBar.ProgressFill').setSmooth('w', this._listW * progress / 100, {
            duration: 0.3
          });
          this.tag('ProgressBar.ProgressText').text.text = "Running tests... (".concat(completedCount, "/").concat(totalCategories, " categories)");
          results.push({
            category: category.name,
            passRate,
            passed: passedTests,
            total: totalTests
          });
        }
        return results;
      };
      if (AppSettings$1.debug.verbose) console.log("[RunAll] _maxRows=".concat(this._maxRows, ", runnableCategories=").concat(runnableCategories.map(c => c.id).join(', ')));
      try {
        // Split categories into parallel batches
        for (let i = 0; i < runnableCategories.length; i += categoryBatchSize) {
          const maxRows = this._maxRows || 10;
          const batch = runnableCategories.slice(i, i + categoryBatchSize).map(category => {
            const actualIndex = this._categories.findIndex(c => c.id === category.id);
            const columnIndex = Math.floor(actualIndex / maxRows);
            const rowIndex = actualIndex % maxRows;
            if (AppSettings$1.debug.verbose) console.log("[RunAll] category=".concat(category.id, " actualIndex=").concat(actualIndex, " -> ref=Item").concat(columnIndex, "_").concat(rowIndex));
            return {
              category,
              columnIndex,
              rowIndex
            };
          });
          const batchResults = await processCategoryBatch(batch);
          categoryResults.push(...batchResults);
        }

        // All tests complete
        const endTime = Date.now();
        const totalTime = ((endTime - startTime) / 1000).toFixed(2);
        const totalPassed = categoryResults.reduce((sum, r) => sum + r.passed, 0);
        const totalTests = categoryResults.reduce((sum, r) => sum + r.total, 0);
        const summaryText = totalTests === 0 ? "All tests completed in ".concat(totalTime, "s! ").concat(totalPassed, "/").concat(totalTests, " passed (no tests executed)") : "All tests completed in ".concat(totalTime, "s! ").concat(totalPassed, "/").concat(totalTests, " passed (").concat(Math.round(totalPassed / totalTests * 100), "%)");
        this.tag('ProgressBar.ProgressText').patch({
          text: {
            text: summaryText,
            fontStyle: 'bold'
          }
        });
      } finally {
        this._isRunning = false;
        this._setButtonRunningState(false);
      }
    }
    _setCategoryProgress(columnIndex, rowIndex, state, passRate) {
      const ref = "Item".concat(columnIndex, "_").concat(rowIndex);
      const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref);
      if (item && item.tag('CategoryItem')) {
        const {
          colors
        } = AppSettings$1;
        let color;
        if (state === 'running') {
          color = parseInt(colors.categoryRunningBackground, 16);
        } else if (state === 'complete' && passRate !== null && passRate !== undefined) {
          const gradientStep = AppSettings$1.components.menu.passRateGradientStep || 10;
          const steppedPassRate = Math.round(passRate / gradientStep) * gradientStep;
          const ratio = steppedPassRate / 100;
          if (ratio <= 0.33) {
            const colorRed = parseInt(colors.passRateColorDarkRed, 16);
            const colorOrangeRed = parseInt(colors.passRateColorOrangeRed, 16);
            const localRatio = ratio / 0.33;
            color = Lightning$1.StageUtils.mergeColors(colorOrangeRed, colorRed, localRatio);
          } else if (ratio <= 0.66) {
            const colorOrangeRed = parseInt(colors.passRateColorOrangeRed, 16);
            const colorOrange = parseInt(colors.passRateColorOrange, 16);
            const localRatio = (ratio - 0.33) / 0.33;
            color = Lightning$1.StageUtils.mergeColors(colorOrange, colorOrangeRed, localRatio);
          } else {
            const colorOrange = parseInt(colors.passRateColorOrange, 16);
            const colorGreen = parseInt(colors.passRateColorDarkGreen, 16);
            const localRatio = (ratio - 0.66) / 0.34;
            color = Lightning$1.StageUtils.mergeColors(colorGreen, colorOrange, localRatio);
          }
        } else {
          color = parseInt(colors.cardBackground, 16);
        }
        const {
          top: ec_top,
          bottom: ec_bottom
        } = AppSettings$1.embossColors(color);
        item.tag('CategoryItem.Background').patch({
          colorTop: ec_top,
          colorBottom: ec_bottom,
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            radius: 8
          }
        });
        if (state === 'complete') {
          item.tag('CategoryItem').setTestResult(color);
        }
        if (passRate !== null && passRate !== undefined && state === 'complete') {
          const cat = this._categories[columnIndex * (this._maxRows || 10) + rowIndex];
          const baseName = cat ? cat.name : '';
          item.tag('CategoryItem.Name').text.text = "".concat(baseName, " (").concat(passRate, "%)");
        }
      }
    }
    _resetCategoryProgress() {
      const maxRows = this._maxRows || 10;
      for (let i = 0; i < this._categories.length; i++) {
        const columnIndex = Math.floor(i / maxRows);
        const rowIndex = i % maxRows;
        const ref = "Item".concat(columnIndex, "_").concat(rowIndex);
        const item = this.tag('ListContainer.ColumnContainer').childList.getByRef(ref);
        if (item && item.tag('CategoryItem')) {
          item.tag('CategoryItem').clearTestResult();
          const {
            colors
          } = AppSettings$1;
          const {
            top: rcp_top,
            bottom: rcp_bottom
          } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
          item.tag('CategoryItem.Background').patch({
            colorTop: rcp_top,
            colorBottom: rcp_bottom,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
              radius: 8
            }
          });
          item.tag('CategoryItem.Name').patch({
            text: {
              textColor: parseInt(colors.textPrimary, 16)
            }
          });
          item.tag('CategoryItem.Name').text.text = this._categories[i] ? this._categories[i].name : '';
        }
      }
    }
    _getFocused() {
      return this;
    }
  }
  class CategoryItem extends Lightning$1.Component {
    _construct() {
      // Build template dynamically to get scaled values
      const {
        menu
      } = AppSettings$1.components;
      const {
        colors,
        typography
      } = AppSettings$1;
      this._itemWidth = menu.itemWidth;
      this._itemHeight = menu.itemHeight;
      this._colors = colors;
      this._typography = typography;
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
            type: Lightning$1.shaders.RoundedRectangle,
            radius: 8
          }
        },
        HatchOverlay: {
          x: 0,
          y: 0,
          w: 400,
          h: 50,
          visible: false,
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            radius: 8
          }
        },
        Name: {
          x: 20,
          text: {
            fontSize: 24,
            textColor: 0xffffffff
          }
        },
        PressedOverlay: {
          x: 0,
          y: 0,
          w: 400,
          h: 50,
          rect: true,
          color: 0x55000000,
          alpha: 0,
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            radius: 8
          }
        },
        Border: {
          w: 400,
          h: 50,
          rect: true,
          color: 0x00000000,
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            stroke: 4,
            strokeColor: 0xff00d9ff,
            radius: 8
          },
          alpha: 0
        },
        GlowRing: {
          x: 0,
          y: 0,
          w: 400,
          h: 50,
          rect: true,
          color: 0x00000000,
          alpha: 0,
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            stroke: 3,
            strokeColor: 0xff00d9ff,
            radius: 8
          }
        }
      };
    }
    _setup() {
      // Apply scaled values after template is built
      const {
        menu
      } = AppSettings$1.components;
      const {
        colors,
        typography
      } = AppSettings$1;
      const itemH = menu.itemHeight;
      const startY = Math.round((itemH - typography.subtitle.fontSize) / 2) - 3;
      this.patch({
        w: menu.itemWidth,
        h: itemH
      });
      const {
        top: bgTop,
        bottom: bgBottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('Background').patch({
        w: menu.itemWidth,
        h: itemH,
        colorTop: bgTop,
        colorBottom: bgBottom,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: 8
        }
      });
      this.tag('HatchOverlay').patch({
        w: menu.itemWidth,
        h: itemH
      });
      this.tag('Border').patch({
        w: menu.itemWidth,
        h: itemH,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          stroke: 3,
          strokeColor: parseInt(colors.primary, 16),
          radius: 8
        }
      });
      this.tag('Name').patch({
        y: startY,
        text: {
          fontSize: typography.subtitle.fontSize,
          textColor: parseInt(colors.textPrimary, 16)
        }
      });
      const r = AppSettings$1.radii.container;
      this.tag('PressedOverlay').patch({
        w: menu.itemWidth,
        h: itemH,
        color: parseInt(colors.bevelPressedOverlay, 16)
      });
      this.tag('GlowRing').patch({
        w: menu.itemWidth,
        h: itemH,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          stroke: 3,
          strokeColor: parseInt(colors.focusGlowColor, 16),
          radius: r
        }
      });
    }
    set category(value) {
      this._category = value;
      this._isDisabled = !!value.isDisabled;
      this._isRunAllExcluded = !!value.isRunAllExcluded;
      this._hasTestResult = false;
      this.tag('Name').text.text = value.name;
      if (this._isDisabled) {
        this.patch({
          alpha: 0.55
        });
        this.tag('HatchOverlay').visible = false;
      } else if (this._isRunAllExcluded) {
        this.patch({
          alpha: 1
        });
        const w = this._itemWidth;
        const h = this._itemHeight;
        this.tag('HatchOverlay').patch({
          visible: true,
          texture: {
            type: Lightning$1.textures.StaticCanvasTexture,
            content: {
              w,
              h,
              draw: (ctx, canvas) => {
                // Dark translucent base so the card reads as muted
                ctx.fillStyle = 'rgba(0,0,0,0.45)';
                ctx.fillRect(0, 0, canvas.width, canvas.height);
                // Bold diagonal stripes
                ctx.strokeStyle = 'rgba(255,255,255,0.35)';
                ctx.lineWidth = 2.5;
                const step = 10;
                for (let x = -canvas.height; x < canvas.width + canvas.height; x += step) {
                  ctx.beginPath();
                  ctx.moveTo(x, 0);
                  ctx.lineTo(x + canvas.height, canvas.height);
                  ctx.stroke();
                }
              }
            }
          }
        });
      } else {
        this.patch({
          alpha: 1
        });
        this.tag('HatchOverlay').visible = false;
      }
    }
    set isFocused(value) {
      this.setFocus(value);
    }
    setTestResult(color) {
      this._hasTestResult = true;
      this._testResultColor = color;
    }
    clearTestResult() {
      this._hasTestResult = false;
      this._testResultColor = null;
    }
    setFocus(focused) {
      const {
        colors
      } = AppSettings$1;
      const canFocus = focused && !this._isDisabled;

      // Run-all-excluded items (e.g. Lifecycle): fade out the hatch overlay on focus so
      // the card visually "opens up" and the user can clearly see it is selected/active.
      if (this._isRunAllExcluded) {
        this.tag('HatchOverlay').patch({
          smooth: {
            alpha: [focused ? 0 : 1, {
              duration: 0.2
            }]
          }
        });
      }

      // Show teal border + glow if focused, else hide
      this.tag('Border').patch({
        smooth: {
          alpha: [canFocus ? 1 : 0, {
            duration: 0.2
          }]
        }
      });
      this.tag('GlowRing').patch({
        smooth: {
          alpha: [canFocus ? 1 : 0, {
            duration: 0.2
          }]
        }
      });
      if (this._isDisabled) {
        this.tag('Name').patch({
          text: {
            textColor: parseInt(colors.textSecondary, 16)
          }
        });
        return;
      }
      if (!this._hasTestResult) {
        const {
          top: bgTop,
          bottom: bgBottom
        } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
        this.tag('Background').patch({
          smooth: {
            colorTop: [bgTop, {
              duration: 0.2
            }],
            colorBottom: [bgBottom, {
              duration: 0.2
            }]
          }
        });
        this.tag('Name').patch({
          text: {
            textColor: parseInt(colors.textPrimary, 16)
          }
        });
      } else {
        this.tag('Name').patch({
          text: {
            textColor: parseInt(colors.textPrimary, 16)
          }
        });
      }
    }
    press() {
      return new Promise(resolve => {
        this.patch({
          smooth: {
            scale: [0.96, {
              duration: 0.08,
              timingFunction: 'ease-in'
            }]
          }
        });
        this.tag('PressedOverlay').patch({
          smooth: {
            alpha: [1, {
              duration: 0.08
            }]
          }
        });
        setTimeout(() => {
          this.patch({
            smooth: {
              scale: [1, {
                duration: 0.15,
                timingFunction: 'ease-out'
              }]
            }
          });
          this.tag('PressedOverlay').patch({
            smooth: {
              alpha: [0, {
                duration: 0.15
              }]
            }
          });
          setTimeout(resolve, 150);
        }, 100);
      });
    }
  }

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
  class TestRunner extends Lightning$1.Component {
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
              type: Lightning$1.shaders.RoundedRectangle,
              radius: 4
            }
          },
          ProgressFill: {
            w: 0,
            h: 8,
            rect: true,
            color: 0xff2a3f5f,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
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
            type: Lightning$1.shaders.RoundedRectangle,
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
            x: 0,
            y: 0,
            w: 200,
            h: 50,
            rect: true,
            color: 0x55000000,
            alpha: 0,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
              radius: 8
            }
          },
          GlowRing: {
            x: 0,
            y: 0,
            w: 200,
            h: 50,
            rect: true,
            color: 0x00000000,
            alpha: 0,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
              stroke: 3,
              strokeColor: 0xff00d9ff,
              radius: 8
            }
          }
        }
      };
    }
    _setup() {
      const {
        testRunner
      } = AppSettings$1.components;
      const {
        colors,
        typography
      } = AppSettings$1;
      const {
        width: contentW,
        maxH
      } = AppSettings$1.contentArea;

      // Header: title (left) + description (right, same row) + progress bar below
      const titleW = Math.floor(contentW * 0.48);
      const descX = titleW + 24;
      const descW = contentW - descX;
      const descY = Math.round((typography.heading.fontSize - typography.bodySmall.fontSize) / 2);
      const progressBarY = typography.heading.fontSize + 14;
      // headerH = top of ProgressText + ProgressText height + gap
      const headerH = progressBarY + 20 + typography.bodySmall.fontSize + 12;
      const buttonH = testRunner.buttonHeight;
      const bottomH = buttonH + 20;
      const containerH = maxH - headerH - bottomH;
      const buttonY = headerH + containerH + 10;

      // Store for use in run methods
      this._containerW = contentW;
      this._containerH = containerH;
      this._buttonY = buttonY;
      this._columnWidth = Math.floor(contentW / 2) - 10;
      this.tag('CategoryTitle').patch({
        text: {
          fontSize: typography.heading.fontSize,
          textColor: parseInt(colors.primary, 16),
          wordWrapWidth: titleW
        }
      });
      this.tag('CategoryDescription').patch({
        x: descX,
        y: descY,
        text: {
          fontSize: typography.bodySmall.fontSize,
          textColor: parseInt(colors.textTertiary, 16),
          wordWrapWidth: descW
        }
      });
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
      });
      this.tag('TestListContainer').patch({
        y: headerH,
        w: contentW,
        h: containerH
      });
      const {
        top: btnTop,
        bottom: btnBottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('RunButton').patch({
        y: buttonY,
        w: testRunner.buttonWidth,
        h: buttonH,
        colorTop: btnTop,
        colorBottom: btnBottom,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: 8
        }
      });
      this.tag('RunButton.Label').patch({
        x: testRunner.buttonWidth / 2,
        y: buttonH / 2,
        text: {
          fontSize: typography.subtitle.fontSize,
          textColor: parseInt(colors.textPrimary, 16)
        }
      });
      this.tag('RunButton.PressedOverlay').patch({
        w: testRunner.buttonWidth,
        h: buttonH,
        color: parseInt(colors.bevelPressedOverlay, 16)
      });
      this.tag('RunButton.GlowRing').patch({
        w: testRunner.buttonWidth,
        h: buttonH,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          stroke: 3,
          strokeColor: parseInt(colors.focusGlowColor, 16),
          radius: 8
        }
      });
    }
    _init() {
      this._tests = [];
      this._selectedTestIndex = 0;
      this._selectedColumn = 0;
      this._selectedRow = 0;
      this._fireboltAPI = new FireboltAPI();
      this._focusOnButton = false;
      this._isRunning = false;
      this._navigationTimeout = null;
      // NOTE: _containerW, _containerH, _buttonY, _columnWidth, _maxRows are
      // computed in _setup() which runs before _init() — do not reset them here.
    }
    _active() {
      this._focusOnButton = false;
      this._isRunning = false;
      this._selectedTestIndex = 0;
      this._selectedColumn = 0;
      this._selectedRow = 0;
      this._unfocusButton();
      if (this._tests.length > 0) {
        this._focusItem(0);
      }
    }
    loadTests(category) {
      this._category = category;
      this._focusOnButton = false;
      this._selectedTestIndex = 0;
      this._isRunning = false;
      if (this._navigationTimeout) {
        clearTimeout(this._navigationTimeout);
        this._navigationTimeout = null;
      }

      // Reset progress bar to grey (inactive state)
      const {
        colors
      } = AppSettings$1;
      this.tag('ProgressBar.ProgressFill').patch({
        w: 0,
        color: parseInt(colors.focusedBackground, 16)
      });
      this.tag('ProgressBar.ProgressText').text.text = '';
      this.tag('CategoryTitle').text.text = "".concat(category.name, " APIs");
      this.tag('CategoryDescription').text.text = category.description || '';
      this.signal('onStatus', 'Loading tests...');
      this._tests = [];
      this._createTestList();
      this._fireboltAPI.getTestsForCategory(category.id).then(tests => {
        this._tests = tests.filter(t => t.type !== 'event' && t.type !== 'gateway-event');
        this._createTestList();
        this.signal('onStatus', "".concat(this._tests.length, " tests available"));
        if (this._tests.length > 0) {
          this._selectedTestIndex = 0;
          this._focusItem(0);
        }
      }).catch(error => {
        console.error('Error loading tests:' + JSON.stringify(error));
        this._tests = [];
        this._createTestList();
        this.signal('onStatus', 'No tests available');
      });
      this._clearAllResults();
    }
    _createTestList() {
      const spacing = AppSettings$1.components.testRunner.testItemSpacing;
      const columnWidth = this._columnWidth || 740;
      this._maxRows = Math.max(1, Math.floor((this._containerH - 20) / spacing));
      const items = [];
      for (let i = 0; i < this._tests.length; i++) {
        const col = Math.floor(i / this._maxRows);
        const row = i % this._maxRows;
        items.push({
          ref: "Test".concat(i),
          x: col * columnWidth,
          y: row * spacing,
          TestItem: {
            type: TestItem,
            test: this._tests[i],
            isFocused: false
          }
        });
      }
      this.tag('TestListContainer.TestListContent').children = items;
      this._selectedColumn = 0;
      this._selectedRow = 0;
      this._updateScroll();
      if (this._tests.length > 0) {
        this._selectedTestIndex = 0;
        this._focusItem(0);
      }
    }
    _handleUp() {
      if (this._focusOnButton) {
        this._focusOnButton = false;
        this._unfocusButton();
        const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows);
        this._selectedRow = colLen - 1;
        this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow;
        this._focusItem(this._selectedTestIndex);
      } else if (this._selectedRow > 0) {
        this._unfocusItem(this._selectedTestIndex);
        this._selectedRow--;
        this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow;
        this._focusItem(this._selectedTestIndex);
      }
    }
    _handleDown() {
      if (this._focusOnButton) return;
      const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows);
      if (this._selectedRow < colLen - 1) {
        this._unfocusItem(this._selectedTestIndex);
        this._selectedRow++;
        this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow;
        this._focusItem(this._selectedTestIndex);
      } else if (this._selectedColumn === 0) {
        this._unfocusItem(this._selectedTestIndex);
        this._focusOnButton = true;
        this._focusButton();
      }
    }
    _handleLeft() {
      if (this._isRunning || this._focusOnButton) return;
      if (this._selectedColumn > 0) {
        this._unfocusItem(this._selectedTestIndex);
        this._selectedColumn--;
        const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows);
        if (this._selectedRow > colLen - 1) this._selectedRow = colLen - 1;
        this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow;
        this._focusItem(this._selectedTestIndex);
        this._updateScroll();
      }
    }
    _handleRight() {
      if (this._isRunning || this._focusOnButton) return;
      const totalColumns = Math.ceil(this._tests.length / this._maxRows);
      if (this._selectedColumn < totalColumns - 1) {
        this._unfocusItem(this._selectedTestIndex);
        this._selectedColumn++;
        const colLen = Math.min(this._maxRows, this._tests.length - this._selectedColumn * this._maxRows);
        if (this._selectedRow > colLen - 1) this._selectedRow = colLen - 1;
        this._selectedTestIndex = this._selectedColumn * this._maxRows + this._selectedRow;
        this._focusItem(this._selectedTestIndex);
        this._updateScroll();
      }
    }
    _updateScroll() {
      const visibleColumns = 2;
      const columnWidth = this._columnWidth || 740;
      let contentX = 10;
      if (this._selectedColumn >= visibleColumns) {
        contentX = 10 - (this._selectedColumn - visibleColumns + 1) * columnWidth;
      }
      this.tag('TestListContainer.TestListContent').setSmooth('x', contentX, {
        duration: 0.3
      });
    }
    _handleEnter() {
      if (this._isRunning) return;
      if (this._focusOnButton) {
        this._pressButtonAnimation(() => this._runAllTests());
      } else {
        this._runSingleTest(this._selectedTestIndex);
      }
    }
    async _runSingleTest(index) {
      if (this._isRunning) return;
      this._isRunning = true;
      const test = this._tests[index];
      const {
        colors
      } = AppSettings$1;
      this.tag('ProgressBar.ProgressFill').patch({
        color: parseInt(colors.primary, 16),
        w: 0
      });
      this.tag('ProgressBar.ProgressText').text.text = "Running: ".concat(test.name, "...");
      this.signal('onStatus', "Running: ".concat(test.name, "..."));
      const result = await this._fireboltAPI.runTest(test);

      // Fill progress bar only after result is received
      this.tag('ProgressBar.ProgressFill').patch({
        smooth: {
          w: [this._containerW, {
            duration: 0.3
          }]
        }
      });
      const child = this.tag('TestListContainer.TestListContent').children[index];
      if (child) child.tag('TestItem').updateResult(result);
      this.signal('onStatus', {
        isResponse: true,
        method: test.method,
        name: test.name,
        response: result.message,
        success: result.success
      });
      this.tag('ProgressBar.ProgressText').text.text = result.success ? "\u25CF Passed" : "\u25CF Failed";
      this._isRunning = false;

      // Reset progress bar after 2s
      setTimeout(() => {
        this.tag('ProgressBar.ProgressFill').patch({
          smooth: {
            w: [0, {
              duration: 0.3
            }]
          },
          color: parseInt(colors.focusedBackground, 16)
        });
        this.tag('ProgressBar.ProgressText').text.text = '';
      }, 2000);
    }
    async _runAllTests() {
      if (this._isRunning) return;
      this._isRunning = true;
      const {
        colors
      } = AppSettings$1;

      // Visually disable button during execution
      const {
        top: runTop,
        bottom: runBottom
      } = AppSettings$1.embossColors(parseInt(colors.focusedBackground, 16));
      this.tag('RunButton').patch({
        colorTop: runTop,
        colorBottom: runBottom,
        Label: {
          text: {
            text: 'Running...',
            textColor: parseInt(colors.textSecondary, 16)
          }
        }
      });

      // Change progress bar to cyan when active
      this.tag('ProgressBar.ProgressFill').patch({
        color: parseInt(colors.primary, 16)
      });
      this.signal('onStatus', 'Running all tests...');
      const results = [];
      const totalTests = this._tests.length;
      const progressBarWidth = this._containerW;
      for (let i = 0; i < totalTests; i++) {
        const test = this._tests[i];
        const progress = (i + 1) / totalTests * 100;
        this.signal('onStatus', "Running ".concat(i + 1, "/").concat(totalTests, ": ").concat(test.name, "..."));

        // Scroll to keep the running column visible
        const runningCol = Math.floor(i / this._maxRows);
        if (runningCol !== this._selectedColumn) {
          this._selectedColumn = runningCol;
          this._updateScroll();
        }
        const result = await this._fireboltAPI.runTest(test);
        results.push(result);

        // Update progress bar only after result is received
        this.tag('ProgressBar.ProgressFill').patch({
          smooth: {
            w: [progressBarWidth * (i + 1) / totalTests, {
              duration: 0.3
            }]
          }
        });
        this.tag('ProgressBar.ProgressText').text.text = "".concat(Math.round(progress), "%");

        // Child index matches loop index since this._tests has no event tests
        const child = this.tag('TestListContainer.TestListContent').children[i];
        if (child) child.tag('TestItem').updateResult(result);
        this.signal('onStatus', {
          isResponse: true,
          method: test.method,
          name: test.name,
          response: result.message,
          success: result.success
        });
      }
      this._isRunning = false;
      const passed = results.filter(r => r.success).length;
      this.signal('onStatus', "Complete: ".concat(passed, "/").concat(totalTests, " tests passed"));

      // Re-enable button
      const {
        top: doneTop,
        bottom: doneBottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('RunButton').patch({
        colorTop: doneTop,
        colorBottom: doneBottom,
        Label: {
          text: {
            text: 'Run All Tests',
            textColor: parseInt(colors.textPrimary, 16)
          }
        }
      });

      // Only navigate to results if component is still visible
      this._navigationTimeout = setTimeout(() => {
        // Reset progress bar to grey after completion
        this.tag('ProgressBar.ProgressFill').patch({
          w: 0,
          color: parseInt(colors.focusedBackground, 16)
        });
        this.tag('ProgressBar.ProgressText').text.text = '';
        if (this.visible) {
          this.signal('onTestComplete', {
            category: this._category,
            results
          });
        }
        this._navigationTimeout = null;
      }, 1000);
    }
    _focusItem(index) {
      var _this$tag$children$in;
      const item = (_this$tag$children$in = this.tag('TestListContainer.TestListContent').children[index]) === null || _this$tag$children$in === void 0 ? void 0 : _this$tag$children$in.tag('TestItem');
      if (item) item.setFocus(true);
    }
    _unfocusItem(index) {
      var _this$tag$children$in2;
      const item = (_this$tag$children$in2 = this.tag('TestListContainer.TestListContent').children[index]) === null || _this$tag$children$in2 === void 0 ? void 0 : _this$tag$children$in2.tag('TestItem');
      if (item) item.setFocus(false);
    }
    _focusButton() {
      const {
        colors
      } = AppSettings$1;
      const {
        top,
        bottom
      } = AppSettings$1.embossColors(parseInt(colors.primary, 16));
      this.tag('RunButton').patch({
        smooth: {
          colorTop: [top, {
            duration: 0.2
          }],
          colorBottom: [bottom, {
            duration: 0.2
          }],
          scale: [1.05, {
            duration: 0.2
          }]
        }
      });
      this.tag('RunButton.GlowRing').patch({
        smooth: {
          alpha: [1, {
            duration: 0.2
          }]
        }
      });
    }
    _unfocusButton() {
      const {
        colors
      } = AppSettings$1;
      const {
        top,
        bottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('RunButton').patch({
        smooth: {
          colorTop: [top, {
            duration: 0.2
          }],
          colorBottom: [bottom, {
            duration: 0.2
          }],
          scale: [1, {
            duration: 0.2
          }]
        }
      });
      this.tag('RunButton.GlowRing').patch({
        smooth: {
          alpha: [0, {
            duration: 0.2
          }]
        }
      });
    }
    _pressButtonAnimation(callback) {
      const btn = this.tag('RunButton');
      btn.patch({
        smooth: {
          scale: [0.97, {
            duration: 0.08,
            timingFunction: 'ease-in'
          }]
        }
      });
      btn.tag('PressedOverlay').patch({
        smooth: {
          alpha: [1, {
            duration: 0.08
          }]
        }
      });
      setTimeout(() => {
        btn.patch({
          smooth: {
            scale: [1.05, {
              duration: 0.15,
              timingFunction: 'ease-out'
            }]
          }
        });
        btn.tag('PressedOverlay').patch({
          smooth: {
            alpha: [0, {
              duration: 0.15
            }]
          }
        });
        if (callback) callback();
      }, 100);
    }
    _clearAllResults() {
      const content = this.tag('TestListContainer.TestListContent');
      if (content && content.children) {
        content.children.forEach(child => {
          const item = child.tag('TestItem');
          if (item && item.clearResult) {
            item.clearResult();
          }
        });
      }
    }
    _inactive() {
      // Cancel any running tests when component becomes inactive
      this._isRunning = false;

      // Clear any pending navigation timeout
      if (this._navigationTimeout) {
        clearTimeout(this._navigationTimeout);
        this._navigationTimeout = null;
      }

      // Reset progress bar to grey
      const {
        colors
      } = AppSettings$1;
      this.tag('ProgressBar.ProgressFill').patch({
        w: 0,
        color: parseInt(colors.focusedBackground, 16)
      });
      this.tag('ProgressBar.ProgressText').text.text = '';
    }
    _getFocused() {
      return this;
    }
  }
  class TestItem extends Lightning$1.Component {
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
          x: 0,
          y: 0,
          w: 580,
          h: 60,
          rect: true,
          color: 0x00000000,
          alpha: 0,
          shader: {
            type: Lightning$1.shaders.RoundedRectangle,
            stroke: 3,
            strokeColor: 0xff00d9ff,
            radius: 6
          }
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
      };
    }
    _setup() {
      const {
        testRunner
      } = AppSettings$1.components;
      const {
        colors,
        typography
      } = AppSettings$1;
      const {
        width: contentW
      } = AppSettings$1.contentArea;
      const itemWidth = Math.floor(contentW / 2) - 20;
      const itemHeight = testRunner.testItemHeight;
      const nameHeight = typography.body.fontSize;
      const methodHeight = typography.caption.fontSize;
      const totalTextHeight = nameHeight + methodHeight + 4; // 4px gap between texts
      const startY = Math.round((itemHeight - totalTextHeight) / 2) - 2;
      this.patch({
        w: itemWidth,
        h: itemHeight,
        clipping: false
      });
      const {
        top: bgTop,
        bottom: bgBottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('Background').patch({
        w: itemWidth,
        h: itemHeight,
        colorTop: bgTop,
        colorBottom: bgBottom,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: 6
        }
      });
      const ri = AppSettings$1.radii.card;
      this.tag('GlowRing').patch({
        w: itemWidth,
        h: itemHeight,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          stroke: 3,
          strokeColor: parseInt(colors.focusGlowColor, 16),
          radius: ri
        }
      });
      this.tag('Status').patch({
        y: startY,
        text: {
          fontSize: typography.body.fontSize,
          textColor: parseInt(colors.textTertiary, 16)
        }
      });
      this.tag('Name').patch({
        y: startY,
        text: {
          fontSize: typography.body.fontSize,
          textColor: parseInt(colors.textPrimary, 16),
          wordWrapWidth: itemWidth - 50
        }
      });
      this.tag('Method').patch({
        y: startY + nameHeight + 4,
        text: {
          fontSize: typography.caption.fontSize,
          textColor: parseInt(colors.textSecondary, 16),
          wordWrapWidth: itemWidth - 50
        }
      });
    }
    set test(value) {
      this._test = value;
      this.tag('Name').text.text = value.name;
      this.tag('Method').text.text = value.method;
    }
    set isFocused(value) {
      this.setFocus(value);
    }
    setFocus(focused) {
      const {
        colors
      } = AppSettings$1;
      const baseColor = parseInt(focused ? colors.focusedBackground : colors.cardBackground, 16);
      const {
        top,
        bottom
      } = AppSettings$1.embossColors(baseColor);
      this.tag('Background').patch({
        smooth: {
          colorTop: [top, {
            duration: 0.2
          }],
          colorBottom: [bottom, {
            duration: 0.2
          }]
        },
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: 6
        }
      });
      this.tag('GlowRing').patch({
        smooth: {
          alpha: [focused ? 1 : 0, {
            duration: 0.2
          }]
        }
      });
    }
    clearResult() {
      const {
        colors
      } = AppSettings$1;
      this.tag('Status').patch({
        text: {
          text: '○',
          textColor: parseInt(colors.textTertiary, 16)
        }
      });
      const {
        top,
        bottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('Background').patch({
        colorTop: top,
        colorBottom: bottom
      });
    }
    updateResult(result) {
      const {
        colors
      } = AppSettings$1;
      if (result.success) {
        this.tag('Status').patch({
          text: {
            text: '\u25cf',
            textColor: parseInt(colors.success, 16)
          }
        });
        const {
          top,
          bottom
        } = AppSettings$1.embossColors(parseInt(colors.cardBackgroundPass, 16));
        this.tag('Background').patch({
          smooth: {
            colorTop: [top, {
              duration: 0.3
            }],
            colorBottom: [bottom, {
              duration: 0.3
            }]
          }
        });
      } else {
        this.tag('Status').patch({
          text: {
            text: '\u25cf',
            textColor: parseInt(colors.error, 16)
          }
        });
        const {
          top,
          bottom
        } = AppSettings$1.embossColors(parseInt(colors.cardBackgroundFail, 16));
        this.tag('Background').patch({
          smooth: {
            colorTop: [top, {
              duration: 0.3
            }],
            colorBottom: [bottom, {
              duration: 0.3
            }]
          }
        });
      }
    }
  }

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
  class ResultsPanel extends Lightning$1.Component {
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
            type: Lightning$1.shaders.RoundedRectangle,
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
      };
    }
    _setup() {
      const {
        colors,
        typography
      } = AppSettings$1;
      const {
        width: contentW,
        maxH
      } = AppSettings$1.contentArea;
      const titleH = typography.heading.fontSize + 10;
      const summaryH = typography.bodyLarge.fontSize + 10;
      const listY = titleH + summaryH + 10;
      const listH = maxH - listY - 10;
      const listW = contentW;

      // Store for use in _createResultsList and ResultItem
      this._listW = listW;
      this._listH = listH;
      this._resultItemWidth = Math.floor(listW / 2) - 20;
      this._messageWordWrapWidth = this._resultItemWidth - 200;
      this.tag('Title').patch({
        text: {
          fontSize: typography.heading.fontSize,
          textColor: parseInt(colors.primary, 16)
        }
      });
      this.tag('Summary').patch({
        y: titleH,
        text: {
          fontSize: typography.bodyLarge.fontSize,
          textColor: parseInt(colors.textPrimary, 16)
        }
      });
      this.tag('ResultsList').patch({
        y: listY,
        w: listW,
        h: listH
      });
    }
    showResults(data) {
      const {
        category,
        results
      } = data;
      const passed = results.filter(r => r.success).length;
      const failed = results.length - passed;
      this.tag('Title').text.text = "".concat(category.name, " - Test Results");
      this.tag('Summary').text.text = "Total: ".concat(results.length, " | Passed: ").concat(passed, " | Failed: ").concat(failed);
      this._selectedColumn = 0;
      this._createResultsList(results);
    }
    _createResultsList(results) {
      const spacing = AppSettings$1.components.resultsPanel.resultItemSpacing;
      const listH = this._listH || 600;
      const listW = this._listW || AppSettings$1.contentArea.width;
      const maxRows = Math.max(1, Math.floor((listH - 20) / spacing));
      const columnWidth = Math.floor(listW / 2);

      // Store for scroll navigation
      this._maxRows = maxRows;
      this._columnWidth = columnWidth;
      this._totalColumns = Math.ceil(results.length / maxRows);
      const items = results.map((result, index) => {
        const col = Math.floor(index / maxRows);
        const row = index % maxRows;
        return {
          ref: "Result".concat(index),
          x: col * columnWidth,
          y: row * spacing,
          ResultItem: {
            type: ResultItem,
            result: result
          }
        };
      });
      this.tag('ResultsList.ResultsContent').children = items;
      this._updateScroll();
    }
    _updateScroll() {
      const visibleColumns = 2;
      const columnWidth = this._columnWidth || 860;
      let contentX = 10;
      if (this._selectedColumn >= visibleColumns) {
        contentX = 10 - (this._selectedColumn - visibleColumns + 1) * columnWidth;
      }
      this.tag('ResultsList.ResultsContent').setSmooth('x', contentX, {
        duration: 0.3
      });
    }
    _handleLeft() {
      if (this._selectedColumn > 0) {
        this._selectedColumn--;
        this._updateScroll();
      }
    }
    _handleRight() {
      if (this._selectedColumn < (this._totalColumns || 1) - 1) {
        this._selectedColumn++;
        this._updateScroll();
      }
    }
    _getFocused() {
      return this;
    }
  }
  class ResultItem extends Lightning$1.Component {
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
      };
    }
    _setup() {
      const {
        resultsPanel
      } = AppSettings$1.components;
      const {
        colors,
        typography
      } = AppSettings$1;
      const {
        width: contentW
      } = AppSettings$1.contentArea;
      const itemWidth = Math.floor(contentW / 2) - 20;
      const itemHeight = resultsPanel.resultItemHeight;

      // Single row: Status + Name + Source are all on the same Y — center on one line height
      const nameHeight = typography.body.fontSize;
      const startY = Math.round((itemHeight - nameHeight) / 2) - 3;
      this.patch({
        w: itemWidth,
        h: itemHeight
      });
      const {
        top: bgTop,
        bottom: bgBottom
      } = AppSettings$1.embossColors(parseInt(colors.cardBackground, 16));
      this.tag('Background').patch({
        w: itemWidth,
        h: itemHeight,
        colorTop: bgTop,
        colorBottom: bgBottom,
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: AppSettings$1.radii.card
        }
      });

      // Re-apply result color: _setup() runs after set result() in the Lightning lifecycle,
      // so Background would otherwise revert to cardBackground.
      if (this._result) {
        const bgKey = this._result.success ? 'cardBackgroundPass' : 'cardBackgroundFail';
        const {
          top: rTop,
          bottom: rBottom
        } = AppSettings$1.embossColors(parseInt(colors[bgKey], 16));
        this.tag('Background').patch({
          colorTop: rTop,
          colorBottom: rBottom
        });
      }
      this.tag('Status').patch({
        y: startY,
        text: {
          fontSize: typography.icon.fontSize
        }
      });
      this.tag('Name').patch({
        y: startY,
        text: {
          fontSize: typography.body.fontSize,
          textColor: parseInt(colors.textPrimary, 16),
          wordWrapWidth: itemWidth - 250
        }
      });
      this.tag('Source').patch({
        x: itemWidth - 170,
        y: startY + 2,
        text: {
          fontSize: typography.bodySmall.fontSize,
          textColor: parseInt(colors.textSecondary, 16)
        }
      });
    }
    set result(value) {
      const {
        colors
      } = AppSettings$1;
      this._result = value;
      if (value.success) {
        this.tag('Status').patch({
          text: {
            text: '\u25cf',
            textColor: parseInt(colors.success, 16)
          }
        });
        const {
          top,
          bottom
        } = AppSettings$1.embossColors(parseInt(colors.cardBackgroundPass, 16));
        this.tag('Background').patch({
          colorTop: top,
          colorBottom: bottom
        });
      } else {
        this.tag('Status').patch({
          text: {
            text: '\u25cf',
            textColor: parseInt(colors.error, 16)
          }
        });
        const {
          top,
          bottom
        } = AppSettings$1.embossColors(parseInt(colors.cardBackgroundFail, 16));
        this.tag('Background').patch({
          colorTop: top,
          colorBottom: bottom
        });
      }
      this.tag('Name').text.text = value.test.name;
      this.tag('Source').text.text = "[".concat(value.backend || 'unknown', "]");
    }
  }

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
  class DimensionRuler extends Lightning$1.Component {
    static _template() {
      return {
        HorizontalRuler: {
          y: 0
        },
        VerticalRuler: {
          x: 0
        },
        SafeAreaMarkers: {}
      };
    }
    _init() {
      this._stageWidth = this.stage.w || 1920;
      this._stageHeight = this.stage.h || 1080;
      this._createRulers();
    }
    _createRulers() {
      this._createHorizontalRuler();
      this._createVerticalRuler();
    }
    setSafeArea(safeArea) {
      this._safeArea = safeArea;
      this._createSafeAreaMarkers();
    }
    _createSafeAreaMarkers() {
      if (!this._safeArea) return;
      const {
        offsetX,
        offsetY,
        width,
        height
      } = this._safeArea;
      const lineWidth = 2;
      const dashLength = 20;
      const gapLength = 10;
      const color = 0xffff0000;
      const markers = [];

      // Top horizontal line (dashed)
      const topDashes = Math.floor(width / (dashLength + gapLength));
      for (let i = 0; i < topDashes; i++) {
        markers.push({
          ref: "TopDash".concat(i),
          x: offsetX + i * (dashLength + gapLength),
          y: offsetY,
          w: dashLength,
          h: lineWidth,
          rect: true,
          color: color
        });
      }

      // Bottom horizontal line (dashed)
      const bottomY = offsetY + height;
      for (let i = 0; i < topDashes; i++) {
        markers.push({
          ref: "BottomDash".concat(i),
          x: offsetX + i * (dashLength + gapLength),
          y: bottomY,
          w: dashLength,
          h: lineWidth,
          rect: true,
          color: color
        });
      }

      // Left vertical line (dashed)
      const leftDashes = Math.floor(height / (dashLength + gapLength));
      for (let i = 0; i < leftDashes; i++) {
        markers.push({
          ref: "LeftDash".concat(i),
          x: offsetX,
          y: offsetY + i * (dashLength + gapLength),
          w: lineWidth,
          h: dashLength,
          rect: true,
          color: color
        });
      }

      // Right vertical line (dashed)
      const rightX = offsetX + width;
      for (let i = 0; i < leftDashes; i++) {
        markers.push({
          ref: "RightDash".concat(i),
          x: rightX,
          y: offsetY + i * (dashLength + gapLength),
          w: lineWidth,
          h: dashLength,
          rect: true,
          color: color
        });
      }

      // Corner markers (solid squares)
      const cornerSize = 10;
      markers.push({
        ref: 'TopLeftCorner',
        x: offsetX - cornerSize / 2,
        y: offsetY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      }, {
        ref: 'TopRightCorner',
        x: rightX - cornerSize / 2,
        y: offsetY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      }, {
        ref: 'BottomLeftCorner',
        x: offsetX - cornerSize / 2,
        y: bottomY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      }, {
        ref: 'BottomRightCorner',
        x: rightX - cornerSize / 2,
        y: bottomY - cornerSize / 2,
        w: cornerSize,
        h: cornerSize,
        rect: true,
        color: color
      });

      // Labels showing safe area dimensions
      markers.push({
        ref: 'SafeAreaLabel',
        x: offsetX + width / 2 - 100,
        y: offsetY - 35,
        text: {
          text: "Safe Area: ".concat(Math.round(width), "x").concat(Math.round(height)),
          fontSize: 16,
          textColor: 0xffff0000
        }
      }, {
        ref: 'OffsetLabel',
        x: offsetX + 5,
        y: offsetY + 5,
        text: {
          text: "Offset: (".concat(Math.round(offsetX), ", ").concat(Math.round(offsetY), ")"),
          fontSize: 14,
          textColor: 0xffff0000
        }
      });
      this.tag('SafeAreaMarkers').children = markers;
    }
    _createHorizontalRuler() {
      const increment = 100;
      const majorIncrement = 500;
      const rulerHeight = 30;
      const items = [];

      // Background bar for horizontal ruler
      items.push({
        ref: 'Background',
        w: this._stageWidth,
        h: rulerHeight,
        rect: true,
        color: 0xcc000000
      });

      // Create tick marks and labels
      for (let x = 0; x <= this._stageWidth; x += increment) {
        const isMajor = x % majorIncrement === 0;
        const tickHeight = isMajor ? 20 : 10;
        items.push({
          ref: "Tick".concat(x),
          x: x,
          y: rulerHeight - tickHeight,
          w: 1,
          h: tickHeight,
          rect: true,
          color: 0xffffffff
        });
        if (isMajor) {
          items.push({
            ref: "Label".concat(x),
            x: x + 3,
            y: 2,
            text: {
              text: "".concat(x),
              fontSize: 14,
              textColor: 0xffffffff
            }
          });
        }
      }
      this.tag('HorizontalRuler').children = items;
    }
    _createVerticalRuler() {
      const increment = 100;
      const majorIncrement = 500;
      const rulerWidth = 30;
      const items = [];

      // Background bar for vertical ruler
      items.push({
        ref: 'Background',
        w: rulerWidth,
        h: this._stageHeight,
        rect: true,
        color: 0xcc000000
      });

      // Create tick marks and labels
      for (let y = 0; y <= this._stageHeight; y += increment) {
        const isMajor = y % majorIncrement === 0;
        const tickWidth = isMajor ? 20 : 10;
        items.push({
          ref: "Tick".concat(y),
          x: rulerWidth - tickWidth,
          y: y,
          w: tickWidth,
          h: 1,
          rect: true,
          color: 0xffffffff
        });
        if (isMajor) {
          items.push({
            ref: "Label".concat(y),
            x: 3,
            y: y + 3,
            text: {
              text: "".concat(y),
              fontSize: 14,
              textColor: 0xffffffff
            }
          });
        }
      }
      this.tag('VerticalRuler').children = items;
    }
    set enabled(value) {
      this.visible = value;
    }
  }

  var appSettings = {
  	stage: {
  		w: 1920,
  		h: 1080,
  		clearColor: "0xff0f1419",
  		useImageWorker: false
  	},
  	debug: false,
  	version: "1.0.0"
  };
  var platformSettings = {
  	path: "./static",
  	log: false,
  	showVersion: false
  };
  var settings$1 = {
  	appSettings: appSettings,
  	platformSettings: platformSettings
  };

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
  class App extends Lightning$1.Component {
    _construct() {
      const stageW = this.stage.w || window.innerWidth;
      const stageH = this.stage.h || window.innerHeight;
      AppSettings$1.initScale(stageW, stageH);
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
                type: Lightning$1.shaders.RoundedRectangle,
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
                  type: Lightning$1.shaders.RoundedRectangle,
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
              x: -12,
              y: -12,
              w: 624,
              h: 104,
              rect: true,
              color: 0x000097cc,
              shader: {
                type: Lightning$1.shaders.RoundedRectangle,
                radius: 14
              }
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
                type: Lightning$1.shaders.RoundedRectangle,
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
                type: Lightning$1.shaders.RoundedRectangle,
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
                type: Lightning$1.shaders.RoundedRectangle,
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
            x: 0,
            y: 0,
            w: 100,
            h: 100,
            ConsoleBg: {
              w: 100,
              h: 100,
              rect: true,
              color: 0x15ffffff,
              shader: {
                type: Lightning$1.shaders.RoundedRectangle,
                radius: 12
              }
            },
            ConsoleTitle: {
              x: 20,
              y: 12,
              text: {
                text: 'API Console',
                fontSize: 18,
                textColor: 0xffa8b3cf
              }
            },
            ConsoleViewport: {
              x: 0,
              y: 42,
              w: 100,
              h: 100,
              clipping: true,
              rect: true,
              color: 0x00000000,
              ConsoleLines: {
                x: 12,
                y: 0
              }
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
          x: 0,
          y: 0,
          Overlay: {
            w: 1920,
            h: 1080,
            rect: true,
            color: 0xaa000000
          },
          Box: {
            x: 660,
            y: 390,
            w: 600,
            h: 260,
            rect: true,
            color: 0xff1e2936,
            shader: {
              type: Lightning$1.shaders.RoundedRectangle,
              radius: 16
            },
            Title: {
              mount: 0.5,
              x: 300,
              y: 52,
              text: {
                text: 'Exit App?',
                fontSize: 36,
                textColor: 0xffffffff
              }
            },
            Message: {
              mount: 0.5,
              x: 300,
              y: 104,
              text: {
                text: 'Are you sure you want to exit?',
                fontSize: 22,
                textColor: 0xffa8b3cf
              }
            },
            BtnYes: {
              x: 60,
              y: 158,
              w: 200,
              h: 60,
              rect: true,
              color: 0xffef4444,
              shader: {
                type: Lightning$1.shaders.RoundedRectangle,
                radius: 10
              },
              Label: {
                mount: 0.5,
                x: 100,
                y: 30,
                text: {
                  text: 'Yes, Exit',
                  fontSize: 22,
                  textColor: 0xffffffff
                }
              }
            },
            BtnNo: {
              x: 340,
              y: 158,
              w: 200,
              h: 60,
              rect: true,
              color: 0xff2a3f5f,
              shader: {
                type: Lightning$1.shaders.RoundedRectangle,
                radius: 10
              },
              Label: {
                mount: 0.5,
                x: 100,
                y: 30,
                text: {
                  text: 'No, Stay',
                  fontSize: 22,
                  textColor: 0xffffffff
                }
              }
            }
          }
        }
      };
    }
    _setup() {
      const {
        layout,
        colors,
        typography,
        safeArea
      } = AppSettings$1;
      this.tag('Background').patch({
        color: parseInt(colors.background, 16)
      });
      this.tag('SafeContainer').patch({
        x: safeArea.offsetX,
        y: safeArea.offsetY
      });
      this.tag('SafeContainer.Header').patch({
        x: layout.padding.left,
        y: layout.padding.top
      });
      const headerW = Math.round(safeArea.width - layout.padding.left - layout.padding.right);
      this.tag('SafeContainer.Header.HeaderBg').patch({
        w: headerW + 20,
        color: parseInt(colors.titleBoxBgColor, 16),
        shader: {
          type: Lightning$1.shaders.RoundedRectangle,
          radius: 12,
          stroke: 2,
          strokeColor: parseInt(colors.titleBoxBorderColor, 16)
        }
      });
      this.tag('SafeContainer.Header.HeaderGlow').patch({
        w: headerW + 24
      });
      this.tag('SafeContainer.Header.ConnectionStatus').patch({
        x: headerW - 240
      });
      this.tag('SafeContainer.Header.Clock').patch({
        x: headerW - 240
      });
      const warningColor = parseInt(colors.warning, 16);
      this.tag('SafeContainer.Header.ConnectionStatus.Dot').patch({
        color: warningColor
      });
      this.tag('SafeContainer.Header.ConnectionStatus.Label').patch({
        text: {
          textColor: warningColor
        }
      });
      this._startHeaderGlow();
      this.tag('SafeContainer.Header.Title').patch({
        text: {
          text: " Firebolt\xAE API Test Tool",
          fontSize: typography.title.fontSize,
          textColor: parseInt(colors.titleTextColor, 16)
        }
      });
      this.tag('SafeContainer.Header.Subtitle').patch({
        text: {
          fontSize: (typography.headerSubtitle || typography.subtitle).fontSize,
          textColor: parseInt(colors.subtitleTextColor, 16)
        }
      });
      this.tag('SafeContainer.Content').patch({
        y: layout.content.offsetY,
        x: layout.padding.left
      });
      this._fireboltAPI = new FireboltAPI();
      this._fireboltAPI.onEventLog = (label, payload) => this._addEventLogEntry(label, payload);
      this._connectionStatusTimer = null;
      this._updateFireboltVersion();
      this._startConnectionMonitor();
      this._startClock();
      this.tag('SafeContainer.Legends').patch({
        zIndex: 10,
        x: layout.legends.offsetX,
        y: layout.legends.offsetY,
        w: layout.legends.width,
        h: layout.legends.height
      });
      const s0 = parseInt(colors.passRateStop0, 16);
      const s1 = parseInt(colors.passRateStop1, 16);
      const s2 = parseInt(colors.passRateStop2, 16);
      const s3 = parseInt(colors.passRateStop3, 16);
      this.tag('SafeContainer.Legends.GradientBar1').patch({
        colorLeft: s0,
        colorRight: s1
      });
      this.tag('SafeContainer.Legends.GradientBar2').patch({
        colorLeft: s1,
        colorRight: s2
      });
      this.tag('SafeContainer.Legends.GradientBar3').patch({
        colorLeft: s2,
        colorRight: s3
      });
      const consoleX = layout.padding.left;
      const consoleY = layout.legends.offsetY;
      const consoleW = layout.legends.offsetX - layout.padding.left - 16;
      const consoleH = layout.legends.height;
      this.tag('SafeContainer.APIConsole').patch({
        x: consoleX,
        y: consoleY,
        w: consoleW,
        h: consoleH
      });
      this.tag('SafeContainer.APIConsole.ConsoleBg').patch({
        w: consoleW,
        h: consoleH
      });
      const viewportH = consoleH - 44;
      this.tag('SafeContainer.APIConsole.ConsoleViewport').patch({
        w: consoleW,
        h: viewportH
      });
      this._consoleViewportH = viewportH;
      this._consoleLineW = consoleW - 28;
      this._consoleLineH = 48;
      this._consoleEntries = [];

      // Enable/disable dimension ruler based on settings
      if (AppSettings$1.debug.showDimensionRuler) {
        this.tag('DimensionRuler').visible = true;
        this.tag('DimensionRuler').setSafeArea(safeArea);
      }
    }
    async _updateFireboltVersion() {
      try {
        if (!this._fireboltAPI) {
          this.tag('SafeContainer.Header.Subtitle').patch({
            text: {
              text: 'Firebolt SDK (Unavailable)'
            }
          });
          return;
        }
        const status = await this._fireboltAPI.getConnectionStatus();
        const versionInfo = await this._fireboltAPI.getVersionInfo();
        const target = status.endpoint ? status.endpoint.replace('ws://', '') : 'unknown-target';
        if (versionInfo.sdkVersion) {
          this.tag('SafeContainer.Header.Subtitle').patch({
            text: {
              text: "Firebolt SDK v".concat(versionInfo.sdkVersion, " | ").concat(target)
            }
          });
        } else if (status.state === 'disconnected') {
          this.tag('SafeContainer.Header.Subtitle').patch({
            text: {
              text: "Firebolt SDK (Disconnected) | ".concat(target)
            }
          });
        } else if (status.state === 'mock') {
          this.tag('SafeContainer.Header.Subtitle').patch({
            text: {
              text: "Firebolt SDK (Mock Mode) | ".concat(target)
            }
          });
        } else {
          this.tag('SafeContainer.Header.Subtitle').patch({
            text: {
              text: "Firebolt SDK (Connected) | ".concat(target)
            }
          });
        }
      } catch (error) {
        this.tag('SafeContainer.Header.Subtitle').patch({
          text: {
            text: 'Firebolt SDK (Status Unknown)'
          }
        });
      }
    }
    _startHeaderGlow() {
      const {
        colors
      } = AppSettings$1;
      const colorOff = parseInt(colors.headerGlowColorOff, 16);
      const colorOn = parseInt(colors.headerGlowColorOn, 16);
      const glow = this.tag('SafeContainer.Header.HeaderGlow');
      const pulse = () => {
        glow.patch({
          smooth: {
            color: [colorOn, {
              duration: 1.8,
              timingFunction: 'ease-in-out'
            }]
          }
        });
        this._glowTimer = setTimeout(() => {
          glow.patch({
            smooth: {
              color: [colorOff, {
                duration: 1.8,
                timingFunction: 'ease-in-out'
              }]
            }
          });
          this._glowTimer = setTimeout(pulse, 1900);
        }, 1900);
      };
      pulse();
    }
    _startClock() {
      const version = settings$1.appSettings && settings$1.appSettings.version ? " v".concat(settings$1.appSettings.version) : '';
      const update = () => {
        const now = new Date();
        const h = String(now.getUTCHours()).padStart(2, '0');
        const m = String(now.getUTCMinutes()).padStart(2, '0');
        const s = String(now.getUTCSeconds()).padStart(2, '0');
        const ms = String(now.getUTCMilliseconds()).padStart(3, '0');
        this.tag('SafeContainer.Header.Clock').patch({
          text: {
            text: "".concat(version, "   ").concat(h, ":").concat(m, ":").concat(s, ".").concat(ms, " UTC")
          }
        });
      };
      update();
      this._clockTimer = setInterval(update, 50);
    }
    _startConnectionMonitor() {
      this._updateConnectionStatus();
      if (this._connectionStatusTimer) {
        clearInterval(this._connectionStatusTimer);
      }
      this._connectionStatusTimer = setInterval(() => {
        this._updateConnectionStatus();
      }, 5000);
    }
    _stopConnectionMonitor() {
      if (this._connectionStatusTimer) {
        clearInterval(this._connectionStatusTimer);
        this._connectionStatusTimer = null;
      }
    }
    _detach() {
      this._stopConnectionMonitor();
      if (this._clockTimer) {
        clearInterval(this._clockTimer);
        this._clockTimer = null;
      }
      if (this._glowTimer) {
        clearTimeout(this._glowTimer);
        this._glowTimer = null;
      }
      if (this._fireboltAPI) {
        this._fireboltAPI.unsubscribeAllEvents();
      }
    }
    async _updateConnectionStatus() {
      if (!this._fireboltAPI) {
        return;
      }
      try {
        const status = await this._fireboltAPI.getConnectionStatus();
        const {
          colors
        } = AppSettings$1;
        let color = parseInt(colors.warning, 16);
        if (status.state === 'connected') {
          color = parseInt(colors.connectionStatusConnected, 16);
        } else if (status.state === 'disconnected') {
          color = parseInt(colors.error, 16);
        }
        this.tag('SafeContainer.Header.ConnectionStatus.Dot').patch({
          color
        });
        this.tag('SafeContainer.Header.ConnectionStatus.Label').patch({
          text: {
            text: status.label,
            textColor: color
          }
        });
      } catch (error) {
        const color = parseInt(AppSettings$1.colors.error, 16);
        this.tag('SafeContainer.Header.ConnectionStatus.Dot').patch({
          color
        });
        this.tag('SafeContainer.Header.ConnectionStatus.Label').patch({
          text: {
            text: 'Status Check Failed',
            textColor: color
          }
        });
      }
    }
    _init() {
      this._setState('Menu');
      if (this._fireboltAPI) {
        this._fireboltAPI.subscribeAllEvents().catch(e => {
          console.error('subscribeAllEvents failed: ' + (e && e.message ? e.message : JSON.stringify(e)));
        });
      }
    }

    // Allow only directional keys, Enter, and Backspace. All other keys are consumed here.
    _captureKey(e) {
      const ALLOWED = new Set([37, 38, 39, 40,
      // ArrowLeft, ArrowUp, ArrowRight, ArrowDown
      13,
      // Enter
      8 // Backspace
      ]);
      return !ALLOWED.has(e.keyCode);
    }
    _onMenuSelect(category) {
      this.tag('SafeContainer.Content.TestRunner').loadTests(category);
      this._setState('TestRunner');
    }
    _onTestComplete(results) {
      this.tag('SafeContainer.Content.ResultsPanel').showResults(results);
      this._setState('Results');
    }
    _onTestRunnerStatus(data) {
      if (data && typeof data === 'object' && data.isResponse) {
        this._addConsoleEntry(data);
      } else if (typeof data === 'string' && data) {
        this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = data;
      }
    }
    _addConsoleEntry(data) {
      const {
        colors
      } = AppSettings$1;
      const i = this._consoleEntries.length;
      const lineH = this._consoleLineH;
      const wrapW = this._consoleLineW;
      const lines = this.tag('SafeContainer.APIConsole.ConsoleViewport.ConsoleLines');
      const child = lines.stage.c({
        ref: "CLEntry".concat(i),
        y: i * lineH,
        w: wrapW,
        h: lineH,
        Req: {
          x: 0,
          y: 1,
          text: {
            text: "Request:  ".concat(data.method),
            fontSize: 15,
            textColor: parseInt(colors.textSecondary, 16),
            wordWrapWidth: wrapW
          }
        },
        Res: {
          x: 0,
          y: 23,
          text: {
            text: "Response: ".concat(data.response),
            fontSize: 15,
            textColor: data.success ? parseInt(colors.success, 16) : parseInt(colors.error, 16),
            wordWrapWidth: wrapW
          }
        }
      });
      lines.childList.add(child);
      this._consoleEntries.push(data);
      this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = 'API Console';
      const totalH = this._consoleEntries.length * lineH;
      const maxScroll = Math.max(0, totalH - this._consoleViewportH);
      if (maxScroll > 0) {
        lines.setSmooth('y', -maxScroll, {
          duration: 0.2
        });
      }
    }
    _addEventLogEntry(label, payload) {
      const {
        colors
      } = AppSettings$1;
      const i = this._consoleEntries.length;
      const lineH = this._consoleLineH;
      const wrapW = this._consoleLineW;
      const lines = this.tag('SafeContainer.APIConsole.ConsoleViewport.ConsoleLines');
      const child = lines.stage.c({
        ref: "CLEntry".concat(i),
        y: i * lineH,
        w: wrapW,
        h: lineH,
        Req: {
          x: 0,
          y: 1,
          text: {
            text: "Event:   ".concat(label),
            fontSize: 15,
            textColor: parseInt(colors.primary, 16),
            wordWrapWidth: wrapW
          }
        },
        Res: {
          x: 0,
          y: 23,
          text: {
            text: "Payload: ".concat(payload),
            fontSize: 15,
            textColor: parseInt(colors.textSecondary, 16),
            wordWrapWidth: wrapW
          }
        }
      });
      lines.childList.add(child);
      this._consoleEntries.push({
        label,
        payload,
        isEvent: true
      });
      this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = 'API Console';
      const totalH = this._consoleEntries.length * lineH;
      const maxScroll = Math.max(0, totalH - this._consoleViewportH);
      if (maxScroll > 0) {
        lines.setSmooth('y', -maxScroll, {
          duration: 0.2
        });
      }
    }
    _clearConsole() {
      if (!this._consoleEntries) {
        this._consoleEntries = [];
        return;
      }
      this._consoleEntries = [];
      const lines = this.tag('SafeContainer.APIConsole.ConsoleViewport.ConsoleLines');
      lines.children = [];
      lines.patch({
        y: 0
      });
      this.tag('SafeContainer.APIConsole.ConsoleTitle').text.text = 'API Console';
    }
    static _states() {
      return [class Menu extends this {
        _getFocused() {
          return this.tag('SafeContainer.Content.Menu');
        }
        _handleBack() {
          this._setState('ExitConfirm');
          return true;
        }
        $enter() {
          this.tag('SafeContainer.Content.Menu').visible = true;
          this.tag('SafeContainer.Content.TestRunner').visible = false;
          this.tag('SafeContainer.Content.ResultsPanel').visible = false;
          this.tag('SafeContainer.Legends').visible = true;
          this._clearConsole();
        }
      }, class TestRunner extends this {
        _getFocused() {
          return this.tag('SafeContainer.Content.TestRunner');
        }
        _handleBack() {
          // Block back navigation if tests are running
          const testRunner = this.tag('SafeContainer.Content.TestRunner');
          if (testRunner._isRunning) {
            return true; // Consume key — do NOT propagate; returning false closes the app
          }
          this._setState('Menu');
        }
        $enter() {
          this.tag('SafeContainer.Content.Menu').visible = false;
          this.tag('SafeContainer.Content.TestRunner').visible = true;
          this.tag('SafeContainer.Content.ResultsPanel').visible = false;
          this.tag('SafeContainer.Legends').visible = true;
        }
      }, class Results extends this {
        _getFocused() {
          return this.tag('SafeContainer.Content.ResultsPanel');
        }
        _handleBack() {
          this._setState('Menu');
        }
        $enter() {
          this.tag('SafeContainer.Content.Menu').visible = false;
          this.tag('SafeContainer.Content.TestRunner').visible = false;
          this.tag('SafeContainer.Content.ResultsPanel').visible = true;
          this.tag('SafeContainer.Legends').visible = true;
        }
      }, class ExitConfirm extends this {
        _getFocused() {
          return this;
        }
        $enter() {
          this._exitFocusYes = true;
          this.tag('ExitDialog').visible = true;
          this._highlightExitBtn();
        }
        $exit() {
          this.tag('ExitDialog').visible = false;
        }
        _handleLeft() {
          this._exitFocusYes = true;
          this._highlightExitBtn();
          return true;
        }
        _handleRight() {
          this._exitFocusYes = false;
          this._highlightExitBtn();
          return true;
        }
        _handleEnter() {
          if (this._exitFocusYes) {
            this.application.closeApp();
            window.close();
          } else {
            this._setState('Menu');
          }
          return true;
        }
        _handleBack() {
          this._setState('Menu');
          return true;
        }
        _highlightExitBtn() {
          const yes = this._exitFocusYes;
          this.tag('ExitDialog.Box.BtnYes').patch({
            color: yes ? 0xffef4444 : 0xff2a3f5f
          });
          this.tag('ExitDialog.Box.BtnNo').patch({
            color: yes ? 0xff2a3f5f : 0xff00aa55
          });
        }
      }];
    }
  }

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
  function index$6 () {
    return Launch(App, ...arguments);
  }

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  const settings = { platform: {} };
  const subscribers = {};

  const initSettings = (appSettings, platformSettings) => {
    settings['app'] = appSettings;
    settings['platform'] = {
      logLevel: 'WARN',
      ...platformSettings,
    };
    settings['user'] = {};
  };

  const publish = (key, value) => {
    subscribers[key] &&
      subscribers[key].forEach((subscriber) => subscriber(value));
  };

  const dotGrab$1 = (obj = {}, key) => {
    const keys = key.split('.');
    for (let i = 0; i < keys.length; i++) {
      obj = obj[keys[i]] = obj[keys[i]] !== undefined ? obj[keys[i]] : {};
    }
    return typeof obj === 'object'
      ? Object.keys(obj).length
        ? obj
        : undefined
      : obj
  };

  var Settings = {
    get(type, key, fallback = undefined) {
      const val = dotGrab$1(settings[type], key);
      return val !== undefined ? val : fallback
    },
    has(type, key) {
      return !!this.get(type, key)
    },
    set(key, value) {
      settings['user'][key] = value;
      publish(key, value);
    },
    subscribe(key, callback) {
      subscribers[key] = subscribers[key] || [];
      subscribers[key].push(callback);
    },
    unsubscribe(key, callback) {
      if (callback) {
        const index =
          subscribers[key] && subscribers[key].findIndex((cb) => cb === callback);
        index > -1 && subscribers[key].splice(index, 1);
      } else {
        if (key in subscribers) {
          subscribers[key] = [];
        }
      }
    },
    clearSubscribers() {
      for (const key of Object.getOwnPropertyNames(subscribers)) {
        delete subscribers[key];
      }
    },
    setLogLevel(logLevel) {
      settings.platform.logLevel = logLevel;
    },
    getLogLevel() {
      return settings.platform.logLevel
    },
  };

  const MAX_QUEUED_MESSAGES = 100;

  class WebsocketTransport {
    constructor(endpoint) {
      this._endpoint = endpoint;
      this._ws = null;
      this._connected = false;
      this._queue = [];
      this._callbacks = [];
    }

    send(msg) {
      this._connect();

      if (this._connected) {
        this._ws.send(msg);
      } else {
        if (this._queue.length < MAX_QUEUED_MESSAGES) {
          this._queue.push(msg);
        }
      }
    }

    receive(callback) {
      if (!callback) return
      this._connect();
      this._callbacks.push(callback);
    }

    _notifyCallbacks(message) {
      for (let i = 0; i < this._callbacks.length; i++) {
        setTimeout(() => this._callbacks[i](message), 1);
      }
    }

    _connect() {
      if (this._ws) return
      this._ws = new WebSocket(this._endpoint, ['jsonrpc']);
      this._ws.addEventListener('message', (message) => {
        this._notifyCallbacks(message.data);
      });
      this._ws.addEventListener('error', (message) => {});
      this._ws.addEventListener('close', (message) => {
        this._ws = null;
        this._connected = false;
      });
      this._ws.addEventListener('open', (message) => {
        this._connected = true;
        for (let i = 0; i < this._queue.length; i++) {
          this._ws.send(this._queue[i]);
        }
        this._queue = [];
      });
    }
  }

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  const win$2 = typeof window !== 'undefined' ? window : {};
  win$2.__firebolt = win$2.__firebolt || {};

  initSettings({}, { log: true });

  let implementation;
  let _callback;

  function send$1(json) {
    implementation = getImplementation();

    if (Settings.getLogLevel() === 'DEBUG') {
      console.debug(
        'Sending message to transport: \n' +
          JSON.stringify(json, { indent: '\t' }),
      );
    }

    implementation.send(JSON.stringify(json));
  }

  function receive$1(callback) {
    if (implementation) {
      implementation.receive(callback);
    } else {
      _callback = callback;
    }
  }

  function getImplementation() {
    if (implementation) {
      return implementation
    }

    if (win$2.__firebolt.transport) {
      implementation = win$2.__firebolt.transport;
    } else if (win$2.__firebolt.endpoint) {
      // Only adds RPCv2=true query parameter when using bidirectional SDK.
      // This parameter will not be present when using unidirectional SDK.
      // Unidirectional endpoint is handled in Gateway/Unidirectional.mjs
      const endpoint = win$2.__firebolt.endpoint;
      const url = endpoint + (endpoint.includes('?') ? '&' : '?') + 'RPCv2=true';
      implementation = new WebsocketTransport(url);
    } else {
      implementation = MockTransport;
    }

    win$2.__firebolt.transport = implementation;
    implementation.receive(_callback);

    _callback = undefined;

    return implementation
  }

  var Transport = {
    send: send$1,
    receive: receive$1,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  const providers = {};
  const interfaces = {};
  const listeners$1 = {};

  // Request that the app provide fulfillment of an method
  async function request$2(id, method, params, transforms) {
    let result, error;
    try {
      result = await getProviderResult(method, params);
    } catch (e) {
      error = e;
    }

    const response = {
      jsonrpc: '2.0',
      id: id,
    };

    if (error) {
      // todo: make sure this conforms to JSON-RPC schema for errors
      response.error = error;
    } else {
      response.result = result;
      if (result === undefined) {
        response.result = null;
      }
    }

    Transport.send(response);
  }

  async function notify$1(method, params) {
    if (listeners$1[method]) {
      listeners$1[method](params);
      return
    }
    // All SDKs use a global transport where they register their callbacks for the responses.
    // When the global transport receives a message, it calls all registered callbacks with that same message.
    // That's why we can't generate an error here, as the response could be for an event registered by another SDK.
    //console.log( `Notification not implemented: ${method}`);
  }

  // Register a provider implementation with an interface name
  function provide$1(interfaceName, provider) {
    providers[interfaceName] = provider;
  }

  // Register a notification listener with an event name
  function subscribe$1(event, callback) {
    listeners$1[event] = callback;
  }

  function unsubscribe$2(event) {
    delete listeners$1[event];
  }

  function simulate$1(event, value) {
    listeners$1[event](value);
  }

  async function getProviderResult(method, params = {}) {
    const split = method.split('.');
    method = split.pop();
    const interfaceName = split.join('.');

    if (providers[interfaceName]) {
      if (providers[interfaceName][method]) {
        // sort the params into an array based on the interface parameter order
        const parameters = interfaces[interfaceName].methods
          .find((m) => m.name === method)
          .parameters.map((p) => params[p])
          .filter((p) => p !== undefined);
        return await providers[interfaceName][method](...parameters)
      }
      throw `Method not implemented: ${method}`
    }
    throw `Interface not provided: ${interfaceName}`
  }

  var PlatformApi = {
    request: request$2,
    notify: notify$1,
    provide: provide$1,
    subscribe: subscribe$1,
    unsubscribe: unsubscribe$2,
    simulate: simulate$1,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  const win$1 = typeof window !== 'undefined' ? window : {};
  win$1.__firebolt = win$1.__firebolt || {};

  // JSON RPC id generator, to be shared across all SDKs
  class JsonRpcIdIterator {
    constructor() {
      this._id = 1;
    }
    getJsonRpcId() {
      return this._id++
    }
  }

  let idGenerator = win$1.__firebolt.idGenerator || new JsonRpcIdIterator();
  win$1.__firebolt.idGenerator = idGenerator;

  const promises = {};
  const deprecated = {};

  // request = { method: string, params: object, id: boolean }[]
  // request with no `id` property are assumed to NOT be notifications, i.e. id must be set to false explicitly
  async function batch$1(requests) {
    if (Array.isArray(requests)) {
      const processed = requests.map((req) =>
        processRequest(req.method, req.params, req.id, req.id === false),
      );

      // filter requests exclude notifications, as they don't need promises
      const promises = processed
        .filter((req) => req.id)
        .map((request) => addPromiseToQueue(request.id));

      Transport.send(processed);

      // Using Promise.all get's us batch blocking for free
      return Promise.all(promises)
    }
    throw `Bulk requests must be in an array`
  }

  // Request that the server provide fulfillment of an method
  async function request$1(method, params) {
    const json = processRequest(method, params);
    const promise = addPromiseToQueue(json.id);
    Transport.send(json);
    return promise
  }

  function response(id, result, error) {
    const promise = promises[id];

    if (promise) {
      if (error !== undefined) {
        promises[id].reject(error);
      } else {
        promises[id].resolve(result);
      }

      // TODO make sure this works
      delete promises[id];
    }
  }

  function deprecate$1(method, alternative) {
    deprecated[method] = alternative;
  }

  function addPromiseToQueue(id) {
    return new Promise((resolve, reject) => {
      promises[id] = {};
      promises[id].promise = this;
      promises[id].resolve = resolve;
      promises[id].reject = reject;
    })
  }

  function processRequest(method, params, notification = false) {
    if (deprecated[method]) {
      console.warn(`WARNING: ${method}() is deprecated. ` + deprecated[method]);
    }

    const id = !notification && idGenerator.getJsonRpcId();
    const jsonrpc = '2.0';
    const json = { jsonrpc, method, params };

    !notification && (json.id = id);

    return json
  }

  var AppApi = {
    request: request$1,
    batch: batch$1,
    response,
    deprecate: deprecate$1,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  Transport.receive(async (message) => {
    let json; try { json = JSON.parse(message); } catch(e) { return; }
    if (Array.isArray(json)) {
      json.forEach((message) => processMessage(message));
    } else {
      processMessage(json);
    }
  });

  function processMessage(json) {
    if (Settings.getLogLevel() === 'DEBUG') {
      console.debug(
        'Receiving message from transport: \n' +
          JSON.stringify(json, { indent: '\t' }),
      );
    }

    if (json.method !== undefined) {
      if (json.id !== undefined) {
        PlatformApi.request(json.id, json.method, json.params);
      } else {
        let params = json.params;
        // TODO: Check if json.params is an array and if so, convert to a key-value object
        // This is necessary because for Ripple, if params is an array this means that
        // the callback function is expected to have one argument - the array itself.
        // This is not compliant with the JSON-RPC specification, i.e. it should be removed.
        if (Array.isArray(params)) {
          params = { value: json.params };
        }
        PlatformApi.notify(json.method, params);
      }
    } else if (json.id !== undefined) {
      AppApi.response(json.id, json.result, json.error);
    }
  }

  async function batch(requests) {
    if (Array.isArray(requests)) {
      return await AppApi.batch(requests)
    } else {
      throw 'Gateway.batch() requires an array of requests: { method: String, params: Object, id: Boolean }'
    }
  }

  async function request(method, params) {
    if (Array.isArray(method)) {
      throw 'Use Gateway.batch() for batch requests.'
    } else {
      return await AppApi.request(method, params)
    }
  }

  async function notify(method, params) {
    if (Array.isArray(method)) {
      throw 'Use Gateway.batch() for batch requests.'
    } else {
      return await AppApi.notify(method, params)
    }
  }

  function subscribe(event, callback) {
    PlatformApi.subscribe(event, callback);
  }

  function unsubscribe$1(event) {
    PlatformApi.unsubscribe(event);
  }

  function simulate(event, value) {
    PlatformApi.simulate(event, value);
  }

  function provide(interfaceName, provider) {
    PlatformApi.provide(interfaceName, provider);
  }

  function deprecate(method, alternative) {
    AppApi.deprecate(method, alternative);
  }

  var Bidirectional = {
    request,
    notify,
    batch,
    subscribe,
    unsubscribe: unsubscribe$1,
    simulate,
    provide,
    deprecate,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  function removeNullOptionalParams(params, numOfOptionalParams) {
    // Iterate over all params starting backwrods and if the param is null remove it from the params object.
    // If it is undefined that means the param is not provided, which is Ok.
    // We should not get a call with numOfOptionalParams === 0, but if we do, just return the params as are.
    const keys = Object.keys(params);
    let paramsIndex = keys.length - 1;
    while (paramsIndex >= 0 && numOfOptionalParams > 0) {
      const key = keys[paramsIndex];
      if (params[key] === null) {
        delete params[key];
        console.warn(
          'WARNING: null values for optional params will be disallowed in a future Firebolt version. Parameter: ' +
            key,
        );
      } else if (params[key] === undefined) ; else {
        // if an optional param is provided we should stop removing params as if we continue we will change the order of the params
        break
      }
      paramsIndex--;
      numOfOptionalParams--;
    }
    return params
  }
  Bidirectional.removeNullOptionalParams = removeNullOptionalParams;

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  const win = typeof window !== 'undefined' ? window : {};

  let listener;

  let mock$1;
  const pending = [];

  let callback;
  let testHarness;

  if (win.__firebolt && win.__firebolt.testHarness) {
    testHarness = win.__firebolt.testHarness;
  }

  function send(message) {
    const json = JSON.parse(message);
    // handle bulk sends
    if (Array.isArray(json)) {
      json.forEach((json) => send(JSON.stringify(json)));
      return
    }

    //let [module, method] = json.method.split('.')

    if (testHarness && testHarness.send) {
      testHarness.send(message);
    }

    if (json.method) {
      if (mock$1) handle(json);
      else pending.push(json);
    } else if (json.id !== undefined && requests[json.id]) {
      const promise = requests[json.id];
      if (json.error !== undefined) {
        promise.reject(json.error);
      } else {
        promise.resolve(json.result);
      }

      delete requests[json.id];
    }
  }

  function handle(json) {
    let result;
    try {
      result = getResult(json.method, json.params);
      setTimeout(() =>
        callback(
          JSON.stringify({
            jsonrpc: '2.0',
            result: result,
            id: json.id,
          }),
        ),
      );
    } catch (error) {
      setTimeout(() =>
        callback(
          JSON.stringify({
            jsonrpc: '2.0',
            error: {
              code: -32602,
              message:
                'Invalid params (this is a mock error from the mock transport layer)',
            },
            id: json.id,
          }),
        ),
      );
    }
  }

  function receive(_callback) {
    callback = _callback;

    if (testHarness && typeof testHarness.initialize === 'function') {
      testHarness.initialize({
        emit: (module, method, value) => {
          Bidirectional.simulate(`${module}.${method}`, value);
        },
        listen: function (...args) {
          listener(...args);
        },
      });
    }
  }

  function event$1(module, event, data) {
    callback(
      JSON.stringify({
        jsonrpc: '2.0',
        method: `${module}.${event}`,
        params: {
          value: data,
        },
      }),
    );
  }

  function receiveMessage(message) {
    callback(message);
  }
  const requests = [];

  function dotGrab(obj = {}, key) {
    const keys = key.split('.');
    let ref = obj;
    for (let i = 0; i < keys.length; i++) {
      ref = (Object.entries(ref).find(
        ([k, v]) => k.toLowerCase() === keys[i].toLowerCase(),
      ) || [null, {}])[1];
    }
    return ref
  }

  function getResult(method, params) {
    let api = dotGrab(mock$1, method);

    if (method.match(/^[a-zA-Z]+\.on[A-Za-z]+$/)) {
      api = {
        event: method,
        listening: true,
      };
    }

    if (typeof api === 'function') {
      let result = params == null ? api() : api(params);
      if (result === undefined) {
        result = null;
      }
      return result
    } else return api
  }

  function setMockResponses(m) {
    mock$1 = m;

    pending.forEach((json) => handle(json));
    pending.length = 0;
  }

  var MockTransport = {
    send: send,
    receiveMessage: receiveMessage,
    handle: handle,
    event: event$1,
    receive: receive,
  };

  function router (params, callbackOrValue, contextParameterCount) {
    const numArgs = params ? Object.values(params).length : 0;

    if (numArgs === contextParameterCount && callbackOrValue === undefined) {
      // getter
      return 'getter'
    } else if (
      numArgs === contextParameterCount &&
      typeof callbackOrValue === 'function'
    ) {
      // subscribe
      return 'subscriber'
    } else if (numArgs === 0 && typeof callbackOrValue === 'function') {
      // subscribe
      return 'subscriber'
    } else if (
      numArgs === contextParameterCount &&
      callbackOrValue !== undefined
    ) {
      // setter
      return 'setter'
    }

    return null
  }

  const mocks = {};

  function mock(module, method, params, value, contextParameterCount, def) {
    const type = router(params, value, contextParameterCount);
    const hash = contextParameterCount
      ? '.' +
        Object.keys(params)
          .filter((key) => key !== 'value')
          .map((key) => params[key])
          .join('.')
      : '';
    const key = `${module}.${method}${hash}`;

    if (type === 'getter') {
      const value = mocks.hasOwnProperty(key) ? mocks[key] : def;
      return value
    } else if (type === 'subscriber') ; else if (type === 'setter') {
      mocks[key] = value;
      return null
    }
  }

  var MockProps = {
    mock: mock,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Accessibility = {
    audioDescription: function (params) {
      return MockProps.mock(
        'Accessibility',
        'audioDescription',
        params,
        undefined,
        0,
        true,
      )
    },
    closedCaptionsSettings: function (params) {
      return MockProps.mock(
        'Accessibility',
        'closedCaptionsSettings',
        params,
        undefined,
        0,
        { enabled: true, preferredLanguages: ['eng', 'spa'] },
      )
    },
    highContrastUI: function (params) {
      return MockProps.mock(
        'Accessibility',
        'highContrastUI',
        params,
        undefined,
        0,
        true,
      )
    },
    voiceGuidanceSettings: function (params) {
      return MockProps.mock(
        'Accessibility',
        'voiceGuidanceSettings',
        params,
        undefined,
        0,
        { enabled: true, rate: 0.8, navigationHints: true },
      )
    },
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Advertising = {
    advertisingId: {
      ifa: 'bd87dd10-8d1d-4b93-b1a6-a8e5d410e400',
      ifa_type: 'sspid',
      lmt: '0',
    },
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Device = {
    uid: 'ee6723b8-7ab3-462c-8d93-dbf61227998e',
    deviceClass: 'ott',
    uptime: 123456,
    timeInActiveState: 654321,
    chipsetId: 'BCM72180',
    hdr: function (params) {
      return MockProps.mock('Device', 'hdr', params, undefined, 0, {
        hdr10: true,
        hdr10Plus: true,
        dolbyVision: true,
        hlg: true,
      })
    },
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Localization = {
    country: function (params) {
      return MockProps.mock('Localization', 'country', params, undefined, 0, 'US')
    },
    preferredAudioLanguages: function (params) {
      return MockProps.mock(
        'Localization',
        'preferredAudioLanguages',
        params,
        undefined,
        0,
        ['spa', 'eng'],
      )
    },
    presentationLanguage: function (params) {
      return MockProps.mock(
        'Localization',
        'presentationLanguage',
        params,
        undefined,
        0,
        'en-US',
      )
    },
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Metrics = {
    ready: true,
    signIn: true,
    signOut: true,
    startContent: true,
    stopContent: true,
    page: true,
    error: true,
    mediaLoadStart: true,
    mediaPlay: true,
    mediaPlaying: true,
    mediaPause: true,
    mediaWaiting: true,
    mediaSeeking: true,
    mediaSeeked: true,
    mediaRateChanged: true,
    mediaRenditionChanged: true,
    mediaEnded: true,
    event: true,
    appInfo: null,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Network = {
    connected: function (params) {
      return MockProps.mock('Network', 'connected', params, undefined, 0, true)
    },
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Discovery = {
    watched: true,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  var _Platform = {
    localization: _Localization,
    device: _Device,
    accessibility: _Accessibility,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  let listenerId = 0;

  // holds two maps of ${module}.${event} => listenerId, e.g. callback method id
  // note that one callback can listen to multiple events, e.g. 'discovery.*'
  // internal is only available via a private export that we use to ensure our modules know about
  // events before the apps using the SDK (otherwise state errors can happen)
  const listeners = {
    internal: {},
    external: {},

    // Several convenience functions below for checking both internal & external lists w/ one operation

    // gets a merge list of ids for a single event key
    get: (key) => {
      return Object.assign(
        Object.assign({}, listeners.internal[key]),
        listeners.external[key],
      )
    },
    // adds a callback/id to a key on the external list only
    set: (key, id, value) => {
      listeners.external[key] = listeners.external[key] || {};
      listeners.external[key][id] = value;
    },
    // adds a callback/id to a key on the internal list only
    setInternal: (key, id, value) => {
      listeners.internal[key] = listeners.internal[key] || {};
      listeners.internal[key][id] = value;
    },
    // finds the key for an id in either list (it can only be in one)
    find: (id) => {
      let key
      ;[listeners.internal, listeners.external].find((group) => {
        key = Object.keys(group).find((key) => group[key][id]);
        if (key) return true
      });
      return key
    },
    // removes an id from either list
    remove: (id) => {
  [listeners.internal, listeners.external].forEach((group) => {
        Object.keys(group).forEach((key) => {
          if (group[key] && group[key][id]) {
            delete group[key][id];
            if (Object.values(group[key]).length === 0) {
              delete group[key];
            }
          }
        });
      });
    },
    // removes a key from both lists if _internal is true, otherwise only the external list
    removeKey: (key, _internal = false) => {
      _internal && listeners.internal[key] && delete listeners.internal[key];
      listeners.external[key] && delete listeners.external[key];
    },
    // gives a list of all keys
    keys: () => {
      return Array.from(
        new Set(
          Object.keys(listeners.internal).concat(Object.keys(listeners.external)),
        ),
      )
    },
    // counts how many listeners are in a key across both lists
    count: (key) => {
      return Object.values(listeners.get(key)).length
    },
  };

  // holds a map of ${module}.${event} => Transport.send calls (only called once per event)
  // note that the keys here MUST NOT contain wild cards
  const oncers = [];
  const validEvents = {};
  const validContext = {};

  const registerEvents = (module, events) => {
    validEvents[module] = events.concat();
  };

  const callCallbacks = (key, params) => {
    const args = Object.values(params);
    const callbacks = Object.entries(listeners.internal[key] || {}).concat(
      Object.entries(listeners.external[key] || {}),
    );
    callbacks.forEach(([listenerId, callback]) => {
      if (oncers.indexOf(parseInt(listenerId)) >= 0) {
        oncers.splice(oncers.indexOf(parseInt(listenerId)), 1);
        delete listeners.external[key][listenerId];
      }
      if (args.length <= callback.length) {
        callback.apply(null, args);
      } else if (
        args.length > 0 &&
        callback.length > 0 &&
        callback.length < args.length
      ) {
        // read the params the callback takes and if they are less then the size of args remove the extra params from the front
        const callbackArgs = args.slice(-callback.length, args.length);
        callback.apply(null, callbackArgs);
      } else {
        callback.call(null);
      }
    });
  };

  const doListen = function (
    module,
    event,
    callback,
    context,
    once,
    internal = false,
  ) {

    if (typeof callback !== 'function') {
      return Promise.reject('No valid callback function provided.')
    } else {
      if (module === '*') {
        return Promise.reject('No valid module name provided')
      }

      //TODO: This is a patch and should be removed! Adds "on" if the event does not start with "on".
      // Added temporarily for backward compatibility with older event namings
      if (event != '*' && !event.startsWith('on')) {
        event = 'on' + event[0].toUpperCase() + event.substring(1);
        console.warn(
          `Event names should begin with 'on'. Using '${module + '.' + event}' instead.`,
        );
      }

      const wildcard = event === '*';
      const events = wildcard ? validEvents[module] : [event]; // explodes wildcards into an array
      const promises = [];
      const hasContext = Object.values(context).length > 0;
      const contextKey = Object.keys(context)
        .sort()
        .map((key) => key + '=' + JSON.stringify(context[key]))
        .join('&');

      listenerId++;

      if (once) {
        oncers.push(listenerId);
      }

      events.forEach((event) => {
        const key = module + '.' + event + (hasContext ? `.${contextKey}` : '');

        if (Object.values(listeners.get(key)).length === 0) {
          const args = Object.assign({ listen: true }, context);

          const subscriber = module + '.' + event;
          const notifier = module + '.' + event;

          Bidirectional.subscribe(notifier, (params) => {
            callCallbacks(key, params);
          });
          const promise = Bidirectional.request(subscriber, args);
          promises.push(promise);
        }

        const setter = internal ? listeners.setInternal : listeners.set;

        if (wildcard) {
          setter(key, '' + listenerId, (value) => callback(event, value));
        } else {
          setter(key, '' + listenerId, callback);
        }
      });

      let resolve, reject;
      let p = new Promise((res, rej) => {
        resolve = res;
        reject = rej;
      });

      // Iterate and resolve/reject through the list of promises sequentially
      const templistenerId = listenerId;
      if (promises.length) {
        promises.reduce((prevPromise, currentPromise) => {
          return prevPromise
            .then(() => currentPromise)
            .then((responses) => {
              resolve(templistenerId);
            })
            .catch((error) => {
              if (event === '*') {
                resolve(templistenerId);
              } else {
                // Remove the failed listener
                doClear(templistenerId, event, context);
                reject(error);
              }
            })
        }, Promise.resolve());
      } else {
        resolve(listenerId);
      }

      return p
    }
  };

  const getListenArgs = function (...args) {
    const callback = args.pop();
    const [module, event, context] = getClearArgs(...args);

    return [module, event, callback, context]
  };

  const getClearArgs = function (...args) {
    const module = args.shift() || '*';
    const event = args.shift() || '*';
    const context = {};

    // populate context based on registered context for this module/event
    // if an argument is not registered in validContext, that is ok meaning it is not a context argument
    // Needed after introducing x-contextual-params
    for (let i = 0; args.length; i++) {
      let currentArg = args.shift();
      if (
        validContext[module] &&
        validContext[module][event] &&
        validContext[module][event][i]
      ) {
        context[validContext[module][event][i]] = currentArg;
      }
    }
    return [module, event, context]
  };

  const once$4 = function (...args) {
    const [module, event, callback, context] = getListenArgs(...args);
    return doListen(module, event, callback, context, true)
  };

  const listen$4 = function (...args) {
    const [module, event, callback, context] = getListenArgs(...args);
    return doListen(module, event, callback, context, false)
  };

  const clear$4 = function (...args) {
    if (args && args.length && typeof args[0] === 'number') {
      return doClear(args[0])
    } else if (args && args.length && typeof args[1] === 'number') {
      return doClear(args[1])
    } else {
      const [moduleOrId, event, context] = getClearArgs(...args);
      return doClear(moduleOrId, event, context)
    }
  };

  const unsubscribe = (key, context) => {
    const [module, event] = key.split('.').slice(0, 2);
    const args = Object.assign({ listen: false }, context);
    Bidirectional.request(module + '.' + event, args);
    Bidirectional.unsubscribe(`${module}.${event}`);
  };

  // TODO: clear needs to go through Transport Layer
  const doClear = function (moduleOrId = false, event = false, context) {
    if (event === '*') {
      event = false;
    }

    if (typeof moduleOrId === 'number') {
      const searchId = moduleOrId.toString();
      const key = listeners.find(searchId);

      if (key) {
        listeners.remove(searchId);
        if (listeners.count(key) === 0) {
          unsubscribe(key);
        }
        return true
      }
      return false
    } else {
      if (!moduleOrId && !event) {
        listeners.keys().forEach((key) => {
          listeners.removeKey(key);
          unsubscribe(key);
        });
      } else if (!event) {
        listeners.keys().forEach((key) => {
          if (key.indexOf(moduleOrId) === 0) {
            listeners.removeKey(key);
            unsubscribe(key);
          }
        });
      } else {
        const hasContext = Object.values(context).length > 0;
        const contextKey = Object.keys(context)
          .sort()
          .map((key) => key + '=' + JSON.stringify(context[key]))
          .join('&');
        const key =
          moduleOrId + '.' + event + (hasContext ? `.${contextKey}` : '');

        listeners.removeKey(key);
        unsubscribe(key, context);
      }
    }
  };

  var Events = {
    listen: listen$4,
    once: once$4,
    clear: clear$4,
  };

  function prop(
    moduleName,
    key,
    params,
    callbackOrValue = undefined,
    immutable,
    readonly,
    contextParameterCount,
  ) {
    const numArgs = Object.values(params).length;
    const type = router(params, callbackOrValue, contextParameterCount);

    if (type === 'getter') {
      return Bidirectional.request(moduleName + '.' + key, params)
    } else if (type === 'subscriber') {
      // subscriber
      if (immutable) {
        throw new Error('Cannot subscribe to an immutable property')
      }
      const subscriber =
        'on' + key[0].toUpperCase() + key.substring(1) + 'Changed';
      return Events.listen(
        moduleName,
        subscriber,
        ...Object.values(params),
        callbackOrValue,
      )
    } else if (type === 'setter') {
      // setter
      if (immutable) {
        throw new Error('Cannot set a value to an immutable property')
      }
      if (readonly) {
        throw new Error('Cannot set a value to a readonly property')
      }
      return Bidirectional.request(
        moduleName + '.set' + key[0].toUpperCase() + key.substring(1),
        Object.assign(
          {
            value: callbackOrValue,
          },
          params,
        ),
      )
    } else if (numArgs < contextParameterCount) {
      throw new Error(
        'Cannot get a value without all required context parameters.',
      )
    } else {
      throw new Error('Property accessed with unexpected number of parameters.')
    }
  }

  var Prop = {
    prop: prop,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  registerEvents('Accessibility', [
    'onAudioDescriptionChanged',
    'onClosedCaptionsSettingsChanged',
    'onHighContrastUIChanged',
    'onVoiceGuidanceSettingsChanged',
  ]);

  // onAudioDescriptionChanged is accessed via listen('onAudioDescriptionChanged, ...)

  // onClosedCaptionsSettingsChanged is accessed via listen('onClosedCaptionsSettingsChanged, ...)

  // onHighContrastUIChanged is accessed via listen('onHighContrastUIChanged, ...)

  // onVoiceGuidanceSettingsChanged is accessed via listen('onVoiceGuidanceSettingsChanged, ...)

  // Methods
  function audioDescription() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Accessibility',
      'audioDescription',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }
  function clear$3(...args) {
    return Events.clear('Accessibility', ...args)
  }

  function closedCaptionsSettings() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Accessibility',
      'closedCaptionsSettings',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }
  function highContrastUI() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Accessibility',
      'highContrastUI',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }
  function listen$3(...args) {
    return Events.listen('Accessibility', ...args)
  }

  function once$3(...args) {
    return Events.once('Accessibility', ...args)
  }

  function voiceGuidanceSettings() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Accessibility',
      'voiceGuidanceSettings',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }

  var accessibility = {
    Events: {
      ON_AUDIO_DESCRIPTION_CHANGED: 'onAudioDescriptionChanged',
      ON_CLOSED_CAPTIONS_SETTINGS_CHANGED: 'onClosedCaptionsSettingsChanged',
      ON_HIGH_CONTRAST_UICHANGED: 'onHighContrastUIChanged',
      ON_VOICE_GUIDANCE_SETTINGS_CHANGED: 'onVoiceGuidanceSettingsChanged',
    },

    audioDescription,
    clear: clear$3,
    closedCaptionsSettings,
    highContrastUI,
    listen: listen$3,
    once: once$3,
    voiceGuidanceSettings,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  // Methods

  function advertisingId() {
    let params = {};

    return Bidirectional.request('Advertising.advertisingId', params)
  }

  var index$5 = {
    advertisingId,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  registerEvents('Device', ['onHdrChanged']);

  // onHdrChanged is accessed via listen('onHdrChanged, ...)

  // Methods
  function clear$2(...args) {
    return Events.clear('Device', ...args)
  }

  function deviceClass() {
    let params = {};

    return Bidirectional.request('Device.deviceClass', params)
  }
  function hdr() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop('Device', 'hdr', params, callbackOrValue, false, true, 0)
  }
  function listen$2(...args) {
    return Events.listen('Device', ...args)
  }

  function once$2(...args) {
    return Events.once('Device', ...args)
  }

  function uid() {
    let params = {};

    return Bidirectional.request('Device.uid', params)
  }

  var device = {
    Events: {
      ON_HDR_CHANGED: 'onHdrChanged',
    },

    /**
     * The type of device
     */
    DeviceClass: {
      OTT: 'ott',
      STB: 'stb',
      TV: 'tv',
    },

    clear: clear$2,
    deviceClass,
    hdr,
    listen: listen$2,
    once: once$2,
    uid,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  registerEvents('Localization', [
    'onCountryChanged',
    'onPreferredAudioLanguagesChanged',
    'onPresentationLanguageChanged',
  ]);

  // onCountryChanged is accessed via listen('onCountryChanged, ...)

  // onPreferredAudioLanguagesChanged is accessed via listen('onPreferredAudioLanguagesChanged, ...)

  // onPresentationLanguageChanged is accessed via listen('onPresentationLanguageChanged, ...)

  // Methods
  function clear$1(...args) {
    return Events.clear('Localization', ...args)
  }

  function country() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Localization',
      'country',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }
  function listen$1(...args) {
    return Events.listen('Localization', ...args)
  }

  function once$1(...args) {
    return Events.once('Localization', ...args)
  }

  function preferredAudioLanguages() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Localization',
      'preferredAudioLanguages',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }
  function presentationLanguage() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Localization',
      'presentationLanguage',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }

  var localization = {
    Events: {
      ON_COUNTRY_CHANGED: 'onCountryChanged',
      ON_PREFERRED_AUDIO_LANGUAGES_CHANGED: 'onPreferredAudioLanguagesChanged',
      ON_PRESENTATION_LANGUAGE_CHANGED: 'onPresentationLanguageChanged',
    },

    clear: clear$1,
    country,
    listen: listen$1,
    once: once$1,
    preferredAudioLanguages,
    presentationLanguage,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  // Methods

  function appInfo(build) {
    let params = { build };

    return Bidirectional.request('Metrics.appInfo', params)
  }

  function error(type, code, description, visible, parameters, agePolicy) {
    let params = { type, code, description, visible, parameters, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 2);

    return Bidirectional.request('Metrics.error', params)
  }

  function event(schema, data, agePolicy) {
    let params = { schema, data, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.event', params)
  }

  function mediaEnded(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaEnded', params)
  }

  function mediaLoadStart(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaLoadStart', params)
  }

  function mediaPause(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaPause', params)
  }

  function mediaPlay(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaPlay', params)
  }

  function mediaPlaying(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaPlaying', params)
  }

  function mediaRateChanged(entityId, rate, agePolicy) {
    let params = { entityId, rate, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaRateChanged', params)
  }

  function mediaRenditionChanged(
    entityId,
    bitrate,
    width,
    height,
    profile,
    agePolicy,
  ) {
    let params = { entityId, bitrate, width, height, profile, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 2);

    return Bidirectional.request('Metrics.mediaRenditionChanged', params)
  }

  function mediaSeeked(entityId, position, agePolicy) {
    let params = { entityId, position, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaSeeked', params)
  }

  function mediaSeeking(entityId, target, agePolicy) {
    let params = { entityId, target, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaSeeking', params)
  }

  function mediaWaiting(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.mediaWaiting', params)
  }

  function page(pageId, agePolicy) {
    let params = { pageId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 1);

    return Bidirectional.request('Metrics.page', params)
  }

  function ready() {
    let params = {};

    return Bidirectional.request('Metrics.ready', params)
  }

  function startContent(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 2);

    return Bidirectional.request('Metrics.startContent', params)
  }

  function stopContent(entityId, agePolicy) {
    let params = { entityId, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 2);

    return Bidirectional.request('Metrics.stopContent', params)
  }

  var index$4 = {
    /**
     *
     */
    ErrorType: {
      NETWORK: 'network',
      MEDIA: 'media',
      RESTRICTION: 'restriction',
      ENTITLEMENT: 'entitlement',
      OTHER: 'other',
    },

    appInfo,
    error,
    event,
    mediaEnded,
    mediaLoadStart,
    mediaPause,
    mediaPlay,
    mediaPlaying,
    mediaRateChanged,
    mediaRenditionChanged,
    mediaSeeked,
    mediaSeeking,
    mediaWaiting,
    page,
    ready,
    startContent,
    stopContent,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  registerEvents('Network', ['onConnectedChanged']);

  // onConnectedChanged is accessed via listen('onConnectedChanged, ...)

  // Methods
  function clear(...args) {
    return Events.clear('Network', ...args)
  }

  function connected() {
    let callbackOrValue = arguments[0];
    let params = {};

    // If there is only one parameter, it must be the callback.
    if (arguments.length === 1 && typeof arguments[0] === 'function') {
      callbackOrValue = arguments[0];
      params = {};
    }

    return Prop.prop(
      'Network',
      'connected',
      params,
      callbackOrValue,
      false,
      true,
      0,
    )
  }
  function listen(...args) {
    return Events.listen('Network', ...args)
  }

  function once(...args) {
    return Events.once('Network', ...args)
  }

  var index$3 = {
    Events: {
      ON_CONNECTED_CHANGED: 'onConnectedChanged',
    },

    clear,
    connected,
    listen,
    once,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  // Methods

  function watched(entityId, progress, completed, watchedOn, agePolicy) {
    let params = { entityId, progress, completed, watchedOn, agePolicy };

    // remove the null params if they are optional
    params = Bidirectional.removeNullOptionalParams(params, 4);

    return Bidirectional.request('Discovery.watched', params)
  }

  var index$2 = {
    watched,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  // public API
  var index$1 = {
    Localization: localization,
    Device: device,
    Accessibility: accessibility,
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  const prepLog = (type, args) => {
    const colors = {
      Info: 'green',
      Debug: 'gray',
      Warn: 'orange',
      Error: 'red',
    };

    args = Array.from(args);
    return [
      '%c' +
        (args.length > 1 && typeof args[0] === 'string' ? args.shift() : type),
      'background-color: ' +
        colors[type] +
        '; color: white; padding: 2px 4px; border-radius: 2px',
      args,
    ]
  };

  var index = {
    info() {
      Settings.get('platform', 'log') &&
        console.log.apply(console, prepLog('Info', arguments));
    },
    debug() {
      Settings.get('platform', 'log') &&
        console.debug.apply(console, prepLog('Debug', arguments));
    },
    error() {
      Settings.get('platform', 'log') &&
        console.error.apply(console, prepLog('Error', arguments));
    },
    warn() {
      Settings.get('platform', 'log') &&
        console.warn.apply(console, prepLog('Warn', arguments));
    },
  };

  /*
   * Copyright 2021 Comcast Cable Communications Management, LLC
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
   *
   * SPDX-License-Identifier: Apache-2.0
   */

  setMockResponses({
    Accessibility: _Accessibility,
    Advertising: _Advertising,
    Device: _Device,
    Localization: _Localization,
    Metrics: _Metrics,
    Network: _Network,
    Discovery: _Discovery,
    Platform: _Platform,
  });

  var firebolt = /*#__PURE__*/Object.freeze({
    __proto__: null,
    Accessibility: accessibility,
    Advertising: index$5,
    Device: device,
    Localization: localization,
    Metrics: index$4,
    Network: index$3,
    Discovery: index$2,
    Platform: index$1,
    Log: index,
    Events: Events,
    Settings: Settings
  });

  return index$6;

})();
//# sourceMappingURL=appBundle.js.map
