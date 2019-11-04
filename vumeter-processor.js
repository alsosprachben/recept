/* vumeter-processor.js: AudioWorkletGlobalScope */
const SMOOTHING_FACTOR = 0.9;
const MINIMUM_VALUE = 0.00001;
class PeriodArray extends AudioWorkletProcessor {
  constructor (options) {
    super();
    this._volume = 0;
    this._updateIntervalInMS = options.processorOptions.updateIntervalInMS;
    this._nextUpdateFrame = 0;
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
    if (input.length > 0) {
      const samples = input[0];
      let sum = 0;
      let rms = 0;
      // Calculated the squared-sum.
      for (let i = 0; i < samples.length; ++i)
        sum += samples[i] * samples[i];
      // Calculate the RMS level and update the volume.
      rms = Math.sqrt(sum / samples.length);
      this._volume = Math.max(rms, this._volume * SMOOTHING_FACTOR);
      // Update and sync the volume property with the main thread.
      this._nextUpdateFrame -= samples.length;
      if (this._nextUpdateFrame < 0) {
        this._nextUpdateFrame += this.intervalInFrames;
        this.port.postMessage({volume: this._volume});
      }
    }
    // Keep on processing if the volume is above a threshold, so that
    // disconnecting inputs does not immediately cause the meter to stop
    // computing its smoothed value.
    //return this._volume >= MINIMUM_VALUE;
    return true;
  }
}
registerProcessor('PeriodArray', PeriodArray);
