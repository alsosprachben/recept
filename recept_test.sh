#!/bin/sh
mkfifo sox.pipe
sox "${1}" --buffer 735 -r 44100 -c 1 -b 32 -e signed-integer -t raw - 2>/dev/null | tee sox.pipe | play --buffer 735  -q -r 44100 -c 1 -b 32 -e signed-integer -t raw - &
cat sox.pipe | pypy recept_mic.py  44100 735 10
