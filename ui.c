#include <ncurses.h>
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "file_system.c"
#include <pthread.h>

static FORM *form;
static FIELD *fields[5];

static WINDOW *win_body, *win_form, *DISPLAY, *MISC_BAR;

int MIN_LINE = 0;
int MAX_CHARS = 100;
int CURRENT_LINE = 0;

int TEST_LINE = 0;

// MISC FUNCTIONS

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

int print_text(int min_line, file_content_t *file_content) {
    int display_max_y = ((int) LINES * 0.6) - 2;

    for (size_t line_index = min_line; line_index < display_max_y; line_index++)
    {
        if (line_index >= file_content->total_line_size) break;

        // If printing selected line, then highlight else no highlight. NEEDS LOCKS
        if (line_index == CURRENT_LINE) wattron(DISPLAY, A_REVERSE);

        mvwprintw(DISPLAY, (line_index % display_max_y) + 1, 4, "%zu: <%s>\n", line_index, file_content->file_content_head[line_index]->text);

        // Make sure highlight is off
        wattroff(DISPLAY, A_REVERSE);

        box(DISPLAY, 0, 0);
        wrefresh(DISPLAY);
    }

    return 1;
}

// SETUP FUNCTIONS

void screensetup(void) {
    // Start screen
    initscr();
    // Get input as it is typed and not when ENTER is pressed
    raw();
    // Do not echo user input to screen
    noecho();
    // Accept keypad input
    keypad(stdscr, TRUE);
    // Makes it so that we read input char by char
    cbreak();
    // Sets wait time for input in tenths of seconds
    halfdelay(2);

    refresh();
}

void text_box_setup() {
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

    set_max_field(fields[0], MAX_CHARS);

	form = new_form(fields);
	assert(form != NULL);
	set_form_win(form, win_form);
	set_form_sub(form, derwin(win_form, 18, 76, 1, 1));
	post_form(form);

	refresh();
	wrefresh(win_body);
	wrefresh(win_form);
}

void display_setup() {
    //file_content_t* file_content = arg->file_content;
    DISPLAY = newwin((int) LINES * 0.6, (int) COLS * 0.8, 2, 0);
    if (DISPLAY == NULL) {
        perror("Couldn't initialize text box");
    }
    box(DISPLAY, 0, 0);

    refresh();
    wrefresh(DISPLAY);
}

// DRIVERS

void text_box_driver()
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

                mvprintw(TEST_LINE  + 3, COLS * 0.8, "%s", trim_whitespaces(field_buffer(fields[0], 0)));
                TEST_LINE++;

                int position = 0;
                while (position <= num_chars - 1){
                    form_driver(form, REQ_NEXT_CHAR);
                    position++;
                }

                while (num_chars > 0){
                    form_driver(form, REQ_DEL_PREV);
                    num_chars--;
                }

                refresh();
                pos_form_cursor(form);

                return;

            case KEY_DOWN:
                form_driver(form, REQ_NEXT_FIELD);
                break;

            case KEY_UP:
                form_driver(form, REQ_NEXT_FIELD);
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

void line_selection_driver(int ch, int max_line) {
    switch(ch) {
        case 'r':
            return;
        case '\n':
            // Select line and send message selection
            return;
        case KEY_UP:
            CURRENT_LINE = (CURRENT_LINE == 0) ? CURRENT_LINE : CURRENT_LINE - 1;
            break;
        case KEY_DOWN:
            CURRENT_LINE = (CURRENT_LINE + 1 == max_line) ? CURRENT_LINE : CURRENT_LINE + 1;
            break;
        default:
            break;
    }
}

void display_driver(file_content_t *file_content) {
    int ch;
    int min_line = 0;
    while (TRUE) {

        while((ch = getch()) == ERR){
            // Print text to display
            if (CURRENT_LINE > min_line + 10) min_line ++;
            print_text(min_line, file_content); 
        }

        // Listen for user input (line selection)
        line_selection_driver(ch, file_content->total_line_size);
        wrefresh(DISPLAY); 

        if (ch == '\n') return;
    };
}

void ui_driver(file_content_t *file_content){
    while (TRUE) {
        display_driver(file_content);
	    text_box_driver();
    }
}


// CLEANING FUNCTIONS

void free_ui() {
    // Free text box elements
    unpost_form(form);
	free_form(form);
	free_field(fields[0]);
	delwin(win_form);
	delwin(win_body);

    // Free screen
    delwin(DISPLAY);

    // Free stdscreen
    endwin();
}

// MAIN THREAD FUNCTION

void*  ui_thread_handler(void *args) {
    file_content_t *file_content = (file_content_t *) args;

    // Setup of UI elements
    screensetup();
    display_setup();
    text_box_setup();
    
    mvprintw(1, 2, "Solar!");
    mvprintw(1, COLS * 0.82, "Your mother's favorite text editor!");
    // Driver that controls user interaction
    ui_driver(file_content);

    // Freeing elements of UI
    free_ui();

    return NULL;
}

int main()
{
	pthread_t ui_thread;
    char *file_name = "Archive/f1.txt";
    FILE *fptr = open_file_append_mode(file_name);
    file_content_t *file_content = read_file_to_file_content(fptr);


    if (pthread_create(&ui_thread, NULL, ui_thread_handler, (void *) file_content)) {
        perror("Couldn't create display thread:");
        exit(2);
    }


    if (pthread_join(ui_thread, NULL)) {
        perror("Couldn't join display thread");
        exit(2);
    }

    
    // //print_file_content(file_content);
    // //clean_file_system(fptr, file_content);

    // // Setup of UI elements
    // screensetup();
    // display_setup();
    // text_box_setup();

    // // Driver that controls user interaction
    // ui_driver(file_content);
}