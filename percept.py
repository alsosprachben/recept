#!/usr/bin/env python

def main():
	import random
	from time import sleep, time
	from sys import stdout

	while True:
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


if __name__ == "__main__":
	main()
