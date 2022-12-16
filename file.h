#pragma once

typedef struct line_struct
{
  char *text;
  int lock;
  char *owner;
} line_t;

// Logs struct
//  Linked list of log entries because I don't want to deal with dynamic arrays.
typedef struct log_entry {
    char *user_name;
    char *new_line;
    int line_index;
    char action;
    struct log_entry *next;

    // Only matters for head node.
    struct log_entry *last;
    int log_size;
} log_entry_t;

typedef struct file_content_struct
{
  char *file_name;
  int server_fd;
  char *user_name;
  line_t **file_content_head;
  size_t total_line_size;
  char* is_blocked;
  log_entry_t *log_head;
} file_content_t;
