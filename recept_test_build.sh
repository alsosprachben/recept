#!/bin/sh
cc -g -Ofast -flto -Wall -DRECEPT_TEST $@ bar.c screen.c sampler.c sampler_ui.c recept.c -o ./recept_test
