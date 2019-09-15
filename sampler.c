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
	sampler_ptr->buf_consume_cursor = 0;
	sampler_ptr->buf_produce_cursor = 0;
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

	available = sampler_ptr->buf_size - sampler_ptr->buf_produce_cursor;

	if (available < 0) {
		errno = EFAULT;
		return -1;
	} else if (available == 0) {
		received = read(sampler_ptr->fileno, sampler_ptr->buf + sampler_ptr->buf_consume_cursor, sampler_ptr->buf_size - sampler_ptr->buf_consume_cursor);
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

	available = sampler_ptr->buf_produce_cursor - sampler_ptr->buf_consume_cursor;

	if (available < sampler_ptr->sample_size) {
		sampler_ptr->buf_consume_cursor = 0;
		sampler_ptr->buf_produce_cursor = 0;
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

	received = filesampler_supply(sampler_ptr);
	if (received == -1) {
		return -1;
	}

	available = sampler_ptr->buf_produce_cursor - sampler_ptr->buf_consume_cursor;

	if (available >= sampler_ptr->sample_size) {
		/* read from and increment consume cursor */
		*sample_ptr = sampler_ptr->buf + sampler_ptr->buf_consume_cursor;
		sampler_ptr->buf_consume_cursor += sampler_ptr->sample_size;
		return 1;
	}

	return 0;
}

#ifdef SAMPLER_TEST
int main() {
	return 0;
}
#endif
