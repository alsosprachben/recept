#!/bin/sh
COLUMNS="${1}"
LINES="${2}"
rec -q --buffer $(printf "print (%i - 1) * 2" $LINES | python) -r 44100 -c 2 -b 16 -e signed-integer -t raw - | ./osc_test $COLUMNS $LINES
