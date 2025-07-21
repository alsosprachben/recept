// AudioWorkletProcessor for ukulele string detection using recept.js primitives

import Complex from './complex.js';
import * as tau from './tau.js';
import { ExponentialSmoother, TimeSmoothing } from './recept.js';

// Utility class for transposing one sensor's complex value onto another
class Monochord {
  constructor(sourcePeriod, targetPeriod, ratio) {
    this.source_period = sourcePeriod;
    this.target_period = targetPeriod;
    this.ratio = ratio;
    this._construct();
  }

  _construct() {
    this.period = this.source_period * this.ratio;
    this.offset = this.target_period - this.period;
    this.phi_offset = this.offset / this.target_period;
    this.value = tau.rect(this.phi_offset);
  }

  rotate(cval) {
    const rot = new Complex(this.value.re, this.value.im);
    return cval.mul(rot);
  }
}

function complexDelta(cval, priorCval) {
  const a = cval.re, b = cval.im;
  const c = priorCval.re, d = priorCval.im;
  const denom = c*c + d*d;
  if (denom === 0) {
    return new Complex(0, 0);
  }
  // (a+bi)/(c+di) = [(a c + b d) + i(b c - a d)] / (c² + d²)
  return new Complex(
    (a*c + b*d) / denom,
    (b*c - a*d) / denom
  );
}

class Delta {
  constructor() {
    this.priorSequence = null;
  }
  sample(value) {
    let delta = null;
    if (this.priorSequence !== null) {
      delta = value - this.priorSequence;
    }
    this.priorSequence = value;
    return delta;
  }
}

class PhaseDelta {
  constructor() {
    this.priorAngle = null;
  }
  sample(angleValue) {
    let delta = null;
    if (this.priorAngle !== null) {
      delta = complexDelta(angleValue, this.priorAngle);
    }
    this.priorAngle = angleValue;
    return delta;
  }
}

class Lifecycle {
  constructor(max_r = 1.0) {
    this.max_r = max_r;
    this.cval = new Complex(0, 0);
    this.energy = 0;
    this.entropy = 0;
    this.F = 0; // Free Energy: F = energy - entropy
    this.B = 0; // Bound Energy: B = entropy (energy - F)
    this.r = 0;
    this.phi = 0;
    this.cycle = 0;
    this.lifecycle = 0;
  }
  sample(cval) {
    this.cval = cval;
    this.entropy = cval.re;
    this.energy = cval.im;
    this.F = this.energy - this.entropy; // F = energy - entropy
    this.B = this.entropy;               // B = entropy (U - F)
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
  constructor(max_r = 1.0, response_factor = 100) {
    super(max_r);
    this.response_factor = response_factor;
    this.d_avg_state = new ExponentialSmoother(0.0);
    this.dd_avg_state = new ExponentialSmoother(0.0);
    this.d = 0;
    this.dd = 0;
  }
  sample(v1, v2, v3) {
    /*
    const d1 = v2 - v1;
    const d2 = v3 - v2;
    const dd = d2 - d1;
    */
    const d1 = v1 - v2;
    const d2 = v2 - v3;
    const dd = d1 - d2;
    this.d = d1;
    this.dd = dd;
    this.d_avg = this.d_avg_state.sample(d1, this.response_factor);
    this.dd_avg = this.dd_avg_state.sample(dd, this.response_factor);
    const c = new Complex(this.d_avg, this.dd_avg);
    return super.sample(c);
  }
}

class IterLifecycle extends Lifecycle {
  constructor(max_r = 1.0) {
    super(max_r);
    this.dState = new Delta();
    this.ddState = new Delta();
    this.d = 0.0;
    this.dd = 0.0;
    this.c = new Complex(0, 0);
  }

  sample(value) {
    let d = this.dState.sample(value);
    if (d == null) d = 0.0;
    this.d = d;

    let dd = this.ddState.sample(d);
    if (dd == null) dd = 0.0;
    this.dd = dd;

    this.c = new Complex(this.d, this.dd);
    return super.sample(this.c);
  }
}

class PeriodSensor {
  constructor(period, phase, windowFactor = 1.0) {
    this.period = period;
    this.phase = phase;
    this.smoother = new TimeSmoothing(period, phase, windowFactor, new Complex(0, 0));
    this.cval = new Complex(0, 0);
    this.r = 0;
    this.phaseDelta = new PhaseDelta();
    this.phi_t = 0;
    this.instant_frequency = 0;
  }
  sample(time, value) {
    const [, cval] = this.smoother.sample(time, value);
    this.cval = cval;
    const [r] = tau.polar(cval);
    this.r = r;

    const delta = this.phaseDelta.sample(cval);
    if (delta !== null) {
      const [, dphi] = tau.polar(delta);
      this.phi_t = dphi;
    } else {
      this.phi_t = 0;
    }
    this.instant_frequency = (1 / this.period) - this.phi_t;
    return cval;
  }

  addCval(cval) {
    this.cval = this.cval.add(cval);
    const [r] = tau.polar(this.cval);
    this.r = r;
  }
}

class PeriodScaleSpaceSensor {
  constructor(period, phase, response_period, scale_factor = 1.75, period_factor = 1.0, sample_rate = 1.0) {
    this.period = period;
    this.phase = phase;
    this.response_period = response_period;
    this.scale_factor = scale_factor;
    this.sample_rate = sample_rate;
    this.period_sensors = [
      new PeriodSensor(period, phase, period_factor * Math.pow(scale_factor, -1)),
      new PeriodSensor(period, phase, period_factor * Math.pow(scale_factor, -2)),
      new PeriodSensor(period, phase, period_factor * Math.pow(scale_factor, -3))
    ];
    this.period_lifecycle = new DeriveLifecycle(period, response_period);
    this.beat_lifecycle = new IterLifecycle(period);
    this.monochords = [];
  }

  sampleSensor(time, value) {
    for (const ps of this.period_sensors) {
      ps.sample(time, value);
    }
  }

  sampleMonochords() {
    for (const { source, mc } of this.monochords) {
      for (let i = 0; i < this.period_sensors.length; i++) {
        const rotated = mc.rotate(source.period_sensors[i].cval);
        this.period_sensors[i].addCval(rotated);
      }
    }
  }

  sampleLifecycle() {
    const r = this.period_sensors.map(ps => {
      const [mag] = tau.polar(ps.cval);
      ps.r = mag;
      return mag;
    });
    this.period_lifecycle.sample(r[0], r[1], r[2]);
    this.beat_lifecycle.sample(this.period_lifecycle.lifecycle);
    const phi_t = this.period_sensors[0].phi_t;
    const instFreqCycles = (1 / this.period) - phi_t;
    this.period_lifecycle.instant_frequency = instFreqCycles * this.sample_rate;
  }

  sample(time, value) {
    this.sampleSensor(time, value);
    this.sampleMonochords();
    this.sampleLifecycle();
  }

  getMonochord(targetSensor, ratio) {
    return new Monochord(this.period, targetSensor.period, ratio);
  }

  superimposeMonochordOn(targetSensor, mc) {
    for (let i = 0; i < this.period_sensors.length; i++) {
      const rotated = mc.rotate(this.period_sensors[i].cval);
      targetSensor.period_sensors[i].addCval(rotated);
    }
  }

  addMonochord(sourceSensor, ratio) {
    this.monochords.push({ source: sourceSensor, mc: sourceSensor.getMonochord(this, ratio) });
  }
}

class UkePeriodArray {
  constructor(sample_rate, octaveBandwidth = 12, bandwidthFactor = 1.0) {
    this.sample_rate = sample_rate;
    // sensitivity parameters
    this.octaveBandwidth = octaveBandwidth;
    this.bandwidthFactor = bandwidthFactor;
    const resp = sample_rate / 60; // smoothing interval (Hz)
    const cycleArea = 1.0 / (1.0 - Math.exp(-1.0)); // match C's cycle_area
    // derive period bandwidth from octave bandwidth
    const periodBandwidth = cycleArea / (Math.pow(2.0, 1.0 / octaveBandwidth) - 1.0);
    const C4 = 440 * Math.pow(2, (-12 + 3) / 12);
    const ratio_M3rd = 5.0 / 4.0;
    const ratio_5th = 3.0 / 2.0;
    const ratio_M6th = 5.0 / 3.0;
    const G4 = C4 * ratio_5th;
    const E4 = C4 * ratio_M3rd;
    const A4 = C4 * ratio_M6th;
    // construct sensors with bandwidth-scaled period factor
    this.sensors = [
      new PeriodScaleSpaceSensor(sample_rate / C4, 0, resp, cycleArea, periodBandwidth * bandwidthFactor, sample_rate),
      new PeriodScaleSpaceSensor(sample_rate / E4, 0, resp, cycleArea, periodBandwidth * bandwidthFactor, sample_rate),
      new PeriodScaleSpaceSensor(sample_rate / G4, 0, resp, cycleArea, periodBandwidth * bandwidthFactor, sample_rate),
      new PeriodScaleSpaceSensor(sample_rate / A4, 0, resp, cycleArea, periodBandwidth * bandwidthFactor, sample_rate)
    ];
    // Mix the C string into the higher strings using monochords
    this.sensors[1].addMonochord(this.sensors[0], ratio_M3rd);
    this.sensors[2].addMonochord(this.sensors[0], ratio_5th);
    this.sensors[3].addMonochord(this.sensors[0], ratio_M6th);
    this.names = ['C4', 'E4', 'G4', 'A4'];
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

class GuitarPeriodArray {
  constructor(sampleRate, octaveBandwidth = 12, bandwidthFactor = 1.0) {
    this.sampleRate = sampleRate
    this.octaveBandwidth = octaveBandwidth
    this.bandwidthFactor = bandwidthFactor

    // smoothing interval (Hz)
    const resp = sampleRate / 60
    // match C's cycle_area
    const cycleArea = 1.0 / (1.0 - Math.exp(-1.0))
    // derive period bandwidth from octave bandwidth
    const periodBandwidth = cycleArea / (Math.pow(2.0, 1.0 / octaveBandwidth) - 1.0)

    // standard guitar tuning: E2, A2, D3, G3, B3, E4
    const stringMidis = [40, 45, 50, 55, 59, 64]
    this.names = ['E2', 'A2', 'D3', 'G3', 'B3', 'E4']

    this.sensors = stringMidis.map(midi => {
      const freq = 440 * Math.pow(2, (midi - 69) / 12)
      const period = sampleRate / freq
      return new PeriodScaleSpaceSensor(
        period, 0, resp, cycleArea, periodBandwidth * bandwidthFactor, sampleRate
      )
    })

    // superimpose the low E string (index 0) onto all others via monochords
    for (let i = 1; i < this.sensors.length; i++) {
      const semitoneDiff = stringMidis[i] - stringMidis[0]
      const ratio = Math.pow(2, semitoneDiff / 12)
      this.sensors[i].addMonochord(this.sensors[0], ratio)
    }
  }

  sample(time, value) {
    for (const s of this.sensors) {
      s.sample(time, value)
    }
  }

  values() {
    return this.sensors.map(s => s.period_lifecycle)
  }
}

class HarpsichordPeriodArray {
  constructor(sampleRate, lowMidi = 24, highMidi = 96, octaveBandwidth = 12, bandwidthFactor = 1.0) {
    this.sampleRate = sampleRate;
    // sensitivity parameters
    this.octaveBandwidth = octaveBandwidth;
    this.bandwidthFactor = bandwidthFactor;
    // smoothing interval (Hz)
    const resp = sampleRate / 60;
    // match C's cycle_area from recept.js
    const cycleArea = 1.0 / (1.0 - Math.exp(-1.0));
    // derive period bandwidth from octave bandwidth
    const periodBandwidth = cycleArea / (Math.pow(2.0, 1.0 / octaveBandwidth) - 1.0);

    this.sensors = [];
    this.names = [];
    for (let midi = lowMidi; midi <= highMidi; midi++) {
      // equal-tempered frequency: A4 = MIDI 69 = 440Hz
      const freq = 440 * Math.pow(2, (midi - 69) / 12);
      const period = sampleRate / freq;
      // sensor with bandwidth-scaled period factor
      this.sensors.push(
        new PeriodScaleSpaceSensor(period, 0, resp, cycleArea, periodBandwidth * bandwidthFactor, sampleRate)
      );
      this.names.push(`MIDI ${midi}`);
    }
  }

  sample(time, value) {
    for (const sensor of this.sensors) {
      sensor.sample(time, value);
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
    const sr = globalThis.sampleRate;  // Worklet global audio context sample rate
    console.log(`UkeProcessor initialized with sample rate: ${sr}`);
    this.guitar = new GuitarPeriodArray(sr, 12); // 12 semitones per octave
    // this.uke = new UkePeriodArray(sr);
    // this.harpsichord = new HarpsichordPeriodArray(sr, 48, 52); //24, 96); // MIDI range for harpsichord
    this.sampleRate = sr;
    this._updateIntervalInMS = (options.processorOptions && options.processorOptions.updateIntervalInMS) || 16.67;
    this._nextUpdateFrame = 0;
    this.frame = 0;
    this.current_time = 0;
    this.start_time = 0;
    this.sample_n = 0;
    this.logical_sample_time = 0;
    this.physical_sample_time = 0;
    // receive current_time from main thread
    this.port.onmessage = (ev) => {
      if (ev.data.type === 'current_time') {
        this.current_time = ev.data.time;
        if (this.start_time === 0) {
          this.start_time = this.current_time;
        }
        //console.log(`Current time updated: ${this.current_time}`);
      }
    };
    this.port.start();
  }

  get intervalInFrames() {
    // compute update interval in frames
    return this._updateIntervalInMS / 1000 * this.sampleRate;
  }

  process(inputs, outputs, parameters) {
    const input = inputs[0];
    const channel = input[0];
    this.sample_n += channel.length;
    this.logical_sample_time = this.sample_n / this.sampleRate + this.start_time;

    // if current time is past the sample time, return true
    if (this.logical_sample_time < this.current_time + 0.1) {
      console.log(`Ahead of time by ${this.current_time - this.logical_sample_time} seconds`);
      return true;
    }
    if (input.length === 0) return true;
    for (let i = 0; i < input[0].length; ++i) {
      const t = this.frame; // use sample index to match period units
      // average all input channels
      let sample = 0.0;
      for (let j = 0; j < inputs.length; ++j) {
        sample += input[j][i];
      }
      sample /= inputs.length; // average across channels
      // normalize between 0 and 1 by dividing by the max sample value, using the worklet bit depth
      sample *= 32768.0; // assuming 16-bit audio input

      this.guitar.sample(t, sample); // scale to match python example
      //this.uke.sample(t, sample); // scale to match python example
      //this.harpsichord.sample(t, sample); // scale to match python example
      this.frame += 1;
    }

    this._nextUpdateFrame -= channel.length;
    if (this._nextUpdateFrame < 0) {
      this._nextUpdateFrame += this.intervalInFrames;
      
      const names = this.guitar.names;
      const values = this.guitar.values();

      // const names = this.uke.names;
      // const values = this.uke.values();
      
      // const names = this.harpsichord.names;
      // const values = this.harpsichord.values();
      
      const dict = {};
      for (let i = 0; i < names.length; i++) {
        dict[names[i]] = values[i];
      }
      this.port.postMessage({ type: 'receptions', values: dict });
    }
    return true;
  }
}

registerProcessor('uke-processor', UkeProcessor);
