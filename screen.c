#include "screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>

int screen_clear() {
	int rc;
	rc = printf("%s", ESCAPE_CLEAR);
	if (rc == -1) {
		return -1;
	}
	rc = fflush(stdout);
	if (rc == -1) {
		return -1;
	}

	return 0;
}

int screen_draw(struct screen *screen_ptr) {
	int rc;
	return printf("%s", screen_ptr->buf);
	if (rc == -1) {
		return -1;
	}
	rc = fflush(stdout);
	if (rc == -1) {
		return -1;
	}

	return 0;
}

char *screen_pos(struct screen *screen_ptr, int column, int row) {
	return &screen_ptr->frame[row * screen_ptr->columns + column];
}

/*
 * Set `size_t n` to be one larger than the amount of space you want printed,
 * to allow space for interacting with the terminator generated from the underlying `vsnprintf()`.
 * If `new_terminator` is `'\0'` then restore the terminator with what was previously in the frame buffer.
 * Take care at the end of the frame buffer.
 * The string terminator will never be null, but rather either:
 *  1. the new_terminator, or
 *  2. the previous character in the terminal position (if new_terminator is null).
 */
int screen_nprintf(struct screen *screen_ptr, int column, int row, size_t n, char new_terminator, const char *format, ...) {
	int rc;
	va_list args;
	char *s;
	char terminator;
	int i;
	int fmt_n;

	va_start(args, format);
	rc = vsnprintf(&terminator, 0, format, args);
	if (rc == -1) {
		return -1;
	}
	va_end(args);

	fmt_n = rc;
	if (fmt_n == 0) {
		return 0;
	}

	if (fmt_n >= n) {
		fmt_n = n - 1;
	}
	
	s = screen_pos(screen_ptr, column, row);

	if (new_terminator == '\0') {
		terminator = s[fmt_n];
	} else {
		terminator = new_terminator;
	}

	va_start(args, format);
	rc = vsnprintf(s, n, format, args);
	if (rc != -1) {
		s[fmt_n] = terminator;
	}
	va_end(args);

	return rc;
}

void screen_blank(struct screen *screen_ptr) {
	int column;
	int row;

	for (column = 0; column < screen_ptr->columns; column++) {
		for (row = 0; row < screen_ptr->rows; row++) {
			*screen_pos(screen_ptr, column, row) = ' ';
		}
	}
}

int screen_init(struct screen *screen_ptr, int columns, int rows) {
	screen_ptr->columns = columns;
	screen_ptr->rows = rows;
	screen_ptr->screen_size = columns * rows;
	screen_ptr->buf_size = sizeof (ESCAPE_RESET) - 1 + screen_ptr->screen_size;
	screen_ptr->buf = malloc(screen_ptr->buf_size);
	screen_ptr->frame = screen_ptr->buf + sizeof (ESCAPE_RESET) - 1;
	if (screen_ptr->buf == NULL) {
		errno = ENOMEM;
		return -1;
	}

	screen_ptr->buf[0] = ESCAPE_RESET[0];
	screen_ptr->buf[1] = ESCAPE_RESET[1];
	screen_ptr->buf[2] = ESCAPE_RESET[2];
	screen_ptr->buf[3] = ESCAPE_RESET[3];
	screen_blank(screen_ptr);

	screen_clear();

	return 0;
}

void screen_deinit(struct screen *screen_ptr) {
	if (screen_ptr->buf != NULL) {
		free(screen_ptr->buf);
		screen_ptr->buf = NULL;
		screen_ptr->frame = NULL;
	}
}

#ifdef SCREEN_TEST
int main(int argc, char *argv[]) {
	int rc;
	int columns;
	int rows;
	struct screen screen;

	if (argc < 3) {
		perror("please specify columns and rows");
		return -1;
	}

	rc = sscanf(argv[1], "%i", &columns);
	if (rc == 1) {
	} else if (rc == -1) {
		perror("sscanf");
	} else {
		perror("expecting integer columns as the first argument");
	}
	
	rc = sscanf(argv[2], "%i", &rows);
	if (rc == 1) {
	} else if (rc == -1) {
		perror("sscanf");
	} else {
		perror("expecting integer rows as the second argument");
	}

	rc = screen_init(&screen, columns, rows);
	if (rc == -1) {
		perror("screen_init");
		return -1;
	}

	*screen_pos(&screen, 0, 0) = '1';
	*screen_pos(&screen, 1, 0) = '2';
	*screen_pos(&screen, 2, 0) = '3';
	*screen_pos(&screen, 3, 0) = '4';

	screen_draw(&screen);

	screen_deinit(&screen);
	if (screen.buf != NULL) {
		errno = EINVAL;
		perror("screen_deinit");
		return -1;
	}

	return 0;
}
#endif
