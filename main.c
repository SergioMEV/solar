#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ui.h"
#include "file_system.h"

int main(void)
{
    // Setup screen
    screensetup();

    FILE *fptr;
    char *get_line_buffer = NULL;
    size_t buffer_size = 0;
    size_t read_size;

    int display_max_x, display_max_y;
    getmaxyx(DISPLAY, display_max_y, display_max_x);

    char *line_number = (char *)malloc(10 * sizeof(char));

    file_content_t *file_content = init_file_content();

    fptr = fopen("Archive/f1.txt", "r+");
    if (fptr == NULL)
    {
        perror("Unable to open the file.");
        exit(1);
    }
    while ((read_size = getline(&get_line_buffer, &buffer_size, fptr)) != -1)
    {
        line_t *new_line = init_line();
        get_line_buffer[read_size - 1] = '\0';
        new_line->text = get_line_buffer;
        get_line_buffer = NULL;
        add_line(file_content, new_line);
    }
    // when we break out of the loop, we need to free the space allocated for the EOF
    // to avoid memory leaks!
    free(get_line_buffer);

    // Warning: restricting line number to be smaller than 10^10
    char *line_number_str = (char *)malloc(10 * sizeof(char));
    for (size_t line_number_int = 0; line_number_int < file_content->line_size; line_number_int++)
    {
        // convert line number to string for printing.
        sprintf(line_number_str, "%zu", line_number_int);
        // TODO: Figure out how to convert the linenumber to string
        mvwprintw(DISPLAY, line_number_int % display_max_y, 1, line_number_str);
        mvwprintw(DISPLAY, line_number_int % display_max_y, 4, file_content->file_content_head[line_number_int]->text);

        box(DISPLAY, 0, 0);
        wrefresh(DISPLAY);
    }
    free(line_number_str);

    fprintf(fptr, "\nhahaha, new stuff");
    // print_file_content(file_content);
    destroy_file_contnt(file_content);
    fclose(fptr);

    endwin();
}