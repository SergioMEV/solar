#include "ui.h"
// #include <forms.h>

int MIN_LINE = 0;

void screensetup(void)
{
    // Start screen
    initscr();
    // Get input as it is typed and not when ENTER is pressed
    raw();
    // Do not echo user input to screen
    noecho();
    // Accept keypad input
    keypad(stdscr, TRUE);
    // Adds CTRL + C option to terminate
    cbreak();

    // Bounds
    int lower_bound = LINES - 1;
    int upper_bound = 2;
    int right_bound = COLS - 1;
    int left_bound = 0;

    // Creating windows
    DISPLAY = newwin((int)LINES * 0.7, (int)COLS * 0.8, upper_bound, left_bound);
    if (DISPLAY == NULL)
    {
        perror("Couldn't initialize text box");
    }

    TEXT_BOX = newwin((int)LINES * 0.2, (int)COLS - 1, 2 + (int)LINES * 0.7, left_bound);
    if (TEXT_BOX == NULL)
    {
        perror("Couldn't initialize text box");
    }

    MISC_BAR = newwin((int)LINES * 0.7, (int)COLS * 0.2, upper_bound, (int)COLS * 0.8);
    if (MISC_BAR == NULL)
    {
        perror("Couldn't initialize text box");
    }
    refresh();

    // Adding borders around windows and refreshing them
    box(DISPLAY, 0, 0);
    box(TEXT_BOX, 0, 0);
    box(MISC_BAR, 0, 0);

    wrefresh(DISPLAY);
    wrefresh(TEXT_BOX);
    wrefresh(MISC_BAR);

    // Title
    move(1, 2);
    printw("Solar!");
    refresh();
}

// int print_text(int display_max_y, file_content_t *file_content)
// {
//     char *line_number = (char *)malloc(10 * sizeof(char));
//     // loop through array while line in line_range
//     int i = MIN_LINE; // MIN_LINE equals smallest line currently visible

//     while (i <= display_max_y + MIN_LINE)
//     {
//         sprintf(line_number, "%d", i);
//         mvwprintw(DISPLAY, i % display_max_y, 1, line_number);
//         mvwprintw(DISPLAY, i % display_max_y, 4, file_content->file_content_head[i]->text);
//     }

//     return 1;
// }