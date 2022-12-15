#pragma once

typedef struct line_struct
{
  char *text;
  int lock;
  char* owner;
} line_t;

typedef struct file_content_struct
{
  char *file_name;
  int server_fd;
  char *user_name;
  line_t **file_content_head;
  size_t total_line_size;
  char* is_blocked;
} file_content_t;
