#include "recept.h"
#include "bar.h"
#include "tau.h"

#include <math.h>

double complex delta_dc(double complex cval, double complex prior_cval) {
	if (prior_cval != 0.0) {
		return cval / prior_cval;
	} else {
		return 0.0;
	}
}

/* struct exponential_smoothing_d[c] */

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

/* struct delta_d[c] */

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

/* struct distribution_d[c] */

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

/* struct weighted_distribution_d[c] */

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

/* struct apex_d[c] */

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

/* struct dynamic_window_d */

void dynamic_window_d_init(struct dynamic_window_d *dw_d_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration) {
	dw_d_ptr->td = target_duration;
	delta_d_init(&dw_d_ptr->s, has_prior, prior_value);
	exponential_smoothing_d_init(&dw_d_ptr->ed, window_size, initial_duration);
}
double dynamic_window_d_sample(struct dynamic_window_d *dw_d_ptr, double sequence_value) {
	int has_duration_since;
	double duration_since;
	double expected_duration;

	has_duration_since = delta_d_sample(&dw_d_ptr->s, sequence_value, &duration_since);
	if (has_duration_since) {
		expected_duration = exponential_smoothing_d_sample(&dw_d_ptr->ed, duration_since);
		return dw_d_ptr->td / expected_duration;
	} else {
		return dw_d_ptr->td;
	}
}

/* struct smooth_duration_d[c] */

void smooth_duration_d_init(struct smooth_duration_d *sd_d_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double initial_value) {
	dynamic_window_d_init(&sd_d_ptr->dw, target_duration, window_size, has_prior, prior_value, initial_duration);
	exponential_smoother_d_init(&sd_d_ptr->v, initial_value);
}
double smooth_duration_d_sample(struct smooth_duration_d *sd_d_ptr, double value, double sequence_value) {
	double w;

	w = dynamic_window_d_sample(&sd_d_ptr->dw, sequence_value);
	return exponential_smoother_d_sample(&sd_d_ptr->v, value, w);
}

void smooth_duration_dc_init(struct smooth_duration_dc *sd_dc_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double complex initial_value) {
	dynamic_window_d_init(&sd_dc_ptr->dw, target_duration, window_size, has_prior, prior_value, initial_duration);
	exponential_smoother_dc_init(&sd_dc_ptr->v, initial_value);
}
double complex smooth_duration_dc_sample(struct smooth_duration_dc *sd_dc_ptr, double complex value, double sequence_value) {
	double w;

	w = dynamic_window_d_sample(&sd_dc_ptr->dw, sequence_value);
	return exponential_smoother_dc_sample(&sd_dc_ptr->v, value, w);
}

/* struct smooth_duration_distribution_d[c] */

void smooth_duration_distribution_d_init(struct smooth_duration_distribution_d *sdd_d_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double initial_value) {
	dynamic_window_d_init(&sdd_d_ptr->dw, target_duration, window_size, has_prior, prior_value, initial_duration);
	distribution_d_init(&sdd_d_ptr->v, initial_value);
}
void smooth_duration_distribution_d_sample(struct smooth_duration_distribution_d *sdd_d_ptr, double value, double sequence_value, double *ave_ptr, double *dev_ptr) {
	double w;

	w = dynamic_window_d_sample(&sdd_d_ptr->dw, sequence_value);
	distribution_d_sample(&sdd_d_ptr->v, value, w, ave_ptr, dev_ptr);
}

void smooth_duration_distribution_dc_init(struct smooth_duration_distribution_dc *sdd_dc_ptr, double target_duration, double window_size, int has_prior, double prior_value, double initial_duration, double complex initial_value) {
	dynamic_window_d_init(&sdd_dc_ptr->dw, target_duration, window_size, has_prior, prior_value, initial_duration);
	distribution_dc_init(&sdd_dc_ptr->v, initial_value);
}
void smooth_duration_distribution_dc_sample(struct smooth_duration_distribution_dc *sdd_dc_ptr, double complex value, double sequence_value, double complex *ave_ptr, double complex *dev_ptr) {
	double w;

	w = dynamic_window_d_sample(&sdd_dc_ptr->dw, sequence_value);
	distribution_dc_sample(&sdd_dc_ptr->v, value, w, ave_ptr, dev_ptr);
}

/* struct time_result */

void time_result_init(struct time_result *tr_ptr) {
	tr_ptr->time_delta = 1.0;	
	tr_ptr->time_value = 0.0;
	tr_ptr->time_glissando = 0.0;
}

/* struct time_smoothing_d */

void time_smoothing_d_init(struct time_smoothing_d *ts_d_ptr, struct time_result *tr_ptr, double period, double phase, double window_factor, double complex initial_value) {
	ts_d_ptr->period = period;
	ts_d_ptr->phase = phase;
	exponential_smoother_dc_init(&ts_d_ptr->v, initial_value);
	ts_d_ptr->wf = window_factor;
	time_result_init(tr_ptr);
}
void time_smoothing_d_sample(struct time_smoothing_d *ts_d_ptr, struct time_result *tr_ptr, double time, double complex value) {
	tr_ptr->time_value = exponential_smoother_dc_sample(&ts_d_ptr->v, value * rect1((time + ts_d_ptr->phase) / ts_d_ptr->period), ts_d_ptr->period * ts_d_ptr->wf);
}

/* struct dynamic_time_smoothing_d */

void dynamic_time_smoothing_d_init(struct dynamic_time_smoothing_d *dts_d_ptr, struct time_result *tr_ptr, double period, double phase, double window_factor, double complex initial_value, double initial_delta) {
	time_smoothing_d_init(&dts_d_ptr->ts, tr_ptr, period, phase, window_factor, initial_value);
	exponential_smoother_d_init(&dts_d_ptr->glissando, initial_delta);
}
void dynamic_time_smoothing_d_update_period(struct dynamic_time_smoothing_d *dts_d_ptr, double period) {
	exponential_smoother_d_sample(&dts_d_ptr->glissando, period - dts_d_ptr->ts.period, period * dts_d_ptr->ts.wf);

	dts_d_ptr->ts.phase = dts_d_ptr->ts.phase / dts_d_ptr->ts.period * period;
	dts_d_ptr->ts.period = period;
}
void dynamic_time_smoothing_d_update_phase(struct dynamic_time_smoothing_d *dts_d_ptr, double phase) {
	dts_d_ptr->ts.phase = phase;
}
void dynamic_time_smoothing_d_glissando_sample(struct dynamic_time_smoothing_d *dts_d_ptr, struct time_result *tr_ptr, double time, double complex value, double period) {
	if (period > 0) {
		dynamic_time_smoothing_d_update_period(dts_d_ptr, period);
	}
	tr_ptr->time_glissando = dts_d_ptr->glissando.v;

	time_smoothing_d_sample(&dts_d_ptr->ts, tr_ptr, time, value);
}
void dynamic_time_smoothing_d_sample(struct dynamic_time_smoothing_d *dts_d_ptr, struct time_result *tr_ptr, double time, double complex value) {
	dynamic_time_smoothing_d_glissando_sample(dts_d_ptr, tr_ptr, time, value, 0);
}

/* percept_result */

void percept_result_polar(struct percept_result *pr_ptr) {
	pr_ptr->r   =         cabs(pr_ptr->cval);
	pr_ptr->phi = rad2tau(carg(pr_ptr->cval));
}
void percept_result_rect(struct percept_result *pr_ptr) {
	pr_ptr->cval = rect(pr_ptr->phi, pr_ptr->r);
}

/* struct monochord */

void monochord_construct(struct monochord *mc_ptr) {
	mc_ptr->period = mc_ptr->source_period * mc_ptr->ratio;
	mc_ptr->offset = mc_ptr->target_period - mc_ptr->period;
	mc_ptr->phi_offset = mc_ptr->offset / mc_ptr->target_period;
	mc_ptr->value = rect1(mc_ptr->phi_offset);
}

void monochord_init(struct monochord *mc_ptr, double source_period, double target_period, double ratio) {
	mc_ptr->source_period = source_period;
	mc_ptr->target_period = target_period;
	mc_ptr->ratio = ratio;
	monochord_construct(mc_ptr);
}

void monochord_rotate(struct monochord *mc_ptr, struct percept_result *pr_ptr) {
	pr_ptr->cval *= mc_ptr->value;
	pr_ptr->phi = fmod(pr_ptr->phi + mc_ptr->phi_offset + 0.5, 1) - 0.5;
}

/* struct period_result */

void percept_result_dup_monochord(struct percept_result *pr_dup_ptr, struct percept_result *pr_ptr, struct monochord *mc_ptr) {
	*pr_dup_ptr = *pr_ptr;
	monochord_rotate(mc_ptr, pr_dup_ptr);
}
void percept_result_superimpose(struct percept_result *pr_target_ptr, struct percept_result *pr_source_ptr) {
	pr_target_ptr->cval += pr_source_ptr->cval;
	percept_result_polar(pr_target_ptr);
}

/* struct period_percept */
struct period_percept {
	struct percept_field  field;
	double timestamp;
	struct time_result    time;
	struct percept_result value;
};

void period_percept_init(struct period_percept *pp_ptr, struct percept_field field, double timestamp, struct time_result time, struct percept_result value) {
	pp_ptr->field = field;
	pp_ptr->timestamp = timestamp;
	pp_ptr->time = time;
	pp_ptr->value = value;
}

void period_percept_superimpose_from_percept(struct period_percept *pp_source_ptr, struct period_percept *pp_target_ptr, struct monochord *mc_ptr) {
	struct percept_result mc_pr;
	percept_result_dup_monochord(&mc_pr, &pp_source_ptr->value, mc_ptr);
	percept_result_superimpose(&pp_target_ptr->value, &mc_pr);
}

/* struct period_recept */
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

void period_recept_init(struct period_recept *pr_ptr, struct period_percept *phase, struct period_percept *prior_phase) {
	double phi_t;

	pr_ptr->phase = phase;
	pr_ptr->prior_phase = prior_phase;
	pr_ptr->field.period        = (pr_ptr->phase->field.period        + pr_ptr->prior_phase->field.period)        / 2;
	pr_ptr->field.period_factor = (pr_ptr->phase->field.period_factor + pr_ptr->prior_phase->field.period_factor) / 2;
	pr_ptr->field.glissando     = (pr_ptr->phase->field.glissando     + pr_ptr->prior_phase->field.glissando)     / 2;

	pr_ptr->frequency = 1.0 / pr_ptr->field.period;

	pr_ptr->value.cval = delta_dc(pr_ptr->phase->value.cval, pr_ptr->prior_phase->value.cval);
	percept_result_polar(&pr_ptr->value);
	pr_ptr->duration = pr_ptr->phase->timestamp - pr_ptr->prior_phase->timestamp;

	if (pr_ptr->duration > 0) {
		phi_t = pr_ptr->value.phi / pr_ptr->duration;
	} else {
		phi_t = 0.0;
	}

	pr_ptr->instant_frequency = pr_ptr->frequency - phi_t;
	pr_ptr->instant_period    = 1.0 / pr_ptr->instant_frequency;
}

/* struct period_concept */
struct period_concept {
	struct period_sensor *sensor_ptr;
	struct percept_field  field;
	double weight_factor;

	struct period_percept *percept_ptr;
	struct period_percept *prior_percept_ptr;

	struct exponential_smoother_d avg_instant_period_state;
	struct delta_d instant_period_delta_state;
	struct exponential_smoother_d instant_period_stddev_state;
};

void period_concept_init(struct period_concept *pc_ptr, struct period_sensor *sensor_ptr, struct percept_field field, double weight_factor) {
	pc_ptr->sensor_ptr    = sensor_ptr;
	pc_ptr->field         = field;
	pc_ptr->weight_factor = weight_factor;

	pc_ptr->percept_ptr       = 0;
	pc_ptr->prior_percept_ptr = 0;
}

void period_concept_setup(struct period_concept *pc_ptr) {
	pc_ptr->prior_percept_ptr = pc_ptr->percept_ptr;

	exponential_smoother_d_init(&pc_ptr->avg_instant_period_state, pc_ptr->prior_percept_ptr->field.period);
	delta_d_init(&pc_ptr->instant_period_delta_state, 0, 0);
	exponential_smoother_d_init(&pc_ptr->instant_period_stddev_state, pc_ptr->prior_percept_ptr->field.period);
}

void period_concept_sample_recept(struct period_concept *pc_ptr) {
	
}

void period_concept_perceive(struct period_concept *pc_ptr) {
	if (pc_ptr->prior_percept_ptr == 0) {
		period_concept_setup(pc_ptr);
	}

	period_concept_sample_recept(pc_ptr);

	pc_ptr->prior_percept_ptr = pc_ptr->percept_ptr;
}

void period_concept_sample_percept(struct period_concept *pc_ptr, struct period_percept *percept_ptr) {
	pc_ptr->percept_ptr = percept_ptr;
	period_concept_perceive(pc_ptr);
}

#ifdef RECEPT_TEST

int main() {
	return 0;
}

#endif
