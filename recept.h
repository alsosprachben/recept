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
	struct receptive_field *field_ptr;
	struct receptive_value *value_ptr;
	struct exponential_smoother_dc v;
};

struct dynamic_time_smoothing_d {
	struct time_smoothing_d ts;
	struct exponential_smoother_d period_state;
	struct exponential_smoother_d glissando_state;
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
	double timestamp;
	struct receptive_field field;
	struct receptive_value value;
};
struct period_recept {
	struct receptive_field field;
	struct period_percept *phase;
	struct period_percept *prior_phase;

	double frequency;
	double instant_period;
	double instant_frequency;
	struct receptive_value value;

	double duration;
};
struct period_concept_state {
	struct exponential_smoother_d avg_instant_period_state;
	struct delta_d                instant_period_delta_state;
	struct exponential_smoother_d instant_period_stddev_state;
};
struct period_concept {
	struct period_recept *recept_ptr;

	double avg_instant_period;
	double avg_instant_period_offset;

	int    has_instant_period_delta;
	double instant_period_delta;

	double instant_period_stddev;
};
struct period_sensor {
	struct receptive_field field;
	struct receptive_value value;
	struct dynamic_time_smoothing_d sensor_state;

	struct period_percept percept;
	int                   has_prior_percept;
	struct period_percept prior_percept;
	struct period_recept  recept;
	struct period_concept concept;
	struct period_concept_state concept_state;
};





#endif
