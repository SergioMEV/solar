#include <stdio.h>
#include <stdlib.h>
#include "file.h"

line_t *init_line()
{
  line_t *new_line = (line_t *)malloc(sizeof(line_t));
  new_line->text = NULL;
  return new_line;
}

file_content_t *init_file_content()
{
  file_content_t *file_content = (file_content_t *)malloc(sizeof(file_content_t));
  file_content->file_content_head = NULL;
  file_content->total_line_size = 0;
  return file_content;
}

void add_line(file_content_t *file_content, line_t *new_line)
{
  file_content->file_content_head = (line_t **)realloc((void *)file_content->file_content_head,
                                                       (file_content->total_line_size + 1) * sizeof(line_t *));
  file_content->file_content_head[file_content->total_line_size++] = new_line;
}

void destroy_file_content(file_content_t *file_content)
{
  for (size_t line_index = 0; line_index < file_content->total_line_size; line_index++)
  {
    free(file_content->file_content_head[line_index]->text);
    free(file_content->file_content_head[line_index]);
  }
  free(file_content->file_content_head);
  free(file_content);
}

void print_file_content(file_content_t *file_content)
{
  for (size_t line_index = 0; line_index < file_content->total_line_size; line_index++)
  {
    printf("Line %zu: <%s>\n", line_index, file_content->file_content_head[line_index]->text);
  }
}

FILE *open_file_append_mode(char *file_name)
{
  FILE *fptr;

  file_content_t *file_content = init_file_content();

  fptr = fopen(file_name, "r+");
  if (fptr == NULL)
  {
    printf("Unable to open the file.");
    exit(1);
  }
  return fptr;
}

file_content_t *read_file_to_file_content(FILE *fptr)
{
  char *get_line_buffer = NULL;
  size_t buffer_size = 0;
  size_t read_size;

  file_content_t *file_content = init_file_content();
  while ((read_size = getline(&get_line_buffer, &buffer_size, fptr)) != -1)
  {
    line_t *new_line = init_line();
    get_line_buffer[read_size - 1] = '\0';
    new_line->text = get_line_buffer;
    get_line_buffer = NULL;
    add_line(file_content, new_line);
  }
  // when we break out of the loop, we need to free the space allocated for the EOF
  // to avoid memory leaks!
  free(get_line_buffer);
  return file_content;
}

void clean_file_system(FILE *fptr, file_content_t *file_content)
{
  destroy_file_content(file_content);
  fclose(fptr);
}

int main()
{
  char *file_name = "Archive/f1.txt";
  FILE *fptr = open_file_append_mode(file_name);
  file_content_t *file_content = read_file_to_file_content(fptr);
  print_file_content(file_content);
  clean_file_system(fptr, file_content);

  return 0;
}