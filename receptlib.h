#ifndef RECEPTLIB_H
#define RECEPTLIB_H

#include <complex.h>

double complex delta_dc(double complex cval, double complex prior_cval);

/* exponential smoothing (double) */
struct exponential_smoother_d;
void exponential_smoother_d_init(struct exponential_smoother_d *es_d_ptr, double initial_value);
double exponential_smoother_d_sample(struct exponential_smoother_d *es_d_ptr, double value, double factor);
/* exponential smoothing (double complex) */
struct exponential_smoother_dc;
void exponential_smoother_dc_init(struct exponential_smoother_dc *es_d_ptr, double complex initial_value);
double complex exponential_smoother_dc_sample(struct exponential_smoother_dc *es_d_ptr, double complex value, double factor);

/* exponential smoothing (double) of a fixed window size */
struct exponential_smoothing_d;
void exponential_smoothing_d_init(struct exponential_smoothing_d *esg_d_ptr, double window_size, double initial_value);
double exponential_smoothing_d_sample(struct exponential_smoothing_d *esg_d_ptr, double value);
/* exponential smoothing (double complex) of a fixed window size */
struct exponential_smoothing_dc;
void exponential_smoothing_dc_init(struct exponential_smoothing_dc *esg_dc_ptr, double window_size, double complex initial_value);
double exponential_smoothing_dc_sample(struct exponential_smoothing_dc *esg_dc_ptr, double complex value);

/* derivative of a sequence of (double) */
struct delta_d;
void delta_d_init(struct delta_d *d_d_ptr, int has_prior, double prior_sequence);
int delta_d_sample(struct delta_d *d_d_ptr, double sequence_value, double *delta_value_ptr);
/* derivative of a sequence of (double complex) */
struct delta_dc;
void delta_dc_init(struct delta_dc *d_dc_ptr, int has_prior, double complex prior_sequence);
int delta_dc_sample(struct delta_dc *d_dc_ptr, double complex sequence_value, double complex *delta_value_ptr);

/* Infinite Impulse Response (IIR) distribution, represented by exponentially smoothed average and deviation (double). */
struct distribution_d;
void distribution_d_init(struct distribution_d *dist_d_ptr, double initial_value);
void distribution_d_sample(struct distribution_d *dist_d_ptr, double value, double factor, double *ave_ptr, double *dev_ptr);
/* Infinite Impulse Response (IIR) distribution, represented by exponentially smoothed average and deviation (double complex). */
struct distribution_dc;
void distribution_dc_init(struct distribution_dc *dist_dc_ptr, double complex initial_value);
void distribution_dc_sample(struct distribution_dc *dist_dc_ptr, double complex value, double factor, double complex *ave_ptr, double complex *dev_ptr);

/* Infinite Impulse Response (IIR) distribution, represented by exponentially smoothed average and deviation, (double) with pre-defined window size. */
struct weighted_distribution_d;
void weighted_distribution_d_init(struct weighted_distribution_d *wdist_d_ptr, double initial_value, double window_size);
void weighted_distribution_d_sample(struct weighted_distribution_d *wdist_d_ptr, double value, double *ave_ptr, double *dev_ptr);
/* Infinite Impulse Response (IIR) distribution, represented by exponentially smoothed average and deviation, (double complex) with pre-defined window size. */
struct weighted_distribution_dc;
void weighted_distribution_dc_init(struct weighted_distribution_dc *wdist_dc_ptr, double complex initial_value, double window_size);
void weighted_distribution_dc_sample(struct weighted_distribution_dc *wdist_dc_ptr, double complex value, double complex *ave_ptr, double complex *dev_ptr);


/* Returns the given (double) sample if it is changing direction. That is, return on the derivative changing sign. */
struct apex_d;
void apex_d_init(struct apex_d *ax_d_ptr, int has_prior, double prior_sequence);
int apex_d_sample(struct apex_d *ax_d_ptr, double sequence_value, double *delta_value_ptr);
/* Returns the given (double complex) sample if it is changing direction. That is, return on the derivative changing sign. */
struct apex_dc;
void apex_dc_init(struct apex_dc *ax_dc_ptr, int has_prior, double complex prior_sequence);
int apex_dc_sample(struct apex_dc *ax_dc_ptr, double complex sequence_value, double complex *delta_value_ptr);

/* Provide a dynamically-adjusted (double) window size for a particular duration, by a given time sequence. */
struct dynamic_window_d;
/* Provide a target duration, and a window size over the number of given sequence events. */
void dynamic_window_d_init(struct dynamic_window_d *dw_d_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration);
/* Provide the next sequence value, and get an updated time window targeting the initialized duration. */
double dynamic_window_d_sample(struct dynamic_window_d *dw_d_ptr, double sequence_value);

/* a (double) expential smoother windowed by a (double) dynamic window */
struct smooth_duration_d;
void smooth_duration_d_init(struct smooth_duration_d *sd_d_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double initial_value);

/* a (double complex) expential smoother windowed by a (double) dynamic window */
struct smooth_duration_dc;
void smooth_duration_dc_init(struct smooth_duration_dc *sd_dc_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double complex initial_value);

/* a (double) expential smoothed distribition windowed by a (double) dynamic window */
struct smooth_duration_distribution_d;
void smooth_duration_distribution_d_init(struct smooth_duration_distribution_d *sdd_d_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double initial_value);
void smooth_duration_distribution_d_sample(struct smooth_duration_distribution_d *sdd_d_ptr, double value, double sequence_value, double *ave_ptr, double *dev_ptr);
/* a (double complex) expential smoothed distribition windowed by a (double) dynamic window */
struct smooth_duration_distribution_dc;
void smooth_duration_distribution_dc_init(struct smooth_duration_distribution_dc *sdd_dc_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double complex initial_value);
void smooth_duration_distribution_dc_sample(struct smooth_duration_distribution_dc *sdd_dc_ptr, double complex value, double sequence_value, double complex *ave_ptr, double complex *dev_ptr);

/* Infinite Impulse Response cosine transform */

/* receptive field */
struct receptive_field {
	double period;
	double phase;
	double period_factor;
	double phase_factor;
	double glissando;
};

/* percept's periodic value (Z-transform, frequency domain value, representing the state of the receptive field) */
struct receptive_value {
	double timestamp;
	double complex cval;
	double r;
	double phi;
};

void receptive_value_init(struct receptive_value *rv_ptr, double complex cval);
void receptive_value_polar(struct receptive_value *rv_ptr);
void receptive_value_rect(struct receptive_value *rv_ptr);

struct time_smoothing_d;
void time_smoothing_d_init(struct time_smoothing_d *ts_d_ptr, struct receptive_field *field_ptr, struct receptive_value *value_ptr);
void time_smoothing_d_sample(struct time_smoothing_d *ts_d_ptr, double time, double value);

/* time smoothing, but with mutable period component, tracking period delta, or the "glissando receptor factor". */
struct dynamic_time_smoothing_d;
void dynamic_time_smoothing_d_init(            struct dynamic_time_smoothing_d *dts_d_ptr, struct receptive_field *field_ptr, struct receptive_value *value_ptr, double initial_glissando);
void dynamic_time_smoothing_d_update_period(   struct dynamic_time_smoothing_d *dts_d_ptr, double period);
void dynamic_time_smoothing_d_update_phase(    struct dynamic_time_smoothing_d *dts_d_ptr, double phase);
void dynamic_time_smoothing_d_glissando_sample(struct dynamic_time_smoothing_d *dts_d_ptr, double time, double value, double period);
void dynamic_time_smoothing_d_sample(          struct dynamic_time_smoothing_d *dts_d_ptr, double time, double value);
void dynamic_time_smoothing_d_effective_field( struct dynamic_time_smoothing_d *dts_d_ptr, struct receptive_field *field_ptr);

struct monochord;
void receptive_value_dup_monochord(struct receptive_value *rv_dup_ptr, struct receptive_value *rv_ptr, struct monochord *mc_ptr);

/* monochord digital up/down converter (rotates the spectrum, shifting the frequency/period) */
struct monochord;
void monochord_init(struct monochord *mc_ptr, double source_period, double target_period, double ratio);
void monochord_rotate(struct monochord *mc_ptr, struct receptive_value *rv_ptr);

/* Helmholtz sensory layers */
/* Physical Percept: Representation of Periodic Value */
struct period_percept {
	double timestamp;
	struct receptive_field field;
	struct receptive_value value;
};
void period_percept_init(struct period_percept *pp_ptr, struct dynamic_time_smoothing_d *dts_d_ptr, double time);
void period_percept_superimpose_from_percept(struct period_percept *pp_source_ptr, struct period_percept *pp_target_ptr, struct monochord *mc_ptr);

/* Physiological Recept: Deduction of Periodic Value */
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
void period_recept_init(struct period_recept *pr_ptr, struct period_percept *phase, struct period_percept *prior_phase);

/* Psychological Concept: Persistence of Periodic Value */
/* IIR filter state (memory/persistence) */
struct period_concept_state;
void period_concept_state_init(struct period_concept_state *pcs_ptr, struct receptive_field *field_ptr);
/* representation of the concept */
struct period_concept {
	struct period_recept *recept_ptr;

	double avg_instant_period;
	double avg_instant_period_offset;

	int    has_instant_period_delta;
	double instant_period_delta;

	double instant_period_stddev;
};
void period_concept_init(struct period_concept *pc_ptr, struct period_concept_state *pcs_ptr, struct period_recept *recept_ptr);

/* Sense Organ: holds all of the state, and drives the pathways */
struct period_sensor;
struct receptive_field *period_sensor_get_receptive_field(struct period_sensor *ps_ptr);
struct receptive_value *period_sensor_get_receptive_value(struct period_sensor *ps_ptr);
struct period_concept *period_sensor_get_concept(struct period_sensor *ps_ptr);
void period_sensor_init(struct period_sensor *ps_ptr);
void period_sensor_receive(struct period_sensor *ps_ptr);
void period_sensor_sample(struct period_sensor *ps_ptr, double time, double value);
void period_sensor_update_period(struct period_sensor *ps_ptr, double period);
void period_sensor_update_phase(struct period_sensor *ps_ptr, double phase);
void period_sensor_update_from_concept(struct period_sensor *ps_ptr, struct period_concept *pc_ptr);

/* Complex Lifecycle/Frequency */
struct lifecycle {
	double max_r;
	double F; /* Free Energy, where cval.real is negative entropy, and cval.imag is negative energy. */
	double r;
	double phi;
	int    cycle;
	double lifecycle;

	double complex cval;
};

struct lifecycle;
void lifecycle_init(struct lifecycle *lc_ptr, double max_r);
double lifecycle_sample(struct lifecycle *lc_ptr, double complex cval);

struct lifecycle_derive;
void lifecycle_derive_init(struct lifecycle_derive *lcd_ptr, double max_r, double response_factor);
double lifecycle_derive_sample_direct(struct lifecycle_derive *lcd_ptr, double v1, double v2, double v3);
double lifecycle_derive_sample_avg(struct lifecycle_derive *lcd_ptr, double v1, double v2, double v3);

struct lifecycle_iter;
void lifecycle_iter_init(struct lifecycle_iter *lci_ptr, double max_r);
double lifecycle_iter_sample(struct lifecycle_iter *lci_ptr, double value);

/* Period Scale-Space */
struct scale_space_value {
	struct period_concept *concept_ptr;
	struct lifecycle *period_lifecycle_ptr;
	struct lifecycle *beat_lifecycle_ptr;
};

struct period_scale_space_sensor;
unsigned int period_scale_space_sensor_monochord_max(struct period_scale_space_sensor *sss_ptr);
struct receptive_field *period_scale_space_sensor_get_receptive_field(struct period_scale_space_sensor *sss_ptr);
void period_scale_space_sensor_set_response_period(struct period_scale_space_sensor *sss_ptr, double response_period);
void period_scale_space_sensor_set_scale_factor(struct period_scale_space_sensor *sss_ptr, double scale_factor);
void period_scale_space_sensor_init(struct period_scale_space_sensor *sss_ptr);
void period_scale_space_sensor_sample_sensor(struct period_scale_space_sensor *sss_ptr, double time, double value);
void period_scale_space_sensor_sample_monochords(struct period_scale_space_sensor *sss_ptr);
void period_scale_space_sensor_sample_lifecycle(struct period_scale_space_sensor *sss_ptr);
void period_scale_space_sensor_values(struct period_scale_space_sensor *sss_ptr, struct scale_space_value *ss_value);
void period_scale_space_sensor_sample(struct period_scale_space_sensor *sss_ptr, struct scale_space_value *ss_value, double time, double value);
void period_scale_space_sensor_init_monochord(struct period_scale_space_sensor *sss_ptr, struct monochord *mc_ptr, struct period_scale_space_sensor *target_sss_ptr, double monochord_ratio);
void period_scale_space_sensor_superimpose_monochord_on(struct period_scale_space_sensor *sss_ptr, struct period_scale_space_sensor *target_sss_ptr, struct monochord *mc_ptr);
int  period_scale_space_sensor_add_monochord(struct period_scale_space_sensor *sss_ptr, struct period_scale_space_sensor *source_sss_ptr, double monochord_ratio);

struct period_array;
struct receptive_field *period_array_get_receptive_field(struct period_array *pa_ptr);
void period_array_init(struct period_array *pa_ptr, double response_period, double octave_bandwidth, double scale_factor);
unsigned int period_array_period_sensor_max(struct period_array *pa_ptr);
unsigned int period_array_period_sensor_count(struct period_array *pa_ptr);
struct scale_space_entry *period_array_get_entries(struct period_array *pa_ptr);
int period_array_add_period_sensor(struct period_array *pa_ptr, double period, double bandwidth_factor);
int period_array_add_monochord(struct period_array *pa_ptr, int source_sss_descriptor, int target_sss_descriptor, double monochord_ratio);
void period_array_sample(struct period_array *pa_ptr, double time, double value);
void period_array_sample_sensor(struct period_array *pa_ptr, double time, double value);
void period_array_sample_lifecycle(struct period_array *pa_ptr);
void period_array_sample_monochords(struct period_array *pa_ptr);
void period_array_values(struct period_array *pa_ptr);

#endif
