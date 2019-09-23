#include "recept.h"
#include "bar.h"
#include "tau.h"

double complex delta_dc(double complex cval, double complex prior_cval) {
	if (prior_cval != 0.0) {
		return cval / prior_cval;
	} else {
		return 0.0;
	}
}

void exponential_smoother_d_init(struct exponential_smoother_d *es_d_ptr, double initial_value) {
	es_d_ptr->v = initial_value;
}
double exponential_smoother_d_sample(struct exponential_smoother_d *es_d_ptr, double value, double factor) {
	es_d_ptr->v += (value - es_d_ptr->v) / factor;
	return es_d_ptr->v;
}

void exponential_smoother_dc_init(struct exponential_smoother_dc *es_d_ptr, double complex initial_value) {
	es_d_ptr->v = initial_value;
}
double complex exponential_smoother_dc_sample(struct exponential_smoother_dc *es_d_ptr, double complex value, double factor) {
	es_d_ptr->v += (value - es_d_ptr->v) / factor;
	return es_d_ptr->v;
}

void exponential_smoothing_d_init(struct exponential_smoothing_d *esg_d_ptr, double window_size, double initial_value) {
	esg_d_ptr->w = window_size;
	exponential_smoother_d_init(&esg_d_ptr->v, initial_value);
}
double exponential_smoothing_d_sample(struct exponential_smoothing_d *esg_d_ptr, double value) {
	return exponential_smoother_d_sample(&esg_d_ptr->v, value, esg_d_ptr->w);
}

void exponential_smoothing_dc_init(struct exponential_smoothing_dc *esg_dc_ptr, double window_size, double complex initial_value) {
	esg_dc_ptr->w = window_size;
	exponential_smoother_dc_init(&esg_dc_ptr->v, initial_value);
}
double exponential_smoothing_dc_sample(struct exponential_smoothing_dc *esg_dc_ptr, double complex value) {
	return exponential_smoother_dc_sample(&esg_dc_ptr->v, value, esg_dc_ptr->w);
}

void delta_d_init(struct delta_d *d_d_ptr, int has_prior, double prior_sequence) {
	d_d_ptr->has_prior = has_prior;
	d_d_ptr->prior_sequence = prior_sequence;
}
int delta_d_sample(struct delta_d *d_d_ptr, double sequence_value, double *delta_value_ptr) {
	int has_value;
	if (d_d_ptr->has_prior) {
		has_value = 0;
	} else {
		has_value = 1;
		*delta_value_ptr = sequence_value - d_d_ptr->prior_sequence;
	}

	return has_value;
}

void delta_dc_init(struct delta_dc *d_dc_ptr, int has_prior, double complex prior_sequence) {
	d_dc_ptr->has_prior = has_prior;
	d_dc_ptr->prior_sequence = prior_sequence;
}
int delta_dc_sample(struct delta_dc *d_dc_ptr, double complex sequence_value, double complex *delta_value_ptr) {
	int has_value;
	if (d_dc_ptr->has_prior) {
		has_value = 0;
	} else {
		has_value = 1;
		*delta_value_ptr = delta_dc(sequence_value, d_dc_ptr->prior_sequence);
	}

	return has_value;
}

void distribution_d_init(struct distribution_d *dist_d_ptr, double initial_value) {
	exponential_smoother_d_init(&dist_d_ptr->ave, initial_value);
	exponential_smoother_d_init(&dist_d_ptr->dev, initial_value);
}
void distribution_d_sample(struct distribution_d *dist_d_ptr, double value, double factor, double *ave_ptr, double *dev_ptr) {
	double deviation = fabs(dist_d_ptr->ave.v - value);

	exponential_smoother_d_sample(&dist_d_ptr->ave, value, factor);
	exponential_smoother_d_sample(&dist_d_ptr->dev, deviation, factor);

	*ave_ptr = dist_d_ptr->ave.v;
	*dev_ptr = dist_d_ptr->dev.v;
}

void distribution_dc_init(struct distribution_dc *dist_dc_ptr, double complex initial_value) {
	exponential_smoother_dc_init(&dist_dc_ptr->ave, initial_value);
	exponential_smoother_dc_init(&dist_dc_ptr->dev, initial_value);
}
void distribution_dc_sample(struct distribution_dc *dist_dc_ptr, double complex value, double factor, double complex *ave_ptr, double complex *dev_ptr) {
	double deviation = cabs(delta_dc(dist_dc_ptr->ave.v, value));

	exponential_smoother_dc_sample(&dist_dc_ptr->ave, value, factor);
	exponential_smoother_dc_sample(&dist_dc_ptr->dev, deviation, factor);

	*ave_ptr = dist_dc_ptr->ave.v;
	*dev_ptr = dist_dc_ptr->dev.v;
}

void weighted_distribution_d_init(struct weighted_distribution_d *wdist_d_ptr, double initial_value, double window_size) {
	distribution_d_init(&wdist_d_ptr->dist, initial_value);
	wdist_d_ptr->w = window_size;
}
void weighted_distribution_d_sample(struct weighted_distribution_d *wdist_d_ptr, double value, double *ave_ptr, double *dev_ptr) {
	distribution_d_sample(&wdist_d_ptr->dist, value, wdist_d_ptr->w, ave_ptr, dev_ptr);
}

void weighted_distribution_dc_init(struct weighted_distribution_dc *wdist_dc_ptr, double complex initial_value, double window_size) {
	distribution_dc_init(&wdist_dc_ptr->dist, initial_value);
	wdist_dc_ptr->w = window_size;
}
void weighted_distribution_dc_sample(struct weighted_distribution_dc *wdist_dc_ptr, double complex value, double complex *ave_ptr, double complex *dev_ptr) {
	distribution_dc_sample(&wdist_dc_ptr->dist, value, wdist_dc_ptr->w, ave_ptr, dev_ptr);
}

void apex_d_init(struct apex_d *ax_d_ptr, int has_prior, double prior_sequence) {
	delta_d_init(&ax_d_ptr->delta, has_prior, prior_sequence);
	ax_d_ptr->prior_is_positive = 1;
}
int apex_d_sample(struct apex_d *ax_d_ptr, double sequence_value, double *delta_value_ptr) {
	int has_value;
	int is_positive;

	has_value = delta_d_sample(&ax_d_ptr->delta, sequence_value, delta_value_ptr);
	if (has_value) {
		is_positive = *delta_value_ptr >= 0;
		if (ax_d_ptr->prior_is_positive != is_positive) {
			ax_d_ptr->prior_is_positive = is_positive;
			return 1;
		}
	}

	return 0;
}
void apex_dc_init(struct apex_dc *ax_dc_ptr, int has_prior, double complex prior_sequence) {
	delta_dc_init(&ax_dc_ptr->delta, has_prior, prior_sequence);
	ax_dc_ptr->prior_is_positive = 1;
}
int apex_dc_sample(struct apex_dc *ax_dc_ptr, double complex sequence_value, double complex *delta_value_ptr) {
	int has_value;
	int is_positive;

	has_value = delta_dc_sample(&ax_dc_ptr->delta, sequence_value, delta_value_ptr);
	if (has_value) {
		is_positive = creal(*delta_value_ptr) >= 0;
		if (ax_dc_ptr->prior_is_positive != is_positive) {
			ax_dc_ptr->prior_is_positive = is_positive;
			return 1;
		}
	}

	return 0;
}



#ifdef RECEPT_TEST

int main() {
	return 0;
}

#endif
