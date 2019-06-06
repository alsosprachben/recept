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


escape_clear = "\033[2J"
escape_reset = "\033[;H"

class Screen:
	def __init__(self):
		self._init()

	def _init(self):
		self.buf = escape_reset

	def printf(self, fmt, *args):
		self.buf += fmt % args

	def clear(self):
		from sys import stdout
		stdout.write(escape_clear)
		stdout.flush()

	def out(self):
		from sys import stdout
		stdout.write(self.buf)
		self._init()

	def flush(self):
		from sys import stdout
		self.out()
		stdout.flush()

screen = Screen()
