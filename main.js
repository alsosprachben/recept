// Main thread script for uke tuner demo

(async function() {
  // dynamically import shared bar rendering routines
  const { bar, barLog, signedBar, signedBarLogp1, barLogp1 } = await import('./bar.js');
  const { noteFromFreq } = await import('./pitch.js');

  const output = document.getElementById('output');
  const header = document.getElementById('header');
  header.textContent =
    '    note           receptor       free energy         entropy       energy        phase           cycle';

  if (navigator.mediaDevices) {
    navigator.mediaDevices.getUserMedia({ audio: true }).then(stream => {
      const ctx = new AudioContext();
      const source = ctx.createMediaStreamSource(stream);
      ctx.audioWorklet.addModule('uke-worklet.js').then(() => {
        const node = new AudioWorkletNode(ctx, 'uke-processor');
        node.port.onmessage = ev => {
          if (ev.data.type != 'receptions') return;

          const vals = ev.data.values;
          // keys are name, values are lc life cycle values
          let sensorName;
          let lines = '';
          for (sensorName in vals) {
            const lc = vals[sensorName];

            let pc = - lc.energy - lc.entropy; // compute pc as energy + entropy
            if (lc.energy > 0) {
              pc = 0; // ensure pc is non-negative
            }
            let phase = lc.phi;
            if (phase < 0) {
              phase += 1;
            }

            let noteStr = '             ';
            if (lc.instant_frequency && lc.instant_frequency > 0) {
              const { intN, note, octave, cents } = noteFromFreq(lc.instant_frequency);
              const sign = cents < 0 ? '-' : cents > 0 ? '+' : ' ';
              const centsVal = Math.abs(Math.round(cents)).toString().padStart(2, '0');
              noteStr = `${note}${String(octave).padStart(2)} ${String(intN).padStart(2)}${sign}${centsVal.padEnd(3, ' ')}`;
            }

            lines += `${sensorName} ${noteStr} ` +
                     `${barLog(   pc,          lc.max_r || 1)} ` +
                     `${signedBarLogp1(lc.F,        lc.max_r || 1)} ` +
                     `${signedBarLogp1(lc.entropy,  lc.max_r || 1)} ` +
                     `${signedBarLogp1(lc.energy,   lc.max_r || 1)} ` +
                     `${bar(           phase,       1)} ` +
                     `${(-lc.cycle).toFixed(3).padStart(10)}\n`;
          }
          output.textContent = lines;
          node.port.postMessage({ type: 'current_time', time: ctx.currentTime });
        };
        source.connect(node);
      });
    });
  } else {
    output.textContent = 'getUserMedia not supported';
  }
})();

