#include "bar.h"

#include <math.h>
#include <unistd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void bar_init(union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit) {
	bar_ptr->barvar.bar_head.bar_type = bar_type;
	bar_ptr->barvar.bar_head.bar_unit = bar_unit;
	bar_ptr->barvar.bar_head.bar_size = sizeof (union bar_u) - sizeof (struct bar_header) - 1;
	bar_ptr->barvar.bar_head.bar_flags = 0;
}

void bar_init_size(union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit, uint8_t bar_size) {
	bar_ptr->barvar.bar_head.bar_type = bar_type;
	bar_ptr->barvar.bar_head.bar_unit = bar_unit;
	bar_ptr->barvar.bar_head.bar_size = bar_size;
	bar_ptr->barvar.bar_head.bar_flags = 0;
}
void bar_init_buf(union bar_u *bar_ptr, enum bar_orientation bar_type, enum bar_scale bar_unit, wchar_t *buf, uint8_t bar_size) {
	bar_ptr->barvar.bar_head.bar_type = bar_type;
	bar_ptr->barvar.bar_head.bar_unit = bar_unit;
	bar_ptr->barvar.bar_head.bar_size = bar_size;
	bar_ptr->barvar.bar_head.bar_flags = BAR_BUF_PTR;
	bar_ptr->barptr.buf = buf;
}

/*
 * Draw the bar into the specified buffer, where scaling is already calculated.
 *
 * `bufpos` may overflow buflen, and is constrained by this drawing function.
 */
void bar_draw(union bar_u *bar_ptr, wchar_t *buf, size_t buflen, double bufpos, int left) {
	static wchar_t *remainder_map = BAR_REMAINDER_MAP;

	int    bufpos_i; /* `bufpos` floored to an int */
	double bufpos_r; /* `bufpos` remainder of floored value */
	int    i;        /* index into buf up to buflen */

	bufpos = MAX(0, MIN(buflen, bufpos)); /* constrain bufpos to between 0 and `buflen` */
	bufpos_i = (int) floor(bufpos);
	bufpos_r = bufpos - bufpos_i;

	if (left) {
		for (i = buflen - 1; i >= ((int) buflen - 1) - bufpos_i; i--) {
			buf[i] = '#';
		}
		for (i = ((int) buflen - 2) - bufpos_i; i >= 0; i--) {
			buf[i] = ' ';
		}
		/* bufpos_r = 1.0 - bufpos_r; */
		i = buflen - bufpos_i - 1;
	} else {
		for (i = 0; i < bufpos_i; i++) {
			buf[i] = '#';
		}
		for (i = bufpos_i + 1; i < buflen; i++) {
			buf[i] = ' ';
		}
		i = bufpos_i;
	}

	if (i >= 0 && i < buflen) {
		buf[i] = remainder_map[(int) floor(bufpos_r * BAR_REMAINDER_LEN)];
	}
}

void bar_scale(union bar_u *bar_ptr, double *n_ptr, double *d_ptr) {
	switch (bar_ptr->barvar.bar_head.bar_unit) {
		case bar_linear:
			break;
		case bar_logp1:
			if (*n_ptr < 0) {
				*n_ptr = -log(1.0 - *n_ptr);
			} else {
				*n_ptr =  log(1.0 + *n_ptr);
			}
			*d_ptr = log(1.0 + *d_ptr);
			break;
		case bar_log:
			if (*n_ptr <= 0) {
				*n_ptr = 0.0;
			} else {
				*n_ptr = log(*n_ptr);
			}
			*d_ptr = log(*d_ptr);
			if (*n_ptr < -*d_ptr) {
				*n_ptr = -*d_ptr;
			}
			break;
		case bar_log_signed:
			if (*n_ptr < 0.0) {
				*n_ptr = log(- *n_ptr);
				if (*n_ptr > 0.0) {
					*n_ptr = 0.0;
				}
			} else if (*n_ptr > 0.0) {
				*n_ptr = log(  *n_ptr);
				if (*n_ptr < 0.0) {
					*n_ptr = 0.0;
				}
			} else {
				*n_ptr = 0.0;
			}
			*d_ptr = log(*d_ptr);
			break;
	}
}

void bar_draw_blank(union bar_u *bar_ptr, wchar_t *buf, size_t buflen) {
	int i;

	for (i = 0; i < buflen; i++) {
		buf[i] = ' ';
	}
}
void bar_draw_unsigned(union bar_u *bar_ptr, wchar_t *buf, size_t buflen, double n, double d, int neg) {
	double bufpos;

	if (d == 0) {
		bar_draw_blank(bar_ptr, buf, buflen);
		return;
	}

	bar_scale(bar_ptr, &n, &d);

	bufpos = n * (buflen - 1) / d;

	if (neg) {
		buf[buflen - 1] = BAR_NEG_LEGEND;
		bar_draw(bar_ptr, buf, buflen - 1, bufpos, neg);
	} else {
		buf[0] = BAR_POS_LEGEND;
		bar_draw(bar_ptr, buf + 1, buflen - 1, bufpos, neg);
	}
}
void bar_draw_signed(union bar_u *bar_ptr, wchar_t *buf, size_t buflen, double n, double d) {
	if (d == 0 || buflen < 3) {
		bar_draw_blank(bar_ptr, buf, buflen);
		return;
	}

	bar_scale(bar_ptr, &n, &d);

	if (buflen % 2 == 1) {
		/* odd length, use one center character */
		buf[buflen / 2] = BAR_SIG_LEGEND;
		if (n < 0) {
			bar_draw(      bar_ptr, buf,                    (buflen - 1) / 2, -n * ((buflen - 1) / 2) / d, 1);
			bar_draw_blank(bar_ptr, buf + (buflen / 2 + 1), (buflen - 1) / 2);
		} else {
			bar_draw(      bar_ptr, buf + (buflen / 2 + 1), (buflen - 1) / 2,  n * ((buflen - 1) / 2) / d, 0);
			bar_draw_blank(bar_ptr, buf,                    (buflen - 1) / 2);
		}
	} else {
		/* even length, use two center characters */
		buf[(buflen - 2) / 2 + 0] = BAR_NEG_LEGEND;
		buf[(buflen - 2) / 2 + 1] = BAR_POS_LEGEND;
		if (n < 0) {
			bar_draw(      bar_ptr, buf,                          (buflen - 2) / 2, -n * ((buflen - 2) / 2) / d, 1);
			bar_draw_blank(bar_ptr, buf + ((buflen - 2) / 2) + 2, (buflen - 2) / 2);
		} else {
			bar_draw(      bar_ptr, buf + ((buflen - 2) / 2) + 2, (buflen - 2) / 2,  n * ((buflen - 2) / 2) / d, 0);
			bar_draw_blank(bar_ptr, buf,                          (buflen - 2) / 2);
		}
	}
}

wchar_t *bar_get_buf(union bar_u *bar_ptr) {
	if (bar_ptr->barvar.bar_head.bar_flags & BAR_BUF_PTR) {
		return bar_ptr->barptr.buf;
	} else {
		return bar_ptr->barvar.buf;
	}
}

void bar_set(union bar_u *bar_ptr, double n, double d) {
	switch (bar_ptr->barvar.bar_head.bar_type) {
		case bar_positive:
			bar_draw_unsigned(bar_ptr, bar_get_buf(bar_ptr), bar_ptr->barvar.bar_head.bar_size, n, d, 0);
			break;
		case bar_negative:
			bar_draw_unsigned(bar_ptr, bar_get_buf(bar_ptr), bar_ptr->barvar.bar_head.bar_size, n, d, 1);
			break;
		case bar_signed:
			bar_draw_signed(bar_ptr, bar_get_buf(bar_ptr), bar_ptr->barvar.bar_head.bar_size, n, d);
			break;
		case bar_null:
			break;
		
	}

	/* terminate as a string */
	if (bar_ptr->barvar.bar_head.bar_flags & BAR_BUF_PTR) {
		/* do not terminate when provided an external buffer */
	} else {
		bar_get_buf(bar_ptr)[bar_ptr->barvar.bar_head.bar_size] = '\0';
	}
}



#ifdef BAR_TEST
#include <stdio.h>

int main() {
	union bar_u bar;
	int s;

	bar_init(&bar, bar_signed, bar_linear);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init(&bar, bar_signed, bar_log);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init(&bar, bar_positive, bar_linear);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init(&bar, bar_positive, bar_log);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init(&bar, bar_negative, bar_linear);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init(&bar, bar_negative, bar_log);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");



	bar_init_size(&bar, bar_signed, bar_linear, 14);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init_size(&bar, bar_signed, bar_log, 14);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init_size(&bar, bar_positive, bar_linear, 14);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init_size(&bar, bar_positive, bar_log, 14);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init_size(&bar, bar_negative, bar_linear, 14);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");

	bar_init_size(&bar, bar_negative, bar_log, 14);
	for (s = -10000; s <= 10000; s += 1000) {
		bar_set(&bar, s, 10000);
		printf("%ls\n", bar_get_buf(&bar));
	}
	printf("\n");
	wchar_t buf[15];
	bar_init_buf(&bar, bar_positive, bar_linear, buf, sizeof(buf));
	bar_set(&bar, 5000, 10000);
	printf("%ls\n", bar_get_buf(&bar));
	printf("%ls\n", buf);
	bar_init_buf(&bar, bar_negative, bar_linear, buf, sizeof(buf));
	bar_set(&bar, 5000, 10000);
	printf("%ls\n", bar_get_buf(&bar));
	printf("%ls\n", buf);
	bar_init_buf(&bar, bar_signed, bar_linear, buf, sizeof(buf));
	bar_set(&bar, 5000, 10000);
	printf("%ls\n", bar_get_buf(&bar));
	printf("%ls\n", buf);

}
#endif
