"""
Pre-computed Windows
"""

from recept import ExponentialSmoother


class Window:
	"""
	Repeating Iterator over a specified periodic window.
	"""

	def __init__(self, window):
		self.w = window
		self.i = 0
		self.n = len(window)

	def next(self):
		"""return the next window element"""
		v = self.w[self.i]
		self.i = (self.i + 1) % self.n
		return v


class CosineWindow(Window):
	"""
	A window of the cosine at the specified period and phase.
	"""

	def __init__(self, period, phase):
		from math import cos, pi
		Window.__init__(self, [cos(2.0 * pi * ((float(i) + phase) / period)) for i in range(period)])


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

	def __init__(self, window=None, window_factor=1, initial_value=0.0):
		if window is None:
			window = [1.0, -1.0]
		self.w = Window(window)
		self.wf = window_factor
		self.v = ExponentialSmoother(initial_value)

	def sample(self, value):
		return self.v.sample(value * self.w.next(), self.w.n * self.wf)


class FrequencySmoothing:
	"""
	Sense a particular period and phase, using precomputed windows of integral size.
	"""

	def __init__(self, period, phase, window_factor=1, real_initial_value=0.0, imag_initial_value=0.0):
		self.real_w = CosineWindow(period, phase)
		self.imag_w = SineWindow(period, phase)
		self.real_v = ExponentialSmoother(real_initial_value)
		self.imag_v = ExponentialSmoother(imag_initial_value)
		self.wf = window_factor

	def sample(self, value):
		return (
			self.real_v.sample(value * self.real_w.next(), self.real_w.n * self.wf)
			+ 1j * self.imag_v.sample(value * self.imag_w.next(), self.imag_w.n * self.wf)
		)


class MovingAverage:
	def __init__(self, period):
		from collections import deque
		self.period = period
		self.window = deque([0.0] * self.period, self.period)
		self.avg = 0.0

	def sample(self, value):
		self.avg += value - self.window.popleft()

		self.window.append(value)
		return self.avg / self.period
