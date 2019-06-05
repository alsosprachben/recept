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


