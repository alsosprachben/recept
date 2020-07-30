"""
Tau module: tau instead of radians, to make working in periods easier
"""

from cmath import rect as rad_rect, polar as rad_polar, pi
radian_cycle = pi * 2


def rad2tau(mag, rad):
    return mag, ((rad / radian_cycle) + 0.5) % 1 - 0.5


def tau2rad(mag, tau):
    return mag, tau * radian_cycle


def rect(period, mag=1):
    return rad_rect(*tau2rad(mag, period))


def polar(cval):
    return rad2tau(*rad_polar(cval))
