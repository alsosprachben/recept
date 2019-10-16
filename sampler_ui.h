#ifndef SAMPLER_UI_H
#define SAMPLER_UI_H

struct sampler_ui {
	/* input parameters */
	int columns;
	int rows;
	int fps;
	int fd;
	int sample_rate;

	/* state */
	double efps;
	int mod;
	int frame;
	struct screen screen;
	struct filesampler sampler;
};

struct screen *sampler_ui_get_screen(struct sampler_ui *sui_ptr);
int sampler_ui_draw(struct sampler_ui *sui_ptr);
int sampler_ui_frame_ready(struct sampler_ui *sui_ptr);
int sampler_ui_init(struct sampler_ui *sui_ptr);
int sampler_ui_deinit(struct sampler_ui *sui_ptr);


#endif
