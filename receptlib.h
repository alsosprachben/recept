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

#endif
