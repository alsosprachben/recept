#ifndef SAMPLER_H
#define SAMPLER_H

#include <unistd.h>

struct filesampler {
	int fileno;
	size_t sample_rate;
	size_t sample_size;
	size_t chunk_size;
	int hit_eof;
	char *buf;
	size_t buf_size;
	size_t buf_produce_cursor;
	size_t buf_consume_cursor;
};

int filesampler_init(struct filesampler *sampler_ptr, int fileno, size_t sample_rate, size_t bit_depth, size_t chunks_size);
void filesampler_deinit(struct filesampler *sampler_ptr);

int filesampler_demand_next(struct filesampler *sampler_ptr, char **sample_ptr);

#endif
