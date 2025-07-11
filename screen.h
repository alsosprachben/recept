#ifndef SCREEN_H
#define SCREEN_H

#define ESCAPE_CLEAR L"\033[2J"
#define ESCAPE_CLEAR_LEN 4
#define ESCAPE_RESET L"\033[;H"
#define ESCAPE_RESET_LEN 4

#include <unistd.h>
#include <wchar.h>

struct screen {
	int columns;
	int rows;
	wchar_t *buf;
	wchar_t *frame;
	size_t screen_size;
	size_t buf_size;
};

int screen_init(struct screen *screen_ptr, int columns, int rows);
void screen_deinit(struct screen *screen_ptr);

wchar_t *screen_pos(struct screen *screen_ptr, int column, int row);
int screen_draw(struct screen *screen_ptr);
int screen_nprintf(struct screen *screen_ptr, int column, int row, size_t n, wchar_t new_terminator, const wchar_t *format, ...);
#endif
