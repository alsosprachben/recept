// JavaScript port of bar.py
const defaultBarSize = 14;

function bar(n, d, s = defaultBarSize, left = false) {
  try {
    let mn = Math.max(0, Math.min(n, d));
    if (left) {
      mn = d - mn;
    }
    const sn = mn * s / d;
    const si = Math.floor(sn);
    let sr = sn % 1;

    let s1, s2, s3;
    if (left) {
      s1 = ' '.repeat(si);
      s3 = '#'.repeat(s - si - 1);
      sr = (1.0 - sr) % 1;
    } else {
      s1 = '#'.repeat(si);
      s3 = ' '.repeat(s - si - 1);
    }

    if (si < s) {
      const idx = Math.floor(sr * 4);
      s2 = [' ', '-', '+', '='][idx];
    } else {
      s2 = '';
    }
    return `${s1}${s2}${s3}`;
  } catch (e) {
    return ' '.repeat(s);
  }
}

function barLog(n, d, s = defaultBarSize, base = Math.E, start = 1.0, left = false) {
  try {
    const logBase = Math.log(base);
    return bar(Math.log(start + n) / logBase, Math.log(start + d) / logBase, s, left);
  } catch (e) {
    return ' '.repeat(s);
  }
}

function signedBar(n, d, s = defaultBarSize / 2) {
  if (n >= 0.0) {
    return ' '.repeat(s) + '|' + bar(n, d, s);
  } else {
    return bar(-n, d, s, true) + '|' + ' '.repeat(s);
  }
}

function signedBarLog(n, d, s = defaultBarSize / 2, base = Math.E, start = 1.0) {
  if (n > 0.0) {
    return ' '.repeat(s) + '|' + barLog(n, d, s, base, start);
  } else if (n < 0.0) {
    return barLog(-n, d, s, base, start, true) + '|' + ' '.repeat(s);
  } else {
    return ' '.repeat(s) + '|' + ' '.repeat(s);
  }
}

module.exports = {
  bar,
  barLog,
  signedBar,
  signedBarLog,
};
