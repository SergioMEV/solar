#include <ncurses.h>
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "file_system.c"
#include "constants.h"
#include <pthread.h>

static FORM *text_form;
static FIELD *fields[2];

static WINDOW *TEXT_BODY, *TEXT_FORM_BOX, *DISPLAY, *INSTRUCTIONS_BAR;

int CURRENT_LINE_INDEX = 0;
int CURRENT_ACTION;

char *CURRENT_LINE; 

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

int print_text(int min_line, int display_max_y, file_content_t *file_content) {
    werase(DISPLAY);

    for (size_t line_index = min_line; line_index < (display_max_y + min_line); line_index++)
    {
        if (line_index >= (file_content->total_line_size)) break;

        // If printing selected line, then highlight else no highlight. NEEDS LOCKS
        if (line_index == CURRENT_LINE_INDEX) wattron(DISPLAY, A_REVERSE);

        mvwprintw(DISPLAY, (line_index - min_line) + 1, 4, "%zu: %s\n", line_index, file_content->file_content_head[line_index]->text);

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
    TEXT_BODY = newwin((int) LINES * 0.3, (int) COLS - 1, 2 + (int) LINES * 0.6, 0);
    if (TEXT_BODY == NULL) {
        perror("Couldn't initialize text box");
    }
	box(TEXT_BODY, 0, 0);

    // Text form
	TEXT_FORM_BOX = derwin(TEXT_BODY, ((int) LINES * 0.3) - 3, (int) COLS - 3, 2, 1);
	if (TEXT_FORM_BOX == NULL) {
        perror("Couldn't initialize text form");
    }
	box(TEXT_FORM_BOX, 0, 0);

    // Instructions and text label
	mvwprintw(TEXT_BODY, 1, 2, "ENTER: Modify line.    i: Insert line at index.    d: Delete line at index.    n: Append new line to end of file.  UP/DOWN Arrows: Scroll through lines.");
	mvwprintw(TEXT_FORM_BOX, 1, 1, "Text:");

    // Creating fields
	fields[0] = new_field(1, MAX_CHARS, 5 + (int) LINES * 0.6, 8, 0, 0);
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
	set_form_win(text_form, TEXT_FORM_BOX);
	set_form_sub(text_form, derwin(TEXT_FORM_BOX, 18, 76, 1, 1));
	post_form(text_form);

	refresh();
	wrefresh(TEXT_BODY);
	wrefresh(TEXT_FORM_BOX);
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

void instructions_setup() {
    // Creating window
    INSTRUCTIONS_BAR = newwin((int) LINES * 0.6, (int) COLS * 0.2 - 3, 2, (int) COLS * 0.8 + 2);
    if (INSTRUCTIONS_BAR == NULL) {
        perror("Couldn't initialize text box");
    }
    box(INSTRUCTIONS_BAR, 0, 0);

    // Writing instructions
    mvwprintw(INSTRUCTIONS_BAR, 1, 1, "Instructions:");

    mvwprintw(INSTRUCTIONS_BAR, 2, 1, "- Use the arrow keys to select a line and");
    mvwprintw(INSTRUCTIONS_BAR, 3, 1, "press ENTER to edit it.");

    mvwprintw(INSTRUCTIONS_BAR, 4, 1, "- To insert a new line at the selected");
    mvwprintw(INSTRUCTIONS_BAR, 5, 1, "index, press 'i'.");

    mvwprintw(INSTRUCTIONS_BAR, 6, 1, "- To delete a selected line, press 'd'.");
    mvwprintw(INSTRUCTIONS_BAR, 7, 1, "- To append a new line, press 'n'.");
    
    refresh();
    wrefresh(INSTRUCTIONS_BAR);
}

// DRIVERS

bool text_box_driver(file_content_t *file_content)
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

                // Append line to file content at current line 
                line_t *line = init_line_with_text(trim_whitespaces(field_buffer(fields[0], 0)));

                // If we are modifying a line, delete the original line
                if (CURRENT_ACTION == ACTION_MODIFY) remove_line(file_content, CURRENT_LINE_INDEX);   
                // Add line to file_content             
                add_line(file_content, line, CURRENT_LINE_INDEX);

                // Send line message to server
                //string_concatenate(username, CURRENT_LINE, CURRENT_ACTION, trim_whitespaces(field_buffer(fields[0], 0)));

                // Clearing form
                for (int position = 0; position <= num_chars - 1; position++){
                    form_driver(text_form, REQ_NEXT_CHAR);
                }
                while (num_chars > 0){
                    form_driver(text_form, REQ_DEL_PREV);
                    num_chars--;
                }

                // Refreshing screen and resetting cursor
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

        wrefresh(TEXT_FORM_BOX);
    }

    return FALSE;
}

void line_selection_driver(file_content_t *file_content, int ch, int max_line) {
    switch(ch) {
        // Action keys
        case '\n':
            // Modify line
            CURRENT_ACTION = ACTION_MODIFY;
            return;
        case 'd': 
            // Delete line
            CURRENT_ACTION = ACTION_DELETE;
            // Delete from local file content
            if (max_line >= 0) remove_line(file_content, CURRENT_LINE_INDEX);            
            if (CURRENT_LINE_INDEX == max_line) CURRENT_LINE--; 
            
            // Create query
            // string_concatenate(username, CURRENT_LINE, CURRENT_ACTION, "");

            // Send message
            // send_message()

            return;
        case 'n': 
            // Append line
            CURRENT_ACTION = ACTION_APPEND;
            // Update current line index
            CURRENT_LINE_INDEX = max_line + 1;
            return;
        case 'i':
            // Insert line
            CURRENT_ACTION = ACTION_INSERT;
            // Set current line to next line 
            CURRENT_LINE++;
            return;
        case 'r': 
            // Refresh
            return;
        // Arrow keys
        case KEY_UP:
            CURRENT_LINE_INDEX = (CURRENT_LINE_INDEX == 0) ? CURRENT_LINE_INDEX : CURRENT_LINE_INDEX - 1;
            break;
        case KEY_DOWN:
            CURRENT_LINE_INDEX = (CURRENT_LINE_INDEX == max_line) ? CURRENT_LINE_INDEX : CURRENT_LINE_INDEX + 1;
            break;
    }
}

bool display_driver(file_content_t *file_content) {
    int ch;
    int min_line = 0; 
    int display_max_y = ((int) LINES * 0.6) - 2;

    while (TRUE) {

        while((ch = getch()) == ERR){
            // Print text to display
            if (CURRENT_LINE_INDEX >= min_line + display_max_y ) { 
                min_line++;
            } else if (CURRENT_LINE_INDEX < min_line) {
                min_line--;
            }

            print_text(min_line, display_max_y, file_content); 
        }

        // Listen for user input (line selection)
        line_selection_driver( file_content,ch, file_content->total_line_size - 1);
        
        // Refresh display
        wrefresh(DISPLAY); 

        // Handle display input exit
        if (ch == '\n') { // Selected a line to modify
            // Set CURRENT_LINE for display
            CURRENT_LINE = file_content->file_content_head[CURRENT_LINE_INDEX]->text;
            return TRUE;
        } else if (ch == 'n' || ch == 'i') { // Appending new line to file
            CURRENT_LINE = "";
            return TRUE;
        } else if (ch == KEY_F(1)) { // Exiting program
            return FALSE;
        } 
    };
}

void ui_driver(file_content_t *file_content){
    while (TRUE) {
        if(!display_driver(file_content)) return;
	    if(!text_box_driver(file_content)) return;
    }
}


// CLEANING FUNCTIONS

void free_ui() {
    // Free text box elements
    unpost_form(text_form);
	free_form(text_form);
	free_field(fields[0]);
	delwin(TEXT_FORM_BOX);
	delwin(TEXT_BODY);

    // Free display
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
    instructions_setup();
    
    // Titles
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
    FILE *fptr = open_file_read_mode(file_name);
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