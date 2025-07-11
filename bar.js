// JavaScript port of bar.py
const defaultBarSize = 14;

// Unicode bar fill and remainder map to match C implementation
const fillChar = '\u2588';
const remainderMap = [' ', '\u258F', '\u258E', '\u258D', '\u258C', '\u258B', '\u258A', '\u2589'];

export function bar(n, d, s = defaultBarSize, left = false) {
  try {
    // constrain and compute filled count
    const val = Math.max(0, Math.min(n, d));
    const filled = left ? d - val : val;
    const sn = filled * s / d;
    let si = Math.floor(sn);
    // clamp to valid range so we never overflow the bar width
    if (si >= s) {
      si = s - 1;
    }
    let sr = sn - si;
    // clamp fractional remainder to [0,1]
    if (sr < 0) sr = 0;
    else if (sr > 1) sr = 1;
    // leading and trailing segments
    const s1 = left ? ' '.repeat(si) : fillChar.repeat(si);
    const s3 = left ? fillChar.repeat(s - si - 1) : ' '.repeat(s - si - 1);
    if (left) sr = (1.0 - sr) % 1;
    // remainder character
    // compute remainder index and clamp to valid range
    let idx = si < s ? Math.floor(sr * remainderMap.length) : -1;
    if (idx >= remainderMap.length) {
      idx = remainderMap.length - 1;
    }
    let s2 = '';
    if (idx >= 0) {
      s2 = remainderMap[idx];
    }
    // ensure full width even when s2 is empty
    const barStr = `${s1}${s2}${s3}`;
    return barStr.length < s ? barStr + ' '.repeat(s - barStr.length) : barStr;
  } catch (e) {
    return ' '.repeat(s);
  }
}

export function barLog(n, d, s = defaultBarSize, left = false) {
  try {
    // C-style pure log scale
    let n2 = n <= 0 ? 0 : Math.log(n);
    let d2 = Math.log(d);
    if (n2 < -d2) n2 = -d2;
    return bar(n2, d2, s, left);
  } catch (e) {
    return ' '.repeat(s);
  }
}

export function barLogp1(n, d, s = defaultBarSize, left = false) {
  try {
    // C-style log+1 scale (logp1)
    const n2 = n < 0 ? -Math.log(1 - n) : Math.log(1 + n);
    const d2 = Math.log(1 + d);
    return bar(n2, d2, s, left);
  } catch (e) {
    return ' '.repeat(s);
  }
}

export function signedBar(n, d, s = defaultBarSize / 2) {
  if (n >= 0.0) {
    return ' '.repeat(s) + '|' + bar(n, d, s);
  } else {
    return bar(-n, d, s, true) + '|' + ' '.repeat(s);
  }
}

export function signedBarLogp1(n, d, s = defaultBarSize / 2) {
  if (n > 0.0) {
    return ' '.repeat(s) + '|' + barLogp1(n, d, s);
  } else if (n < 0.0) {
    return barLogp1(-n, d, s, true) + '|' + ' '.repeat(s);
  } else {
    return ' '.repeat(s) + '|' + ' '.repeat(s);
  }
}

