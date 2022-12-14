#include <stdbool.h>

// MISC FUNCTIONS
static char *trim_whitespaces(char *str);
int print_text(int min_line, int display_max_y, file_content_t *file_content);
void modify_action_display(char action, int line_index);

// SETUP FUNCTIONS
void screensetup(void);
void text_box_setup();
void instructions_setup();

// DRIVERS

bool text_box_driver(file_content_t *file_content);
void line_selection_driver(file_content_t *file_content, int ch, int max_line);
bool display_driver(file_content_t *file_content);

// MAIN THREAD FUNCTION
void *ui_thread_handler(void *args);