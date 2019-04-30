#!/usr/bin/env python

def main():
	import random
	from time import sleep, time
	from sys import stdout

	target_time = time()
	while True:
		"""
		for i in range(50):
			x = random.normalvariate(10, 5)
			sleep(random.expovariate(10.0))
			stdout.write("%f\n" % x)
			stdout.flush()

		for i in range(50):
			x = random.normalvariate(50, 25)
			sleep(random.expovariate(10.0))
			stdout.write("%f\n" % x)
			stdout.flush()

		"""
		n = 60
		for i in range(n):
			from math import pi, cos
			x = cos(2.0 * pi * i / n) * 100
			target_time += 1.0/60
			current_time = time()
			sleep(max(target_time - current_time, 0))
			stdout.write("%f\n" % x)
			stdout.flush()



if __name__ == "__main__":
	main()
