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

def bar(n, d, s):
	mn = max(0, min(n, d))
	sn = float(mn) * s / d
	si = int(sn // 1)
	sr = sn % 1
	s1 = "#" * si
	if si < s:
		s2 = [" ", "-", "+", "="][int(sr * 4)]
		#s2 = "." if sr > 0.5 else " "
	else:
		s2 = ""
	s3 = " " * (s - si - 1)
	return "%s%s%s" % (s1, s2, s3)

def bar_log(n, d, s):
	from math import log
	return bar(log(n), log(d), s)

"""
Keyed Sequence Clustering
"""

def iter_print(sequence):
	print "A %r" % (sequence,)
	for element in sequence:
		print "B %r" % (element,)
		yield element

def priors(sequence, previous = None):
	for element in sequence:
		yield (previous, element)
		previous = element

def enkey(sequence, key_func):
	return ((key_func(element) if element is not None else 0, element) for element in sequence)

def keyed_derive(keyed_sequence, previous = None):
	return ((post_key - pre_key, (pre, post)) for (pre_key, pre), (post_key, post) in priors(keyed_sequence, (0, previous)))
		
def keyed_sorted(keyed_sequence):
	return sorted(keyed_sequence, key = lambda (key, element): key)

def keyed_edged(keyed_sequence, factor = 0.0):
	return ((cmp(key, factor), element) for key, element in keyed_sequence)

def gather_clusters(sequence, key_func, tension_factor = 1.0):
	clusters = []
	cluster = []
	in_cluster = False
	for delta, (pre, post) in keyed_derive(keyed_sorted(enkey(sequence, key_func))):
		#print delta, -key_func(post) / post.period_factor * tension_factor
		if delta < -key_func(post) / post.period_factor * tension_factor:
			# within cluster threshold
			if not in_cluster:
				if len(cluster) > 0:
					clusters.append((False, cluster))
				cluster = [post]
			else:
				cluster.append(post)
			in_cluster = True
		else:
			# outside cluster threshold
			if in_cluster:
				if len(cluster) > 0:
					clusters.append((True, cluster))
				cluster = [post]
			else:
				cluster.append(post)
			in_cluster = False

	if len(cluster) > 0:
		clusters.append((in_cluster, cluster))

	return clusters
		

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

	def update_period(self, period):
		self.phase = float(self.phase) / self.period * period
		self.period = period

	def update_phase(self, phase):
		self.phase = phase

class EventSmoothing:
	def __init__(self, period, phase, initial_value = 0.0+0.0j):
		self.period = period
		self.phase = phase
		self.v = ExponentialSmoother(initial_value)
	
class PeriodSensor:
	class Sensation:
		def __init__(self,
			period,
			period_factor,

			instant_period_offset,
			instant_period,
			avg_instant_period_offset,
			avg_instant_period,

			phi,
			r,
			phi_t,
			r_t,
			avg_phi_t
		):
			(
				self.period,
				self.period_factor,

				self.instant_period_offset,
				self.instant_period,
				self.avg_instant_period_offset,
				self.avg_instant_period,

				self.phi,
				self.r,
				self.phi_t,
				self.r_t,
				self.avg_phi_t
			) = (
				period,
				period_factor,

				instant_period_offset,
				instant_period,
				avg_instant_period_offset,
				avg_instant_period,

				phi,
				r,
				phi_t,
				r_t,
				avg_phi_t
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
	
		def __str__(self):
			return "%08.3f/%08.3f -> %08.3f: { r: [%s] %08.3f, avg(phi/t): [%s][%s] %08.3f }\n" % (
				self.period,
				self.period_factor,

				self.avg_instant_period,

				bar_log(self.r,           self.period, 48),
				self.r,

				bar(self.phi_t     + 0.5, 1.0,         16),
				bar(self.avg_phi_t + 0.5, 1.0,         16),

				self.avg_phi_t,
			)

	def __init__(self, period, phase, period_factor = 1.0, phase_factor = 1.0, initial_value = 0.0+0.0j):
		self.period = period
		self.period_factor = period_factor
		self.phase  = phase
		self.phase_factor = phase_factor
		self.sensor = TimeSmoothing(period, phase, period_factor, initial_value)

		self.phase_delta = PhaseDelta()
		self.avg_instant_period = ExponentialSmoother(period)

	def update_period(self, period):
		self.period = period
		self.sensor.update_period(period)

	def update_phase(self, phase):
		self.phase = phase
		self.sensor.update_phase(phase)

	def update_period_from_sensation(self, sensation):
		self.update_period(sensation.avg_instant_period)

	def sample(self, time, time_value):
		from cmath import phase, pi, rect
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

		avg_instant_period = self.avg_instant_period.sample(instant_period, instant_period * self.phase_factor)
		avg_instant_period_offset = avg_instant_period - period

		avg_phi_t = (1.0 / period) - (1.0 / avg_instant_period)

		return self.Sensation(
			period,
			self.period_factor,

			instant_period_offset,
			instant_period,
			avg_instant_period_offset,
			avg_instant_period,

			phi,
			r,
			phi_t,
			r_t,
			avg_phi_t
		)


class PeriodArray:
	def sample(self, time, value):
		return [
			period_sensor.sample(time, value)
			for period_sensor in self.period_sensors
		]

	def by_cluster(self, sensations, tension_factor = 1.0):
		key_func     = lambda s: -s.avg_instant_period
		return gather_clusters(sensations, key_func, tension_factor)

	def by_unison(self, sensations, consonance_factor = 1.0):
		previous_sensation = None
		tonal_groups = []
		tonal_group  = []
		for sensation in sorted(sensations, key = lambda s: -s.avg_instant_period):
			if previous_sensation is not None:
				if sensation.consonant(previous_sensation, consonance_factor):
					tonal_groups.append(tonal_group)
					tonal_group = []

			tonal_group.append(sensation)

			previous_sensation = sensation

		if len(tonal_group) > 1:
			tonal_groups.append(tonal_group)

		return tonal_groups
		

class LogPeriodArray(PeriodArray):
	def __init__(self, period, scale = 12, octaves = 1, period_factor = 1.0, phase_factor = 1.0):
		self.period_factor = period_factor
		self.phase_factor  = phase_factor
		self.period_sensors = [
			PeriodSensor(period * 2 ** (float(n)/scale), 0.0, period_factor / ((2.0 ** (1.0/scale)) - 1), phase_factor)
			for n in range(- scale * octaves, 1)
		]

class LinearPeriodArray(PeriodArray):
	def __init__(self, sampling_rate, start_frequency, stop_frequency, step_frequency, period_factor = 1, phase_factor = 1):
		self.period_factor = period_factor
		self.phase_factor  = phase_factor
		self.period_sensors = [
			PeriodSensor(1.0 / (float(frequency) / sampling_rate), 0.0, period_factor / (float(step_frequency) / sampling_rate) * (float(frequency) / sampling_rate), phase_factor)
			for frequency in reversed(range(start_frequency, stop_frequency, step_frequency))
		]



def main2():
	from time import time

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

	dev = Delta(0.0)

	frame = 0
	n = None
	while True:
		frame += 1
		t = time()
		line = stdin.readline()
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
		results_ratio  = [num/den for num, den in zip(results_dev, results)]
		results_d1     = [sd_list_d1[i].sample(v) for i, v in enumerate(results)]
		results_dev_d1 = [dev_sd_list_d1[i].sample(v) for i, v in enumerate(results_dev)]

		stdout.write("\033[2J\033[;Hevent at time %.3f frame %i sample %i: %06.2f, %06.2f\n%r\n%r\n%r\n%r\n%r\n" % (t, frame, sample, n, nd, liststr(results), liststr(results_dev), liststr(results_ratio), liststr(results_d1), liststr(results_dev_d1)))



def main():
	escape_clear = "\033[2J"
	escape_reset = "\033[;H"

	from sys import stdin, stdout
	from time import time, sleep

	use_log = True

	if use_log:
		pa1 = LogPeriodArray(100, 12, 5, 1.0, 10.0)
		pa2 = LogPeriodArray(100, 12, 5, 10.0, 10.0)
	else:
		pa1 = LinearPeriodArray(48000, 400, 5000, 100, 1.0, 10.0)
		pa2 = LinearPeriodArray(48000, 400, 5000, 100, 10.0, 10.0)

	frame_rate  = 30
	sample_rate = 48000
	wave_period = 60
	sweep       = False

	sample = 0
	frame  = 0

	draw_time = time() + 1.0 / frame_rate
	draw_sample = sample + float(sample_rate) / frame_rate
	sleep_time = 0.0
	slept_for = 0.0
	x = 0.0
	stdout.write(escape_clear)
	stdout.flush()
	while True:
		sample += 1

		if sweep:
			wave_period *= 0.99999
			if wave_period < 30.0:
				wave_period = 60

		x += 1.0 / wave_period

		j = int(float(sample) / sample_rate) % 3

		from math import pi, cos
		if j  == 0:
			n = cos(2.0 * pi * x) * 100
		elif j == 1:
			n = 100.0 - (200 * (x % 1))
		else:
			n = 100 if x % 1 < 0.5 else -100

		t = float(sample) / sample_rate

		sensations1 = pa1.sample(sample, n)
		for sensor, sensation in zip(pa2.period_sensors, sensations1):
			sensor.update_period_from_sensation(sensation)
		sensations2 = pa2.sample(sample, n)

		if sample < draw_sample:
			continue

		"""
		print sleep_time, slept_for
		current_time = time()
		sleep_time = draw_time - current_time
		if (sleep_time > 0):
			sleep(sleep_time)
			slept_for = time() - current_time
		else:
			sleep_time = 0.0
			slept_for = 0.0
		"""

		draw_time += 1.0 / frame_rate
		draw_sample += float(sample_rate) / frame_rate
	
		frame += 1

		report1 = "".join(str(sensation) for sensation in sensations1)
		out = ""
		out += escape_reset
		out += " event at time %.3f frame %i sample %i: %06.2f\n\n" % (t, frame, sample, n)
		out += "%s\n" % report1
		#print pa2.by_cluster(sensations2)
		for (in_cluster, tonal_group) in pa2.by_cluster(sensations2):
			report = ("%s                    " % ("+" if in_cluster else "-")).join(str(sensation) for sensation in tonal_group)
			sum_weighted_period = sum((sensation.r * sensation.r * sensation.avg_instant_period) for sensation in tonal_group)
			sum_weights         = sum( sensation.r * sensation.r                                 for sensation in tonal_group)
			avg_weight          = sum_weights ** 0.5
			weighted_period = sum_weighted_period / sum_weights
			out += "%s %08.3f %08.3f: %s" % ("+" if in_cluster else "-", weighted_period, avg_weight, report)

		stdout.write(out)
		stdout.flush()

if __name__ == "__main__":
	main()	
