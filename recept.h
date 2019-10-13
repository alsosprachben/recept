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

struct dynamic_window_d {
	double td;
	struct delta_d s;
	struct exponential_smoothing_d ed;
};

struct smooth_duration_d {
	struct dynamic_window_d dw;
	struct exponential_smoother_d v;
};
struct smooth_duration_dc {
	struct dynamic_window_d dw;
	struct exponential_smoother_dc v;
};

struct smooth_duration_distribution_d {
	struct dynamic_window_d dw;
	struct distribution_d v;
};
struct smooth_duration_distribution_dc {
	struct dynamic_window_d dw;
	struct distribution_dc v;
};

struct time_smoothing_d {
	double period;
	double phase;
	struct exponential_smoother_dc v;
	double wf;
};

struct dynamic_time_smoothing_d {
	struct time_smoothing_d ts;
	struct exponential_smoother_d glissando;
};

struct monochord {
	double source_period;
	double target_period;
	double ratio;

	double period;
	double offset;
	double phi_offset;
	double complex value;
};

struct period_percept {
	struct percept_field  field;
	double timestamp;
	struct time_result    time;
	struct percept_result value;
};
struct period_recept {
	struct percept_field   field;
	struct period_percept *phase;
	struct period_percept *prior_phase;

	double frequency;
	double instant_period;
	double instant_frequency;
	struct percept_result value;

	double duration;
};
struct period_concept {
	struct period_sensor *sensor_ptr;
	struct percept_field  field;
	double weight_factor;

	struct period_percept *percept_ptr;
	struct period_percept prior_percept;
	int               has_prior_percept;

	struct exponential_smoother_d avg_instant_period_state;
	struct delta_d instant_period_delta_state;
	struct exponential_smoother_d instant_period_stddev_state;
};





#endif
