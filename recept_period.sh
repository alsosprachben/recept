#!/bin/sh
sample_rate=44100
chunk_size=735
frame_size=735
oversample=10

chunk_size=1
frame_size=1
oversample=1

pypy recept_period.py $sample_rate $chunk_size $frame_size $oversample
