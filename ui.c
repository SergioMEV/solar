#include <ncurses.h>
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "file.h"

static FORM *form;
static FIELD *fields[5];
//static WINDOW *win_body, *win_form;
static WINDOW *win_body, *win_form, *DISPLAY, *MISC_BAR;

int MIN_LINE = 0;
int MAX_CHARS = 100;
int CURRENT_LINE = 0;

/*
 * This is useful because ncurses fill fields blanks with spaces.
 */
static char* trim_whitespaces(char *str)
{
	char *end;

	// trim leading space
	while(isspace(*str))
		str++;

	if(*str == 0) // all spaces?
		return str;

	// trim trailing space
	end = str + strnlen(str, 128) - 1;

	while(end > str && isspace(*end))
		end--;

	// write new null terminator
	*(end+1) = '\0';

	return str;
}

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

    MISC_BAR = newwin((int) LINES * 0.7, (int) COLS * 0.2, 2,  (int) COLS * 0.8);
    if (MISC_BAR == NULL) {
        perror("Couldn't initialize text box");
    }
    
    box(DISPLAY, 0, 0);
    box(win_body, 0, 0);
    box(MISC_BAR, 0, 0);

    wrefresh(DISPLAY);
    wrefresh(win_body);
    wrefresh(MISC_BAR);

    refresh();
}

void driver()
{
    int ch, num_chars;

    num_chars = 0;
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case '\n':
            case KEY_ENTER: // Handle inputting line to data structure
                // Making sure form is up to date
                form_driver(form, REQ_NEXT_FIELD);
                form_driver(form, REQ_PREV_FIELD);


                mvprintw(0, 0, "%s", trim_whitespaces(field_buffer(fields[0], 0)));

                // int position = 0;
                // while (position <= num_chars - 1){
                //     form_driver(form, REQ_NEXT_CHAR);
                //     position++;
                // }

                // while (num_chars > 0){
                //     form_driver(form, REQ_DEL_PREV);
                //     num_chars--;
                // }
                // need to clear buffer after line is submitted

                refresh();
                pos_form_cursor(form);

                break;

            case KEY_DOWN:
                if (CURRENT_LINE < 100) CURRENT_LINE ++; // Need to use file->size here LOCKS
                break;

            case KEY_UP:
                if(CURRENT_LINE < 0) CURRENT_LINE --; // LOCKS
                break;

            case KEY_LEFT:
                form_driver(form, REQ_PREV_CHAR);
                break;

            case KEY_RIGHT:
                form_driver(form, REQ_NEXT_CHAR);
                break;

            case KEY_BACKSPACE:
                num_chars--;
                if (num_chars >= MAX_CHARS) continue;

                form_driver(form, REQ_DEL_PREV);
                break;

            case KEY_DC:
                form_driver(form, REQ_DEL_CHAR);
                break;

            default:
                num_chars++;
                if (num_chars >= MAX_CHARS) continue;

                form_driver(form, ch);
                break;
        }

        wrefresh(win_form);
    }
}

void text_box_handler(){
    win_body = newwin((int) LINES * 0.3, (int) COLS - 1, 2 + (int) LINES * 0.6, 0);
    if (win_body == NULL) {
        perror("Couldn't initialize text box");
    }
	assert(win_body != NULL);
	box(win_body, 0, 0);
	win_form = derwin(win_body, ((int) LINES * 0.3) - 3, (int) COLS - 3, 2, 1);
	assert(win_form != NULL);
	box(win_form, 0, 0);
	mvwprintw(win_body, 1, 2, "Instructions here");
	mvwprintw(win_form, 1, 1, "Text:");

	fields[0] = new_field(1, MAX_CHARS, 5 + (int) LINES * 0.6, 7, 0, 0);
	fields[1] = NULL;
	assert(fields[0] != NULL);

	set_field_buffer(fields[0], 0, "");

	set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

	form = new_form(fields);
	assert(form != NULL);
	set_form_win(form, win_form);
	set_form_sub(form, derwin(win_form, 18, 76, 1, 1));
	post_form(form);

	refresh();
	wrefresh(win_body);
	wrefresh(win_form);
    
	driver();

	unpost_form(form);
	free_form(form);
	free_field(fields[0]);
	delwin(win_form);
	delwin(win_body);
}

int print_text(int min_line, int display_max_y, file_content_t *file_content) {
    char *line_number = (char*)malloc(10 * sizeof(char));
    // loop through array while line in line_range 
    int i = MIN_LINE; //MIN_LINE equals smallest line currently visible

    while (i <= display_max_y + MIN_LINE) {
        // Print number
        sprintf(line_number, "%d", i);
        mvwprintw(DISPLAY, i % display_max_y, 1, line_number);
        
        // If printing selected line, then highlight else no highlight. NEEDS LOCKS
        if (i == CURRENT_LINE) wattron(DISPLAY, A_REVERSE);

        // Print line
        mvwprintw(DISPLAY, i % display_max_y, 4, file_content->file_content_head[i]->text);

        // Make sure highlight is off
        wattroff(DISPLAY, A_REVERSE);
    }

    return 1;
}

void display_handler(file_content_t *file_content) {

    DISPLAY = newwin((int) LINES * 0.7, (int) COLS * 0.8, 2, 0);
    if (DISPLAY == NULL) {
        perror("Couldn't initialize text box");
    }
    box(DISPLAY, 0, 0);

    refresh();
    wrefresh(DISPLAY);
    
    int min_line = 0;
    while(TRUE) {
        if (CURRENT_LINE > min_line + 10) min_line ++;

        print_text(min_line, 10, file_content); 
        wrefresh(DISPLAY);
    }
    
    delwin(DISPLAY);
}

int main()
{
	screensetup();
    // text_box_handler(); // has to be in a thread
    // display_handler(); // has to be in a thread

	endwin();

	return 0;
}