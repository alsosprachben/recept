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

class PeriodicSmoothing:
	def __init__(self, window = [1.0, -1.0], initial_value = 0.0):
		self.w = WIndow(window)
		self.v = ExponentialSmoother(initial_value)

	def sample(self, value):
		return self.v.sample(value * self.w.next(), self.w.n)

class FrequenchySmoothing:
	def __init__(self, period, phase):
		from math import cos, sin
		

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

	
def liststr(l):
	return ", ".join("%06.2f" % e for e in l)

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

	n = None
	while True:
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

		results        = [sd.sample(n,  t) for sd in sd_list]
		results_dev    = [sd.sample(abs(nd), t) for sd in dev_sd_list]
		results_ratio  = [n/d for n, d in zip(results_dev, results)]
		results_d1     = [sd_list_d1[i].sample(v) for i, v in enumerate(results)]
		results_dev_d1 = [dev_sd_list_d1[i].sample(v) for i, v in enumerate(results_dev)]
		stdout.write("event at time %.3f: %06.2f, %06.2f\n%r\n%r\n%r\n%r\n%r\n" % (t, n, nd, liststr(results), liststr(results_dev), liststr(results_ratio), liststr(results_d1), liststr(results_dev_d1)))
		stdout.flush()

if __name__ == "__main__":
	main()	
