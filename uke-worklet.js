// AudioWorkletProcessor for ukulele string detection using recept.js primitives

import Complex from './complex.js';
import * as tau from './tau.js';
import { ExponentialSmoother, TimeSmoothing } from './recept.js';

class Lifecycle {
  constructor(max_r = 1.0) {
    this.max_r = max_r;
    this.cval = new Complex(0, 0);
    this.F = 0;
    this.r = 0;
    this.phi = 0;
    this.cycle = 0;
    this.lifecycle = 0;
  }
  sample(cval) {
    this.cval = cval;
    this.F = cval.re - cval.im;
    const [r, phi] = tau.polar(cval);
    const prevPhi = this.phi;
    this.r = r;
    this.phi = phi;
    if (this.phi - prevPhi > 0.5) {
      this.cycle -= 1;
    } else if (this.phi - prevPhi < -0.5) {
      this.cycle += 1;
    }
    this.lifecycle = this.cycle + this.phi;
    return this.lifecycle;
  }
}

class DeriveLifecycle extends Lifecycle {
  constructor(max_r = 1.0, response_factor = 1000) {
    super(max_r);
    this.response_factor = response_factor;
    this.d_avg_state = new ExponentialSmoother(0.0);
    this.dd_avg_state = new ExponentialSmoother(0.0);
    this.d = 0;
    this.dd = 0;
  }
  sample(v1, v2, v3) {
    const d1 = v2 - v1;
    const d2 = v3 - v2;
    const dd = d2 - d1;
    this.d = this.d_avg_state.sample(d1, this.response_factor);
    this.dd = this.dd_avg_state.sample(dd, this.response_factor);
    const c = new Complex(this.d, this.dd);
    return super.sample(c);
  }
}

class PeriodSensor {
  constructor(period, phase, windowFactor = 1.0) {
    this.period = period;
    this.smoother = new TimeSmoothing(period, phase, windowFactor, new Complex(0, 0));
    this.r = 0;
  }
  sample(time, value) {
    const [, cval] = this.smoother.sample(time, value);
    const [r] = tau.polar(cval);
    this.r = r;
    return r;
  }
}

class PeriodScaleSpaceSensor {
  constructor(period, phase, response_period, scale_factor = 1.75, period_factor = 1.0) {
    this.period_sensors = [
      new PeriodSensor(period, phase, period_factor * Math.pow(scale_factor, -1)),
      new PeriodSensor(period, phase, period_factor * Math.pow(scale_factor, -2)),
      new PeriodSensor(period, phase, period_factor * Math.pow(scale_factor, -3))
    ];
    this.period_lifecycle = new DeriveLifecycle(period, response_period);
  }
  sample(time, value) {
    const r = this.period_sensors.map(ps => ps.sample(time, value));
    this.period_lifecycle.sample(r[0], r[1], r[2]);
  }
}

class UkePeriodArray {
  constructor(sample_rate) {
    this.sample_rate = sample_rate;
    const resp = sample_rate / 20; // response period like python example
    const C4 = 440 * Math.pow(2, (-12 + 3) / 12);
    const ratio_M3rd = 5.0 / 4.0;
    const ratio_5th = 3.0 / 2.0;
    const ratio_M6th = 5.0 / 3.0;
    const G4 = C4 * ratio_5th;
    const E4 = C4 * ratio_M3rd;
    const A4 = C4 * ratio_M6th;
    this.sensors = [
      new PeriodScaleSpaceSensor(sample_rate / C4, 0, resp),
      new PeriodScaleSpaceSensor(sample_rate / E4, 0, resp),
      new PeriodScaleSpaceSensor(sample_rate / G4, 0, resp),
      new PeriodScaleSpaceSensor(sample_rate / A4, 0, resp)
    ];
  }
  sample(time, value) {
    for (const s of this.sensors) {
      s.sample(time, value);
    }
  }
  values() {
    return this.sensors.map(s => s.period_lifecycle);
  }
}

const SMOOTHING_FACTOR = 0.5;

class UkeProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    this.uke = new UkePeriodArray(sampleRate);
    this._updateIntervalInMS = (options.processorOptions && options.processorOptions.updateIntervalInMS) || 16.67;
    this._nextUpdateFrame = 0;
    this.frame = 0;
    this.port.start();
  }

  get intervalInFrames() {
    return this._updateIntervalInMS / 1000 * sampleRate;
  }

  process(inputs, outputs, parameters) {
    const input = inputs[0];
    if (input.length === 0) return true;
    const channel = input[0];
    for (let i = 0; i < channel.length; ++i) {
      const t = this.frame / sampleRate;
      this.uke.sample(t, channel[i]);
      this.frame += 1;
    }

    this._nextUpdateFrame -= channel.length;
    if (this._nextUpdateFrame < 0) {
      this._nextUpdateFrame += this.intervalInFrames;
      const values = this.uke.values().map(lc => ({
        F: lc.F,
        entropy: lc.cval.re,
        neg_energy: lc.cval.im,
        phase: lc.phi,
        amp: lc.r,
        onset_amp: lc.phi < 0 ? lc.r : 0,
        cycle: lc.lifecycle,
      }));
      this.port.postMessage({ values });
    }
    return true;
  }
}

registerProcessor('uke-processor', UkeProcessor);
