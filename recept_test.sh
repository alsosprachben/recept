#!/bin/sh
./recept_test -c $(tput cols) -l $(tput lines) -r 44100 -f 60 -b 32 -p input.sock
