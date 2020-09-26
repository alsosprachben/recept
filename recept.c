#include <math.h>

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

/* struct exponential_smoothing_d[c] */

void exponential_smoother_d_init(struct exponential_smoother_d *es_d_ptr, double initial_value) {
	es_d_ptr->v = initial_value;
}
double exponential_smoother_d_sample(struct exponential_smoother_d *es_d_ptr, double value, double factor) {
	es_d_ptr->v += (value - es_d_ptr->v) / factor;
	return es_d_ptr->v;
}

void exponential_smoother_dc_init(struct exponential_smoother_dc *es_dc_ptr, double complex initial_value) {
	es_dc_ptr->v = initial_value;
}
double complex exponential_smoother_dc_sample(struct exponential_smoother_dc *es_dc_ptr, double complex value, double factor) {
	es_dc_ptr->v += (value - es_dc_ptr->v) / factor;
	return es_dc_ptr->v;
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

/* struct time_smoothing_d */

void time_smoothing_d_init(struct time_smoothing_d *ts_d_ptr, struct receptive_field *field_ptr, struct receptive_value *value_ptr) {
	ts_d_ptr->field_ptr = field_ptr;
	ts_d_ptr->value_ptr = value_ptr;
	exponential_smoother_dc_init(&ts_d_ptr->v, value_ptr->cval);
}
void time_smoothing_d_sample(struct time_smoothing_d *ts_d_ptr, double time, double value) {
	ts_d_ptr->value_ptr->cval = exponential_smoother_dc_sample(&ts_d_ptr->v, rect1((time + ts_d_ptr->field_ptr->phase) / ts_d_ptr->field_ptr->period) * value, ts_d_ptr->field_ptr->period * ts_d_ptr->field_ptr->period_factor);
	receptive_value_polar(ts_d_ptr->value_ptr);
	ts_d_ptr->value_ptr->timestamp = time;
}

/* struct dynamic_time_smoothing_d */

void dynamic_time_smoothing_d_init(struct dynamic_time_smoothing_d *dts_d_ptr, struct receptive_field *field_ptr, struct receptive_value *value_ptr, double initial_glissando) {
	time_smoothing_d_init(&dts_d_ptr->ts, field_ptr, value_ptr);
	exponential_smoother_d_init(&dts_d_ptr->period_state,    field_ptr->period);
	exponential_smoother_d_init(&dts_d_ptr->glissando_state, initial_glissando);
}
void dynamic_time_smoothing_d_update_period(struct dynamic_time_smoothing_d *dts_d_ptr, double period) {
	exponential_smoother_d_sample(&dts_d_ptr->period_state,    period,                                   period * dts_d_ptr->ts.field_ptr->period_factor);
	exponential_smoother_d_sample(&dts_d_ptr->glissando_state, period - dts_d_ptr->ts.field_ptr->period, period * dts_d_ptr->ts.field_ptr->period_factor);

	dts_d_ptr->ts.field_ptr->phase = dts_d_ptr->ts.field_ptr->phase / dts_d_ptr->ts.field_ptr->period * period;
	dts_d_ptr->ts.field_ptr->period = period;
}
void dynamic_time_smoothing_d_update_phase(struct dynamic_time_smoothing_d *dts_d_ptr, double phase) {
	dts_d_ptr->ts.field_ptr->phase = phase;
}
void dynamic_time_smoothing_d_glissando_sample(struct dynamic_time_smoothing_d *dts_d_ptr, double time, double value, double period) {
	if (period > 0) {
		dynamic_time_smoothing_d_update_period(dts_d_ptr, period);
	}
	time_smoothing_d_sample(&dts_d_ptr->ts, time, value);
}
void dynamic_time_smoothing_d_sample(struct dynamic_time_smoothing_d *dts_d_ptr, double time, double value) {
	dynamic_time_smoothing_d_glissando_sample(dts_d_ptr, time, value, 0);
}
void dynamic_time_smoothing_d_effective_field(struct dynamic_time_smoothing_d *dts_d_ptr, struct receptive_field *field_ptr) {
	*field_ptr = *dts_d_ptr->ts.field_ptr;

	field_ptr->period = dts_d_ptr->period_state.v;
	field_ptr->glissando = dts_d_ptr->period_state.v;
}

/* receptive_field */
void receptive_field_init(struct receptive_field *field_ptr) {
	field_ptr->period = 0;
	field_ptr->phase = 0;
	field_ptr->period_factor = 0;
	field_ptr->phase_factor = 0;
	field_ptr->glissando = 0;
}

/* receptive_value */

void receptive_value_polar(struct receptive_value *rv_ptr) {
	rv_ptr->r   =         cabs(rv_ptr->cval);
	rv_ptr->phi = rad2tau(carg(rv_ptr->cval));
}
void receptive_value_rect(struct receptive_value *rv_ptr) {
	rv_ptr->cval = rect(rv_ptr->phi, rv_ptr->r);
}
void receptive_value_init(struct receptive_value *rv_ptr, double complex cval) {
	rv_ptr->cval = cval;
	receptive_value_polar(rv_ptr);
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

void monochord_rotate(struct monochord *mc_ptr, struct receptive_value *rv_ptr) {
	rv_ptr->cval *= mc_ptr->value;
	rv_ptr->phi = fmod(rv_ptr->phi + mc_ptr->phi_offset + 0.5, 1) - 0.5;
}

/* struct period_result */

void receptive_value_dup_monochord(struct receptive_value *rv_dup_ptr, struct receptive_value *rv_ptr, struct monochord *mc_ptr) {
	*rv_dup_ptr = *rv_ptr;
	monochord_rotate(mc_ptr, rv_dup_ptr);
}
void receptive_value_superimpose(struct receptive_value *rv_target_ptr, struct receptive_value *rv_source_ptr) {
	rv_target_ptr->cval += rv_source_ptr->cval;
	receptive_value_polar(rv_target_ptr);
}

/* struct period_percept */
void period_percept_init(struct period_percept *pp_ptr, struct dynamic_time_smoothing_d *dts_d_ptr, double time) {
	dynamic_time_smoothing_d_effective_field(dts_d_ptr, &pp_ptr->field);
	pp_ptr->value = *dts_d_ptr->ts.value_ptr;
	pp_ptr->timestamp = time;
}

void period_percept_superimpose_from_percept(struct period_percept *pp_source_ptr, struct period_percept *pp_target_ptr, struct monochord *mc_ptr) {
	struct receptive_value mc_pr;
	receptive_value_dup_monochord(&mc_pr, &pp_source_ptr->value, mc_ptr);
	receptive_value_superimpose(&pp_target_ptr->value, &mc_pr);
}

/* struct period_recept */
void period_recept_init(struct period_recept *pr_ptr, struct period_percept *phase, struct period_percept *prior_phase) {
	double phi_t;

	pr_ptr->phase = phase;
	pr_ptr->prior_phase = prior_phase;
	pr_ptr->field = pr_ptr->phase->field;
	pr_ptr->field.period        = (pr_ptr->phase->field.period        + pr_ptr->prior_phase->field.period)        / 2;
	pr_ptr->field.glissando     = (pr_ptr->phase->field.glissando     + pr_ptr->prior_phase->field.glissando)     / 2;

	pr_ptr->frequency = 1.0 / pr_ptr->field.period;

	pr_ptr->value.timestamp = pr_ptr->phase->value.timestamp;
	pr_ptr->value.cval = delta_dc(pr_ptr->phase->value.cval, pr_ptr->prior_phase->value.cval);
	receptive_value_polar(&pr_ptr->value);
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
void period_concept_state_init(struct period_concept_state *pcs_ptr, struct receptive_field *field_ptr) { 
	exponential_smoother_d_init(&pcs_ptr->avg_instant_period_state, field_ptr->period);
	delta_d_init(&pcs_ptr->instant_period_delta_state, 0, 0);
	exponential_smoother_d_init(&pcs_ptr->instant_period_stddev_state, field_ptr->period);
}

void period_concept_init(struct period_concept *pc_ptr, struct period_concept_state *pcs_ptr, struct period_recept *recept_ptr) {
	pc_ptr->recept_ptr = recept_ptr;

	/* average instantaneous period */
	pc_ptr->avg_instant_period = exponential_smoother_d_sample(&pcs_ptr->avg_instant_period_state, recept_ptr->instant_period, recept_ptr->field.period * recept_ptr->field.phase_factor);
	pc_ptr->avg_instant_period_offset = pc_ptr->avg_instant_period - recept_ptr->field.period;

	/* deviation of average */
	pc_ptr->has_instant_period_delta = delta_d_sample(&pcs_ptr->instant_period_delta_state, pc_ptr->avg_instant_period, &pc_ptr->instant_period_delta);
	if ( ! pc_ptr->has_instant_period_delta) {
		pc_ptr->instant_period_delta = pc_ptr->avg_instant_period;
		pc_ptr->has_instant_period_delta = 1;
	}
	/* standard deviation of average (dis-convergence on an average instant period) */
	pc_ptr->instant_period_stddev = exponential_smoother_d_sample(&pcs_ptr->instant_period_stddev_state, fabs(pc_ptr->instant_period_delta), fabs(recept_ptr->instant_period * recept_ptr->field.phase_factor));
}

/* struct period sensor */
struct receptive_field *period_sensor_get_receptive_field(struct period_sensor *ps_ptr) {
	return &ps_ptr->field;
}
struct receptive_value *period_sensor_get_receptive_value(struct period_sensor *ps_ptr) {
	return &ps_ptr->value;
}
struct period_concept *period_sensor_get_concept(struct period_sensor *ps_ptr) {
	return &ps_ptr->concept;
}
void period_sensor_init(struct period_sensor *ps_ptr) {
	dynamic_time_smoothing_d_init(&ps_ptr->sensor_state, &ps_ptr->field, &ps_ptr->value, 0);
	period_concept_state_init(&ps_ptr->concept_state, &ps_ptr->field);
	ps_ptr->has_prior_percept = 0;
}

void period_sensor_receive(struct period_sensor *ps_ptr) {
	period_recept_init(&ps_ptr->recept, &ps_ptr->percept, &ps_ptr->prior_percept);
	period_concept_init(&ps_ptr->concept, &ps_ptr->concept_state, &ps_ptr->recept);
}

void period_sensor_sample(struct period_sensor *ps_ptr, double time, double value) {
	if (ps_ptr->has_prior_percept) {
		ps_ptr->prior_percept = ps_ptr->percept;
	}
	dynamic_time_smoothing_d_sample(&ps_ptr->sensor_state, time, value);
	period_percept_init(&ps_ptr->percept, &ps_ptr->sensor_state, time);
	if ( ! ps_ptr->has_prior_percept) {
		/* when no prior, make prior the same as current */
		ps_ptr->prior_percept = ps_ptr->percept;
		ps_ptr->has_prior_percept = 1;
	}

	period_sensor_receive(ps_ptr);
}

void period_sensor_update_period(struct period_sensor *ps_ptr, double period) {
	ps_ptr->field.period = period;
	dynamic_time_smoothing_d_update_period(&ps_ptr->sensor_state, period);
}
void period_sensor_update_phase(struct period_sensor *ps_ptr, double phase) {
	ps_ptr->field.phase = phase;
	dynamic_time_smoothing_d_update_phase(&ps_ptr->sensor_state, phase);
}
void period_sensor_update_from_concept(struct period_sensor *ps_ptr, struct period_concept *pc_ptr) {
	if (pc_ptr->avg_instant_period > 2.0) {
		period_sensor_update_period(ps_ptr, pc_ptr->avg_instant_period);
	}
}

/* Scale-Space Event Lifecycle Sensors */

/* struct lifecycle */
void lifecycle_init(struct lifecycle *lc_ptr, double max_r) {
	lc_ptr->max_r = max_r;
	lc_ptr->F = 0.0;
	lc_ptr->r = 0.0;
	lc_ptr->phi = 0.0;
	lc_ptr->cycle = 0;
	lc_ptr->lifecycle = 0.0;
}
double lifecycle_sample(struct lifecycle *lc_ptr, double complex cval) {
	double prev_phi;

	lc_ptr->cval = cval;
	lc_ptr->F = creal(cval) - cimag(cval);
	prev_phi = lc_ptr->phi;
	lc_ptr->r   =         cabs(lc_ptr->cval);
	lc_ptr->phi = rad2tau(carg(lc_ptr->cval));
	if (        lc_ptr->phi - prev_phi >  0.5) {
		lc_ptr->cycle--;
	}  else if (lc_ptr->phi - prev_phi < -0.5) {
		lc_ptr->cycle++;
	}

	lc_ptr->lifecycle = lc_ptr->cycle + lc_ptr->phi;

	return lc_ptr->lifecycle;
}

/* struct livecycle_derive (struct livecycle) */
void lifecycle_derive_init(struct lifecycle_derive *lcd_ptr, double max_r, double response_factor) {
	lifecycle_init(&lcd_ptr->lc, max_r);
	lcd_ptr->response_factor = response_factor;

	exponential_smoother_d_init(&lcd_ptr->d_avg_state, 0.0);
	exponential_smoother_d_init(&lcd_ptr->dd_avg_state, 0.0);
	/* direct samples */
	lcd_ptr->d    = 0.0;
	lcd_ptr->dd   = 0.0;
	lcd_ptr->cval = CMPLX(0.0, 0.0);
	/* avg samples */
	lcd_ptr->d_avg    = 0.0;
	lcd_ptr->dd_avg   = 0.0;
	lcd_ptr->cval_avg = CMPLX(0.0, 0.0);
}

void lifecycle_derive_derive(struct lifecycle_derive *lcd_ptr, double v1, double v2, double v3) {
	double d1, d2; /* edges of v1, v2, v3 - original sequence */
	double dd;     /* edges of   d1, d2   - first  derivative */
	               /*              dd     - second derivative */

	d1 = v2 - v1;
	d2 = v3 - v2;
	dd = d2 - d1;

	lcd_ptr->d  = d1;
	lcd_ptr->dd = dd;
}

double lifecycle_derive_sample_direct(struct lifecycle_derive *lcd_ptr, double v1, double v2, double v3) {
	lifecycle_derive_derive(lcd_ptr, v1, v2, v3);
	lcd_ptr->cval = CMPLX(lcd_ptr->d, lcd_ptr->dd);
	return lifecycle_sample(&lcd_ptr->lc, lcd_ptr->cval);
}

double lifecycle_derive_sample_avg(struct lifecycle_derive *lcd_ptr, double v1, double v2, double v3) {
	lifecycle_derive_derive(lcd_ptr, v1, v2, v3);

	lcd_ptr->d_avg  = exponential_smoother_d_sample(&lcd_ptr->d_avg_state,  lcd_ptr->d,  lcd_ptr->response_factor);
	lcd_ptr->dd_avg = exponential_smoother_d_sample(&lcd_ptr->dd_avg_state, lcd_ptr->dd, lcd_ptr->response_factor);

	lcd_ptr->cval_avg = CMPLX(lcd_ptr->d_avg, lcd_ptr->dd_avg);
	return lifecycle_sample(&lcd_ptr->lc, lcd_ptr->cval_avg);
}

/* struct livecycle_iter (struct livecycle) */
void lifecycle_iter_init(struct lifecycle_iter *lci_ptr, double max_r) {
	lifecycle_init(&lci_ptr->lc, max_r);
	delta_d_init(&lci_ptr->d_state, 0, 0.0);
	delta_d_init(&lci_ptr->dd_state, 0, 0.0);
	lci_ptr->d    = 0.0;
	lci_ptr->dd   = 0.0;
	lci_ptr->cval = CMPLX(0.0, 0.0);
}

double lifecycle_iter_sample(struct lifecycle_iter *lci_ptr, double value) {
	(void) delta_d_sample(&lci_ptr->d_state,  value,      &lci_ptr->d);
	(void) delta_d_sample(&lci_ptr->dd_state, lci_ptr->d, &lci_ptr->dd);
	lci_ptr->cval = CMPLX(lci_ptr->d, lci_ptr->dd);
	return lifecycle_sample(&lci_ptr->lc, lci_ptr->cval);
}

/* Period Scale-Space */
unsigned int period_scale_space_sensor_monochord_max(struct period_scale_space_sensor *sss_ptr) {
	return sizeof (sss_ptr->monochords) / sizeof (sss_ptr->monochords[0]);
}
struct receptive_field *period_scale_space_sensor_get_receptive_field(struct period_scale_space_sensor *sss_ptr) {
	return &sss_ptr->field;
}
void period_scale_space_sensor_set_response_period(struct period_scale_space_sensor *sss_ptr, double response_period) {
	sss_ptr->response_period = response_period;
}
void period_scale_space_sensor_set_scale_factor(struct period_scale_space_sensor *sss_ptr, double scale_factor) {
	sss_ptr->scale_factor = scale_factor;
}
void period_scale_space_sensor_init(struct period_scale_space_sensor *sss_ptr) {
	struct receptive_field *field_ptr;
	struct receptive_value *value_ptr;
	int i;

	for (i = 0; i < 3; i++) {	
		field_ptr = period_sensor_get_receptive_field(&sss_ptr->period_sensors[i]);
		*field_ptr = sss_ptr->field;
		field_ptr->period_factor *= pow(sss_ptr->scale_factor, -1.0 - i);
		value_ptr = period_sensor_get_receptive_value(&sss_ptr->period_sensors[i]);
		value_ptr->cval = CMPLX(0.0, 0.0);
		period_sensor_init(&sss_ptr->period_sensors[i]);
	}

	lifecycle_derive_init(&sss_ptr->period_lifecycle, sss_ptr->field.period, sss_ptr->response_period);
	lifecycle_iter_init(  &sss_ptr->beat_lifecycle,   sss_ptr->field.period);

	sss_ptr->monochord_count = 0;
}

void period_scale_space_sensor_sample_sensor(struct period_scale_space_sensor *sss_ptr, double time, double value) {
	period_sensor_sample(&sss_ptr->period_sensors[0], time, value);
	period_sensor_sample(&sss_ptr->period_sensors[1], time, value);
	period_sensor_sample(&sss_ptr->period_sensors[2], time, value);
}

void period_scale_space_sensor_sample_monochords(struct period_scale_space_sensor *sss_ptr) {
	int i;
	for (i = 0; i < sss_ptr->monochord_count; i++) {
		period_scale_space_sensor_superimpose_monochord_on(sss_ptr->monochords[i].source_sss_ptr, sss_ptr, &sss_ptr->monochords[i].monochord);
	}
}

void period_scale_space_sensor_sample_lifecycle(struct period_scale_space_sensor *sss_ptr) {
	lifecycle_derive_sample_avg(&sss_ptr->period_lifecycle,
		sss_ptr->period_sensors[0].percept.value.r, 
		sss_ptr->period_sensors[1].percept.value.r, 
		sss_ptr->period_sensors[2].percept.value.r);
	lifecycle_iter_sample(&sss_ptr->beat_lifecycle, sss_ptr->period_lifecycle.lc.lifecycle);
}

void period_scale_space_sensor_values(struct period_scale_space_sensor *sss_ptr, struct scale_space_value *ss_value) {
	ss_value->concept_ptr          = &sss_ptr->period_sensors[0].concept;
	ss_value->period_lifecycle_ptr = &sss_ptr->period_lifecycle.lc;
	ss_value->beat_lifecycle_ptr   = &sss_ptr->beat_lifecycle.lc;
}

void period_scale_space_sensor_sample(struct period_scale_space_sensor *sss_ptr, struct scale_space_value *ss_value, double time, double value) {
	period_scale_space_sensor_sample_sensor(sss_ptr, time, value);
	period_scale_space_sensor_sample_monochords(sss_ptr);
	period_scale_space_sensor_sample_lifecycle(sss_ptr);
	period_scale_space_sensor_values(sss_ptr, ss_value);
}

void period_scale_space_sensor_init_monochord(struct period_scale_space_sensor *sss_ptr, struct monochord *mc_ptr, struct period_scale_space_sensor *target_sss_ptr, double monochord_ratio) {
	monochord_init(mc_ptr, sss_ptr->field.period, target_sss_ptr->field.period, monochord_ratio);
}

void period_scale_space_sensor_superimpose_monochord_on(struct period_scale_space_sensor *sss_ptr, struct period_scale_space_sensor *target_sss_ptr, struct monochord *mc_ptr) {
	period_percept_superimpose_from_percept(&sss_ptr->period_sensors[0].percept, &target_sss_ptr->period_sensors[0].percept, mc_ptr);
	period_percept_superimpose_from_percept(&sss_ptr->period_sensors[1].percept, &target_sss_ptr->period_sensors[1].percept, mc_ptr);
	period_percept_superimpose_from_percept(&sss_ptr->period_sensors[2].percept, &target_sss_ptr->period_sensors[2].percept, mc_ptr);
	
	period_sensor_receive(&target_sss_ptr->period_sensors[0]);
	period_sensor_receive(&target_sss_ptr->period_sensors[1]);
	period_sensor_receive(&target_sss_ptr->period_sensors[2]);
}

int period_scale_space_sensor_add_monochord(struct period_scale_space_sensor *sss_ptr, struct period_scale_space_sensor *source_sss_ptr, double monochord_ratio) {
	if (sss_ptr->monochord_count == period_scale_space_sensor_monochord_max(sss_ptr)) {
		return -1;
	}

	sss_ptr->monochords[sss_ptr->monochord_count].source_sss_ptr = source_sss_ptr;
	period_scale_space_sensor_init_monochord(sss_ptr, &sss_ptr->monochords[sss_ptr->monochord_count].monochord, sss_ptr, monochord_ratio);
	sss_ptr->monochord_count++;

	return 0;
}

/* Sensor Arrays */
struct receptive_field *period_array_get_receptive_field(struct period_array *pa_ptr) {
	return &pa_ptr->field;
}

void period_array_init(struct period_array *pa_ptr, double response_period, double octave_bandwidth, double scale_factor) {
	pa_ptr->response_period = response_period;
	pa_ptr->scale_factor = scale_factor;
	pa_ptr->octave_bandwidth = octave_bandwidth;
	/* self.period_bandwidth = 1.0 / ((2.0 ** (1.0 / self.octave_bandwidth)) - 1) */
	pa_ptr->period_bandwidth = 1.0 / (pow(2.0, 1.0 / pa_ptr->octave_bandwidth) - 1);
	pa_ptr->scale_space_sensor_count = 0;
}

unsigned int period_array_period_sensor_max(struct period_array *pa_ptr) {
	return sizeof (pa_ptr->scale_space_entries) / sizeof (pa_ptr->scale_space_entries[0]);
}
unsigned int period_array_period_sensor_count(struct period_array *pa_ptr) {
	return pa_ptr->scale_space_sensor_count;
}
struct scale_space_entry *period_array_get_entries(struct period_array *pa_ptr) {
	return pa_ptr->scale_space_entries;
}
int period_array_add_period_sensor(struct period_array *pa_ptr, double period, double bandwidth_factor) {
	struct period_scale_space_sensor *sss_ptr;
	struct receptive_field *field_ptr;

	if (pa_ptr->scale_space_sensor_count == period_array_period_sensor_max(pa_ptr)) {
		return -1;
	}

	sss_ptr = &pa_ptr->scale_space_entries[pa_ptr->scale_space_sensor_count].sensor;

	field_ptr = period_scale_space_sensor_get_receptive_field(sss_ptr);
	*field_ptr = pa_ptr->field;
	field_ptr->period = period;
	field_ptr->period_factor = pa_ptr->period_bandwidth * bandwidth_factor;
	period_scale_space_sensor_set_response_period(sss_ptr, pa_ptr->response_period);
	period_scale_space_sensor_set_scale_factor(   sss_ptr, pa_ptr->scale_factor);
	period_scale_space_sensor_init(sss_ptr);

	return pa_ptr->scale_space_sensor_count++;
}

int period_array_populate(struct period_array *pa_ptr, double octaves, double bandwidth_factor) {
	int rc;
	int n;

	for (n = - pa_ptr->octave_bandwidth * octaves; n <= 0; n++) {
		rc = period_array_add_period_sensor(pa_ptr, pa_ptr->field.period * pow(2, n / pa_ptr->octave_bandwidth), bandwidth_factor);
		if (rc == -1) {
			return -1;
		}
	}

	return 0;
}

int period_array_add_monochord(struct period_array *pa_ptr, int source_sss_descriptor, int target_sss_descriptor, double monochord_ratio) {
	return period_scale_space_sensor_add_monochord(&pa_ptr->scale_space_entries[target_sss_descriptor].sensor, &pa_ptr->scale_space_entries[source_sss_descriptor].sensor, monochord_ratio);
}

void period_array_sample(struct period_array *pa_ptr, double time, double value) {
	int i;

	for (i = 0; i < pa_ptr->scale_space_sensor_count; i++) {
		period_scale_space_sensor_sample(&pa_ptr->scale_space_entries[i].sensor, &pa_ptr->scale_space_entries[i].value, time, value);
	}
}

void period_array_sample_sensor(struct period_array *pa_ptr, double time, double value) {
	int i;

	for (i = 0; i < pa_ptr->scale_space_sensor_count; i++) {
		period_scale_space_sensor_sample_sensor(&pa_ptr->scale_space_entries[i].sensor, time, value);
	}
}

void period_array_sample_lifecycle(struct period_array *pa_ptr) {
	int i;

	for (i = 0; i < pa_ptr->scale_space_sensor_count; i++) {
		period_scale_space_sensor_sample_lifecycle(&pa_ptr->scale_space_entries[i].sensor);
	}
}

void period_array_sample_monochords(struct period_array *pa_ptr) {
	int i;

	for (i = 0; i < pa_ptr->scale_space_sensor_count; i++) {
		period_scale_space_sensor_sample_monochords(&pa_ptr->scale_space_entries[i].sensor);
	}
}

void period_array_values(struct period_array *pa_ptr) {
	int i;

	for (i = 0; i < pa_ptr->scale_space_sensor_count; i++) {
		period_scale_space_sensor_values(&pa_ptr->scale_space_entries[i].sensor, &pa_ptr->scale_space_entries[i].value);
	}
}

int midi_note(double sample_rate, double period, double A4, double *n_ptr) {
	static double n_A4 = 69;
	double Hz;

	/*
	f = 440 * (2 ^ (n/12)
	f / 440 = (2 ^ (n/12)
	log(f / 440) / log(2)  = n/12
	12 * (log(f / 440 ) / log(2)) = n
	*/

	if (period <= 0.0) {
		*n_ptr = 0.0;
		return -1;
	}

	Hz = sample_rate / period;
	*n_ptr = 12.0 * (log(Hz / A4) / M_LN2) + n_A4;

	return 0;
}

#define NOTE_FMT L"%2i%s%3.0f"
int note(double sample_rate, double period, double A4, int *octave_ptr, wchar_t **note_name_ptr, double *cents_ptr) {
	int rc;
	static wchar_t *notes[] = {L"C /B#", L"C#/Db", L"D /D ", L"D#/Eb", L"E /Fb", L"F /E# ", L"F#/Gb", L"G /G ", L"G#/Ab", L"A /A ", L"A#/Bb", L"B /Cb"};
	double n;
	int note;
	int octave;
	int octave_note;
	double cents;

	rc = midi_note(sample_rate, period, A4, &n);
	if (rc == -1) {
		return -1;
	}

	note = floor(n + 0.5);

	octave = note / 12 - 1;
	octave_note = note % 12;
	cents = 100.0 * (fmod(n + 0.5, 1.0) - 0.5);

	if (octave_note < 0 || octave_note > 11) {
		return -1;
	}

	*octave_ptr    = octave;
	*note_name_ptr = notes[octave_note];
	*cents_ptr     = cents;

	return 0;
}

#ifdef RECEPT_TEST
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bar.h"
#include "sampler_ui.h"

int main(int argc, char *argv[]) {
	int rc;

	struct sampler_ui sampler_ui;
	int row;
	int rows;
	int columns;
	double sample_value;
	double sample_time;
	int    sample_count;
	wchar_t *rowbuf;
	union bar_u *c1_rows;
	union bar_u *c2_rows;
	union bar_u *c3_rows;
	union bar_u *c4_rows;
	union bar_u *phase_rows;
	struct receptive_field *field_ptr;
	struct period_array array;
	struct scale_space_entry *scale_space_entries;
	struct scale_space_entry *entry_ptr;
	double cycle_area;
	int field_count;
	int octave_bandwidth;
	double octave_count;
	double period_response_Hz;
	int starting_note;

	rc = sampler_ui_getopts(&sampler_ui, argc, argv);
	if (rc == -1) {
		perror("sampler_ui_getopts");
		return -1;
	}
	argc -= rc;
	argv += rc;

	rc = sampler_ui_init(&sampler_ui);
	if (rc == -1) {
		perror("sampler_ui_init");
		return -1;
	}

	rows    = sampler_ui_get_rows(   &sampler_ui);
	columns = sampler_ui_get_columns(&sampler_ui);

	c1_rows = calloc(rows, sizeof (*c1_rows));
	if (c1_rows == NULL) {
		perror("calloc");
		return -1;
	}
	c2_rows = calloc(rows, sizeof (*c2_rows));
	if (c2_rows == NULL) {
		perror("calloc");
		return -1;
	}
	c3_rows = calloc(rows, sizeof (*c3_rows));
	if (c3_rows == NULL) {
		perror("calloc");
		return -1;
	}
	c4_rows = calloc(rows, sizeof (*c4_rows));
	if (c4_rows == NULL) {
		perror("calloc");
		return -1;
	}
	phase_rows = calloc(rows, sizeof (*phase_rows));
	if (phase_rows == NULL) {
		perror("calloc");
		return -1;
	}

	/* BEGIN CONFIG */
	period_response_Hz = 60.0; /* averages results at this rate, for smoothing */
	field_count = 48; /* number of receptor fields */
	octave_bandwidth = 12; /* how many receptor fields per octave */
	starting_note = -9 -12; /* where 0 is A=440 */
	/* END CONFIG */

	/* constants */
	octave_count = ((double) field_count) / octave_bandwidth; /* derived from config */
	cycle_area = 1.0 / (1.0 - exp(-1.0)); /* the area under the curve of the exponential distribution, part of power calibration */
	
	field_ptr = period_array_get_receptive_field(&array);
	field_ptr->period = sampler_ui_get_sample_rate(&sampler_ui) / (440 * pow(2, (((double) starting_note)/12)) );
	field_ptr->phase = 0.0;
	field_ptr->phase_factor = cycle_area;
	period_array_init(&array, sampler_ui_get_sample_rate(&sampler_ui) / period_response_Hz, octave_bandwidth, cycle_area);
	rc = period_array_populate(&array, octave_count, 1.0);
	scale_space_entries = period_array_get_entries(&array);
	screen_nprintf(sampler_ui_get_screen(&sampler_ui), 0,                           0, 20, '\0', L"%s", "    Tonal Phase     ");
	screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20,                          0, 22, '\0', L"%s", " Sensor <note> Sensed ");
	screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11,                0, 20, '\0', L"%s", "| Receptor Model    ");
	screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11 + 20,           0, 20, '\0', L"%s", "       Entropy      ");
	screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11 + 20 + 20,      0, 20, '\0', L"%s", "      - Energy      ");
	screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11 + 20 + 20 + 20, 0, 20, '\0', L"%s", "     Free Energy    ");
	for (row = 0; row < period_array_period_sensor_count(&array); row++) {
		entry_ptr = &scale_space_entries[row];

		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 0, row + 1);
		bar_init_buf(&phase_rows[row], bar_signed, bar_linear, rowbuf, 20);

		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11, row + 1);
		bar_init_buf(&c1_rows[row], bar_positive, bar_log, rowbuf, 40);

		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11 + 20, row + 1);
		bar_init_buf(&c2_rows[row], bar_signed, bar_logp1, rowbuf, 20);

		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11 + 20 + 20, row + 1);
		bar_init_buf(&c3_rows[row], bar_signed, bar_logp1, rowbuf, 20);

		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 20 + 11 + 11 + 20 + 20 + 20, row + 1);
		bar_init_buf(&c4_rows[row], bar_signed, bar_logp1, rowbuf, 20);

	}
	for (;;) {
		do {
			rc = filesampler_demand_next(sampler_ui_get_sampler(&sampler_ui), &sample_value);
			if (rc == -1) {
				perror("sampler_ui_demand_next");
				return -1;
			}
			sample_time  = filesampler_get_sample_time( sampler_ui_get_sampler(&sampler_ui));
			sample_count = filesampler_get_sample_count(sampler_ui_get_sampler(&sampler_ui));
		} while (rc == 0);

		period_array_sample(&array, (double) sample_count, sample_value * 10000);

		if (filesampler_check_draw(sampler_ui_get_sampler(&sampler_ui))) {
			int octave;
			wchar_t *note_name;
			double cents;

			filesampler_mark_draw(sampler_ui_get_sampler(&sampler_ui));

			for (row = 0; row < period_array_period_sensor_count(&array); row++) {
				entry_ptr = &scale_space_entries[row];
				int rc;
				struct period_concept *concept_ptr;
				struct lifecycle *lc_ptr;
				double pc;

				concept_ptr = entry_ptr->value.concept_ptr;
				lc_ptr = entry_ptr->value.period_lifecycle_ptr;
				pc      = cabs(CMPLX(cimag(lc_ptr->cval) < 0.0  ? -cimag(lc_ptr->cval) : 0.0, lc_ptr->F < 0.0 ? lc_ptr->F : 0.0));

				rc = note(sampler_ui_get_sample_rate(&sampler_ui), concept_ptr->recept_ptr->field.period, 440.0, &octave, &note_name, &cents);
				if (rc == 0) {
					screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20, row + 1, 11, '\0', NOTE_FMT, octave, note_name, cents);
				}
				if (pc == 0.0) {
					screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20 + 11, row + 1, 11, '\0', L"%s", L"           ");
				} else {
					rc = note(sampler_ui_get_sample_rate(&sampler_ui), concept_ptr->avg_instant_period, 440.0, &octave, &note_name, &cents);
					if (rc == 0) {
						screen_nprintf(sampler_ui_get_screen(&sampler_ui), 20 + 11, row + 1, 11, '\0', NOTE_FMT, octave, note_name, cents);
					}
				}
				bar_set(&phase_rows[row], lc_ptr->phi, 0.5); /* Phase */
				/*
				bar_set(&c1_rows[row],   entry_ptr->sensor.period_sensors[0].percept.value.r, concept_ptr->recept_ptr->field.period);
				bar_set(&c2_rows[row],   entry_ptr->sensor.period_sensors[1].percept.value.r, concept_ptr->recept_ptr->field.period);
				bar_set(&c3_rows[row],   entry_ptr->sensor.period_sensors[2].percept.value.r, concept_ptr->recept_ptr->field.period);
				*/
				bar_set(&c1_rows[row],   pc * 100,              lc_ptr->max_r * 10000); /*      Force */
				bar_set(&c2_rows[row],   creal(lc_ptr->cval),   lc_ptr->max_r);         /*      Entropy */
				bar_set(&c3_rows[row],   cimag(lc_ptr->cval),   lc_ptr->max_r);         /*    - Energy */
				bar_set(&c4_rows[row],         lc_ptr->F,       lc_ptr->max_r);         /* Free Energy */
			}

			rc = note(sampler_ui_get_sample_rate(&sampler_ui), 2.0, 440.0, &octave, &note_name, &cents);
			if (rc == 0) {
				screen_nprintf(sampler_ui_get_screen(&sampler_ui), columns - 20, 0, 20, '\0', L"Nyquist: " NOTE_FMT, octave, note_name, cents);
			}
			screen_nprintf(sampler_ui_get_screen(&sampler_ui), columns - 20, 1, 20, '\0', L"time: %f", sample_time);
			screen_draw(sampler_ui_get_screen(&sampler_ui));
		}
	}

	rc = sampler_ui_deinit(&sampler_ui);
	if (rc == -1) {
		perror("sampler_ui_deinit");
		return -1;
	}

	return 0;
}

#endif
