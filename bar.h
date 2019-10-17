#ifndef BAR_H
#define BAR_H

#include <stdint.h>

enum bar_orientation {
	bar_null = 0,
	bar_positive,
	bar_signed,
	bar_negative,
};
#define BAR_POS_LEGEND '|'
#define BAR_SIG_LEGEND '|'
#define BAR_NEG_LEGEND '|'
#define BAR_REMAINDER_MAP " -+="
#define BAR_REMAINDER_LEN 4
#define BAR_FILL '#'

enum bar_scale {
	bar_linear = 0,
	bar_log,
	bar_logp1
};

#define BAR_BUF_PTR 1
struct bar_header {
	enum bar_orientation bar_type;
	enum bar_scale       bar_unit;
	uint8_t bar_size;
	int bar_flags;
};

union bar_u {
	struct barvar_s {
		struct bar_header bar_head;
		char buf[];
	} barvar;
	struct bar15_s {
		struct bar_header bar_head;
		char buf[16];
	} bar15;
	struct barptr_s {
		struct bar_header bar_head;
		char *buf;
	} barptr;
};

void bar_init(     union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit);
void bar_init_size(union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit, uint8_t bar_size);
void bar_init_buf( union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit, char *buf, uint8_t bar_size);


void bar_set(union bar_u *bar_ptr, double n, double d);
char *bar_get_buf(union bar_u *bar_ptr);

#endif
