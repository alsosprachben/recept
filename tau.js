// JavaScript port of tau.py - works in tau units instead of radians.
export const radianCycle = Math.PI * 2;

function rad2tau(mag, rad) {
  // now exactly matches Python/C
  const x = rad/(2*Math.PI) + 0.5;
  const m = ((x % 1) + 1) % 1;       // true modulo into [0…1)
  return [ mag, m - 0.5 ];
}

export function tau2rad(mag, tau) {
  return [mag, tau * radianCycle];
}

export function rect(period, mag = 1) {
  const [m, rad] = tau2rad(mag, period);
  return { re: Math.cos(rad) * m, im: Math.sin(rad) * m };
}

export function polar(cval) {
  const mag = Math.hypot(cval.re, cval.im);
  const rad = Math.atan2(cval.im, cval.re);
  const [, tau] = rad2tau(mag, rad);
  return [mag, tau];
}

