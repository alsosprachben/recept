// Main thread script for uke tuner demo

const output = document.getElementById('output');

function bar(n, d, size = 14) {
  const mn = Math.max(0, Math.min(n, d));
  const sn = mn * size / d;
  const si = Math.floor(sn);
  const sr = sn % 1;
  const s1 = '#'.repeat(si);
  const s3 = ' '.repeat(size - si - 1);
  const s2 = si < size ? [' ', '-', '+', '='][Math.floor(sr * 4)] : '';
  return s1 + s2 + s3;
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
          const F = vals[i].F;
          lines += `${names[i]} F:${F.toFixed(4)} ${bar(Math.abs(F), 1.0)}\n`;
        }
        output.textContent = lines;
      };
      source.connect(node);
    });
  });
} else {
  output.textContent = 'getUserMedia not supported';
}

