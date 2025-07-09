// JavaScript port of tau.py - works in tau units instead of radians.
const radianCycle = Math.PI * 2;

function rad2tau(mag, rad) {
  return [mag, ((rad / radianCycle) + 0.5) % 1 - 0.5];
}

function tau2rad(mag, tau) {
  return [mag, tau * radianCycle];
}

function rect(period, mag = 1) {
  const [m, rad] = tau2rad(mag, period);
  return { re: Math.cos(rad) * m, im: Math.sin(rad) * m };
}

function polar(cval) {
  const mag = Math.sqrt(cval.re * cval.re + cval.im * cval.im);
  const rad = Math.atan2(cval.im, cval.re);
  const [, tau] = rad2tau(mag, rad);
  return [mag, tau];
}

module.exports = {
  rad2tau,
  tau2rad,
  rect,
  polar,
  radianCycle,
};
