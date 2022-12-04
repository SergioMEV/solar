typedef struct line_struct
{
  char *text;
} line_t;

typedef struct file_content_struct
{
  line_t **file_content_head;
  size_t total_line_size;
  size_t line_capacity;
} file_content_t;