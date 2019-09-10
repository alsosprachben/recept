#include "bar.h"

void bar_init(union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit) {
	bar_ptr->barvar.bar_head.bar_type = bar_type;
	bar_ptr->barvar.bar_head.bar_unit = bar_unit;
	bar_ptr->barvar.bar_head.bar_size = sizeof (union bar_u) - sizeof (struct bar_header);
}

void bar_init_size(union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit, uint8_t bar_size) {
	bar_ptr->barvar.bar_head.bar_type = bar_type;
	bar_ptr->barvar.bar_head.bar_unit = bar_unit;
	bar_ptr->barvar.bar_head.bar_size = bar_size;
}

void bar_set(union bar_u *bar_ptr, double n) {
	
}
