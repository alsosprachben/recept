#!/bin/sh
sample_rate=44100
chunk_size=735
frame_size=735
oversample=7

# raspberry pi USB mic
#export AUDIODEV=hw:1,0 AUDIODRIVER=alsa

rec --buffer $chunk_size -r $sample_rate -c 1 -b 32 -e signed-integer -t raw - 2>/dev/null | pypy recept_mic.py $sample_rate $chunk_size $frame_size $oversample
