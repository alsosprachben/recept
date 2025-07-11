// Simple Complex number helper class
export default class Complex {
  constructor(re = 0, im = 0) {
    this.re = re;
    this.im = im;
  }

  static fromPolar(r, tau) {
    const rad = tau * 2 * Math.PI;
    return new Complex(r * Math.cos(rad), r * Math.sin(rad));
  }

  add(other) {
    return new Complex(this.re + other.re, this.im + other.im);
  }

  sub(other) {
    return new Complex(this.re - other.re, this.im - other.im);
  }

  mul(other) {
    return new Complex(
      this.re * other.re - this.im * other.im,
      this.re * other.im + this.im * other.re
    );
  }

  div(other) {
    const denom = other.re * other.re + other.im * other.im;
    if (denom === 0) return new Complex(0, 0);
    return new Complex(
      (this.re * other.re + this.im * other.im) / denom,
      (this.im * other.re - this.re * other.im) / denom
    );
  }

  abs() {
    return Math.hypot(this.re, this.im);
  }

  toString() {
    return `${this.re}+${this.im}j`;
  }
}

