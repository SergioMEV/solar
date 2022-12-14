#pragma once

typedef struct line_struct
{
  char *text;
} line_t;

typedef struct file_content_struct
{
  char *file_name;
  int server_fd;
  char *user_name;
  line_t **file_content_head;
  size_t total_line_size;
} file_content_t;