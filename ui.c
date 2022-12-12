#include <ncurses.h>
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "file_system.c"
#include <pthread.h>

static FORM *text_form;
static FIELD *fields[2];

static WINDOW *text_body, *text_form_box, *DISPLAY, *MISC_BAR;

int MIN_LINE = 0;
int MAX_CHARS = 100;
int CURRENT_LINE_INDEX = 0;
char *CURRENT_LINE;
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
        if (line_index == CURRENT_LINE_INDEX) wattron(DISPLAY, A_REVERSE);

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
    // Text box
    text_body = newwin((int) LINES * 0.3, (int) COLS - 1, 2 + (int) LINES * 0.6, 0);
    if (text_body == NULL) {
        perror("Couldn't initialize text box");
    }
	box(text_body, 0, 0);

    // Text form
	text_form_box = derwin(text_body, ((int) LINES * 0.3) - 3, (int) COLS - 3, 2, 1);
	if (text_form_box == NULL) {
        perror("Couldn't initialize text form");
    }
	box(text_form_box, 0, 0);

    // Instructions and text label
	mvwprintw(text_body, 1, 2, "Choose a line to edit and type new text below.");
	mvwprintw(text_form_box, 1, 1, "Text:");

    // Creating fields
	fields[0] = new_field(1, MAX_CHARS, 5 + (int) LINES * 0.6, 7, 0, 0);
    if (fields[0] == NULL) {
        perror("Couldn't initialize text form field");
    }

    fields[1] = NULL;

    // Field options
	set_field_buffer(fields[0], 0, "");
	set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_max_field(fields[0], MAX_CHARS);

    // Creating form
	text_form = new_form(fields);
	if (text_form == NULL) {
        perror("Couldn't initialize text form field");
    }
	set_form_win(text_form, text_form_box);
	set_form_sub(text_form, derwin(text_form_box, 18, 76, 1, 1));
	post_form(text_form);

	refresh();
	wrefresh(text_body);
	wrefresh(text_form_box);
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

bool text_box_driver()
{
    int ch, num_chars;
    num_chars = 0;

    // Add line text to field
    for (int i = 0; CURRENT_LINE[i] != '\0'; i++){
        form_driver(text_form, CURRENT_LINE[i]);
        num_chars++;
    }

    // Receive user input
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case '\n':
            case KEY_ENTER: // Handle inputting line to data structure
                // Making sure form is up to date
                form_driver(text_form, REQ_NEXT_FIELD);
                form_driver(text_form, REQ_PREV_FIELD);

                mvprintw(TEST_LINE  + 3, COLS * 0.8, "%s", trim_whitespaces(field_buffer(fields[0], 0)));
                TEST_LINE++;

                int position = 0;
                while (position <= num_chars - 1){
                    form_driver(text_form, REQ_NEXT_CHAR);
                    position++;
                }

                while (num_chars > 0){
                    form_driver(text_form, REQ_DEL_PREV);
                    num_chars--;
                }

                refresh();
                pos_form_cursor(text_form);
                return TRUE;

            case KEY_DOWN:
                form_driver(text_form, REQ_NEXT_FIELD);
                break;

            case KEY_UP:
                form_driver(text_form, REQ_NEXT_FIELD);
                break;

            case KEY_LEFT:
                form_driver(text_form, REQ_PREV_CHAR);
                break;

            case KEY_RIGHT:
                form_driver(text_form, REQ_NEXT_CHAR);
                break;

            case KEY_BACKSPACE:
                num_chars--;
                if (num_chars >= MAX_CHARS) continue;

                form_driver(text_form, REQ_DEL_PREV);
                break;

            case KEY_DC:
                form_driver(text_form, REQ_DEL_CHAR);
                break;

            default:
                num_chars++;
                if (num_chars >= MAX_CHARS) continue;

                form_driver(text_form, ch);
                break;
        }

        wrefresh(text_form_box);
    }

    return FALSE;
}

void line_selection_driver(int ch, int max_line) {
    switch(ch) {
        case 'r':
            return;
        case '\n':
            // Select line and send message selection
            return;
        case KEY_UP:
            CURRENT_LINE_INDEX = (CURRENT_LINE_INDEX == 0) ? CURRENT_LINE_INDEX : CURRENT_LINE_INDEX - 1;
            break;
        case KEY_DOWN:
            CURRENT_LINE_INDEX = (CURRENT_LINE_INDEX + 1 == max_line) ? CURRENT_LINE_INDEX : CURRENT_LINE_INDEX + 1;
            break;
        default:
            break;
    }
}

bool display_driver(file_content_t *file_content) {
    int ch;
    int min_line = 0;
    while (TRUE) {

        while((ch = getch()) == ERR){
            // Print text to display
            if (CURRENT_LINE_INDEX > min_line + 10) min_line ++;
            print_text(min_line, file_content); 
        }

        // Listen for user input (line selection)
        line_selection_driver(ch, file_content->total_line_size);
        
        // Refresh display
        wrefresh(DISPLAY); 

        // Handle display input exit
        if (ch == '\n') {
            // Set CURRENT_LINE for display
            CURRENT_LINE = file_content->file_content_head[CURRENT_LINE_INDEX]->text;
            return TRUE;
        } else if (ch == KEY_F(1)) {
            return FALSE;
        }
    };
}

void ui_driver(file_content_t *file_content){
    while (TRUE) {
        if(!display_driver(file_content)) return;
	    if(!text_box_driver()) return;
    }
}


// CLEANING FUNCTIONS

void free_ui() {
    // Free text box elements
    unpost_form(text_form);
	free_form(text_form);
	free_field(fields[0]);
	delwin(text_form_box);
	delwin(text_body);

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

}