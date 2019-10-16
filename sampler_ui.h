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

struct screen *sampler_ui_get_screen(struct sampler_ui *sui_ptr);
int sampler_ui_draw(struct sampler_ui *sui_ptr);
int sampler_ui_frame_ready(struct sampler_ui *sui_ptr);

#endif
