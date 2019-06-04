#!/usr/bin/env python

"""
Infinite Impulse Response (IIR) filters for periodic analysis
"""

"""
Formatting
"""

def liststr(l):
	return ", ".join("%06.2f" % e for e in l)

def listdiststr(l):
	return ", ".join("%06.2f/%06.2f" % e for e in l)

def listcompstr(l):
	return ", ".join("%06.2f+%06.2fj" % (c.real, c.imag) for c in l)

def listangstr(l):
	from math import pi
	from cmath import phase
	return "".join("%06.2f = %06.2f (@ %03.2f)\n" % (f, abs(c), (phase(c) / (2.0 * pi)) % 1.0) for f, c in l)

def bar(n, d, s, left = False):
	try:
		mn = max(0, min(n, d))
		if left:
			mn = d - mn

		sn = float(mn) * s / d
		si = int(sn // 1)
		sr = sn % 1

		if left:
			s1 = " " * si
			s3 = "#" * (s - si - 1)
			sr = (1.0 - sr) % 1
		else:
			s1 = "#" * si
			s3 = " " * (s - si - 1)

		if si < s:
			s2 = [" ", "-", "+", "="][int(sr * 4)]
		else:
			s2 = ""

		return "%s%s%s" % (s1, s2, s3)
	except ValueError:
		return " " * int(s)

from math import e
def bar_log(n, d, s, base = e, start = e * 2, left = False):
	try:
		from math import log
		log_base = log(base)
		#print n, d
		#print start + log(n) , log_base, start + log(d) , log_base, s
		return bar(start + log(n) / log_base, start + log(d) / log_base, s, left)
	except ValueError:
		return " " * int(s)

def signed_bar(n, d, s):
	if n >= 0.0:
		return (" " * s) + "|" + bar(n, d, s)
	else:
		return bar(-n, d, s, True) + "|" + (" " * s)

def signed_bar_log(n, d, s, base = e, start = e * 2):
	if n > 0.0:
		return (" " * s) + "|" + bar_log(n, d, s, base, start)
	elif n < 0.0:
		return bar_log(-n, d, s, base, start, True) + "|" + (" " * s)
	else:
		return (" " * s) + "|" + (" " * s)

"""

"""

dist               = lambda x, y: (x * x + y * y) ** 0.5
mult               = lambda x, y: x * y
safe_div           = lambda n, d: float('inf') if d == 0 else n / d
key_func           = lambda sensation: sensation.avg_instant_period
base_sensation     = lambda sensation: sensation.sensor.reference_concept if sensation.sensor.reference_concept is not None else sensation
base_period        = lambda sensation: base_sensation(sensation).percept.period
base_power         = lambda sensation: base_sensation(sensation).percept.r
tension_key_func   = lambda sensation: dist(base_power(sensation), safe_div(sensation.avg_instant_period, abs(sensation.percept.period - sensation.avg_instant_period)))
intensity_key_func = lambda sensation: mult(base_power(sensation), abs(sensation.recept.r_d))
tension_func       = lambda sensation, tension_factor: 1.0 / sensation.period_factor * tension_factor
sensation_ref      = lambda sensation: sensation.sensor.reference_concept
tension_filter_func = sensation_ref
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

def gather_clusters(sequence, key_func, tension_key_func, tension_filter_func, tension_func, tension_factor = 1.0):
	sequence_list = list(sequence)
	tension_key_avg = sum(tension_key_func(e) for e in sequence_list) / len(sequence_list)

	clusters = []
	cluster = []
	in_cluster = False
	for delta, (pre, post) in keyed_derive(keyed_sorted(enkey(sequence_list, key_func))):
		#print delta, -key_func(post) / post.period_factor * tension_factor
		if tension_filter_func(post) and delta < tension_factor * tension_key_func(post) / tension_key_avg:
			# within cluster threshold
			if not in_cluster:
				if len(cluster) > 0:
					cluster.pop()
					if len(cluster) > 0:
						clusters.append((False, cluster))
					cluster = [pre, post]
				else:
					cluster = [post]
			else:
				cluster.append(post)
				cluster.sort(key = base_period)
			in_cluster = True
		else:
			# outside cluster threshold
			if in_cluster:
				if len(cluster) > 0:
					clusters.append((True, cluster))
					cluster.sort(key = base_period)
				cluster = [post]
			else:
				cluster.append(post)
				cluster.sort(key = base_period)

			in_cluster = False

	if len(cluster) > 0:
		clusters.append((in_cluster, cluster))
		cluster.sort(key = base_period)

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

from cmath import rect, polar, pi
rad2tau = lambda mag, rad: (mag, ((rad / (2.0 * pi)) +0.5) % 1 - 0.5)
tau2rad = lambda mag, tau: (mag, pi * 2 * tau)

rect_period  = lambda period, mag=1: rect(*tau2rad(mag, period))
polar_period = lambda cval:                rad2tau(*polar(cval))
delta_period = lambda cval, prior_cval:    cval / prior_cval if prior_cval != 0.0 else 0.0

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
			delta_value = delta_period(angle_value, prior_angle)

		self.prior_angle = angle_value
		return delta_value

class Distribution:
	"""
	Infinite Impulse Response distribution, represented by exponentially smoothed average and deviation.
	"""

	def __init__(self, initial_value = 0.0, prior_sequence = None):
		self.d = Delta(prior_sequence) # derivative (for deviation)

		self.ave = ExponentialSmoother(initial_value) # mean average
		self.dev = ExponentialSmoother(0.0)           # standard deviation

	def sample(self, value, factor):
		deviation = abs(self.d.sample(value))

		mean   = self.ave.sample(value, factor)
		stddev = self.dev.sample(deviation, factor)

		return mean, stddev


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

class SmoothDuration:
	"""i
	An ExponentialSmoother windowed by DynamicWindow (sensitive to a specified duration)
	"""

	def __init__(self, target_duration, window_size, prior_value = None, initial_duration = 0.0, initial_value = 0.0):
		self.dw = DynamicWindow(target_duration, window_size, prior_value, initial_duration)
		self.v = ExponentialSmoother(initial_value)
		
	def sample(self, value, sequence_value):
		w = self.dw.sample(sequence_value)
		return self.v.sample(value, w)

class SmoothDurationDistribution:
	"""
	A Distribution windowed by DynamicWindow (sensitive to a specified duration)
	"""

	def __init__(self, target_duration, window_size, prior_value = None, initial_duration = 0.0, initial_value = 0.0, prior_sequence = 0.0):
		self.dw = DynamicWindow(target_duration, window_size, prior_value, initial_duration)
		self.v = Distribution(initial_value, prior_sequence)
		
	def sample(self, value, sequence_value):
		w = self.dw.sample(sequence_value)
		return self.v.sample(value, w)



"""
Periodic Windows
"""

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
		return (1.0 , self.v.sample(value * rect_period(float(time + self.phase) / self.period), self.period * self.wf))

	def update_period(self, period):
		self.phase = float(self.phase) / self.period * period
		self.period = period

	def update_phase(self, phase):
		self.phase = phase

class ApexTimeSmoothing(TimeSmoothing):
	"""
	Infinite Impulse Response cosine transform, but only sampling at apexes
	"""

	def __init__(self, period, phase, window_factor = 1, initial_value = 0.0+0.0j, prior_value = None, prior_sequence = None):
		TimeSmoothing.__init__(self, period, phase, window_factor, initial_value)
		self.time_apex = TimeApex(prior_value, prior_sequence)

	def sample(self, time, value):
		apex, time_delta = self.time_apex.sample(time, value)
		if apex is not None:
			if time_delta is None:
				time_delta = 1.0
			cval = self.v.sample(value * rect_period(float(time + self.phase) / self.period), self.period * self.wf)
			return (time_delta, cval)
		else:
			return (None, None)

class PeriodPercept:
	"""Physical Percept: Representation of Periodic Value"""

	def __init__(self, period,      period_factor,      time,      value):
		(
		      self.period, self.period_factor, self.time, self.value
		) = (
			   period,      period_factor,      time,      value
		)
		self._init()

	def _init(self):
		self.r, self.phi = polar_period(self.value)

	def __str__(self):
		return "%010.3f / %010.3f: r=%08.3f[%s] phi=%08.3f[%s]" % (self.period, self.period_factor, self.r, bar(self.r, self.period, 16), self.phi, bar_log(self.phi + 0.5, 1.0, 16))

class PeriodRecept:
	"""Physiological Recept: Detection of Periodic Value"""

	def __init__(self,   phase,      prior_phase):
		(
			self.phase, self.prior_phase
		) = (
			     phase,      prior_phase
		)
		self._init()
		
	def _init(self):
		self.period        = (self.phase.period        + self.prior_phase.period       ) / 2.0
		self.period_factor = (self.phase.period_factor + self.prior_phase.period_factor) / 2.0

		self.frequency = 1.0 / self.period

		self.value     = delta_period(self.phase.value, self.prior_phase.value)
		self.duration  = self.phase.time - self.prior_phase.time

		self.r, self.phi = polar_period(self.value)
		if self.duration > 0.0:
			self.phi_t = self.phi / self.duration
		else:
			self.phi_t = 0.0
		self.r_d = self.phase.r - self.prior_phase.r

		#print self.phase.value, self.prior_phase.value, self.value, self.phi, self.duration, self.phi_t

		self.instant_frequency = self.frequency - self.phi_t
		self.instant_period    = 1.0 / self.instant_frequency

	def __str__(self):
		return "%010.3f @ %010.3f / %08.3f: r=%08.3f[%s] r_d=%08.3f[%s] phi_t=%08.3f[%s]" % (self.instant_period, self.period, self.period_factor, self.phase.r, bar_log(self.phase.r, self.period, 16), self.r_d, signed_bar_log(self.r_d, 1.0 / self.period * 8, 8), self.phi_t, signed_bar(self.phi_t, 0.5, 8))
		

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

	def sample_recept(self):
		self.recept = PeriodRecept(self.percept, self.prior_percept)
		self.receive()

	def perceive(self):
		if self.prior_percept is None:
			self._init(self.percept)

		self.sample_recept()
		
		self.prior_percept = self.percept

	def sample_percept(self, period, period_factor, time, value):
		self.percept = PeriodPercept(period, period_factor, time, value)
		self.perceive()

	def __str__(self):
		from math import log
		return "%010.3f / %010.5f <- %s %s" % (self.avg_instant_period, self.instant_period_stddev, self.recept, "" if self.sensor.reference_concept is None else "f_d=%010.5f[%s]" % (self.sensor.reference_concept.percept.r - self.percept.r, signed_bar_log((self.sensor.reference_concept.percept.r - self.percept.r) / self.percept.period, 8, 8)))

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

		self.sensor  = TimeSmoothing(period, phase, period_factor, initial_value)
		self.concept = PeriodConcept(self, phase_factor)

	def sample(self, time, time_value):
		time_delta, value = self.sensor.sample(time, time_value)
		if value is None:
			return None

		self.concept.sample_percept(self.period, self.period_factor, time, value)

		return self.concept

	def update_period(self, period):
		self.period = period
		self.sensor.update_period(period)

	def update_phase(self, phase):
		self.phase = phase
		self.sensor.update_phase(phase)

	def update_period_from_sensation(self, sensation):
		self.reference_concept = sensation
		if sensation.avg_instant_period > 2.0:
			self.update_period(sensation.avg_instant_period)

class ApexPeriodSensor(PeriodSensor):
	def __init__(self, period, phase, period_factor = 1.0, phase_factor = 1.0, initial_value = 0.0+0.0j):
		PeriodSensor.__init__(self, period, phase, period_factor, phase_factor, initial_value)
		self.sensor = ApexTimeSmoothing(period, phase, period_factor, initial_value)

class PeriodArray:
	def sample(self, time, value):
		return [
			period_sensor.sample(time, value)
			for period_sensor in self.period_sensors
		]

	def by_cluster(self, sensations, tension_factor = 1.0):
		return gather_clusters(sensations, key_func, tension_key_func, tension_filter_func, tension_func, tension_factor)

class LogPeriodArray(PeriodArray):
	def __init__(self, period, scale = 12, octaves = 1, period_factor = 1.0, phase_factor = 1.0):
		self.period_factor = period_factor
		self.phase_factor  = phase_factor
		self.period_sensors = [
			PeriodSensor(period * 2 ** (float(n)/scale), 0.0, period_factor / ((2.0 ** (1.0/scale)) - 1), phase_factor)
			for n in range(- scale * octaves, 1)
		]
		"""
		self.period_sensors = []
		for n in range(scale):
			for o in range(octaves):
				self.period_sensors.append(
					PeriodSensor(period * 2 ** ((float(n + o * scale))/scale), 0.0, period_factor / ((2.0 ** (1.0/scale)) - 1), phase_factor)
				)
		"""

class LinearPeriodArray(PeriodArray):
	def __init__(self, sampling_rate, start_frequency, stop_frequency, step_frequency, period_factor = 1, phase_factor = 1):
		self.period_factor = period_factor
		self.phase_factor  = phase_factor
		self.period_sensors = [
			PeriodSensor(1.0 / (float(frequency) / sampling_rate), 0.0, period_factor / (float(step_frequency) / sampling_rate) * (float(frequency) / sampling_rate), phase_factor)
			for frequency in reversed(range(start_frequency, stop_frequency, step_frequency))
		]

class EntropicPeriodArray(PeriodArray):
	def __init__(self, n = 32, period_factor = 1.0, phase_factor = 1.0):
		self.n = n
		self.period_factor = period_factor
		self.phase_factor  = phase_factor
		self.period_sensors = [PeriodSensor(8 * 2 ** (float(i) * 10 / n), 0.0, period_factor * n, phase_factor) for i in range(self.n)]

	def sample(self, time, value):
		sensations = []
		prev_sensation = None
		#for sensor in sorted(self.period_sensors, key = lambda sensor: -abs(sensor.sensor.v.v)):
		for sensor in self.period_sensors:
			sensation = sensor.sample(time, value)
			if prev_sensation is not None:
				sensation.cancel(prev_sensation)

			sensations.append(sensation)

			sensor.update_period_from_sensation(sensation)

			prev_sensation = sensation

		return sensations


escape_clear = "\033[2J"
escape_reset = "\033[;H"

def event_test():
	from sys import stdin, stdout
	from time import time, sleep

	sd_list = [
		SmoothDurationDistribution(duration, 3)
		for duration in range(1, 10)
	]

	stdout.write(escape_clear)
	stdout.flush()
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

		results = [sd.sample(n,  t) for sd in sd_list]

		stdout.write("%sevent at time %.3f sample %i: %06.2f\n%s" % (escape_reset, t, sample, n, listdiststr(results)))
		stdout.flush()

class FileSampler:
	def __init__(self, f, chunk_size = 16, sample_rate = 48000, channels = 2, encoding = "i"):
		self.f = f
		self.sample_rate = sample_rate
		self.channels    = channels
		self.encoding    = encoding
		self.chunk_size  = chunk_size
		self._init_buf()

	def _init_buf(self):
		import array
		self.hit_eof = False
		self.buf     = array.array(self.encoding)

	def next(self):
		if len(self.buf) == 0:
			if not self.hit_eof:
				try:
					self.buf.fromfile(self.f, self.chunk_size)
				except EOFError:
					self.hit_eof = True

			if self.hit_eof:
				raise StopIteration

		return self.buf.pop()

def note(sample_rate, period, A4 = 440.0):
	"""
	f = 440 * (2 ^ (n/12)
	f / 440 = (2 ^ (n/12)
	log(f / 440) / log(2)  = n/12
	12 * (log(f / 440 ) / log(2)) = n
	"""
	from math import log, floor

	notes = ["A /A ", "A#/Bb", "B /Cb",  "B#/C ", "C#/Db", "D /D ", "D#/Eb", "E /Fb", "E#/F ", "F#/Gb", "G /G ", "G#/Ab"]
	Hz = float(sample_rate) / float(period)
	try:
		n = 12.0 * (log(Hz / A4) / log(2))
	except ValueError:
		n = 0
	note = int(floor(n + 0.5))
	octave = 5 + note / 12
	octave_note = note % 12
	cents = 100.0 * ((((n) + 0.5) % 1) - 0.5)

	return "%i%s%3.0f" % (octave, notes[octave_note], cents)

def periodic_test(generate = False):
	from sys import stdin, stdout
	from time import time, sleep

	use_log     = True
	frame_rate  = 60
	oversample  = 10
	sample_rate = 44100.0 / oversample
	wave_period = 240
	wave_power  = 100
	sweep       = False
	sweep_value = 0.99999
	from math import exp, e
	cycle_area = 1.0 / (1.0 - exp(-1))
	first_period_factor  = cycle_area
	first_phase_factor   = first_period_factor * 1
	second_period_factor = first_period_factor * 2
	second_phase_factor  = second_period_factor * 1
	tension_factor       = 1.0

	log_base_period = (float(sample_rate) / (440.0 * 2 ** -3))
	log_octave_steps = 12
	log_octave_count = 5

	linear_freq_start = 50
	linear_freq_stop  = 1550
	linear_freq_step  = 25

	wave_change_rate = 0.1

	fs = FileSampler(stdin, int(sample_rate / frame_rate), sample_rate)

	if use_log:
		pa1 = LogPeriodArray(log_base_period, log_octave_steps, log_octave_count, first_period_factor, first_phase_factor)
		pa2 = LogPeriodArray(log_base_period, log_octave_steps, log_octave_count, second_period_factor, second_phase_factor)
	else:
		pa1 = LinearPeriodArray(sample_rate, linear_freq_start, linear_freq_stop, linear_freq_step, first_period_factor, first_phase_factor)
		pa2 = LinearPeriodArray(sample_rate, linear_freq_start, linear_freq_stop, linear_freq_step, second_period_factor, second_phase_factor)

	#pa1 = EntropicPeriodArray(log_octave_steps * log_octave_count, first_period_factor, first_phase_factor)

	sample = 0
	frame  = 0
	current_wave_period = wave_period

	draw_sample = sample + float(sample_rate) / frame_rate
	x = 0.0
	stdout.write(escape_clear)
	stdout.flush()
	while True:
		sample += 1

		if sweep:
			current_wave_period *= sweep_value
			if current_wave_period < wave_period / 2:
				current_wave_period = wave_period

		x += 1.0 / current_wave_period

		if generate:
			j = int(float(sample) * wave_change_rate / sample_rate) % 3
			#j = 1

			from math import pi, cos
			if j  == 0:
				diff = 5
				n =  cos(2.0 * pi * x)         * wave_power / 4
				#n += cos(2.0 * pi * x * (sample_rate / (float(sample_rate) + wave_period * diff))) * wave_power / 4
				n += cos(2.0 * pi * x * 2)         * wave_power / 4
				#n += cos(2.0 * pi * x * 2 * (sample_rate / (float(sample_rate) + wave_period * diff))) * wave_power / 4
			elif j == 1:
				n = float(wave_power) - (2.0 * wave_power * (x % 1))
			else:
				n = wave_power if x % 1 < 0.5 else -wave_power
		else:
			# read from file
			get_sample = lambda fs: float(fs.next()) / (2**32) * 10000
			#n = sum([get_sample(fs), get_sample(fs), get_sample(fs), get_sample(fs)]) / 4.0
			n = 0.0
			for i in range(oversample):
				n += get_sample(fs)
			n /= oversample
			

		t = float(sample) / sample_rate

		sensations1 = pa1.sample(sample, n)
		if sensations1[0] is None:
			continue
		#for sensor, sensation in zip(pa1.period_sensors, sensations1):
		for sensor, concept in zip(pa2.period_sensors, sensations1):
			sensor.reference_concept = concept
			#sensor.update_period_from_sensation(concept)

		sensations2 = pa2.sample(sample, n)
		if sensations2[0] is None:
			continue

		if sample < draw_sample:
			continue

		draw_sample += float(sample_rate) / frame_rate
	
		frame += 1

		report1 = "\n".join(str(sensation) for sensation in reversed(sensations1))
		report1 += "\n"
		#report1 += "\n%f\n" % current_wave_period
		report2 = "\n".join(str(sensation) for sensation in reversed(sensations2))
		"""
		report2 = ""
		max_strength = 0.0
		avg_strength = sum(sensation.percept.r for sensation in sensations1) / len(sensations1)
		#dev_strength = avg_strength
		dev_strength = sum(abs(delta) for delta, (a, b) in keyed_derive(enkey(sensations1, lambda sensation: sensation.percept.r)))
		#print avg_strength, dev_strength
		for (in_cluster, tonal_group) in reversed(pa2.by_cluster(sensations2, tension_factor)):#second_level)):
			strongest                = sorted(tonal_group, key=intensity_key_func)[-1]
			strongest_weight         = base_power(strongest)
			strongest_period         = strongest.percept.period
			strongest_instant_period = strongest.avg_instant_period

			if in_cluster and not strongest_weight > avg_strength + dev_strength / 100:
				in_cluster = False

			report = ("%s                              " % ("+" if in_cluster else "-")).join("%s\n" % sensation for sensation in reversed(tonal_group))

			#if in_cluster and strongest_weight / strongest_period > max_strength:
			#	max_strength = strongest_weight / strongest_period
			#	report2 = note(sample_rate, strongest_period)

			#sum_weighted_period = sum((sensation.percept.r * sensation.period_factor * sensation.avg_instant_period) for sensation in tonal_group)
			#sum_weights         = sum( sensation.percept.r * sensation.period_factor                                 for sensation in tonal_group)
			#weighted_period = sum_weighted_period / sum_weights

			report2 += ("%s %s %s %08.3f: %s") % ("+" if in_cluster else "-", note(sample_rate, strongest_instant_period) if in_cluster else "         ", "%08.3f" % strongest_instant_period if in_cluster else "        ", strongest_weight, report)
		"""

		out = "%sevent at time %.3f frame %i sample %i: %06.2f\n\n%s\n%s" % (escape_reset, t, frame, sample, n, report1, report2)
		#if report2:
		#	out = "%s%s%s" % (escape_clear, escape_reset, report2)
		#out += " " * 1000

		stdout.write(out)
		stdout.flush()

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
