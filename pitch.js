export function midiNoteFromFreq(freq, A4 = 440.0) {
  return 12.0 * (Math.log(freq / A4) / Math.log(2)) + 69;
}

export function noteFromFreq(freq, A4 = 440.0) {
  const notes = [
    "C /B#", "C#/Db", "D /D ", "D#/Eb",
    "E /Fb", "F /E#", "F#/Gb", "G /G ",
    "G#/Ab", "A /A ", "A#/Bb", "B /Cb"
  ];
  const n = midiNoteFromFreq(freq, A4);
  const intN = Math.floor(n + 0.5);
  const octave = Math.floor(intN / 12) - 1;
  const octaveNote = ((intN % 12) + 12) % 12;
  const cents = 100.0 * (((n + 0.5) % 1) - 0.5);
  return { intN, note: notes[octaveNote], octave, cents };
}
