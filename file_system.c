#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"

// WARNING to avoide complications, we should force a new line at the end of the file.

line_t *init_line_empty()
{
  line_t *new_line = (line_t *)malloc(sizeof(line_t));
  new_line->text = NULL;
  return new_line;
}

file_content_t *init_file_content_empty()
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

char *to_string(file_content_t *file_content)
{
  if (file_content->total_line_size == 0)
    return NULL;
  char *result;
  if ((result = malloc(sizeof(char) * (strlen(file_content->file_content_head[0]->text) + 2))) == NULL) // Reserve memory for new line and null terminator.
  {
    perror("Allocating memory for result in to_string function faild");
    exit(1);
  }
  strcpy(result, file_content->file_content_head[0]->text);
  result[strlen(result) + 1] = '\0';
  result[strlen(result)] = '\n';

  for (size_t line_index = 1; line_index < file_content->total_line_size; line_index++)
  {
    result = realloc(result,
                     sizeof(char) *
                         (strlen(result) +
                          strlen(file_content->file_content_head[line_index]->text) +
                          2)); // Reserve memory for new line and null terminator.
    if (result == NULL)
    {
      perror("Reallocating memory for result in to_string function faild");
      exit(1);
    }
    strcat(result, file_content->file_content_head[line_index]->text);
    if (line_index != file_content->total_line_size - 1)
    {
      // we do not need to add a new line character for the last line.
      result[strlen(result) + 1] = '\0';
      result[strlen(result)] = '\n';
    }
  }
  return result;
}

line_t *init_line_with_text(char *line_text)
{
  line_t *new_line = init_line_empty();
  if ((new_line->text = malloc(sizeof(char) * (strlen(line_text)) + 1)) == NULL)
  {
    perror("Allocating memory for new line struct failed");
    exit(1);
  }
  strcpy(new_line->text, line_text);
  return new_line;
}

file_content_t *init_file_content_with_text(char *file_text)
{
  file_content_t *file_content = init_file_content_empty();
  char *line_text;
  char *line_sep_ptr = file_text;
  line_t *new_line_struct;
  while ((line_text = strsep(&line_sep_ptr, "\n")) != NULL)
  {
    new_line_struct = init_line_with_text(line_text);
    add_line(file_content, new_line_struct);
  }
  return file_content;
}

FILE *open_file_read_mode(char *file_name)
{
  FILE *fptr;

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

  file_content_t *file_content = init_file_content_empty();
  while ((read_size = getline(&get_line_buffer, &buffer_size, fptr)) != -1)
  {
    line_t *new_line = init_line_empty();
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
  FILE *fptr = open_file_read_mode(file_name);
  file_content_t *file_content = read_file_to_file_content(fptr);
  print_file_content(file_content);
  printf("%s", "====================\n");
  char *string = to_string(file_content);
  printf("content: <%s>\n", string);
  printf("%s", "====================\n");
  file_content_t *new_content = init_file_content_with_text(string);
  print_file_content(new_content);
  fprintf(fptr, "%s", string);

  clean_file_system(fptr, file_content);
  destroy_file_content(new_content);
  free(string);

  return 0;
}