#!/usr/bin/env python

def main():
	import random
	from time import sleep, time
	from sys import stdout

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
	
		target_time = time()
		for j in range(3):
			j = 1
			for h in range(100):
				n = 60
				for i in range(n):
					from math import pi, cos
					if j  == 0:
						x = cos(2.0 * pi * i / n) * 100
					elif j == 1:
						x = 100.0 - (200 * i / 60)
					else:
						x = 100 if i % n < n / 2 else -100
					target_time += 1.0/60
					#current_time = time()
					#sleep(max(target_time - current_time, 0))
					stdout.write("%f\n" % x)
					stdout.flush()




if __name__ == "__main__":
	main()
