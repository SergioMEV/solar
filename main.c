#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "file.h"
#include "file_system.h"

int main(void)
{
    // Setup screen
    screensetup();

    // printing text file and letting you choose
    FILE *file = fopen("Archive/test.txt", "r");
    if (!file)
    {
        return false;
    }

    char *read_line = malloc(sizeof(char) * 100);

    int display_max_x, display_max_y;
    int current_line = 1;
    getmaxyx(DISPLAY, display_max_y, display_max_x);

    char *line_number = (char *)malloc(10 * sizeof(char));

    while (fgets(read_line, 100, file))
    {
        sprintf(line_number, "%d", current_line);
        // TODO: Figure out how to convert the linenumber to string
        mvwprintw(DISPLAY, current_line % display_max_y, 1, line_number);
        mvwprintw(DISPLAY, current_line % display_max_y, 4, read_line);
        current_line++;

        box(DISPLAY, 0, 0);
        wrefresh(DISPLAY);
    }

    free(read_line);
    fclose(file);

    getch();
    // while (1) {
    //     addch(getch());
    //     refresh();
    // }

    endwin();
}