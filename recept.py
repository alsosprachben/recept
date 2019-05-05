#!/usr/bin/env python

"""
Infinite Impulse Response (IIR) filters for periodic analysis
"""

"""
Formatting
"""

def liststr(l):
	return ", ".join("%06.2f" % e for e in l)

def listcompstr(l):
	return ", ".join("%06.2f+%06.2fj" % (c.real, c.imag) for c in l)

def listangstr(l):
	from math import pi
	from cmath import phase
	return "".join("%06.2f = %06.2f (@ %03.2f)\n" % (f, abs(c), (phase(c) / (2.0 * pi)) % 1.0) for f, c in l)

def bar(n, d, s, use_log = True):
	from math import log
	nolog = lambda x: x
	if use_log:
		log_func = log
	else:
		log_func = nolog
	chars = [" " for i in range(s)]
	for i in range(int((float(min(log_func(n), log_func(d))) / log_func(d) * s * 2))):
		chars[max(min(i / 2, s - 1), 0)] = "-" if i % 2 == 0 else "="

	if n > d:
		chars[-1] = "!"

	return "".join(chars)


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

"""
Real-Time, Infinite, Dynamic Windows
"""

def complex_period(period, magnitude=1):
	"return the complex number of a specified period and magnitude (default 1)"
	from cmath import rect, pi
	return rect(magnitude, pi * 2 * period)

class ExponentialSmoother:
	def __init__(self, initial_value = 0.0):
		self.v = initial_value

	def sample(self, value, factor):
		self.v += (value - self.v) / factor
		return self.v

class ExponentialSmoothing:
	"""
	`ExponentialSmoother` of a fixed window size.
	"""

	def __init__(self, window_size, initial_value = 0.0):
		self.w = window_size
		self.v = ExponentialSmoother(initial_value)

	def sample(self, value):
		return self.v.sample(value, self.w)

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
			delta_value = angle_value / self.prior_angle if self.prior_angle != 0.0 else 0.0

		self.prior_angle = angle_value
		return delta_value

class SignDelta:
	"""
	Edge Detector, using a cmp derivative.
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


class SmoothDuration:
	def __init__(self, target_duration, window_size, prior_value = None, initial_duration = 0.0, initial_value = 0.0):
		self.dw = DynamicWindow(target_duration, window_size, prior_value, initial_duration)
		self.v = ExponentialSmoother(initial_value)
		
	def sample(self, value, sequence_value):
		w = self.dw.sample(sequence_value)
		return self.v.sample(value, w)

class TimeSmoothing:
	def __init__(self, period, phase, window_factor = 1, initial_value = 0.0+0.0j):
		self.period = period
		self.phase = phase
		self.v = ExponentialSmoother(initial_value)
		self.wf = window_factor

	def sample(self, time, value):
		return self.v.sample(value * complex_period(float(time + self.phase) / self.period), self.period * self.wf)

class EventSmoothing:
	def __init__(self, period, phase, initial_value = 0.0+0.0j):
		self.period = period
		self.phase = phase
		self.v = ExponentialSmoother(initial_value)
	
class PeriodSensor:
	def __init__(self, period, phase, period_factor = 1.0, phase_factor = 1.0, initial_value = 0.0+0.0j):
		self.period = period
		self.period_factor = 1.0
		self.phase  = phase
		self.phase_factor = 1.0
		self.sensor = TimeSmoothing(period, phase, period_factor, initial_value)

		self.phase_delta = PhaseDelta()
		self.avg_instant_period = ExponentialSmoother(0.0)
		self.avg_instant_period_delta = PhaseDelta()
		self.avg_instant_period_delta_avg = ExponentialSmoother(0.0)
		self.phase_factor_convergence_weight = 1.0

	def parameters(self, time, time_value):
		from cmath import phase, pi, rect
		from math import log
		tau = lambda v: ((phase(v) / (2.0 * pi)) +0.5) % 1 - 0.5

		value = self.sensor.sample(time, time_value)

		period = self.period
		delta  = self.phase_delta.sample(value)
		if delta is None:
			delta = 0j

		freq = 1.0 / period

		phi       = tau(value)
		r         = abs(value)
		phi_t     = tau(delta)
		r_t       = abs(delta)

		instant_period        = 1.0 / (1.0 / period - phi_t)
		instant_period_offset = instant_period - period

		avg_instant_period = self.avg_instant_period.sample(instant_period, instant_period * self.phase_factor * self.phase_factor_convergence_weight)
		avg_instant_period_offset = avg_instant_period - period

		avg_phi_t = 1.0 / avg_instant_period

		avg_instant_period_delta = self.avg_instant_period_delta.sample(rect(1.0, avg_instant_period))
		if avg_instant_period_delta is None:
			avg_instant_period_delta = 0j
		avg_instant_period_delta = phase(avg_instant_period_delta)
		if avg_instant_period_delta is None:
			avg_instant_period_delta = 0.0
		#print avg_instant_period
		avg_instant_period_delta_avg = phase(self.avg_instant_period_delta_avg.sample(rect(1.0, avg_instant_period_delta), period * self.phase_factor))
		self.phase_factor_convergence_weight = (1.0 / self.phase_factor) * max(1.0, log(1.0 / (abs(avg_instant_period_delta_avg) if avg_instant_period_delta_avg != 0.0 else 1.0)) / log(10))
		#print self.phase_factor_convergence_weight, avg_instant_period_delta_avg, avg_instant_period_delta, avg_instant_period
		

		return (
			period,

			instant_period_offset,
			instant_period,
			avg_instant_period_offset,
			avg_instant_period,

			bar(r,               period, 48),
			bar(phi       + 0.5, 1.0,    16, False),
			bar(phi_t     + 0.5, 1.0,    16, False),
			bar(avg_phi_t + 0.5, 1.0,    16, False),

			phi,
			r,
			phi_t,
			r_t
		)

	def report(self, time, time_value):
		return "(%08.3f (+ %08.3f = %08.3f, + %08.3f ~ %08.3f)): [%s][%s][%s][%s] { phi: %08.3f, r: %08.3f, phi/t: %08.3f, r/t: %08.3f }\n" % self.parameters(time, time_value)

class PeriodArray:
	def __init__(self, period, scale = 12, octaves = 1, period_factor = 1, phase_factor = 1):
		self.period_sensors = [
			PeriodSensor(period * 2 ** (float(n)/scale), 0.0, period_factor / ((2.0 ** (1.0/scale)) - 1), phase_factor)
			for n in range(- scale * octaves, 1)
		]

	def report(self, time, value):
		return "".join(
			period_sensor.report(time, value)
			for period_sensor in self.period_sensors
		)

def main():
	from sys import stdin, stdout
	from time import time

	dev = Delta(0.0)

	"""
	sd_list = [
		SmoothDuration(duration, 3)
		for duration in range(1, 10)
	]

	dev_sd_list = [
		SmoothDuration(duration, 3)
		for duration in range(1, 10)
	]

	sd_list_d1 = [
		Delta(0.0)
		for duration in range(1, 10)
	]

	dev_sd_list_d1 = [
		Delta(0.0)
		for duration in range(1, 10)
	]

	"""

	#pa = PeriodArray(120, 48, 2, 1.0, 10.0)
	pa = PeriodArray(120, 24, 5, 1.0, 10.0)

	n = None
	frame = 0
	while True:
		frame += 1
		line = stdin.readline()
		t = time()
		try:
			n = float(line.strip())
		except ValueError:
			if n is None:
				continue
			else:
				pass # keep old `n`

		nd = dev.sample(n)

		"""
		results        = [sd.sample(n,  t) for sd in sd_list]
		results_dev    = [sd.sample(abs(nd), t) for sd in dev_sd_list]
		results_ratio  = [num/den for num, den in zip(results_dev, results)]
		results_d1     = [sd_list_d1[i].sample(v) for i, v in enumerate(results)]
		results_dev_d1 = [dev_sd_list_d1[i].sample(v) for i, v in enumerate(results_dev)]
		"""

		result = pa.report(frame, n)

		#if frame % 60 != 1:
		#	continue
	
		#stdout.write("\033[2J\033[;Hevent at time %.3f frame %i: %06.2f, %06.2f\n%r\n%r\n%r\n%r\n%r\n%s\n%s\n%s\n" % (t, frame, n, nd, liststr(results), liststr(results_dev), liststr(results_ratio), liststr(results_d1), liststr(results_dev_d1), listangstr(results_freq), listangstr(results_freq2), listangstr(results_freq3)))
		#stdout.write("\033[2J\033[;H event at time %.3f frame %i: %06.2f, %06.2f\n\n%s\n%s\n%s\n" % (t, frame, n, nd, report, report2, report3))

		stdout.write("\033[2J\033[;H event at time %.3f frame %i: %06.2f, %06.2f\n\n%s\n" % (t, frame, n, nd, result))
		stdout.flush()

if __name__ == "__main__":
	main()	
