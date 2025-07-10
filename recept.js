// JavaScript port of recept.py. This is a partial port focusing on the
// signal processing utilities used by other modules.

import * as bar from './bar.js';
import * as tau from './tau.js';
import Complex from './complex.js';

function complexDelta(cval, priorCval) {
  if (priorCval.re !== 0 || priorCval.im !== 0) {
    return cval.div(priorCval);
  } else {
    return new Complex(0, 0);
  }
}

class ExponentialSmoother {
  constructor(initialValue = 0.0) {
    this.v = initialValue;
  }
  sample(value, factor) {
    // Support smoothing of plain numbers and Complex values.
    if (value instanceof Complex ||
        (value && typeof value === 'object' && 're' in value && 'im' in value)) {
      const re = this.v.re !== undefined ? this.v.re : 0;
      const im = this.v.im !== undefined ? this.v.im : 0;
      const vre = re + (value.re - re) / factor;
      const vim = im + (value.im - im) / factor;
      this.v = new Complex(vre, vim);
    } else {
      this.v += (value - this.v) / factor;
    }
    return this.v;
  }
  toString() {
    if (this.v instanceof Complex ||
        (this.v && typeof this.v === 'object' && 're' in this.v && 'im' in this.v)) {
      return `${this.v.re.toFixed(3)}+${this.v.im.toFixed(3)}j`;
    }
    return this.v.toFixed(3);
  }
}

class ExponentialSmoothing {
  constructor(windowSize, initialValue = 0.0) {
    this.w = windowSize;
    this.v = new ExponentialSmoother(initialValue);
  }
  sample(value) {
    return this.v.sample(value, this.w);
  }
  toString() {
    return `{ave=${this.v}, window=${this.w.toFixed(3)}}`;
  }
}

class Delta {
  constructor(priorSequence = null) {
    this.prior_sequence = priorSequence;
  }
  sample(sequenceValue) {
    let deltaValue = null;
    if (this.prior_sequence !== null) {
      deltaValue = sequenceValue - this.prior_sequence;
    }
    this.prior_sequence = sequenceValue;
    return deltaValue;
  }
}

class PhaseDelta {
  constructor(priorAngle = null) {
    this.prior_angle = priorAngle; // Complex number
  }
  sample(angleValue) {
    let deltaValue = null;
    if (this.prior_angle) {
      deltaValue = complexDelta(angleValue, this.prior_angle);
    }
    this.prior_angle = angleValue;
    return deltaValue;
  }
}

class Distribution {
  constructor(initialValue = 0.0, priorSequence = 0.0) {
    this.ave = new ExponentialSmoother(initialValue);
    this.dev = new ExponentialSmoother(priorSequence);
  }
  sample(value, factor) {
    const deviation = Math.abs(this.ave.v - value);
    this.ave.sample(value, factor);
    this.dev.sample(deviation, factor);
    return this.dist();
  }
  dist() {
    return [this.ave.v, this.dev.v];
  }
  toString() {
    const [a, d] = this.dist();
    return `{mean=${a.toFixed(3)} dev=${d.toFixed(3)}}`;
  }
}

class WeightedDistribution extends Distribution {
  constructor(windowSize = 1.0, initialValue = 0.0, priorSequence = null) {
    super(initialValue, priorSequence);
    this.w = windowSize;
  }
  sample(value) {
    return super.sample(value, this.w);
  }
}

class Apex extends Delta {
  constructor(priorValue = null) {
    super(priorValue);
    this.prior_is_positive = true;
  }
  sample(value) {
    const d = super.sample(value);
    const is_positive = d >= 0;
    if (this.prior_is_positive !== is_positive) {
      this.prior_is_positive = is_positive;
      return value;
    }
    return null;
  }
}

class TimeApex {
  constructor(priorValue = null, priorSequence = null) {
    this.apex = new Apex(priorValue);
    this.delta = new Delta(priorSequence);
  }
  sample(time, value) {
    const apex = this.apex.sample(value);
    if (apex === null) {
      return [null, null];
    } else {
      return [apex, this.delta.sample(time)];
    }
  }
}

class SignDelta {
  constructor(priorValue = null) {
    this.prior_value = priorValue;
  }
  sampleCmp(value) {
    const s = Math.sign(value - this.prior_value);
    this.prior_value = value;
    return s;
  }
  sampleStart(value) {
    return this.sampleCmp(value) === 1;
  }
  sampleStop(value) {
    return this.sampleCmp(value) === -1;
  }
}

class Sign {
  static sample(value) {
    return value > 0;
  }
}

class DynamicWindow {
  constructor(targetDuration, windowSize, priorValue = null, initialDuration = 0.0) {
    this.td = targetDuration;
    this.s = new Delta(priorValue);
    this.ed = new ExponentialSmoothing(windowSize, initialDuration);
  }
  sample(sequenceValue) {
    const duration_since = this.s.sample(sequenceValue);
    if (duration_since === null) {
      return this.td;
    }
    const expected_duration = this.ed.sample(duration_since);
    return this.td / expected_duration;
  }
  toString() {
    const w = (this.ed.v.v > 0.0) ? (this.td / this.ed.v.v) : 0.0;
    return `{window=${w.toFixed(3)}, duration=${this.ed}, target=${this.td}}`;
  }
}

class SmoothDuration {
  constructor(targetDuration, windowSize, priorValue = null, initialDuration = 0.0, initialValue = 0.0) {
    this.dw = new DynamicWindow(targetDuration, windowSize, priorValue, initialDuration);
    this.v = new ExponentialSmoother(initialValue);
  }
  sample(value, sequenceValue) {
    const w = this.dw.sample(sequenceValue);
    return this.v.sample(value, w);
  }
  toString() {
    return `{ave=${this.v}. window=${this.dw}}`;
  }
}

class SmoothDurationDistribution {
  constructor(targetDuration, windowSize, priorValue = null, initialDuration = 0.0, initialValue = 0.0, priorSequence = 0.0) {
    this.dw = new DynamicWindow(targetDuration, windowSize, priorValue, initialDuration);
    this.v = new Distribution(initialValue, priorSequence);
  }
  sample(value, sequenceValue) {
    const w = this.dw.sample(sequenceValue);
    return this.v.sample(value, w);
  }
  toString() {
    return `{dist=${this.v}, window=${this.dw}}`;
  }
}

class TimeSmoothing {
  constructor(period, phase, windowFactor = 1.0, initialValue = new Complex(0, 0)) {
    this.period = period;
    this.phase = phase;
    this.v = new ExponentialSmoother(initialValue);
    this.wf = windowFactor;
  }
  sample(time, value) {
    const rect = tau.rect((time + this.phase) / this.period);
    const val = new Complex(value, 0).mul(new Complex(rect.re, rect.im));
    const cval = this.v.sample(val, this.period * this.wf);
    return [1.0, cval];
  }
}

class DynamicTimeSmoothing extends TimeSmoothing {
  constructor(period, phase, windowFactor = 1.0, initialValue = new Complex(0, 0), initialDelta = 0.0) {
    super(period, phase, windowFactor, initialValue);
    this.glissando = new ExponentialSmoother(initialDelta);
  }
  updatePeriod(period) {
    const glissando_value = this.glissando.sample(period - this.period, period * this.wf);
    this.phase = (this.phase / this.period) * period;
    this.period = period;
    return glissando_value;
  }
  updatePhase(phase) {
    this.phase = phase;
  }
  sample(time, value, period = null) {
    let glissando_value;
    if (period !== null) {
      glissando_value = this.updatePeriod(period);
    } else {
      glissando_value = this.glissando.v;
    }
    const [delta, time_value] = super.sample(time, value);
    return [delta, time_value, glissando_value];
  }
}

class ApexTimeSmoothing extends DynamicTimeSmoothing {
  constructor(period, phase, windowFactor = 1.0, initialValue = new Complex(0, 0), initialDelta = 0.0, priorValue = null, priorSequence = null) {
    super(period, phase, windowFactor, initialValue, initialDelta);
    this.time_apex = new TimeApex(priorValue, priorSequence);
  }
  sample(time, value, period = null) {
    const glissando_value = period !== null ? this.updatePeriod(period) : this.glissando.v;
    const [apex, time_delta] = this.time_apex.sample(time, value);
    if (apex !== null) {
      const td = time_delta === null ? 1.0 : time_delta;
      const rect = tau.rect((time + this.phase) / this.period);
      const cval = this.v.sample(new Complex(value, 0).mul(new Complex(rect.re, rect.im)), this.period * this.wf);
      return [td, cval, glissando_value];
    }
    return [null, null, null];
  }
}

export {
  complexDelta,
  ExponentialSmoother,
  ExponentialSmoothing,
  Delta,
  PhaseDelta,
  Distribution,
  WeightedDistribution,
  Apex,
  TimeApex,
  SignDelta,
  Sign,
  DynamicWindow,
  SmoothDuration,
  SmoothDurationDistribution,
  TimeSmoothing,
  DynamicTimeSmoothing,
  ApexTimeSmoothing,
};
