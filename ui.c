#include <ncurses.h>
#include <unistd.h>
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "file_system.h"
#include "constants.h"
#include "query_util.h"
#include "message.h"
#include "ui.h"
#include <pthread.h>

static FORM *text_form;
static FIELD *fields[2];

static WINDOW *TEXT_BODY, *TEXT_FORM_BOX, *DISPLAY, *MISC_BAR;

int CURRENT_LINE_INDEX;
int CURRENT_ACTION;

char *CURRENT_LINE;

// MISC FUNCTIONS

/*******************************************************************************
 * trim_whitespaces takes in the buffer char array and returns the buffer 
 *  without any leading or trailing spaces. Needed because forms.h adds whitespaces to the buffer/
 *
 * @param
 *    - char *buffer
 * @return char*
 ******************************************************************************/
static char *trim_whitespaces(char *buffer)
{
    char *last_char;

    // Move pointer to first non-space character
    while (isspace(*buffer))
        buffer++;
    if (*buffer == 0) // all spaces?
        return buffer;

    // Find last non-space character
    last_char = buffer + strnlen(buffer, 128) - 1;
    while (last_char > buffer && isspace(*last_char))
        last_char--;

    // Terminate string at space after last non-space character
    *(last_char + 1) = '\0';
    return buffer;
}

/*******************************************************************************
 * print_text takes in the minimum line index on screen, the total number of lines in the display, and the file_content struct 
 *      and prints out the maximum amount of lines possible on the display window, starting from the line with index min_line.
 *
 * @param
 *    - int min_line,
 *    - int display_max_y,
 *    - file_content_t *file_content
 * @return int
 ******************************************************************************/
int print_text(int min_line, int display_max_y, file_content_t *file_content)
{
    werase(DISPLAY);

    for (size_t line_index = min_line; line_index < (display_max_y + min_line); line_index++)
    {
        if (line_index >= (file_content->total_line_size))
            break;

        // If printing selected line, then highlight else no highlight. NEEDS LOCKS
        if (line_index == CURRENT_LINE_INDEX)
            wattron(DISPLAY, A_REVERSE);

        mvwprintw(DISPLAY, (line_index - min_line) + 1, 4, "%zu: %s\n", line_index, file_content->file_content_head[line_index]->text);

        // Make sure highlight is off
        wattroff(DISPLAY, A_REVERSE);

        box(DISPLAY, 0, 0);
        wrefresh(DISPLAY);
    }

    return 1;
}

bool request_access(file_content_t *file_content) {
    file_content->is_blocked = REQUEST_PENDING;
    
    // Ask server to make changes 
    char* request_line_message = query_constructor(file_content->user_name, CURRENT_LINE_INDEX, ACTION_REQUEST, " ");
    if (send_message(file_content->server_fd, request_line_message) == -1)
    {
        perror("Failed to send message to the server");
        exit(EXIT_FAILURE);
    }

    // Wait for response
    while (strcmp(file_content->is_blocked, REQUEST_PENDING) == 0){
        continue;
    }

    // If denied, return false
    if (strcmp(file_content->is_blocked, REQUEST_DENIED) == 0) {
        modify_action_display(-1, CURRENT_LINE_INDEX);
        return FALSE;
    }

    
    return TRUE;
}

/*******************************************************************************
 * modify_action_display takes in the current action and the current line index and
 *      updates the action display inside the form text box.
 *
 * @param
 *    - char action,
 *    - int line_index
 * @return void
 ******************************************************************************/
void modify_action_display(char action, int line_index)
{
    if (action == -1) {
        werase(TEXT_FORM_BOX);
        mvwprintw(TEXT_FORM_BOX, 1, 2, "LINE [%d] LOCKED:", line_index);
        box(TEXT_FORM_BOX, 0, 0);
        wrefresh(TEXT_FORM_BOX);

        return;
    }

    char *full_action;

    int start_index = 1;
    if (line_index >= 100)
    {
        start_index += 2;
    }
    else if (line_index >= 10)
    {
        start_index++;
    }

    switch (action)
    {
    case ACTION_APPEND:
        full_action = "Append";
        break;
    case ACTION_INSERT:
        full_action = "Insert";
        break;
    case ACTION_MODIFY:
        full_action = "Modify";
        break;
    case ACTION_DELETE:
        return;
    default:
        return;
    }

    werase(TEXT_FORM_BOX);
    mvwprintw(TEXT_FORM_BOX, 1, 4 - start_index, "%s @ [%d]:", full_action, line_index);
    box(TEXT_FORM_BOX, 0, 0);
    wrefresh(TEXT_FORM_BOX);
}

// SETUP FUNCTIONS

/*******************************************************************************
 * screen_setup setups miscellanous screen options unique to ncurses.
 *
 * @return void
 ******************************************************************************/
void screen_setup()
{
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

/*******************************************************************************
 * text_box_setup creates the text box, text form box, and the text form.
 *      It also setups the form with the necessary attributes and prints the instructions.
 *
 * @return void
 ******************************************************************************/
void text_box_setup()
{
    // Text box
    TEXT_BODY = newwin((int)LINES * 0.3, (int)COLS - 1, 2 + (int)LINES * 0.6, 0);
    if (TEXT_BODY == NULL)
    {
        perror("Couldn't initialize text box");
    }
    box(TEXT_BODY, 0, 0);

    // Text form
    TEXT_FORM_BOX = derwin(TEXT_BODY, ((int)LINES * 0.3) - 3, (int)COLS - 4, 2, 2);
    if (TEXT_FORM_BOX == NULL)
    {
        perror("Couldn't initialize text form");
    }
    box(TEXT_FORM_BOX, 0, 0);

    // Instructions and text label
    mvwprintw(TEXT_BODY, 1, 4, "ENTER: Modify line.    i: Insert line.    d: Delete line.    n: Append new line.    UP/DOWN Arrows: Scroll.    F1: Exit.");
    modify_action_display(ACTION_MODIFY, CURRENT_LINE_INDEX);

    // Creating fields
    fields[0] = new_field(1, MAX_CHARS, 5 + (int)LINES * 0.6, 19, 0, 0);
    if (fields[0] == NULL)
    {
        perror("Couldn't initialize text form field");
    }

    fields[1] = NULL;

    // Field options
    set_field_buffer(fields[0], 0, "");
    set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_max_field(fields[0], MAX_CHARS);

    // Creating form
    text_form = new_form(fields);
    if (text_form == NULL)
    {
        perror("Couldn't initialize text form field");
    }
    set_form_win(text_form, TEXT_FORM_BOX);
    set_form_sub(text_form, derwin(TEXT_FORM_BOX, 18, 76, 1, 1));
    post_form(text_form);

    refresh();
    wrefresh(TEXT_BODY);
    wrefresh(TEXT_FORM_BOX);
}

/*******************************************************************************
 * display_setup creates the display window and its borders.
 *
 * @return void
 ******************************************************************************/
void display_setup()
{
    // Setting current line to first line.
    CURRENT_LINE_INDEX = 0;

    // file_content_t* file_content = arg->file_content;
    DISPLAY = newwin((int)LINES * 0.6, (int)COLS * 0.8, 2, 0);
    if (DISPLAY == NULL)
    {
        perror("Couldn't initialize text box");
    }
    box(DISPLAY, 0, 0);

    refresh();
    wrefresh(DISPLAY);
}

/*******************************************************************************
 * misc_setup creates the window next to the display and its borders.
 *
 * @return void
 ******************************************************************************/
void misc_setup()
{
    // Creating window
    MISC_BAR = newwin((int)LINES * 0.6, (int)COLS * 0.2 - 3, 2, (int)COLS * 0.8 + 2);
    if (MISC_BAR == NULL)
    {
        perror("Couldn't initialize text box");
    }
    box(MISC_BAR, 0, 0);

    refresh();
    wrefresh(MISC_BAR);
}


// DRIVERS

/*******************************************************************************
 * text_box_driver handles all text_box user input. Handles keyboard input and typing.
 *      It also clears text form after typing. 
 *      Also, sends out message to server with query and updates file_content.
 *      Returns true if user did not exit program, else false.
 *
 * @param
 *    - file_content_t *file_content
 * @return bool
 ******************************************************************************/
bool text_box_driver(file_content_t *file_content)
{
    int ch, num_chars;
    num_chars = 0;

    // Ask for access
    if (ch != 'n' && !request_access(file_content)){
        return TRUE;
    }

    // Add line text to field
    for (int i = 0; CURRENT_LINE[i] != '\0'; i++)
    {
        form_driver(text_form, CURRENT_LINE[i]);
        num_chars++;
    }

    // Receive user input
    while ((ch = getch()) != KEY_F(1))
    {
        switch (ch)
        {
        case '\n':
        case KEY_ENTER: // Handle inputting line to data structure
            // Making sure form is up to date
            form_driver(text_form, REQ_NEXT_FIELD);
            form_driver(text_form, REQ_PREV_FIELD);

            // Make changes
            process_query(file_content, file_content->user_name, CURRENT_LINE_INDEX, CURRENT_ACTION, trim_whitespaces(field_buffer(fields[0], 0)));

            // Send line message to server
            
            char *query = query_constructor(file_content->user_name, CURRENT_LINE_INDEX, CURRENT_ACTION, trim_whitespaces(field_buffer(fields[0], 0)));
            if (send_message(file_content->server_fd, query) == -1)
            {
                perror("Failed to send message to the server");
                exit(EXIT_FAILURE);
            }
            free(query);

            // Clearing form
            for (int position = 0; position <= num_chars - 1; position++)
            {
                form_driver(text_form, REQ_NEXT_CHAR);
            }
            while (num_chars > 0)
            {
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
            if (num_chars >= MAX_CHARS)
                continue;

            form_driver(text_form, REQ_DEL_PREV);
            break;

        case KEY_DC:
            form_driver(text_form, REQ_DEL_CHAR);
            break;

        default:
            num_chars++;
            if (num_chars >= MAX_CHARS)
                continue;

            form_driver(text_form, ch);
            break;
        }

        wrefresh(TEXT_FORM_BOX);
    }

    return FALSE;
}

/*******************************************************************************
 * line_selection_driver handles all line selection and action key input.
 *      It sets the current_action to the action specified by the user.
 *      Also, it sends line delete message in the event of line deletion.
 *      Takes file_content, the inputed character, and the total number of lines in file.   
 *
 * @param
 *    - file_content_t *file_content,
 *    - int ch,
 *    - int max_line
 * @return bool
 ******************************************************************************/
bool line_selection_driver(file_content_t *file_content, int ch, int max_line)
{
    switch (ch)
    {
    // Action keys
    case '\n':
        // Modify line
        CURRENT_ACTION = ACTION_MODIFY;
        return TRUE;
    case 'd':
        // Delete line
        CURRENT_ACTION = ACTION_DELETE;

        // Delete from local file content
        if (max_line > 0 && request_access(file_content)) {
            process_query(file_content, file_content->user_name, CURRENT_LINE_INDEX, CURRENT_ACTION, trim_whitespaces(field_buffer(fields[0], 0)));
        
            // Send line message to server
            char *query = query_constructor(file_content->user_name, CURRENT_LINE_INDEX, CURRENT_ACTION, " ");
            send_message(file_content->server_fd, query);
            free(query);

            if (CURRENT_LINE_INDEX == max_line)
                CURRENT_LINE_INDEX--;
        }

        return FALSE;
    case 'n':
        // Append line
        CURRENT_ACTION = ACTION_APPEND;
        // Update current line index
        CURRENT_LINE_INDEX = max_line + 1;
        return TRUE;
    case 'i':
        // Insert line
        CURRENT_ACTION = ACTION_INSERT;
        return TRUE;
    case 'r':
        // Refresh
        break;
    // Arrow keys
    case KEY_UP:
        CURRENT_LINE_INDEX = (CURRENT_LINE_INDEX == 0) ? CURRENT_LINE_INDEX : CURRENT_LINE_INDEX - 1;
        break;
    case KEY_DOWN:
        CURRENT_LINE_INDEX = (CURRENT_LINE_INDEX == max_line) ? CURRENT_LINE_INDEX : CURRENT_LINE_INDEX + 1;
        break;
    }

    return FALSE;
}

/*******************************************************************************
 * display_driver handles all the high-level logic of the display. 
 *      It is a quasi-scheduler that prints the file content to the screen 
 *      as long as the user hasn't inputed a character. Once it receives an input it passes it off to line_selection_driver.
 *      It also hands off control to the text_box_driver if it receives an action key.
 *      Returns true if user hasn't exited the program, else false.
 *
 * @param
 *    - file_content_t *file_content
 * @return bool
 ******************************************************************************/
bool display_driver(file_content_t *file_content)
{
    int ch;
    int min_line = 0;
    int display_max_y = ((int)LINES * 0.6) - 2;

    while (TRUE)
    {

        while ((ch = getch()) == ERR)
        {
            // Print text to display
            if (CURRENT_LINE_INDEX >= min_line + display_max_y)
            {
                min_line++;
            }
            else if (CURRENT_LINE_INDEX < min_line)
            {
                min_line--;
            }

            print_text(min_line, display_max_y, file_content);
        }

        // Listen for user input (line selection)
        bool action_input = line_selection_driver(file_content, ch, file_content->total_line_size - 1);

        // Action display
        if (ch == KEY_UP || ch == KEY_DOWN)
            CURRENT_ACTION = ACTION_MODIFY;
        modify_action_display(CURRENT_ACTION, CURRENT_LINE_INDEX);
        
        // Refresh display
        wrefresh(DISPLAY);
        

        // Handle display input exit
        if (action_input && ch == '\n')
        { // Selected a line to modify
            // Set CURRENT_LINE for display
            CURRENT_LINE = file_content->file_content_head[CURRENT_LINE_INDEX]->text;
            return TRUE;
        }
        else if (action_input && (ch == 'i' || ch == 'n'))
        { // Appending new line to file
            CURRENT_LINE = "";
            return TRUE;
        }
        else if (action_input && ch == KEY_F(1))
        { // Exiting program
            return FALSE;
        }
    };
}

/*******************************************************************************
 * ui_driver calls the display and text box driver as long as the program hasn't been exited.
 *
 * @param
 *    - file_content_t *file_content
 * @return void
 ******************************************************************************/
void ui_driver(file_content_t *file_content)
{
    while (TRUE)
    {
        if (!display_driver(file_content))
            return;
        if (!text_box_driver(file_content))
            return;
    }
}

// CLEANING FUNCTIONS

/*******************************************************************************
 * free_ui frees all major windows and components of the ui. Cleans up UI before exiting.
 *
 * @return void
 ******************************************************************************/
void free_ui()
{
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

/*******************************************************************************
 * ui_thread_handler is the main point of entry to the UI. Initializes all UI elements,
 *      starts drivers, and cleans up upon clean exit. Takes file_content.
 *
 * @param
 *    - void *args
 * @return void
 ******************************************************************************/
void *ui_thread_handler(void *args)
{
    file_content_t *file_content = (file_content_t *)args;

    // Setup of UI elements
    screen_setup();

    display_setup();
    text_box_setup();
    misc_setup();

    // Titles
    mvprintw(1, 2, "Solar!");
    mvprintw(1, COLS * 0.82, "Your mother's favorite text editor!");

    // Driver that controls user interaction
    ui_driver(file_content);

    // Freeing elements of UI
    free_ui();

    return NULL;
}

// int main()
// {
//     pthread_t ui_thread;
//     char *file_name = "Archive/f1.txt";
//     FILE *fptr = open_file_read_mode(file_name);
//     file_content_t *file_content = init_file_content_with_file(file_name, 0, "student testing", fptr);

//     if (pthread_create(&ui_thread, NULL, ui_thread_handler, (void *)file_content))
//     {
//         perror("Couldn't create display thread:");
//         exit(2);
//     }

//     if (pthread_join(ui_thread, NULL))
//     {
//         perror("Couldn't join display thread");
//         exit(2);
//     }
// }