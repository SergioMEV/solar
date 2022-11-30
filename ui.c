#include <stdlib.h>
#include <ncurses.h>

int main(void) {
    // Start the screen
    initscr();

    // Set up 
    addstr("Hello World!");
    refresh();

}