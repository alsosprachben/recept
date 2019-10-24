#!/bin/sh
rec -q --buffer $(printf "44100.0 / 60
" | bc) -r 44100 -c 1 -b 32 -e signed-integer -t raw - > input.sock 2>/dev/null
