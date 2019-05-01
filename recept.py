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

def bar(n, d, s):
	chars = [" " for i in range(int(s))]
	for i in range(int((float(n) / d * s))):
		chars[i] = "="

	if n > d:
		chars[s-1] = "!"

	return "".join(chars)

class PhaseFreq:
	def __init__(self, initial_values):
		self.prior_values = initial_values

	def derive(self, values):
		from cmath import phase
		results = [
			(freq, post, post / pre if phase(pre) != 0.0 else 0.0)
			for pre, (freq, post) in zip(self.prior_values, values)
		]
		self.prior_values = [value for freq, value in values]
		return results

	def report(self, values):
		from cmath import phase, pi
		tau = lambda v: ((phase(v) / (2.0 * pi)) +0.5) % 1 - 0.5
		return "".join(
			"(%08.3f + %08.3f = %08.3f): %s { phi: %08.3f, r: %08.3f, phi/t: %08.3f, r/t: %08.3f }\n" % (period, (((tau(delta) * period +0.5) % 1) - 0.5) * period, 1.0 / ((1.0/period) - tau(delta)), bar(abs(value), period, 24), tau(value), abs(value), tau(delta),  abs(delta))
			for period, value, delta in self.derive(values)
		)

def main():
	from sys import stdin, stdout
	from time import time

	dev = SequenceDelta(0.0)

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
	freq_list = [
		(int(50 * 2 ** (float(n)/12) + 0.5), FrequencySmoothing(int(50 * 2 ** (float(n)/12) + 0.5), 0, 1.0 / ((2 ** (1.0/12)) - 1)))
		for n in range(-10, 10)
	]
	"""
	scale=6
	freq_list = [
		(50 * 2 ** (float(n)/scale), TimeSmoothing(50 * 2 ** (float(n)/scale), 0, 1.0 / ((2 ** (1.0/scale)) - 1)))
		for n in range(-10, 10)
	]
	freq_state = PhaseFreq(0j for n in range(-10, 10))
	scale=12
	freq_list2 = [
		(50 * 2 ** (float(n)/scale), TimeSmoothing(50 * 2 ** (float(n)/scale), 0, 1.0 / ((2 ** (1.0/scale)) - 1)))
		for n in range(-10, 10)
	]
	freq_state2 = PhaseFreq(0j for n in range(-10, 10))
	scale=24
	freq_list3 = [
		(50 * 2 ** (float(n)/scale), TimeSmoothing(50 * 2 ** (float(n)/scale), 0, 1.0 / ((2 ** (1.0/scale)) - 1)))
		for n in range(-10, 10)
	]
	freq_state3 = PhaseFreq([0j for n in range(-10, 10)])


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
		results_freq   = [(d, f.sample(frame, n)) for d, f in freq_list]
		results_freq2  = [(d, f.sample(frame, n)) for d, f in freq_list2]
		results_freq3  = [(d, f.sample(frame, n)) for d, f in freq_list3]
		
		#stdout.write("\033[2J\033[;Hevent at time %.3f frame %i: %06.2f, %06.2f\n%r\n%r\n%r\n%r\n%r\n%s\n%s\n%s\n" % (t, frame, n, nd, liststr(results), liststr(results_dev), liststr(results_ratio), liststr(results_d1), liststr(results_dev_d1), listangstr(results_freq), listangstr(results_freq2), listangstr(results_freq3)))
		stdout.write("\033[2J\033[;H event at time %.3f frame %i: %06.2f, %06.2f\n\n%s\n%s\n%s\n" % (t, frame, n, nd, freq_state.report(results_freq), freq_state2.report(results_freq2), freq_state3.report(results_freq3)))
		stdout.flush()

if __name__ == "__main__":
	main()	
