#!/bin/sh
rec --buffer 352 -r 44100 -c 1 -b 32 -e signed-integer -t raw - 2>/dev/null | pypy recept_mic.py 
