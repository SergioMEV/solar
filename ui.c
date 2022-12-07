#include <stdlib.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
//#include <forms.h>

// Windows
WINDOW * DISPLAY = NULL;
WINDOW * TEXT_BOX = NULL;
WINDOW * MISC_BAR = NULL;

int MIN_LINE = 0;
int MAX_CHARS = 100;

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

    // Bounds
    int lower_bound = LINES - 1;
    int upper_bound = 2;
    int right_bound = COLS - 1;
    int left_bound = 0;

    // Creating windows
    DISPLAY = newwin((int) LINES * 0.7, (int) COLS * 0.8, upper_bound, left_bound);
    if (DISPLAY == NULL) {
        perror("Couldn't initialize text box");
    }

    TEXT_BOX = newwin((int) LINES * 0.2, (int) COLS - 1, 2 + (int) LINES * 0.7, left_bound);
    if (TEXT_BOX == NULL) {
        perror("Couldn't initialize text box");
    }

    MISC_BAR = newwin((int) LINES * 0.7, (int) COLS * 0.2, upper_bound,  (int) COLS * 0.8);
    if (MISC_BAR == NULL) {
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
    move(1,2);
    printw("Solar!");
    refresh();
}

int text_box_handler(void) {
    int text_max_x, text_max_y;
    getmaxyx(TEXT_BOX, text_max_y, text_max_x);
    
    int c, x;
    x = 2;
    while((c = getch()) && x != 102){
        mvwaddch(TEXT_BOX, 1, x, c);
        wrefresh(TEXT_BOX);

        x++;
    }

    return 1;
}

int print_text(int display_max_y, file_content_t *file_content) {
    char *line_number = (char*)malloc(10 * sizeof(char));
    // loop through array while line in line_range 
    int i = MIN_LINE; //MIN_LINE equals smallest line currently visible

    while (i <= display_max_y + MIN_LINE) {
        sprintf(line_number, "%d", i);
        mvwprintw(DISPLAY, i % display_max_y, 1, line_number);
        mvwprintw(DISPLAY, i % display_max_y, 4, file_content->file_content_head[i]->text);
    }

    return 1;
}

int main(void) {
    // Setup screen
    screensetup();

    // printing text file and letting you choose
    FILE * file = fopen("Archive/test.txt", "r");
    if (!file){
        return false;
    }

    char * read_line = malloc(sizeof(char) * 100);

    int display_max_x, display_max_y;
    int current_line = 1;
    getmaxyx(DISPLAY, display_max_y, display_max_x);

    char *line_number = (char*)malloc(10 * sizeof(char));

    while (fgets(read_line, 100, file)) {
        sprintf(line_number, "%d", current_line);
        // TODO: Figure out how to convert the linenumber to string
        mvwprintw(DISPLAY, current_line % display_max_y, 1, line_number);
        mvwprintw(DISPLAY, current_line % display_max_y, 4,read_line);
        current_line++;

        box(DISPLAY, 0, 0);
        wrefresh(DISPLAY);   
    }
    
    free(read_line);
    fclose(file);
    
    text_box_handler();

    endwin();

}