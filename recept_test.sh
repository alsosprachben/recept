#!/bin/sh
mkfifo sox.pipe

music_path="${1}"

sync_delay="${2}"
if [ "${sync_delay}" ]
then
	true
else
	sync_delay=0.6
fi

enc="--buffer 735 -r 44100 -c 1 -b 32 -e signed-integer -t raw"
sox "${music_path}" $enc - 2>/dev/null | tee sox.pipe | play -q $enc - &
cat sox.pipe | sox $enc - -t raw - pad "${sync_delay}" | pypy recept_mic.py  44100 735 10
