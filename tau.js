// JavaScript port of tau.py - works in tau units instead of radians.
export const radianCycle = Math.PI * 2;

export function rad2tau(mag, rad) {
  return [mag, ((rad / radianCycle) + 0.5) % 1 - 0.5];
}

export function tau2rad(mag, tau) {
  return [mag, tau * radianCycle];
}

export function rect(period, mag = 1) {
  const [m, rad] = tau2rad(mag, period);
  return { re: Math.cos(rad) * m, im: Math.sin(rad) * m };
}

export function polar(cval) {
  const mag = Math.sqrt(cval.re * cval.re + cval.im * cval.im);
  const rad = Math.atan2(cval.im, cval.re);
  const [, tau] = rad2tau(mag, rad);
  return [mag, tau];
}

