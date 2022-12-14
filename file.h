#define FILE_CONTENT_INIT_SIZE 4

typedef struct line_struct
{
  char *text;
} line_t;

typedef struct file_content_struct
{
  char *file_name;
  line_t **file_content_head;
  size_t total_line_size;
} file_content_t;