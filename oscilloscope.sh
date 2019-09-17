#!/bin/sh
COLUMNS="${1}"
LINES="${2}"
rec -q --buffer $(printf "print (%i - 1) * 4" $LINES | python) -r 44100 -c 1 -b 32 -e signed-integer -t raw - | ./osc_test $COLUMNS $LINES
