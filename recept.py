#!/usr/bin/env python

class ExponentialSmoother:
	def __init__(self, initial_value = 0.0):
		self.v = initial_value

	def sample(self, value, factor):
		self.v += (value - self.v) / factor
		return self.v

class ExponentialSmoothing:
	def __init__(self, window_size, initial_value = 0.0):
		self.w = window_size
		self.v = ExponentialSmoother(initial_value)

	def sample(self, value):
		return self.v.sample(value, self.w)

class Window:
	def __init__(self, window):
		self.w = window
		self.i = 0
		self.n = len(window)

	def next(self):
		v = self.w[self.i]
		self.i = (self.i + 1) % self.n
		return v

class CosineWindow(Window):
	def __init__(self, period, phase):
		from math import cos, pi
		self.i = 0
		self.n = period
		self.w = [cos(2.0 * pi * ((float(i) + phase) / period)) for i in range(period)]

class SineWindow(CosineWindow):
	def __init__(self, period, phase):
		CosineWindow.__init__(self, period, phase + (float(period) / 4))

def complex_period(period):
	from cmath import rect, pi
	return rect(1, pi * 2 * period)

class PeriodicSmoothing:
	def __init__(self, window = [1.0, -1.0], window_factor = 1, initial_value = 0.0):
		self.w = WIndow(window)
		self.wf = window_factor
		self.v = ExponentialSmoother(initial_value)

	def sample(self, value):
		return self.v.sample(value * self.w.next(), self.w.n * self.wf)

class FrequencySmoothing:
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

class SequenceDelta:
	def __init__(self, prior_sequence = None):
		self.prior_sequence = prior_sequence

	def sample(self, sequence_value):
		if self.prior_sequence is None:
			delta_value = None
		else:
			delta_value = sequence_value - self.prior_sequence

		self.prior_sequence = sequence_value
		return delta_value

class DynamicWindow:
	def __init__(self, target_duration, window_size, prior_value = None, initial_duration = 0.0):
		self.td = target_duration
		self.s = SequenceDelta(prior_value)
		self.ed = ExponentialSmoothing(window_size, initial_duration)

	def sample(self, sequence_value):
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

class PhaseFreq:
	def __init__(self, initial_values, window_factor = 1):
		self.prior_values = initial_values
		self.average_period = {}
		self.wf = window_factor

	def derive(self, values):
		from cmath import phase
		results = [
			(freq, post, post / pre if phase(pre) != 0.0 else 0.0)
			for pre, (freq, post) in zip(self.prior_values, values)
		]
		self.prior_values = [value for freq, value in values]
		return results

	def avg_period(self, sensor_period, calculated_period, period_weight):
		from math import e, log
		if sensor_period not in self.average_period:
			self.average_period[sensor_period] = ExponentialSmoother(calculated_period)
			return calculated_period
		else:
			instant_period = calculated_period - sensor_period
			#convergence_weight = 10.0 / abs(instant_period)
			return self.average_period[sensor_period].sample(calculated_period, sensor_period * self.wf)

	def parameters(self, period, value, delta):
		from cmath import phase, pi
		tau = lambda v: ((phase(v) / (2.0 * pi)) +0.5) % 1 - 0.5

		freq = 1.0 / period

		phi       = tau(value)
		r         = abs(value)
		phi_t     = tau(delta)
		r_t       = abs(delta)
		avg_phi_t = self.avg_period(period, phi_t, 1.0)

		instant_period     = 1.0 / (1.0 / period - phi_t)
		instant_period_offset = instant_period - period
		avg_instant_period = 1.0 / (1.0 / period - avg_phi_t)
		avg_instant_period_offset = avg_instant_period - period

		return (
			period,
			instant_period_offset,
			instant_period,
			avg_instant_period_offset,
			avg_instant_period,
			bar(r,               period, 24),
			bar(phi       + 0.5, 1.0,    24, False),
			bar(phi_t     + 0.5, 1.0,    24, False),
			bar(avg_phi_t + 0.5, 1.0,    24, False),
			phi,
			r,
			phi_t,
			r_t
		)


	def report(self, values):
		return "".join(
			"(%08.3f (+ %08.3f = %08.3f, + %08.3f ~ %08.3f)): [%s][%s][%s][%s] { phi: %08.3f, r: %08.3f, phi/t: %08.3f, r/t: %08.3f }\n" % self.parameters(period, value, delta)
			for period, value, delta in self.derive(values)
		)

class PeriodArray:
	def __init__(self, period, scale = 12, octaves = 1, sensor_factor = 1, phase_factor = 1):
		self.freq_list = [
			(period * 2 ** (float(n)/scale), TimeSmoothing(period * 2 ** (float(n)/scale), 0, sensor_factor / ((2 ** (1.0/scale)) - 1)))
			for n in range(- scale * octaves, 1)
		]
		self.freq_state = PhaseFreq([0j for n in range(- scale * octaves, 1)], phase_factor)

	def sample(self, time, value):
		return self.freq_state.report([(d, f.sample(time, value)) for d, f in self.freq_list])

def main():
	from sys import stdin, stdout
	from time import time

	dev = SequenceDelta(0.0)

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
		SequenceDelta(0.0)
		for duration in range(1, 10)
	]

	dev_sd_list_d1 = [
		SequenceDelta(0.0)
		for duration in range(1, 10)
	]

	"""

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

		result = pa.sample(frame, n)

		#if frame % 15 != 1:
		#	continue
	
		#stdout.write("\033[2J\033[;Hevent at time %.3f frame %i: %06.2f, %06.2f\n%r\n%r\n%r\n%r\n%r\n%s\n%s\n%s\n" % (t, frame, n, nd, liststr(results), liststr(results_dev), liststr(results_ratio), liststr(results_d1), liststr(results_dev_d1), listangstr(results_freq), listangstr(results_freq2), listangstr(results_freq3)))
		#stdout.write("\033[2J\033[;H event at time %.3f frame %i: %06.2f, %06.2f\n\n%s\n%s\n%s\n" % (t, frame, n, nd, report, report2, report3))

		stdout.write("\033[2J\033[;H event at time %.3f frame %i: %06.2f, %06.2f\n\n%s\n" % (t, frame, n, nd, result))
		stdout.flush()

if __name__ == "__main__":
	main()	
