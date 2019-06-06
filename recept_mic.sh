#!/bin/sh
rec --buffer 735 -r 44100 -c 1 -b 32 -e signed-integer -t raw - 2>/dev/null | pypy recept_mic.py  44100 735 10
#rec --buffer 245 -r 11025 -c 1 -b 32 -e signed-integer -t raw - 2>/dev/null | pypy recept_mic.py  11025 245 1
