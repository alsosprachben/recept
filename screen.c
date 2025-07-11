#include "screen.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>

int screen_clear() {
	int rc;
	rc = fwprintf(stderr, L"%ls", ESCAPE_CLEAR);
	if (rc == -1) {
		return -1;
	}
	rc = fflush(stderr);
	if (rc == -1) {
		return -1;
	}

	return 0;
}

int screen_draw(struct screen *screen_ptr) {
	int rc;
	return fwprintf(stderr, L"%ls", screen_ptr->buf);
	if (rc == -1) {
		return -1;
	}
	rc = fflush(stderr);
	if (rc == -1) {
		return -1;
	}

	return 0;
}

wchar_t *screen_pos(struct screen *screen_ptr, int column, int row) {
	return &screen_ptr->frame[row * screen_ptr->columns + column];
}

/*
 * Set `size_t n` to be exactly the amount of space you want printed,
 * If `new_terminator` is `'\0'` then restore the terminator with what was previously in the frame buffer.
 * Take care at the end of the frame buffer.
 * The string terminator will never be null, but rather either:
 *  1. the new_terminator, or
 *  2. the previous character in the terminal position (if new_terminator is null).
 */
int screen_nprintf(struct screen *screen_ptr, int column, int row, size_t n, wchar_t new_terminator, const wchar_t *format, ...) {
	int rc;
	va_list args;
	wchar_t *s;
	wchar_t terminator;
	int pad_n;

	n++;

	s = screen_pos(screen_ptr, column, row);

	if (new_terminator == '\0') {
		terminator = s[n];
	} else {
		terminator = new_terminator;
	}

	va_start(args, format);
	rc = vswprintf(s, n, format, args);
	if (rc != -1) {
		for (pad_n = rc; pad_n < n; pad_n++) {
			s[pad_n] = L' ';
		}
		s[n] = terminator;
	}
	va_end(args);

	return rc;
}

void screen_blank(struct screen *screen_ptr) {
	int column;
	int row;

	for (column = 0; column < screen_ptr->columns; column++) {
		for (row = 0; row < screen_ptr->rows; row++) {
			*screen_pos(screen_ptr, column, row) = L' ';
		}
	}
	*screen_pos(screen_ptr, screen_ptr->columns + 1, screen_ptr->rows) = '\0';
}

int screen_init(struct screen *screen_ptr, int columns, int rows) {
	setlocale(LC_ALL, "en_US.UTF-8");
	screen_ptr->columns = columns;
	screen_ptr->rows = rows;
	screen_ptr->screen_size = columns * (rows + 1);
	screen_ptr->buf_size = ESCAPE_RESET_LEN + screen_ptr->screen_size + 1;
	screen_ptr->buf = calloc(sizeof (wchar_t), screen_ptr->buf_size);
	screen_ptr->frame = screen_ptr->buf + ESCAPE_RESET_LEN;
	if (screen_ptr->buf == NULL) {
		errno = ENOMEM;
		return -1;
	}

	for (int i = 0; i < ESCAPE_RESET_LEN; i++) {
		screen_ptr->buf[i] = ESCAPE_RESET[i];
	}
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
