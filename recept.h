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

struct period_concept_state {
	struct exponential_smoother_d avg_instant_period_state;
	struct delta_d                instant_period_delta_state;
	struct exponential_smoother_d instant_period_stddev_state;
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

struct lifecycle_derive {
	struct lifecycle lc;
	double response_factor;
	struct exponential_smoother_d d_avg_state;
	struct exponential_smoother_d dd_avg_state;
	
	double d;
	double dd;
	double complex cval;

	double d_avg;
	double dd_avg;
	double complex cval_avg;
};

struct lifecycle_iter {
	struct lifecycle lc;
	struct delta_d d_state;
	struct delta_d dd_state;

	double d;
	double dd;
	double complex cval;
};

struct period_scale_space_sensor;
struct period_scale_space_sensor {
	struct receptive_field field;
	double response_period;
	double scale_factor;

	struct period_sensor period_sensors[3];

	struct lifecycle_derive period_lifecycle;
	struct lifecycle_iter   beat_lifecycle;

	struct monochord_entry {
		struct period_scale_space_sensor *source_sss_ptr;
		struct monochord       monochord;
	} monochords[256];
	unsigned int     monochord_count;
};




#endif
