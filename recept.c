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
void period_percept_init(struct period_percept *pp_ptr, struct dynamic_time_smoothing_d *dts_d_ptr) {
	dynamic_time_smoothing_d_effective_field(dts_d_ptr, &pp_ptr->field);
	pp_ptr->value = *dts_d_ptr->ts.value_ptr;
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
	pc_ptr->avg_instant_period = exponential_smoother_d_sample(&pcs_ptr->avg_instant_period_state, recept_ptr->field.period, recept_ptr->field.period * recept_ptr->field.phase_factor);
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

void period_sensor_sample(struct period_sensor *ps_ptr, double time, double value) {
	if (ps_ptr->has_prior_percept) {
		ps_ptr->prior_percept = ps_ptr->percept;
	}
	dynamic_time_smoothing_d_sample(&ps_ptr->sensor_state, time, value);
	period_percept_init(&ps_ptr->percept, &ps_ptr->sensor_state);
	if ( ! ps_ptr->has_prior_percept) {
		/* when no prior, make prior the same as current */
		ps_ptr->prior_percept = ps_ptr->percept;
		ps_ptr->has_prior_percept = 1;
	}

	period_recept_init(&ps_ptr->recept, &ps_ptr->percept, &ps_ptr->prior_percept);

	period_concept_init(&ps_ptr->concept, &ps_ptr->concept_state, &ps_ptr->recept);
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
	int16_t sample;
	int row;
	int i = 0;
	int sample_i = 0;
	char *rowbuf;
	union bar_u *bar_rows;
	union bar_u *phase_rows;
	struct period_sensor *sensors;
	double step = 50.0;

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

	bar_rows = calloc(sampler_ui_get_rows(&sampler_ui), sizeof (*bar_rows));
	if (bar_rows == NULL) {
		perror("calloc");
		return -1;
	}
	phase_rows = calloc(sampler_ui_get_rows(&sampler_ui), sizeof (*phase_rows));
	if (phase_rows == NULL) {
		perror("calloc");
		return -1;
	}
	sensors = calloc(sampler_ui_get_rows(&sampler_ui), sizeof (*sensors));
	for (row = 0; row < sampler_ui_get_rows(&sampler_ui); row++) {
		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 0, row);
		bar_init_buf(&phase_rows[row], bar_signed, bar_linear, rowbuf, 20);

		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 20, row);
		bar_init_buf(&bar_rows[row], bar_positive, bar_log, rowbuf, sampler_ui_get_columns(&sampler_ui) - 20);

		period_sensor_get_receptive_field(&sensors[row])->period = ((double) sampler_ui_get_sample_rate(&sampler_ui)) /  (step * row);
		period_sensor_get_receptive_field(&sensors[row])->period_factor = 1.0
			/ period_sensor_get_receptive_field(&sensors[row])->period
			* ((double) sampler_ui_get_sample_rate(&sampler_ui))
			/ step
			;
		period_sensor_get_receptive_field(&sensors[row])->phase = 0.0;
		period_sensor_get_receptive_field(&sensors[row])->phase_factor = 44100.0 / 20;
		period_sensor_get_receptive_value(&sensors[row])->cval = 0.0;
		period_sensor_init(&sensors[row]);
	}
	for (;;) {
		for (i = 0; i < sampler_ui_get_rows(&sampler_ui) * sampler_ui_get_mod(&sampler_ui); i++) {
			char *sample_ptr;
			sample_ptr = (char *) &sample;
			do {
				rc = filesampler_demand_next(sampler_ui_get_sampler(&sampler_ui), &sample_ptr);
				if (rc == -1) {
					perror("sampler_ui_demand_next");
					return -1;
				}
			} while (rc == 0);

			for (row = 0; row < sampler_ui_get_rows(&sampler_ui); row++) {
				period_sensor_sample(&sensors[row], (double) (sample_i), 10000.0 * ((double) sample) / (1L << (sampler_ui_get_sample_depth(&sampler_ui) - 1)));
			}

			sample_i++;
		}

		for (row = 0; row < sampler_ui_get_rows(&sampler_ui); row++) {
			struct period_concept *concept_ptr;

			concept_ptr = period_sensor_get_concept(&sensors[row]);

			bar_set(&phase_rows[row], concept_ptr->recept_ptr->phase->value.phi, 0.5);
			bar_set(&bar_rows[row],   concept_ptr->recept_ptr->phase->value.r,   concept_ptr->recept_ptr->field.period);
		}

		screen_nprintf(sampler_ui_get_screen(&sampler_ui), 0, 0, 20, '\0', "time: %f",
			((double) (sample_i)) / sampler_ui_get_sample_rate(&sampler_ui)
		);
		screen_draw(sampler_ui_get_screen(&sampler_ui));
	}

	rc = sampler_ui_deinit(&sampler_ui);
	if (rc == -1) {
		perror("sampler_ui_deinit");
		return -1;
	}

	return 0;
}

#endif
