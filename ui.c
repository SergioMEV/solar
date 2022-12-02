#include <stdlib.h>
#include <ncurses.h>
#include <forms.h>

void screensetup(void) {
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
}

int main(void) {
    // Setup screen
    screensetup();

    // Bounds
    int lower_bound = LINES - 1;
    int upper_bound = 2;
    int right_bound = COLS - 1;
    int left_bound = 0;

    // Creating windows
    WINDOW * display_box = newwin((int) LINES * 0.7, (int) COLS * 0.8, upper_bound, left_bound);
    if (display_box == NULL) {
        perror("Couldn't initialize text box");
    }

    WINDOW * text_box = newwin((int) LINES * 0.2, (int) COLS - 1, 2 + (int) LINES * 0.7, left_bound);
    if (text_box == NULL) {
        perror("Couldn't initialize text box");
    }

    WINDOW * misc_box = newwin((int) LINES * 0.7, (int) COLS * 0.2, upper_bound,  (int) COLS * 0.8);
    if (misc_box == NULL) {
        perror("Couldn't initialize text box");
    }
    refresh();

    // Adding borders around windows and refreshing them
    box(display_box, 0, 0);
    box(text_box, 0, 0);
    box(misc_box, 0, 0);

    wrefresh(display_box);
    wrefresh(text_box);
    wrefresh(misc_box);

    // Title
    move(1,2);
    printw("Solar!");
    refresh();

    getch();
    // while (1) {
    //     addch(getch());
    //     refresh();
    // }
    

    endwin();

}