#include <stdio.h>
#include <stdlib.h>
#include "file.h"

// !!!TODO fix double free

file_content_t *file_content;

line_t *init_line()
{
  line_t *new_line = (line_t *)malloc(sizeof(line_t));
  new_line->text = NULL;
  return new_line;
}

file_content_t *init_file_content()
{
  //file_content_t *file_content = (file_content_t *)malloc(sizeof(file_content_t));
  file_content = (file_content_t *)malloc(sizeof(file_content_t));
  file_content->file_content_head = (line_t **)malloc(sizeof(line_t *) * FILE_CONTENT_INIT_SIZE);
  file_content->total_line_size = 0;
  file_content->line_capacity = FILE_CONTENT_INIT_SIZE;
  return file_content;
}

void add_line(file_content_t *file_content, line_t *new_line)
{
  if (file_content->line_capacity == file_content->total_line_size)
  {
    file_content->file_content_head = (line_t **)realloc((void *)file_content->file_content_head,
                                                         2 * (file_content->line_capacity) * sizeof(line_t *));
    file_content->line_capacity *= 2;
  }
  file_content->file_content_head[file_content->total_line_size++] = new_line;
}

// !!!Warn: when we are not calling this destroy function, there are no memory leak. i expect memory leaks
void destroy_file_contnt(file_content_t *file_content)
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

int main()
{
  FILE *fptr;
  char *get_line_buffer = NULL;
  size_t buffer_size = 0;
  size_t read_size;

  // line_t *line2 = (line_t *)malloc(sizeof(line_t));
  // line2->text = "line2 adfs";
  // line_t *line3 = (line_t *)malloc(sizeof(line_t));
  // line3->text = "line3 fdfsdfsa";
  file_content_t *file_content = init_file_content();

  fptr = fopen("Archive/f1.txt", "r+");
  if (fptr == NULL)
  {
    printf("Unable to open the file.");
    exit(1);
  }
  while ((read_size = getline(&get_line_buffer, &buffer_size, fptr)) != -1)
  {
    line_t *new_line = init_line();
    get_line_buffer[read_size - 1] = '\0';
    new_line->text = get_line_buffer;
    get_line_buffer = NULL;
    add_line(file_content, new_line);
  }

  fprintf(fptr, "\nhahaha, new stuff");
  print_file_content(file_content);
  // destroy_file_contnt(file_content);
  fclose(fptr);

  return 0;
}