#ifndef SAMPLER_UI_H
#define SAMPLER_UI_H

struct sampler_ui {
	/* input parameters */
	int columns;
	int rows;
	int fps;
	int sample_rate;
	int fd;

	/* state */
	double efps;
	int mod;
	int frame;
	struct screen screen;
	struct filesampler sampler;
};

void sampler_ui_config(struct sampler_ui *sui_ptr, int columns, int rows, int fps, int sample_rate, int fd);
int sampler_ui_getopts(struct sampler_ui *sui_ptr, int argc, char *argv[]);
int sampler_ui_init(struct sampler_ui *sui_ptr);
int sampler_ui_deinit(struct sampler_ui *sui_ptr);

int sampler_ui_get_columns(struct sampler_ui *sui_ptr);
int sampler_ui_get_rows(struct sampler_ui *sui_ptr);
int sampler_ui_get_fps(struct sampler_ui *sui_ptr);
int sampler_ui_get_sample_rate(struct sampler_ui *sui_ptr);
int sampler_ui_get_fd(struct sampler_ui *sui_ptr);

double sampler_ui_get_efps(struct sampler_ui *sui_ptr);
int sampler_ui_get_mod(struct sampler_ui *sui_ptr);
int sampler_ui_get_frame(struct sampler_ui *sui_ptr);
struct filesampler *sampler_ui_get_sampler(struct sampler_ui *sui_ptr);
struct screen *sampler_ui_get_screen(struct sampler_ui *sui_ptr);

int sampler_ui_frame_ready(struct sampler_ui *sui_ptr);

#endif
