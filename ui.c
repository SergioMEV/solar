#include <stdlib.h>
#include <ncurses.h>

int main(void) {
    // Start the screen
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    // Set up 
    addstr("Hello World!\n");
    refresh();

    while (1) {
        addch(getch());
        refresh();
    }
    

    endwin();

}