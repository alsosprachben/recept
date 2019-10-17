#include "sampler.h"

#include <stdlib.h>
#include <errno.h>

/* beware of integer overflow in buffer allocation */
int filesampler_init(struct filesampler *sampler_ptr, int fileno, size_t sample_rate, size_t bit_depth, size_t chunk_size) {
	sampler_ptr->fileno = fileno;
	sampler_ptr->sample_rate = sample_rate;
	sampler_ptr->sample_size = bit_depth >> 3; /* bits to bytes */
	sampler_ptr->chunk_size = chunk_size;
	sampler_ptr->buf_size = sampler_ptr->sample_size * chunk_size;
	sampler_ptr->buf = malloc(sampler_ptr->buf_size);
	if (sampler_ptr->buf == NULL) {
		errno = ENOMEM;
		return -1;
	}
	sampler_ptr->buf_consume_cursor = 0; /* prior     is buffered file data (consumed from supply file) */
	sampler_ptr->buf_produce_cursor = 0; /* posterior is buffered file data (produced to demand reader) */
	sampler_ptr->hit_eof = 0;

	return 0;
}
void filesampler_deinit(struct filesampler *sampler_ptr) {
	if (sampler_ptr->buf != NULL) {
		free(sampler_ptr->buf);
		sampler_ptr->buf = NULL;
	}
}

int filesampler_read(struct filesampler *sampler_ptr) {
	ssize_t available;
	ssize_t received;

	available = sampler_ptr->buf_size - sampler_ptr->buf_consume_cursor;

	if (available < 0) {
		errno = EFAULT;
		return -1;
	} else if (available != 0) {
		received = read(sampler_ptr->fileno, sampler_ptr->buf + sampler_ptr->buf_consume_cursor, available);
		if (received == -1) {
			return -1;
		} else if (received == 0) {
			sampler_ptr->hit_eof = 1;
			return 0;
		} else {
			sampler_ptr->buf_consume_cursor += received;
		}
	}

	return 0;
}
int filesampler_supply(struct filesampler *sampler_ptr) {
	ssize_t available;

	if (sampler_ptr->buf_produce_cursor == sampler_ptr->buf_consume_cursor) {
		sampler_ptr->buf_consume_cursor = 0;
		sampler_ptr->buf_produce_cursor = 0;
	}

	available = sampler_ptr->buf_consume_cursor - sampler_ptr->buf_produce_cursor;
	if (available < sampler_ptr->sample_size) {
		return filesampler_read(sampler_ptr);
	}

	return 0;
}

/*
 * iterator that returns the next byte
 */
int filesampler_demand_next(struct filesampler *sampler_ptr, char **sample_ptr) {
	ssize_t received;
	ssize_t available;
	int i;

	received = filesampler_supply(sampler_ptr);
	if (received == -1) {
		return -1;
	}

	available = sampler_ptr->buf_consume_cursor - sampler_ptr->buf_produce_cursor;

	if (available >= sampler_ptr->sample_size) {
		for (i = 0; i < sampler_ptr->sample_size; i++) {
			(*sample_ptr)[i] = sampler_ptr->buf[sampler_ptr->buf_produce_cursor + i];
		}
		sampler_ptr->buf_produce_cursor += sampler_ptr->sample_size;
		return 1;
	}

	return 0;
}

#ifdef OSC_TEST
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

#include "screen.h"
#include "bar.h"

int main(int argc, char *argv[]) {
	int rc;
	int fd;
	int columns;
	int sample_rate;
	int fps;
	double efps;
	int mod;
	int lines;
	int rows;
	int frame;
	struct screen screen;
	struct filesampler sampler;
	int16_t sample;
	int row;
	char *rowbuf;
	union bar_u *bar_rows;

	if (argc < 3) {
		perror("please specify arguments: $COLUMNS $LINES $sample_rate $fps");
		return -1;
	}

	rc = sscanf(argv[1], "%i", &columns);
	if (rc == 1) {
	} else if (rc == -1) {
		perror("sscanf");
	} else {
		perror("expecting integer columns as the first argument");
	}
	
	rc = sscanf(argv[2], "%i", &lines);
	if (rc == 1) {
	} else if (rc == -1) {
		perror("sscanf");
	} else {
		perror("expecting integer rows as the second argument");
	}

	rows = lines - 1; /* leave a row for the cursor at the bottom of the screen */

	if (argc < 4) {
		sample_rate = 44100;
	} else {
		rc = sscanf(argv[3], "%i", &sample_rate);
		if (rc == 1) {
		} else if (rc == -1) {
			perror("sscanf");
		} else {
			perror("expecting integer sample rate as the third argument");
		}
	}

	if (argc < 5) {
		fps = 60;
	} else {
		rc = sscanf(argv[4], "%i", &fps);
		if (rc == 1) {
		} else if (rc == -1) {
			perror("sscanf");
		} else {
			perror("expecting integer frame rate as the forth argument");
		}
	}

	frame = 0;

	mod = (int) (floor(((double) sample_rate) / rows / fps));
	efps = ((double) sample_rate) / rows / mod;

	rc = screen_init(&screen, columns, rows);
	if (rc == -1) {
		perror("screen_init");
		return -1;
	}

	rc = open("input.sock", O_RDONLY);
	if (rc == -1) {
		perror("open");
		return -1;
	}
	fd = rc;
	rc = filesampler_init(&sampler, fd, 44100, 16, rows);
	if (rc == -1) {
		perror("filesampler_init");
		return -1;
	}

	bar_rows = calloc(rows, sizeof (*bar_rows));
	if (bar_rows == NULL) {
		perror("calloc");
		return -1;
	}
	for (row = 0; row < rows; row++) {
		rowbuf = screen_pos(&screen, 0, row);
		bar_init_buf(&bar_rows[row], bar_signed, bar_log, rowbuf, columns);
	}
	for (;;) {
		for (row = 0; row < rows; row++) {
			char *sample_ptr;
			sample_ptr = (char *) &sample;
			rc = filesampler_demand_next(&sampler, &sample_ptr);
			if (rc == -1) {
				perror("filesampler_demand_next");
				return -1;
			}

			if (frame % mod == 0) {
				bar_set(&bar_rows[row], sample, 1L << 15);
			}
		}

		if (frame % mod == 0) {
			screen_nprintf(&screen, 0, 0, 20, '\0', "%i %i %i %i", sample_rate, fps, frame, mod);
			screen_nprintf(&screen, 0, 1, 20, '\0', "Effective FPS: %f", efps);
			screen_draw(&screen);
		}

		frame++;
	}

	screen_deinit(&screen);
	if (screen.buf != NULL) {
		errno = EINVAL;
		perror("screen_deinit");
		return -1;
	}

	return 0;
}
#endif

#ifdef SAMPLER_TEST

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main() {
	int rc;
	int fd;
	struct filesampler sampler;
	int i;
	int16_t sample;

	rc = open("/dev/urandom", O_RDONLY);
	if (rc == -1) {
		perror("open");
		return -1;
	}
	fd = rc;

	rc = filesampler_init(&sampler, fd, 44100, 16, 128);
	if (rc == -1) {
		perror("filesampler_init");
		return -1;
	}

	for (i = 0; i < 10000; i++) {
		char *sample_ptr;
		sample_ptr = (char *) &sample;
		rc = filesampler_demand_next(&sampler, &sample_ptr);
		if (rc == -1) {
			perror("filesampler_demand_next");
			return -1;
		}

		printf("sample %i: %"PRIi16"\n", i, sample);
	}
	
	filesampler_deinit(&sampler);
	if (sampler.buf != NULL) {
		errno = EINVAL;
		perror("filesampler_deinit");
		return -1;
	}

	rc = close(fd);
	if (rc == -1) {
		perror("close");
		return -1;
	}

	return 0;
}
#endif
