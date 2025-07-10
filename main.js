// Main thread script for uke tuner demo

const output = document.getElementById('output');
const header = document.getElementById('header');
header.textContent = 'note          F         entropy    -energy      phase     amplitude   onset-amp      cycle';

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
          const v = vals[i];
          lines += `${names[i]} F:${v.F.toFixed(4)} E:${v.entropy.toFixed(4)} ` +
                   `NE:${v.neg_energy.toFixed(4)} P:${v.phase.toFixed(4)} ` +
                   `A:${v.amp.toFixed(4)} O:${v.onset_amp.toFixed(4)} ` +
                   `C:${v.cycle.toFixed(4)}\n`;
        }
        output.textContent = lines;
      };
      source.connect(node);
    });
  });
} else {
  output.textContent = 'getUserMedia not supported';
}

