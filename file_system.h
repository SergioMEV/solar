
typedef struct line_struct line_t;
typedef struct file_content_struct file_content_t;

line_t *init_line_empty();
file_content_t *init_file_content_empty();
void add_line(file_content_t *file_content, line_t *new_line, size_t line_number);
void destroy_file_contnt(file_content_t *file_content);
void print_file_content(file_content_t *file_content);
line_t *init_line_with_text(char *line_text);
file_content_t *init_file_content_with_text(char *file_text);
char *to_string(file_content_t *file_content);
FILE *open_file_read_mode(char *file_name);
file_content_t *read_file(FILE *fptr);
void clean_file_system(FILE *fptr, file_content_t *file_content);

extern file_content_t *file_content;
