#ifndef BAR_H
#define BAR_H

#include <stdint.h>
#include <wchar.h>

enum bar_orientation {
	bar_null = 0,
	bar_positive,
	bar_signed,
	bar_negative,
};
#define BAR_POS_LEGEND '|'
#define BAR_SIG_LEGEND '|'
#define BAR_NEG_LEGEND '|'
#define BAR_REMAINDER_MAP L" \u258F\u258E\u258D\u258C\u258B\u258A\u2589"
#define BAR_REMAINDER_LEN 8
#define BAR_FILL L'\u2588'

enum bar_scale {
	bar_linear = 0,
	bar_log,
	bar_log_signed,
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
		wchar_t buf[];
	} barvar;
	struct bar15_s {
		struct bar_header bar_head;
		wchar_t buf[16];
	} bar15;
	struct barptr_s {
		struct bar_header bar_head;
		wchar_t *buf;
	} barptr;
};

void bar_init(     union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit);
void bar_init_size(union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit, uint8_t bar_size);
void bar_init_buf( union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit, wchar_t *buf, uint8_t bar_size);


void bar_set(union bar_u *bar_ptr, double n, double d);
wchar_t *bar_get_buf(union bar_u *bar_ptr);

#endif
