#!/usr/bin/env python

def main():
	import random
	from time import sleep, time
	from sys import stdout

	transition = True
	dev = 1

	while True:
		for i in range(400):
			x = random.normalvariate(-50, dev)
			sleep(random.expovariate(100.0))
			stdout.write("%f\n" % x)
			stdout.flush()

		if transition:
			for i in range(100):
				x = random.normalvariate(-50 + i, dev)
				sleep(random.expovariate(100.0))
				stdout.write("%f\n" % x)
				stdout.flush()

		for i in range(400):
			x = random.normalvariate(50, dev)
			sleep(random.expovariate(100.0))
			stdout.write("%f\n" % x)
			stdout.flush()

		if transition:
			for i in range(100):
				x = random.normalvariate(50 - i, dev)
				sleep(random.expovariate(100.0))
				stdout.write("%f\n" % x)
				stdout.flush()


if __name__ == "__main__":
	main()
