#ifndef RECEPT_H
#define RECEPT_H

#include <complex.h>

double complex delta_dc(double complex cval, double complex prior_cval);

/* exponential smoothing (double) */
struct exponential_smoother_d {
	double v;
};
/* exponential smoothing (double complex) */
struct exponential_smoother_dc {
	double complex v;
};

/* exponential smoothing (double) of a fixed window size */
struct exponential_smoothing_d {
	struct exponential_smoother_d;
	double window_size;
};
/* exponential smoothing (double complex) of a fixed window size */
struct exponential_smoothing_dc {
	struct exponential_smoother_dc;
	double window_size;
};

/* derivative of a sequence of (double) */
struct delta_d {
	double prior_sequence;
};
/* derivative of a sequence of (double complex) */
struct delta_dc {
	double complex prior_sequence;
};

/* Infinite Impulse Response (IIR) distribution, represented by exponentially smoothed average and deviation (double). */
struct distribution_d {
	struct exponential_smoother_d ave;
	struct exponential_smoother_d dev;
};
/* Infinite Impulse Response (IIR) distribution, represented by exponentially smoothed average and deviation (double complex). */
struct distribution_d {
	struct exponential_smoother_dc ave;
	struct exponential_smoother_dc dev;
};

#endif
