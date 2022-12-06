#include <ncurses.h>

// Windows
WINDOW *DISPLAY = NULL;
WINDOW *TEXT_BOX = NULL;
WINDOW *MISC_BAR = NULL;

void screensetup(void);
// int print_text(int display_max_y, file_content_t *file_content);