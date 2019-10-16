#include "screen.h"
#include "bar.h"
#include "sampler.h"
#include "sampler_ui.h"

#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int sampler_ui_get_columns(struct sampler_ui *sui_ptr) {
	return sui_ptr->columns;
}
int sampler_ui_get_rows(struct sampler_ui *sui_ptr) {
	return sui_ptr->rows;
}
int sampler_ui_get_fps(struct sampler_ui *sui_ptr) {
	return sui_ptr->fps;
}
int sampler_ui_get_sample_rate(struct sampler_ui *sui_ptr) {
	return sui_ptr->sample_rate;
}
int sampler_ui_get_fd(struct sampler_ui *sui_ptr) {
	return sui_ptr->fd;
}

double sampler_ui_get_efps(struct sampler_ui *sui_ptr) {
	return sui_ptr->efps;
}
int sampler_ui_get_mod(struct sampler_ui *sui_ptr) {
	return sui_ptr->mod;
}
int sampler_ui_get_frame(struct sampler_ui *sui_ptr) {
	return sui_ptr->frame;
}
struct filesampler *sampler_ui_get_sampler(struct sampler_ui *sui_ptr) {
	return &sui_ptr->sampler;
}
struct screen *sampler_ui_get_screen(struct sampler_ui *sui_ptr) {
	return &sui_ptr->screen;
}

int sampler_ui_frame_ready(struct sampler_ui *sui_ptr) {
	return sui_ptr->frame % sui_ptr->mod == 0;
}

void sampler_ui_config(struct sampler_ui *sui_ptr, int columns, int rows, int fps, int sample_rate, int fd) {
	sui_ptr->columns = columns;
	sui_ptr->rows = rows;
	sui_ptr->fps = fps;
	sui_ptr->sample_rate = sample_rate;
	sui_ptr->fd = fd;
}

int sampler_ui_init(struct sampler_ui *sui_ptr) {
	int rc;

	sui_ptr->frame = 0;

	sui_ptr->mod = (int) (floor(((double) sui_ptr->sample_rate) / sui_ptr->rows / sui_ptr->fps));
	sui_ptr->efps = ((double) sui_ptr->sample_rate) / sui_ptr->rows / sui_ptr->mod;

	rc = screen_init(&sui_ptr->screen, sui_ptr->columns, sui_ptr->rows);
	if (rc == -1) {
		return -1;
	}

	rc = filesampler_init(&sui_ptr->sampler, sui_ptr->fd, sui_ptr->sample_rate, 16, sui_ptr->rows);
	if (rc == -1) {
		return -1;
	}

	return 0;
}
int sampler_ui_deinit(struct sampler_ui *sui_ptr) {
	screen_deinit(&sui_ptr->screen);
	if (sui_ptr->screen.buf != NULL) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int sampler_ui_getopts(struct sampler_ui *sui_ptr, int argc, char *argv[]) {
	int rc;
	int c;

	/* defaults */
	sui_ptr->sample_rate = 44100;
	sui_ptr->fps = 60;

	while ((c = getopt(argc, argv, "c:l:r:f:d:p:")) != -1) {
		switch (c) {
			case 'c':
				rc = sscanf(optarg, "%i", &sui_ptr->columns);
				if (rc != 1) {
					errno = EINVAL;
					return -1;
				}
				break;
			case 'l':
				rc = sscanf(optarg, "%i", &sui_ptr->rows);
				if (rc != 1) {
					errno = EINVAL;
					return -1;
				}
				sui_ptr->rows--; /* leave a row for hte cursor at the bottom of the screen */
				break;
			case 'r':
				rc = sscanf(optarg, "%i", &sui_ptr->sample_rate);
				if (rc != 1) {
					errno = EINVAL;
					return -1;
				}
				break;
			case 'f':
				rc = sscanf(optarg, "%i", &sui_ptr->fps);
				if (rc != 1) {
					errno = EINVAL;
					return -1;
				}
				break;
			case 'd':
				rc = sscanf(optarg, "%i", &sui_ptr->fd);
				if (rc != 1) {
					errno = EINVAL;
					return -1;
				}
				break;
			case 'p':
				rc = open(optarg, O_RDONLY);
				if (rc == -1) {
					return -1;
				}
				sui_ptr->fd = rc;
				break;
		}
	}

	return optind;
}

#ifdef SAMPLER_UI_TEST
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int rc;

	struct sampler_ui sampler_ui;
	int16_t sample;
	int row;
	char *rowbuf;
	union bar_u *bar_rows;

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

	bar_rows = calloc(sampler_ui.rows, sizeof (bar_rows));
	if (bar_rows == NULL) {
		perror("calloc");
		return -1;
	}
	for (row = 0; row < sampler_ui_get_rows(&sampler_ui); row++) {
		rowbuf = screen_pos(sampler_ui_get_screen(&sampler_ui), 0, row);
		bar_init_buf(&bar_rows[row], bar_signed, bar_log, rowbuf, sampler_ui.columns);
	}
	for (;;) {
		for (row = 0; row < sampler_ui_get_rows(&sampler_ui); row++) {
			char *sample_ptr;
			sample_ptr = (char *) &sample;
			rc = filesampler_demand_next(sampler_ui_get_sampler(&sampler_ui), &sample_ptr);
			if (rc == -1) {
				perror("sampler_ui_demand_next");
				return -1;
			}

			if (sampler_ui_frame_ready(&sampler_ui)) {
				bar_set(&bar_rows[row], sample, 1L << 15);
			}
		}

		if (sampler_ui_frame_ready(&sampler_ui)) {
			screen_nprintf(sampler_ui_get_screen(&sampler_ui), 0, 0, 20, '\0', "%i %i %i %i", sampler_ui.sample_rate, sampler_ui.fps, sampler_ui.frame, sampler_ui.mod);
			screen_nprintf(sampler_ui_get_screen(&sampler_ui), 0, 1, 20, '\0', "Effective FPS: %f", sampler_ui.efps);
			screen_draw(sampler_ui_get_screen(&sampler_ui));
		}

		sampler_ui.frame++;
	}

	rc = sampler_ui_deinit(&sampler_ui);
	if (rc == -1) {
		perror("sampler_ui_deinit");
		return -1;
	}

	return 0;
}

#endif
