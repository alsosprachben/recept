// Main thread script for uke tuner demo

const output = document.getElementById('output');
const header = document.getElementById('header');
header.textContent =
  '  free energy       entropy        - energy          phase      amplitude      onset-amp           cycle';

function bar(n, d, size = 14, left = false) {
  try {
    let mn = Math.max(0, Math.min(n, d));
    if (left) {
      mn = d - mn;
    }
    const sn = mn * size / d;
    const si = Math.floor(sn);
    let sr = sn % 1;
    let s1, s2, s3;
    if (left) {
      s1 = ' '.repeat(si);
      s3 = '#'.repeat(size - si - 1);
      sr = (1.0 - sr) % 1;
    } else {
      s1 = '#'.repeat(si);
      s3 = ' '.repeat(size - si - 1);
    }
    if (si < size) {
      s2 = [' ', '-', '+', '='][Math.floor(sr * 4)];
    } else {
      s2 = '';
    }
    return `${s1}${s2}${s3}`;
  } catch (e) {
    return ' '.repeat(size);
  }
}

function barLog(n, d, size = 14, base = Math.E, start = 1.0, left = false) {
  try {
    const logBase = Math.log(base);
    return bar(Math.log(start + n) / logBase,
               Math.log(start + d) / logBase,
               size,
               left);
  } catch (e) {
    return ' '.repeat(size);
  }
}

function signedBar(n, d, size = 7) {
  if (n >= 0.0) {
    return ' '.repeat(size) + '|' + bar(n, d, size);
  } else {
    return bar(-n, d, size, true) + '|' + ' '.repeat(size);
  }
}

function signedBarLog(n, d, size = 7, base = Math.E, start = 1.0) {
  if (n > 0.0) {
    return ' '.repeat(size) + '|' + barLog(n, d, size, base, start);
  } else if (n < 0.0) {
    return barLog(-n, d, size, base, start, true) + '|' + ' '.repeat(size);
  } else {
    return ' '.repeat(size) + '|' + ' '.repeat(size);
  }
}

if (navigator.mediaDevices) {
  navigator.mediaDevices.getUserMedia({ audio: true }).then(stream => {
    const ctx = new AudioContext();
    const source = ctx.createMediaStreamSource(stream);
    ctx.audioWorklet.addModule('uke-worklet.js').then(() => {
      const node = new AudioWorkletNode(ctx, 'uke-processor');
      node.port.onmessage = ev => {
        const vals = ev.data.values;
        const names = ['C4', 'E4', 'G4', 'A4'];
        let lines = '';
        for (let i = 0; i < vals.length; i++) {
          const v = vals[i];
          lines += `${names[i]} ` +
                   `${signedBarLog(v.F, 1.0)} ` +
                   `${signedBarLog(v.entropy, 1.0)} ` +
                   `${signedBarLog(v.neg_energy, 1.0)} ` +
                   `${signedBar(v.phase, 0.5)} ` +
                   `${barLog(v.amp, 1.0)} ` +
                   `${barLog(v.onset_amp, 1.0)} ` +
                   `${(-v.cycle).toFixed(3).padStart(10)}\n`;
        }
        output.textContent = lines;
      };
      source.connect(node);
    });
  });
} else {
  output.textContent = 'getUserMedia not supported';
}

