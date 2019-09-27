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
struct time_smoothing_d;
void time_smoothing_d_init(struct time_smoothing_d *ts_d_ptr, double period, double phase, double window_factor, double complex initial_value);
double complex time_smoothing_d_sample(struct time_smoothing_d *ts_d_ptr, double time, double complex value);

#endif
