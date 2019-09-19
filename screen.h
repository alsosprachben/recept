#ifndef SCREEN_H
#define SCREEN_H

#define ESCAPE_CLEAR "\033[2J"
#define ESCAPE_RESET "\033[;H"

#include <unistd.h>

struct screen {
	int columns;
	int rows;
	char *buf;
	char *frame;
	size_t screen_size;
	size_t buf_size;
};

int screen_init(struct screen *screen_ptr, int columns, int rows);
void screen_deinit(struct screen *screen_ptr);

char *screen_pos(struct screen *screen_ptr, int column, int row);
int screen_draw(struct screen *screen_ptr);
int screen_nprintf(struct screen *screen_ptr, int column, int row, size_t n, char new_terminator, const char *format, ...);
#endif
