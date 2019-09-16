#include "screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int screen_clear() {
	return printf("%s", ESCAPE_CLEAR);
}

int screen_draw(struct screen *screen_ptr) {
	return printf("%s", screen_ptr->buf);
}

char *screen_pos(struct screen *screen_ptr, int column, int row) {
	return &screen_ptr->frame[row * screen_ptr->columns + column];
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
