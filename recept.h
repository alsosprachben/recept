#ifndef RECEPT_H
#define RECEPT_H

#include "receptlib.h"

struct exponential_smoother_d {
	double v;
};
struct exponential_smoother_dc {
	double complex v;
};

struct exponential_smoothing_d {
	struct exponential_smoother_d v;
	double w;
};
struct exponential_smoothing_dc {
	struct exponential_smoother_dc v;
	double w;
};

struct delta_d {
	int has_prior;
	double prior_sequence;
};
struct delta_dc {
	int has_prior;
	double complex prior_sequence;
};

struct distribution_d {
	struct exponential_smoother_d ave;
	struct exponential_smoother_d dev;
};
struct distribution_dc {
	struct exponential_smoother_dc ave;
	struct exponential_smoother_dc dev;
};

struct weighted_distribution_d {
	struct distribution_d dist;
	double w;
};
struct weighted_distribution_dc {
	struct distribution_dc dist;
	double w;
};

struct apex_d {
	struct delta_d delta;
	int prior_is_positive;
};
struct apex_dc {
	struct delta_dc delta;
	int prior_is_positive;
};

#endif
