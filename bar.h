#ifndef BAR_H
#define BAR_H

#include <stdint.h>

enum bar_orientation {
	bar_null = 0,
	bar_positive,
	bar_signed,
	bar_negative,
};
#define BAR_POS_LEGEND '['
#define BAR_SIG_LEGEND 'I'
#define BAR_NEG_LEGEND ']'
#define BAR_REMAINDER_MAP " -+="
#define BAR_REMAINDER_LEN 4
#define BAR_FILL '#'

enum bar_scale {
	bar_linear = 0,
	bar_log,
};

struct bar_header {
	enum bar_orientation bar_type;
	enum bar_scale       bar_unit;
	uint8_t bar_size;
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
};

void bar_init(     union bar_u *bar, enum bar_orientation bar_type, enum bar_scale bar_unit);
void bar_init_size(union bar_u *bar, enum bar_orientation bar_type, enum bar_scale bar_unit, uint8_t bar_size);


void bar_set(union bar_u *bar_ptr, double n, double d);

#endif
