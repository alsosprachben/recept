/* vumeter-processor.js: AudioWorkletGlobalScope */
const SMOOTHING_FACTOR = 0.5;
const MINIMUM_VALUE = 0.00001;
class PeriodArray extends AudioWorkletProcessor {
  constructor (options) {
    super();
    this._volume_left = 0;
    this._volume_right = 0;
    this._updateIntervalInMS = options.processorOptions.updateIntervalInMS;
    this._nextUpdateFrame = 0;

    /*
    self._sampleArray = new Float64Array(128);
    self._sampleWasmBuf = Module._malloc(self._sampleArray.BYTES_PER_ELEMENT * 128);
    self._sampleWasmProcess = Module.cwrap("process_rms", "number", ["number", null, "number"]);
    */

    this.port.onmessage = event => {
      if (event.data.updateIntervalInMS)
        this._updateIntervalInMS = event.data.updateIntervalInMS;
    }
    this.port.start();
  }
  get intervalInFrames () {
    return this._updateIntervalInMS / 1000 * sampleRate;
  }
  process (inputs, outputs, parameters) {
    const input = inputs[0];
    // Note that the input will be down-mixed to mono; however, if no inputs are
    // connected then zero channels will be passed in.
    if (input.length >= 2) {
      const samples_left  = input[0];
      const samples_right = input[1];

      /*
      for (let i = 0; i < 128; ++i) {
        self._sampleArray[i] = samples_left;
      }
      Module.HEAPF64.set(self._sampleArray, self._sampleWasmBuf >> 3);
      this._volume_left = this._sampleWasmProcess(this._volume_left, self._sampleWasmBuf, 128);

      for (let i = 0; i < 128; ++i) {
        self._sampleArray[i] = samples_right;
      }
      Module.HEAPF64.set(self._sampleArray, self._sampleWasmBuf >> 3);
      this._volume_right = this._sampleWasmProcess(this._volume_right, self._sampleWasmBuf, 128);
      */

      /**/
      let sum = 0;
      let rms = 0;

      // Calculated the squared-sum.
      for (let i = 0; i < samples_left.length; ++i)
        sum += samples_left[i] * samples_left[i];
      // Calculate the RMS level and update the volume.
      rms = Math.sqrt(sum / samples_left.length);
      this._volume_left = Math.max(rms, this._volume_left * SMOOTHING_FACTOR);

      // Calculated the squared-sum.
      for (let i = 0; i < samples_right.length; ++i)
        sum += samples_right[i] * samples_right[i];
      // Calculate the RMS level and update the volume.
      rms = Math.sqrt(sum / samples_right.length);
      this._volume_right = Math.max(rms, this._volume_right * SMOOTHING_FACTOR);
      /**/

      // Update and sync the volume property with the main thread.
      this._nextUpdateFrame -= samples_left.length;
      if (this._nextUpdateFrame < 0) {
        this._nextUpdateFrame += this.intervalInFrames;
        this.port.postMessage({volume_left: this._volume_left, volume_right: this._volume_right});
      }
    }
    // Keep on processing if the volume is above a threshold, so that
    // disconnecting inputs does not immediately cause the meter to stop
    // computing its smoothed value.
    //return this._volume_left >= MINIMUM_VALUE;
    return true;
  }
}
registerProcessor('PeriodArray', PeriodArray);
