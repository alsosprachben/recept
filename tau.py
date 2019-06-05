"""
Tau module: tau instead of radians, to make working in periods easier
"""

from cmath import rect as rad_rect, polar as rad_polar, pi
radian_cycle = pi * 2

rad2tau = lambda mag, rad: (mag, ((rad / radian_cycle) +0.5) % 1 - 0.5)
tau2rad = lambda mag, tau: (mag,   tau * radian_cycle)

rect  = lambda period, mag=1: rad_rect(*tau2rad(mag, period))
polar = lambda cval:                    rad2tau(*rad_polar(cval))
delta = lambda cval, prior_cval: cval / prior_cval if prior_cval != 0.0 else 0.0


