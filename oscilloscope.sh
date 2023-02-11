#!/bin/sh
COLUMNS="${1}"
LINES="${2}"
echo rec -q --buffer $(printf "print ((%i - 1) * 2)" $LINES | python3) -r 44100 -c 1 -b 16 -e signed-integer -t raw - "|" ./osc_test $COLUMNS $LINES
exit
rec -q --buffer $(printf "print ((%i - 1) * 2)" $LINES | python3) -r 44100 -c 1 -b 16 -e signed-integer -t raw - | ./osc_test $COLUMNS $LINES
