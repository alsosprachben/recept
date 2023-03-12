#!/bin/sh
cc -g -Ofast -Wall -DRECEPT_TEST bar.c screen.c sampler.c sampler_ui.c recept.c $@ -o ./recept_test
emcc  -O3 \
             -Wall -DRECEPT_TEST bar.c screen.c sampler.c sampler_ui.c recept.c $@ -o ./recept_test.html
