#!/usr/bin/env python

"""
Infinite Impulse Response (IIR) filters for periodic analysis
"""

import bar

"""
Pre-computed Windows
"""

class Window:
	"""
	Repeating Iterator over a specified periodic window.
	"""

	def __init__(self, window):
		self.w = window
		self.i = 0
		self.n = len(window)

	def next(self):
		"return the next window element"
		v = self.w[self.i]
		self.i = (self.i + 1) % self.n
		return v

class CosineWindow(Window):
	"""
	A window of the cosine at the specified period and phase.
	"""

	def __init__(self, period, phase):
		from math import cos, pi
		self.i = 0
		self.n = period
		self.w = [cos(2.0 * pi * ((float(i) + phase) / period)) for i in range(period)]

class SineWindow(CosineWindow):
	"""
	A window of the sine at the specified period and phase.
	"""

	def __init__(self, period, phase):
		CosineWindow.__init__(self, period, phase + (float(period) / 4))

class PeriodicSmoothing:
	"""
	Sense a specified periodic window of an integral size.
	"""

	def __init__(self, window = [1.0, -1.0], window_factor = 1, initial_value = 0.0):
		self.w = WIndow(window)
		self.wf = window_factor
		self.v = ExponentialSmoother(initial_value)

	def sample(self, value):
		return self.v.sample(value * self.w.next(), self.w.n * self.wf)

class FrequencySmoothing:
	"""
	Sense a particular period and phase, using precomputed windows of integral size.
	"""

	def __init__(self, period, phase, window_factor = 1, real_initial_value = 0.0, imag_initial_value = 0.0):
		self.real_w = CosineWindow(period, phase)
		self.imag_w =   SineWindow(period, phase)
		self.real_v = ExponentialSmoother(real_initial_value)
		self.imag_v = ExponentialSmoother(imag_initial_value)
		self.wf = window_factor

	def sample(self, value):
		return (
			self.real_v.sample(value * self.real_w.next(), self.real_w.n * self.wf)
		+ 1j *	self.imag_v.sample(value * self.imag_w.next(), self.imag_w.n * self.wf)
		)

class MovingAverage:
	def __init__(self, period):
		from collections  import deque
		self.period = period
		self.window = deque([0.0] * self.period, self.period)
		self.avg = 0.0

	def sample(self, value):
		self.avg += value - self.window.popleft()

		self.window.append(value)
		return self.avg / self.period
	
"""
Real-Time, Infinite, Dynamic Windows
"""

complex_delta = lambda cval, prior_cval: cval / prior_cval if prior_cval != 0.0 else 0.0

class ExponentialSmoother:
	"""
	Exponential Smoothing
	"""

	def __init__(self, initial_value = 0.0):
		self.v = initial_value

	def sample(self, value, factor):
		self.v += (value - self.v) / factor
		return self.v

	def __str__(self):
		return "%08.3f" % self.v

class ExponentialSmoothing:
	"""
	`ExponentialSmoother` of a fixed window size.
	"""

	def __init__(self, window_size, initial_value = 0.0):
		self.w = window_size
		self.v = ExponentialSmoother(initial_value)

	def sample(self, value):
		return self.v.sample(value, self.w)

	def __str__(self):
		return "{ave=%s, window=%08.3f}" % (self.v, self.w)

class Delta:
	"""
	Derivative of a sequence of real numbers.
	"""

	def __init__(self, prior_sequence = None):
		self.prior_sequence = prior_sequence

	def sample(self, sequence_value):
		if self.prior_sequence is None:
			delta_value = None
		else:
			delta_value = sequence_value - self.prior_sequence

		self.prior_sequence = sequence_value
		return delta_value


class PhaseDelta:
	"""
	Derivative of the phase of a complex number. That is, divide the complex sample against the previous. 
	"""

	def __init__(self, prior_angle = None):
		"`prior_angle` must be a complex number"
		self.prior_angle = prior_angle

	def sample(self, angle_value):
		"Both the param and return values are complex numbers."
		from cmath import phase
		if self.prior_angle is None:
			delta_value = None
		else:
			delta_value = complex_delta(angle_value, self.prior_angle)

		self.prior_angle = angle_value
		return delta_value

class Distribution:
	"""
	Infinite Impulse Response distribution, represented by exponentially smoothed average and deviation.
	"""

	def __init__(self, initial_value = 0.0, prior_sequence = None):
		self.ave = ExponentialSmoother(initial_value) # mean average
		self.dev = ExponentialSmoother(0.0)           # standard deviation

	def sample(self, value, factor):
		deviation = abs(self.ave.v - value)

		self.ave.sample(value, factor)
		self.dev.sample(deviation, factor)

		return self.dist()

	def dist(self):
		return self.ave.v, self.dev.v

	def __str__(self):
		return "{mean=%08.3f dev=%08.3f}" % self.dist()


class WeightedDistribution(Distribution):
	"""
	Infinite Impulse Response distribution, represented by exponentially smoothed average and deviation, with pre-defined window wize.
	"""

	def __init__(self, window_size = 1.0, initial_value = 0.0, prior_sequence = None):
		self.w = window_size
		Distribution.__init__(self, initial_value, prior_sequence)

	def sample(value):
		Distribution.sample(self, value, self.w)
	

class Apex(Delta):
	"""
	Returns the given sample if it is changing direction. That is, return on the derivative changing sign.
	"""

	def __init__(self, prior_value = None):
		Delta.__init__(self, prior_value)
		self.prior_is_positive = True

	def sample(self, value):
		is_positive = Delta.sample(self, value) >= 0

		if self.prior_is_positive is not is_positive:
			self.prior_is_positive = is_positive
			return value

		return None

class TimeApex:
	"""
	Returns the given sample, and time since last given sample, if it is changing direction. That is, return on the derivative changing sign.
	"""

	def __init__(self, prior_value = None, prior_sequence = None):
		self.apex = Apex(prior_value)
		self.delta = Delta(prior_sequence)

	def sample(self, time, value):
		apex = self.apex.sample(value)
		if apex is None:
			return (None, None)
		else:
			return (apex, self.delta.sample(time))

class SignDelta:
	"""
	sign of derivative, using a cmp derivative.
	"""

	def __init__(self, prior_value = None):
		self.prior_value = prior_value

	def sample_cmp(self, value):
		"`0`, `+1` start, `-1` stop"
		s = cmp(value, self.prior_value)
		self.prior_value = value
		return s

	def sample_start(self, value):
		"`True` if start"
		return self.sample_cmp(value) == 1

	def sample_stop(self, value):
		"`True` if stop"
		return self.sample_cmp(value) == -1

class Sign:
	"""
	Whether a signal is positive.
	"""

	def __init__(self):
		pass

	def sample(self, value):
		return value > 0

class DynamicWindow:
	"""
	Provide a dynamically-adjusted window size for a particular duration, by a given time sequence.
	"""

	def __init__(self, target_duration, window_size, prior_value = None, initial_duration = 0.0):
		"Provide a target duration, and a window size over the number of given sequence events."
		self.td = target_duration
		self.s = Delta(prior_value)
		self.ed = ExponentialSmoothing(window_size, initial_duration)

	def sample(self, sequence_value):
		"Provide the next sequence value, and get an updated time window targeting the initialized duration."
		duration_since = self.s.sample(sequence_value)
		if duration_since is None:
			return self.td
		else:
			expected_duration = self.ed.sample(duration_since)
			return self.td / expected_duration

	def __str__(self):
		return "{window=%08.3f, duration=%s, target=%r}" % ((self.td / self.ed.v.v) if self.ed.v.v > 0.0 else 0.0, self.ed, self.td)

class SmoothDuration:
	"""
	An `ExponentialSmoother` windowed by `DynamicWindow` (sensitive to a specified duration)
	"""

	def __init__(self, target_duration, window_size, prior_value = None, initial_duration = 0.0, initial_value = 0.0):
		self.dw = DynamicWindow(target_duration, window_size, prior_value, initial_duration)
		self.v = ExponentialSmoother(initial_value)
		
	def sample(self, value, sequence_value):
		w = self.dw.sample(sequence_value)
		return self.v.sample(value, w)

	def __str__(self):
		return "{ave=%s. window=%s}" % (self.v, self.dw)

class SmoothDurationDistribution:
	"""
	A `Distribution` windowed by `DynamicWindow` (sensitive to a specified duration)
	"""

	def __init__(self, target_duration, window_size, prior_value = None, initial_duration = 0.0, initial_value = 0.0, prior_sequence = 0.0):
		self.dw = DynamicWindow(target_duration, window_size, prior_value, initial_duration)
		self.v = Distribution(initial_value, prior_sequence)
		
	def sample(self, value, sequence_value):
		w = self.dw.sample(sequence_value)
		return self.v.sample(value, w)

	def __str__(self):
		return "{dist=%s, window=%s}" % (self.v, self.dw)



"""
Periodic Windows
"""

import tau

class TimeSmoothing:
	"""
	Infinite Impulse Response cosine transform
	"""

	def __init__(self, period, phase, window_factor = 1, initial_value = 0.0+0.0j):
		self.period = period
		self.phase = phase
		self.v = ExponentialSmoother(initial_value)
		self.wf = window_factor

	def sample(self, time, value):
		return (1.0 , self.v.sample(value * tau.rect(float(time + self.phase) / self.period), self.period * self.wf))


class DynamicTimeSmoothing(TimeSmoothing):
	"""
	`TimeSmoothing`, but with mutable period component, tracking period delta, or the "glissando receptor factor".
	"""

	def __init__(self, period, phase, window_factor = 1, initial_value = 0.0+0.0j, initial_delta = 0.0):
		TimeSmoothing.__init__(self, period, phase, window_factor, initial_value)

		# pitch slope; frequency delta, rise-fall rate; the "glissando" receptor factor
		self.glissando = ExponentialSmoother(initial_delta)
		
	def update_period(self, period):
		# delta from previous period
		glissando_value = self.glissando.sample(period - self.period, period * self.wf)
		#print period - self.period, glissando_value

		self.phase = float(self.phase) / self.period * period
		self.period = period

		return glissando_value

	def update_phase(self, phase):
		self.phase = phase

	def sample(self, time, value, period = None):
		if period is not None:
			glissando_value = self.update_period(period)
		else:
			glissando_value = self.glissando.v

		delta, time_value = TimeSmoothing.sample(self, time, value)
		return delta, time_value, glissando_value
		

class ApexTimeSmoothing(DynamicTimeSmoothing):
	"""
	Infinite Impulse Response cosine transform, but only sampling at apexes
	"""

	def __init__(self, period, phase, window_factor = 1, initial_value = 0.0+0.0j, initial_delta = 0.0, prior_value = None, prior_sequence = None):
		DynamicTimeSmoothing.__init__(self, period, phase, window_factor, initial_value, initial_delta)
		self.time_apex = TimeApex(prior_value, prior_sequence)

	def sample(self, time, value, period = None):
		if period is not None:
			glissando_value = self.update_period(period)
		else:
			glissando_value = self.glissando.v

		apex, time_delta = self.time_apex.sample(time, value)
		if apex is not None:
			if time_delta is None:
				time_delta = 1.0
			cval = self.v.sample(value * tau.rect(float(time + self.phase) / self.period), self.period * self.wf)
			return (time_delta, cval, glissando_value)
		else:
			return (None, None, None)

class Monochord:
	def __init__(self, source_period, target_period, ratio):
		self.source_period = source_period
		self.target_period = target_period
		self.ratio = ratio
		self._init()

	def _init(self):
		self.period = self.source_period * self.ratio
		self.offset = self.target_period - self.period
		self.phi_offset = self.offset / self.target_period
		self.value = tau.rect(self.phi_offset)

	def rotate(self, cval, r, phi):
		return (cval * self.value, r, (((phi + self.phi_offset) + 0.5) % 1.0) - 0.5)


class PeriodPercept:
	"""Physical Percept: Representation of Periodic Value"""

	def __init__(self, period,      period_factor,      glissando_factor,      time,      value,      r = None,      phi = None):
		(
		      self.period, self.period_factor, self.glissando_factor, self.time, self.value, self.r,        self.phi
		) = (
			   period,      period_factor,      glissando_factor,      time,      value,      r,             phi
		)
		if r is None:
			self._init()

	def _init(self):
		self.r, self.phi = tau.polar(self.value)

	def produce_monochord_percept(self, monochord):
		rotated_value, rotated_r, rotated_phi = monochord.rotate(self.value, self.r, self.phi)

		return PeriodPercept(monochord.target_period, self.period_factor, self.glissando_factor, self.time, rotated_value, rotated_r, rotated_phi)

	def superimpose_from_percept(self, source_percept):
		self.value += source_percept.value
		self._init()

	def superimpose_monochord_on_percept(self, target_percept, monochord):
		monochord_percept = self.produce_monochord_percept(monochord)
		target_percept.superimpose_from_percept(monochord_percept)

	def __str__(self):
		return "%010.3f + %010.3f / %010.3f: r=%08.3f[%s] phi=%08.3f[%s]" % (self.period, self.glissando_factor, self.period_factor, self.r, bar.bar(self.r, self.period), self.phi, bar.bar_log(self.phi + 0.5, 1.0))

class PeriodRecept:
	"""Physiological Recept: Deduction of Periodic Value"""

	def __init__(self,   phase,      prior_phase):
		(
			self.phase, self.prior_phase
		) = (
			     phase,      prior_phase
		)
		self._init()
		
	def _init(self):
		self.period           = (self.phase.period           + self.prior_phase.period          ) / 2.0
		self.period_factor    = (self.phase.period_factor    + self.prior_phase.period_factor   ) / 2.0
		self.glissando_factor = (self.phase.glissando_factor + self.prior_phase.glissando_factor) / 2.0

		self.frequency = 1.0 / self.period

		self.value     = complex_delta(self.phase.value, self.prior_phase.value)
		self.duration  = self.phase.time - self.prior_phase.time

		self.r, self.phi = tau.polar(self.value)
			
		if self.duration > 0.0:
			self.phi_t = self.phi / self.duration
		else:
			self.phi_t = 0.0
		self.r_d = self.phase.r - self.prior_phase.r

		#print self.phase.value, self.prior_phase.value, self.value, self.phi, self.duration, self.phi_t

		self.instant_frequency = self.frequency - self.phi_t
		self.instant_period    = 1.0 / self.instant_frequency

		#self.instant_distance = self.phi_t * self.period * self.period_factor

	def __str__(self):
		return "%010.3f @ %010.3f + %010.3f / %08.3f: r=%08.3f[%s] r_d=%08.3f[%s] phi_t=%08.3f[%s]" % (self.instant_period, self.period, self.glissando_factor, self.period_factor, self.phase.r, bar.bar_log(self.phase.r, self.period), self.r_d, bar.signed_bar_log(self.r_d, 1.0 / self.period * 8), self.phi_t, bar.signed_bar(self.phi_t, 0.5))

		



class PeriodConcept:
	"""Psychological Concept: Persistence of Periodic Value"""

	def __init__(self, sensor, weight_factor = 1.0):
		self.sensor = sensor
		self.weight_factor = weight_factor
		self.prior_percept = None

	def _init(self, percept):
		self.prior_percept = self.percept

		self.avg_instant_period_state = ExponentialSmoother(percept.period)

		self.instant_period_delta_state  = Delta()
		self.instant_period_stddev_state = ExponentialSmoother(percept.period)

		#self.avg_instant_distance_state = ExponentialSmoother(0.0)

	def receive(self):
		# average instantaneous period
		self.avg_instant_period = self.avg_instant_period_state.sample(self.recept.instant_period, self.recept.period * self.weight_factor)
		self.avg_instant_period_offset = self.avg_instant_period - self.recept.period

		# deviation of average
		self.instant_period_delta = self.instant_period_delta_state.sample(self.avg_instant_period)
		if self.instant_period_delta is None:
			self.instant_period_delta = self.avg_instant_period
		# standard deviation of average (dis-convergence on an average instant period)
		self.instant_period_stddev = self.instant_period_stddev_state.sample(abs(self.instant_period_delta), abs(self.recept.instant_period * self.weight_factor))

		#self.avg_instant_distance = self.avg_instant_distance_state.sample(self.recept.instant_distance, self.recept.period * self.weight_factor)

	def sample_recept(self):
		self.recept = PeriodRecept(self.percept, self.prior_percept)
		self.receive()

	def perceive(self):
		if self.prior_percept is None:
			self._init(self.percept)

		self.sample_recept()
		
		self.prior_percept = self.percept

	def sample_percept(self, period, period_factor, glissando_factor, time, value):
		self.percept = PeriodPercept(period, period_factor, glissando_factor, time, value)
		self.perceive()

	def __str__(self):
		from math import log
		return "%010.3f / %010.5f <- %s" % (
			self.avg_instant_period,
			self.instant_period_stddev,
			self.recept,
			#1.0 / abs(self.avg_instant_distance) if self.avg_instant_distance != 0 else 1.0, bar.bar(1.0 / abs(self.avg_instant_distance) if self.avg_instant_distance != 0 else 1.0, 1.0),
			
		)

	def consonant(self, other, consonance_factor = 1.0, harmonic = 1):
		self_period  = self.avg_instant_period * harmonic
		other_period = other.avg_instant_period

		if self_period < other_period:
			ratio = other_period / self_period
		else:
			ratio = self_period / other_period

		#print ratio, (1.0 + 1.0 / self.period_factor) * consonance_factor, self.period_factor
		return ratio > (1.0 + 1.0 / self.period_factor) * consonance_factor


class PeriodSensor:
	"""Periodic Sensor"""

	def __init__(self, period, phase, period_factor = 1.0, phase_factor = 1.0, initial_value = 0.0+0.0j):
		self.reference_concept = None
		self.period = period
		self.period_factor = period_factor
		self.phase  = phase
		self.phase_factor = phase_factor

		self.sensor  = DynamicTimeSmoothing(period, phase, period_factor, initial_value)
		self.concept = PeriodConcept(self, phase_factor)

	def sample(self, time, time_value):
		time_delta, value, glissando_factor = self.sensor.sample(time, time_value)
		if value is None:
			return None

		self.concept.sample_percept(self.period, self.period_factor, glissando_factor, time, value)

		return self.concept

class DynamicPeriodSensor(PeriodSensor):
	def update_period(self, period):
		self.period = period
		self.sensor.update_period(period)

	def update_phase(self, phase):
		self.phase = phase
		self.sensor.update_phase(phase)

	def update_period_from_sensation(self, sensation):
		#self.reference_concept = sensation
		if sensation.avg_instant_period > 2.0:
			self.update_period(sensation.avg_instant_period)

class ApexPeriodSensor(PeriodSensor):
	def __init__(self, period, phase, period_factor = 1.0, glissando_factor = 0.0, phase_factor = 1.0, initial_value = 0.0+0.0j):
		PeriodSensor.__init__(self, period, phase, period_factor, phase_factor, initial_value)
		self.sensor = ApexTimeSmoothing(period, phase, period_factor, initial_value)

"""
Scale-Space Event Lifecycle Sensors
"""

class Lifecycle:
	"""Complex Lifecycle/Frequency"""
	def __init__(self, max_r = 1.0):
		self._init(max_r)

	def _init(self, max_r = 1.0):
		self.max_r     = max_r
		self.F         = 0.0 # Free Energy, where cval.real is negative entropy, and cval.imag is negative energy.
		self.r         = 0.0
		self.phi       = 0.0
		self.cycle     = 0
		self.lifecycle = 0.0

	def sample(self, cval):
		# complex lifecycle
		self.cval = cval
		self.F = cval.real - cval.imag
		prev_phi = self.phi
		self.r, self.phi = tau.polar(cval)
		if   self.phi - prev_phi >  0.5:
			self.cycle -= 1
		elif self.phi - prev_phi < -0.5:
			self.cycle += 1

		self.lifecycle = self.cycle + self.phi

		return self.lifecycle
			
	def __str__(self):
		return "%s %s %s %s %s %s %010.3f" % (
			bar.signed_bar_log(       self.F,         self.max_r),
			bar.signed_bar_log(       self.cval.real, self.max_r),
			bar.signed_bar_log(       self.cval.imag, self.max_r),
			bar.signed_bar(    self.phi, 0.5),
			bar.bar_log(       self.r, self.max_r),
			bar.bar_log(       self.r if  self.phi < 0 else 0.0, self.max_r),
			-  self.lifecycle,
		)

class DeriveLifecycle(Lifecycle):
	def __init__(self, max_r = 1.0, response_factor = 1000):
		"""response_factor is the receptor granularity, separating amplitude and phase"""
		self.response_factor = response_factor
		self._init(max_r)

	def _init(self, max_r = 1.0):
		self.d_avg_state = ExponentialSmoother(0.0)
		self.dd_avg_state = ExponentialSmoother(0.0)
		Lifecycle._init(self, max_r)

	def derive(self, v1, v2, v3):
		d1, d2 = v2 - v1, v3 - v2
		dd     = d2 - d1
		self.d = d1
		self.dd = dd

	def sample_direct(self, v1, v2, v3):
		self.derive(v1, v2, v3)
		self.c = self.d + self.dd * 1j
		return Lifecycle.sample(self, self.c)
		
	def sample_avg(self, v1, v2, v3):
		self.derive(v1, v2, v3)
		
		self.d_avg = self.d_avg_state.sample(self.d, self.response_factor)
		self.dd_avg = self.dd_avg_state.sample(self.dd, self.response_factor)
		self.c_avg = self.d_avg + self.dd_avg * 1j
		return Lifecycle.sample(self, self.c_avg)

	sample = sample_avg


class IterLifecycle(Lifecycle):
	def __init__(self, max_r = 1.0):
		self.d_state = Delta()
		self.dd_state = Delta()
		Lifecycle._init(self, max_r)

	def sample(self, value):
		self.d  = self.d_state.sample(value)
		if self.d is None:
			self.d = 0.0
		self.dd = self.dd_state.sample(self.d)
		if self.dd is None:
			self.dd = 0.0
		self.c = self.d + self.dd * 1j
		return Lifecycle.sample(self, self.c)

"""
Period Scale-Space
"""
	
class PeriodScaleSpaceSensor:
	def __init__(self, period, phase, response_period, scale_factor = 1.75, period_factor = 1.0, phase_factor = 1.0, initial_value = 0.0+0.0j):
		self.period = period
		self.phase = phase
		self.response_period = response_period
		self.period_sensors = [
			PeriodSensor(period, phase, period_factor * scale_factor ** (-1-scale), phase_factor, initial_value)
			for scale in range(3)
		]
		self.period_lifecycle = DeriveLifecycle(self.period, self.response_period)
		self.beat_lifecycle = IterLifecycle(self.period)
		self.monochords = []

	def sample_sensor(self, time, time_value):
		for period_sensor in self.period_sensors:
			period_sensor.sample(time, time_value)

	def sample_lifecycle(self):
		self.period_lifecycle.sample(self.period_sensors[0].concept.percept.r, self.period_sensors[1].concept.percept.r, self.period_sensors[2].concept.percept.r)
		self.beat_lifecycle.sample(self.period_lifecycle.lifecycle)

	def values(self):
		return self.period_sensors[0].concept, self.period_lifecycle, self.beat_lifecycle

	def sample(self, time, time_value):
		self.sample_sensor(time, time_value)
		self.sample_monochords()
		self.sample_lifecycle()
		return self.values()

	def get_monochord(self, target_percept, monochord_ratio):
		source_period = self.period
		target_period = target_percept.period

		return Monochord(source_period, target_period, monochord_ratio)

	def superimpose_monochord_on(self, other, monochord):
		for source_sensor, target_sensor in zip(self.period_sensors, other.period_sensors):
			source_sensor.concept.percept.superimpose_monochord_on_percept(target_sensor.concept.percept, monochord)
			target_sensor.concept.sample_recept()

	def add_monochord(self, source_percept, monochord_ratio):
		self.monochords.append( (source_percept, source_percept.get_monochord(self, monochord_ratio)) )

	def sample_monochords(self):
		for source_percept, monochord in self.monochords:
			source_percept.superimpose_monochord_on(self, monochord)


class PeriodArray:
	def __init__(self, response_period, octave_bandwidth = 12, scale_factor = 1.75, period_factor = 1.0, phase_factor = 1.0):
		self.period_factor = period_factor
		self.phase_factor  = phase_factor
		self.response_period = response_period
		self.scale_factor = scale_factor
		self.octave_bandwidth = octave_bandwidth
		self.period_bandwidth = 1.0 / ((2.0 ** (1.0 / self.octave_bandwidth)) - 1)
		self.period_sensors = []
		self.populate()

	def add_period_sensor(self, period, monochord_source_sensor = None, monochord_ratio = None, bandwidth_factor = 1.0):
		period_sensor = PeriodScaleSpaceSensor(period, 0.0, self.response_period, self.scale_factor, self.period_bandwidth * bandwidth_factor, self.phase_factor)
		if monochord_source_sensor is not None:
			period_sensor.add_monochord(monochord_source_sensor, monochord_ratio)
		self.period_sensors.append(period_sensor)

	def populate(self):
		"stub"
		return

	def sample(self, time, value):
		return [
			period_sensor.sample(time, value)
			for period_sensor in self.period_sensors
		]

	def sample_sensor(self, time, value):
		for period_sensor in self.period_sensors:
			period_sensor.sample_sensor(time, value)
		
	def sample_lifecycle(self):
		for period_sensor in self.period_sensors:
			period_sensor.sample_lifecycle()

	def sample_monochords(self):
		for period_sensor in self.period_sensors:
			period_sensor.sample_monochords()

	def values(self):
		return [
			period_sensor.values()
			for period_sensor in self.period_sensors
		]


		

class LogPeriodArray(PeriodArray):
	def __init__(self, period, response_period, octaves = 1, octave_bandwidth = 12, scale_factor = 1.75, period_factor = 1.0, phase_factor = 1.0):
		self.period = period
		self.octaves = octaves
		PeriodArray.__init__(self, response_period, octave_bandwidth, scale_factor, period_factor, phase_factor)

	def populate(self):
		for n in range(- self.octave_bandwidth * self.octaves, 1):
			self.add_period_sensor(self.period * 2 ** (float(n)/self.octave_bandwidth))
		
class UkePeriodArray(PeriodArray):
	def __init__(self, sample_rate, response_period, octave_bandwidth = 36, scale_factor = 1.75, period_factor = 1.0, phase_factor = 1.0):
		self.sample_rate = sample_rate
		PeriodArray.__init__(self, response_period, octave_bandwidth, scale_factor, period_factor, phase_factor)

	def populate(self):
		#ratio_M3rd = 2 ** (4.0/12)
		#ratio_5th  = 2 ** (7.0/12)
		#ratio_M6th = 2 ** (9.0/12)
		ratio_M3rd = 5.0 / 4.0
		ratio_5th  = 3.0 / 2.0
		ratio_M6th = 5.0 / 3.0

		C4 = 440 * 2 ** (float(-12 + 3) / 12)
		G4 = C4 * ratio_5th
		E4 = C4 * ratio_M3rd
		A4 = C4 * ratio_M6th

		self.add_period_sensor(self.sample_rate / C4)
		self.add_period_sensor(self.sample_rate / E4)
		self.add_period_sensor(self.sample_rate / G4)
		self.add_period_sensor(self.sample_rate / A4)

		self.add_period_sensor(self.sample_rate / E4, self.period_sensors[0], ratio_M3rd)
		self.add_period_sensor(self.sample_rate / G4, self.period_sensors[0], ratio_5th)
		self.add_period_sensor(self.sample_rate / A4, self.period_sensors[0], ratio_M6th)


class GuitarPeriodArray(PeriodArray):
	def __init__(self, sample_rate, response_period, octave_bandwidth = 36, scale_factor = 1.75, period_factor = 1.0, phase_factor = 1.0):
		self.sample_rate = sample_rate
		PeriodArray.__init__(self, response_period, octave_bandwidth, scale_factor, period_factor, phase_factor)

	def populate(self):
		#ratio_M3rd = 2 ** (4.0/12)
		#ratio_5th  = 2 ** (7.0/12)
		#ratio_M6th = 2 ** (9.0/12)
		ratio_M3rd = 5.0 / 4.0
		ratio_5th  = 3.0 / 2.0
		ratio_M6th = 5.0 / 3.0

		C4 = 440 * 2 ** (float(-12 + 3) / 12)

		E2 = C4 * 2 ** (float(12 * -2 + 4)  / 12)
		A2 = C4 * 2 ** (float(12 * -2 + 9)  / 12)
		D3 = C4 * 2 ** (float(12 * -1 + 2)  / 12)
		G3 = C4 * 2 ** (float(12 * -1 + 7)  / 12)
		B3 = C4 * 2 ** (float(12 * -1 + 11) / 12)
		E4 = C4 * 2 ** (float(          4)  / 12)

		self.add_period_sensor(self.sample_rate / E2)
		self.add_period_sensor(self.sample_rate / A2)
		self.add_period_sensor(self.sample_rate / D3)
		self.add_period_sensor(self.sample_rate / G3)
		self.add_period_sensor(self.sample_rate / B3)
		self.add_period_sensor(self.sample_rate / E4)

		self.add_period_sensor(self.sample_rate / A2, self.period_sensors[0], A2 / E2)
		self.add_period_sensor(self.sample_rate / D3, self.period_sensors[0], D3 / E2)
		self.add_period_sensor(self.sample_rate / G3, self.period_sensors[0], G3 / E2)
		self.add_period_sensor(self.sample_rate / B3, self.period_sensors[0], B3 / E2)
		self.add_period_sensor(self.sample_rate / E4, self.period_sensors[0], E4 / E2)

class LinearPeriodArray(PeriodArray):
	def __init__(self, sampling_rate, response_period, scale_factor = 1.75, start_frequency = 50, stop_frequency = 2000, step_frequency = 50, period_factor = 1, phase_factor = 1):
		self.period_factor = period_factor
		self.phase_factor  = phase_factor
		self.period_sensors = [
			PeriodScaleSpaceSensor(1.0 / (float(frequency) / sampling_rate), 0.0, response_period, scale_factor, period_factor / (float(step_frequency) / sampling_rate) * (float(frequency) / sampling_rate), phase_factor)
			for frequency in reversed(range(start_frequency, stop_frequency, step_frequency))
		]

"""
Duration Scale-Space
"""

class DurationArray:
	def sample(self, time, value):
		return [
			duration_sensor.sample(time, value)
			for duration_sensor in self.duration_sensors
		]

	def sample_sensor(self, time, value):
		for duration_sensor in self.duration_sensors:
			duration_sensor.sample_sensor(time, value)
		
	def sample_lifecycle(self):
		for duration_sensor in self.duration_sensors:
			duration_sensor.sample_lifecycle()

	def values(self):
		return [
			duration_sensor.values()
			for duration_sensor in self.duration_sensors
		]



class DurationScaleSpaceSensor:
	def __init__(self, target_duration, window_size, response_period, scale_factor = 1.75, prior_value = None, initial_duration = 0.0, initial_value = 0.0, prior_sequence = 0.0):
		self.target_duration = target_duration
		self.window_size = window_size
		self.response_period = response_period

		self.duration_sensors = [
			SmoothDurationDistribution(target_duration * scale_factor ** (-scale), window_size, prior_value, initial_duration, initial_value, prior_sequence)
			for scale in range(3)
		]

		self.period_lifecycle = DeriveLifecycle(self.target_duration, self.response_period)
		self.beat_lifecycle = IterLifecycle(self.target_duration)

	def sample_sensor(self, time, time_value):
		for duration_sensor in self.duration_sensors:
			duration_sensor.sample(time_value, time)

	def sample_lifecycle(self):
		reciprocate = lambda v: 1.0 / v if v != 0 else float("inf")
		self.period_lifecycle.sample(reciprocate(self.duration_sensors[0].v.dev.v), reciprocate(self.duration_sensors[1].v.dev.v), reciprocate(self.duration_sensors[2].v.dev.v))
		self.beat_lifecycle.sample(self.period_lifecycle.lifecycle)

	def values(self):
		return self.duration_sensors[0], self.period_lifecycle, self.beat_lifecycle

	def sample(self, time, time_value):
		self.sample_sensor(time, time_value)
		self.sample_lifecycle()
		return self.values()


class LogDurationArray(DurationArray):
	def __init__(self, target_duration, window_size, response_period, scale_factor = 1.75, steps = 12, octaves = 1):
		self.response_period = response_period
		self.scale_factor = scale_factor
		self.duration_sensors = [
			DurationScaleSpaceSensor(target_duration * 2 ** (float(n)/scale), octaves, window_size, response_period, scale_factor)
			for n in range(- scale * octaves, 1)
		]

		


def event_test():
	from sys import stdin, stdout
	from time import time, sleep

	import sampler

	sss = DurationScaleSpaceSensor(.2, 100.0, 5.0) 

	sampler.screen.clear()
	sample = 0
	n = None
	while True:
		t = time()
		line = stdin.readline()
		try:
			n = float(line.strip())
		except ValueError:
			if n is None:
				continue
			else:
				pass # keep old `n`

		sample += 1

		d, lc, blc = sss.sample(t, n)

		sampler.screen.printf("event at time %.3f sample %i: %06.2f\n", t, sample, n)
		sampler.screen.printf(
			"%s\n %s\n %s \n",
			d,
			lc,
			blc,
		)

		sampler.screen.flush()

def midi_note(sample_rate, period, A4 = 440.0):
	"""
	f = 440 * (2 ^ (n/12)
	f / 440 = (2 ^ (n/12)
	log(f / 440) / log(2)  = n/12
	12 * (log(f / 440 ) / log(2)) = n
	"""

	from math import log
	n_A4 = 69

	Hz = float(sample_rate) / float(period)
	try:
		n = 12.0 * (log(Hz / A4) / log(2)) + n_A4
	except ValueError:
		n = 0
	return n

def note(sample_rate, period, A4 = 440.0):
	notes = ["B#/C ", "C#/Db", "D /D ", "D#/Eb", "E /Fb", "E#/F ", "F#/Gb", "G /G ", "G#/Ab", "A /A ", "A#/Bb", "B /Cb"]
	from math import floor
	n = midi_note(sample_rate, period, A4)

	note = int(floor(n + 0.5))

	octave = note / 12 - 1
	octave_note = note % 12
	cents = 100.0 * ((((n) + 0.5) % 1) - 0.5)

	return "%i%s%3.0f" % (octave, notes[octave_note], cents)

def periodic_test(generate = False):
	from sys import stdin, stdout, argv
	from time import time, sleep

	import sampler

	args = dict(enumerate(argv))

	sample_rate = int(args.get(1))
	chunk_size  = int(args.get(2))
	frame_size  = int(args.get(3))
	oversample  = int(args.get(4))

	A4 = 440.0
	C4 = A4 * 2 ** ((-12 + 3.0)/12.0)
	E2 = C4 * 2 ** (float(12 * -2 + 4)  / 12)
	A2 = C4 * 2 ** (float(12 * -2 + 9)  / 12)
	frame_rate  = float(sample_rate) / frame_size
	sample_rate /= oversample
	wave_period = float(sample_rate) / A4
	wave_power  = 100   / oversample
	sweep       = False
	sweep_value = 0.99999
	from math import exp, e
	cycle_area = 1.0 / (1.0 - exp(-1))

	log_base_period = float(sample_rate) / C4 * 2 * 2
	log_octave_steps = 12
	log_octave_count = 5

	wave_change_rate = 0.1

	fs = sampler.FileSampler(stdin, chunk_size, sample_rate, 1)

	pa = GuitarPeriodArray(sample_rate, float(sample_rate) / 20, 36)
	#pa = LogPeriodArray(log_base_period, float(sample_rate) / 20, log_octave_count, log_octave_steps, cycle_area)

	sample = 0
	frame  = 0
	current_wave_period = wave_period

	draw_sample = sample + float(sample_rate) / frame_rate
	x = 0.0
	y = 0.0
	fade = 0.0
	sampler.screen.clear()
	while True:
		sample += 1

		if generate:
			if sweep:
				current_wave_period *= sweep_value
				if current_wave_period < wave_period / 8:
					current_wave_period = wave_period

			diff = 0
			x += (E2 + diff) / sample_rate
			y += (A2 + diff) / sample_rate

			j = int(float(sample) * wave_change_rate / sample_rate) % 3
			j = 0

			from math import pi, cos
			if j  == 0:
				n =  cos(2.0 * pi * x)         * wave_power / 4
				n += cos(2.0 * pi * y)         * wave_power / 4
				#n += cos(2.0 * pi * x * (sample_rate / (float(sample_rate) + wave_period * diff))) * wave_power / 4
				#n += cos(2.0 * pi * x * 2)         * wave_power / 4
				#n += cos(2.0 * pi * x * 2 * (sample_rate / (float(sample_rate) + wave_period * diff))) * wave_power / 4
			elif j == 1:
				n = float(wave_power) - (2.0 * wave_power * (x % 1))
			else:
				n = wave_power if x % 1 < 0.5 else -wave_power

			if int(sample / sample_rate) % 2 == 1:
				supersample = oversample
				while supersample > 0:
					fade *= 0.9998
					n *= fade
					supersample -= 1
			else:
				fade = 1.0
		else:
			# read from file
			get_sample = lambda fs: float(fs.next()) / (oversample * (2**32)) * 10000 
			n = 0.0
			supersample = oversample
			while supersample > 0:
				n += get_sample(fs)
				supersample -= 1
			
		

		# drawing
		t = float(sample) / sample_rate

		draw = False
		if sample >= draw_sample:
			draw = True
			draw_sample += float(sample_rate) / frame_rate
			frame += 1


		if draw:
			sampler.screen.printf("event at time %.3f frame %i sample %i: %06.2f\n", t, frame, sample, n)

		for period_sensor in pa.period_sensors:
			concept, lc, blc = period_sensor.sample(sample, n)

			if draw:	
				sampler.screen.printf(
					"%s %s %s %s \n",
					note(sample_rate, concept.percept.period, A4),
					note(sample_rate, concept.avg_instant_period, A4) if lc.dd_avg < 0 else " " * 9,
					bar.bar_log(      concept.percept.r + lc.F        if lc.dd_avg < 0 else 0,       concept.percept.period),
					lc,
					#blc,
				)
			
			
			

		sampler.screen.flush()

def main():
	from sys import argv, exit

	if   argv[0].endswith("recept_period.py"):
		periodic_test(True)
	if   argv[0].endswith("recept_mic.py"):
		periodic_test(False)
	elif argv[0].endswith("recept_event.py"):
		event_test()
	else:
		pass


if __name__ == "__main__":
	main()	
