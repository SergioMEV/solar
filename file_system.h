
typedef struct line_struct line_t;
typedef struct file_content_struct file_content_t;

line_t *init_line();
file_content_t *init_file_content();
void add_line(file_content_t *file_content, line_t *new_line);
void destroy_file_contnt(file_content_t *file_content);
void print_file_content(file_content_t *file_content);

FILE *open_file_append_mode(char *file_name);
file_content_t *read_file(FILE *fptr);
void clean_file_system(FILE *fptr, file_content_t *file_content);

extern file_content_t *file_content;
