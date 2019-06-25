#!/bin/sh
sample_rate=44100
frame_size=4096
oversample=15

# raspberry pi USB mic
#export AUDIODEV=hw:1,0 AUDIODRIVER=alsa

rec --buffer $frame_size -r $sample_rate -c 1 -b 32 -e signed-integer -t raw - 2>/dev/null | pypy recept_mic.py $sample_rate $frame_size $oversample
