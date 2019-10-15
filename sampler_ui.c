#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

#include "screen.h"
#include "bar.h"
#include "sampler.h"

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

struct screen *sampler_ui_get_screen(struct sampler_ui *sui_ptr) {
	return &sui_ptr->screen;
}
int sampler_ui_draw(struct sampler_ui *sui_ptr) {
	return screen_draw(&sui_ptr->screen);
}
int sampler_ui_frame_ready(struct sampler_ui *sui_ptr) {
	return sui_ptr->frame % sui_ptr->mod == 0;
}

int sampler_ui_init(struct sampler_ui *sui_ptr) {
	int rc;

	sui_ptr->frame = 0;

	sui_ptr->mod = (int) (floor(((double) sui_ptr->sample_rate) / sui_ptr->rows / sui_ptr->fps));
	sui_ptr->efps = ((double) sui_ptr->sample_rate) / sui_ptr->rows / sui_ptr->mod;

	rc = screen_init(&sui_ptr->screen, sui_ptr->columns, sui_ptr->rows);
	if (rc == -1) {
		perror("screen_init");
		return -1;
	}

	rc = filesampler_init(&sui_ptr->sampler, sui_ptr->fd, sui_ptr->sample_rate, 16, sui_ptr->rows);
	if (rc == -1) {
		perror("filesampler_init");
		return -1;
	}

	return 0;
}
int sampler_ui_deinit(struct sampler_ui *sui_ptr) {
	screen_deinit(&sui_ptr->screen);
	if (sui_ptr->screen.buf != NULL) {
		errno = EINVAL;
		perror("screen_deinit");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	int rc;
	int lines;

	struct sampler_ui sampler_ui;
	int16_t sample;
	int row;
	char *rowbuf;
	union bar_u *bar_rows;

	if (argc < 3) {
		perror("please specify arguments: $COLUMNS $LINES $sample_rate $fps");
		return -1;
	}

	rc = sscanf(argv[1], "%i", &sampler_ui.columns);
	if (rc == 1) {
	} else if (rc == -1) {
		perror("sscanf");
		return -1;
	} else {
		perror("expecting integer columns as the first argument");
		return -1;
	}
	
	rc = sscanf(argv[2], "%i", &lines);
	if (rc == 1) {
	} else if (rc == -1) {
		perror("sscanf");
		return -1;
	} else {
		perror("expecting integer rows as the second argument");
		return -1;
	}

	sampler_ui.rows = lines - 1; /* leave a row for the cursor at the bottom of the screen */

	if (argc < 4) {
		sampler_ui.sample_rate = 44100;
	} else {
		rc = sscanf(argv[3], "%i", &sampler_ui.sample_rate);
		if (rc == 1) {
		} else if (rc == -1) {
			perror("sscanf");
			return -1;
		} else {
			perror("expecting integer sample rate as the third argument");
			return -1;
		}
	}

	if (argc < 5) {
		sampler_ui.fps = 60;
	} else {
		rc = sscanf(argv[4], "%i", &sampler_ui.fps);
		if (rc == 1) {
		} else if (rc == -1) {
			perror("sscanf");
			return -1;
		} else {
			perror("expecting integer frame rate as the forth argument");
			return -1;
		}
	}

	rc = open("input.sock", O_RDONLY);
	if (rc == -1) {
		perror("open");
		return -1;
	}
	sampler_ui.fd = rc;

	rc = sampler_ui_init(&sampler_ui);
	if (rc == -1) {
		return -1;
	}

	bar_rows = calloc(sampler_ui.rows, sizeof (bar_rows));
	if (bar_rows == NULL) {
		perror("calloc");
		return -1;
	}
	for (row = 0; row < sampler_ui.rows; row++) {
		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 0, row);
		bar_init_buf(&bar_rows[row], bar_signed, bar_log, rowbuf, sampler_ui.columns);
	}
	for (;;) {
		for (row = 0; row < sampler_ui.rows; row++) {
			char *sample_ptr;
			sample_ptr = (char *) &sample;
			rc = filesampler_demand_next(&sampler_ui.sampler, &sample_ptr);
			if (rc == -1) {
				perror("filesampler_demand_next");
				return -1;
			}

			if (sampler_ui_frame_ready(&sampler_ui)) {
				bar_set(&bar_rows[row], sample, 1L << 15);
			}
		}

		if (sampler_ui_frame_ready(&sampler_ui)) {
			screen_nprintf(sampler_ui_get_screen(&sampler_ui), 0, 0, 20, '\0', "%i %i %i %i", sampler_ui.sample_rate, sampler_ui.fps, sampler_ui.frame, sampler_ui.mod);
			screen_nprintf(sampler_ui_get_screen(&sampler_ui), 0, 1, 20, '\0', "Effective FPS: %f", sampler_ui.efps);
			sampler_ui_draw(&sampler_ui);
		}

		sampler_ui.frame++;
	}

	rc = sampler_ui_deinit(&sampler_ui);
	if (rc == -1) {
		return -1;
	}

	return 0;
}

