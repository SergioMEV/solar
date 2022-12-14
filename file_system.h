#pragma once
#include "file.h"

line_t *init_line_empty();
line_t *init_line_with_text(char *line_text);

file_content_t *init_file_content_empty(char *file_name, int server_fd, char *user_name);
file_content_t *init_file_content_with_text(char *file_name, int server_fd, char *user_name, char *file_text);
file_content_t *init_file_content_with_file(char *file_name, int server_fd, char *user_name, FILE *fptr);

void add_line(file_content_t *file_content, line_t *new_line, size_t line_number);
int remove_line(file_content_t *file_content, size_t remove_line_index);
void modify_line(file_content_t *file_content, size_t modify_line_index, char *modified_line_text);
void print_file_content(file_content_t *file_content);

char *file_content_to_string(file_content_t *file_content);
FILE *open_file_read_mode(char *file_name);

void process_query(file_content_t *file_content, char *user_name,
                   int line_index, char action, char *modified_line_text);

void destroy_file_contnt(file_content_t *file_content);
void clean_file_system(FILE *fptr, file_content_t *file_content);
void export_file_content(char *file_name, file_content_t *file_content);
void process_query(file_content_t *file_content, char *user_name, int line_index, char action, char *modified_line_text);