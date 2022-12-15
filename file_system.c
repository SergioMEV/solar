#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "file_system.h"
#include "query_util.h"

// WARNING to avoide complications, we should force a new line at the end of the file.

/*******************************************************************************
 * init_line_empty function allocates memory for line_t struct. It also assigns
 *    the text field to be NULL.
 *
 * @param void
 * @return line_t*
 ******************************************************************************/
line_t *init_line_empty()
{
  line_t *new_line = (line_t *)malloc(sizeof(line_t));
  new_line->text = NULL;
  new_line->lock = UNLOCKED;
  new_line->owner = (char *)malloc(sizeof(char) * (MAX_USERNAME_LENGTH + 1));
  return new_line;
}

/*******************************************************************************
 * init_line_with_text function allocates memory for line_t struct. It also assigns
 *    It also allocate memory for text field in the line_t.
 *
 * @param char *line_text
 * @return line_t*
 ******************************************************************************/
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

/*******************************************************************************
 * init_file_content_empty function allocates memory for file_content_t struct.
 *      it also assigns NULL to the field file_content_head and 0 to the
 *      field total_line_size.
 *
 * @param
 *    - char *file_name,
 *    - int server_fd
 * @return file_content_t*
 ******************************************************************************/
file_content_t *init_file_content_empty(char *file_name, int server_fd, char *user_name)
{
  file_content_t *file_content = (file_content_t *)malloc(sizeof(file_content_t));
  file_content->file_name = malloc(sizeof(char) * (strlen(file_name) + 1));
  strcpy(file_content->file_name, file_name);
  file_content->user_name = malloc(sizeof(char) * (strlen(user_name) + 1));
  strcpy(file_content->user_name, user_name);
  file_content->server_fd = server_fd;
  file_content->file_content_head = NULL;
  file_content->total_line_size = 0;
  file_content->is_blocked = REQUEST_PENDING;
  file_content->log_head = NULL;
  return file_content;
}

/*******************************************************************************
 * init_file_content_with_text function allocates memory for file_content_t struct.
 *      It also allocate memory for each line_t and text fields.
 *
 * @param
 *    - char *file_name,
 *    - int server_fd,
 *    - char *file_text
 * @return file_content_t*
 ******************************************************************************/
file_content_t *init_file_content_with_text(char *file_name, int server_fd, char *user_name, char *file_text)
{
  file_content_t *file_content = init_file_content_empty(file_name, server_fd, user_name);
  char *line_text;
  char *line_sep_ptr = file_text;
  line_t *new_line_struct;
  while ((line_text = strsep(&line_sep_ptr, "\n")) != NULL)
  {
    new_line_struct = init_line_with_text(line_text);
    add_line(file_content, new_line_struct, -1);
  }
  return file_content;
}

/*******************************************************************************
 * init_file_content_with_file function allocates memory for file_content_t struct.
 *      It also allocate memory for each line_t and text fields.
 *
 * @param
 *    - char *file_name,
 *    - int server_fd,
 *    - FILE *fptr
 * @return file_content_t*
 ******************************************************************************/
file_content_t *init_file_content_with_file(char *file_name, int server_fd, char *user_name, FILE *fptr)
{
  char *get_line_buffer = NULL;
  size_t buffer_size = 0;
  size_t read_size;

  file_content_t *file_content = init_file_content_empty(file_name, server_fd, user_name);
  while ((read_size = getline(&get_line_buffer, &buffer_size, fptr)) != -1)
  {

    get_line_buffer[read_size - 1] = '\0';
    line_t *new_line = init_line_with_text(get_line_buffer);
    get_line_buffer = NULL;
    add_line(file_content, new_line, -1);
  }
  // when we break out of the loop, we need to free the space allocated for the EOF
  // to avoid memory leaks!
  free(get_line_buffer);
  return file_content;
}

/*******************************************************************************
 * add_line function add a new line_t into the file_content at line index specified by
 *    insert_line_index. If users would like to append the new line, set insert_line_index
 *    to be -1.
 *
 * @param
 *    - file_content_t *file_content,
 *    - line_t *new_line,
 *    - size_t insert_line_inde (-1 if appending the new line)
 * @return void (modifying file_content in place)
 ******************************************************************************/
void add_line(file_content_t *file_content, line_t *new_line, size_t insert_line_index)
{
  file_content->file_content_head = (line_t **)realloc((void *)file_content->file_content_head,
                                                       (file_content->total_line_size + 1) * sizeof(line_t *));
  if (insert_line_index == -1)
    insert_line_index = file_content->total_line_size;
  memmove(file_content->file_content_head + insert_line_index + 1,
          file_content->file_content_head + insert_line_index,
          (file_content->total_line_size - insert_line_index) * sizeof(line_t *));
  file_content->total_line_size++;
  file_content->file_content_head[insert_line_index] = new_line;
}

/*******************************************************************************
 * remove_line function removes the line specified by remove_line_index from
 *    the file_content. It also free the memory allocated to the removed line.
 *
 * @param
 *    - file_content_t *file_content,
 *    - size_t remove_line_index
 * @return int
 *    - 0 if removing success, else removing failed
 ******************************************************************************/
int remove_line(file_content_t *file_content, size_t remove_line_index)
{
  if (remove_line_index >= file_content->total_line_size)
  {
    perror("Unable to remove the line because line index overflow");
    return -1;
  }
  free(file_content->file_content_head[remove_line_index]->text);
  free(file_content->file_content_head[remove_line_index]->owner);
  free(file_content->file_content_head[remove_line_index]);
  memmove(file_content->file_content_head + remove_line_index,
          file_content->file_content_head + remove_line_index + 1,
          (file_content->total_line_size - remove_line_index - 1) * sizeof(line_t *));
  file_content->total_line_size--;
  file_content->file_content_head = (line_t **)realloc((void *)file_content->file_content_head,
                                                       (file_content->total_line_size) * sizeof(line_t *));
  return 0;
}

/*******************************************************************************
 * modify_line changes the field text from the line specified by modify_line_index
 *    to a new text. modified_line_text should be a string with allocated memory.
 *
 * @param
 *    - file_content_t *file_content, size_t modify_line_index,
 *    - char *modified_line_text (created by malloc)
 * @return void (modifying file_content in place)
 ******************************************************************************/
void modify_line(file_content_t *file_content, size_t modify_line_index, char *modified_line_text)
{
  free(file_content->file_content_head[modify_line_index]->text);
  file_content->file_content_head[modify_line_index]->text = malloc(sizeof(char) * (strlen(modified_line_text) + 1));
  strcpy(file_content->file_content_head[modify_line_index]->text, modified_line_text);
}

/*******************************************************************************
 * destroy_file_content free all the memory allocated to file_content. Including
 *    the array to store line_t, every line_t structs, and text fields in line_t.
 *
 * @param file_content_t *file_content
 * @return void
 ******************************************************************************/
void destroy_file_content(file_content_t *file_content)
{
  for (size_t line_index = 0; line_index < file_content->total_line_size; line_index++)
  {
    free(file_content->file_content_head[line_index]->text);
    free(file_content->file_content_head[line_index]->owner);
    free(file_content->file_content_head[line_index]);
  }
  
  while(file_content->log_head != NULL) {
    log_entry_t *temp = file_content->log_head;
    file_content->log_head = file_content->log_head->next;
    
    free(temp->user_name); 
    free(temp->new_line); 
    free(temp); 
  }

  free(file_content->file_content_head);
  free(file_content);
}

/*******************************************************************************
 * print_file_content print the content of file_content in the following format.
 *
 *    "Line %zu: <%s>", line_number, text
 *
 * @param file_content_t *file_content
 * @return void
 ******************************************************************************/
void print_file_content(file_content_t *file_content)
{
  printf("%s", "=====================================================\n");
  printf("File content for <%s>\n", file_content->file_name);
  for (size_t line_index = 0; line_index < file_content->total_line_size; line_index++)
  {
    printf("Line %zu: <%s>\n", line_index, file_content->file_content_head[line_index]->text);
  }
}

/*******************************************************************************
 * to_string will create a string with file_content by separating each line with
 *    a new line character. The memory for the string is allocated by malloc.
 *
 * @warning Make sure to free the returned string since it is allocated using malloc.
 *
 * @param file_content_t *file_content
 * @return char*
 ******************************************************************************/
char *file_content_to_string(file_content_t *file_content)
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

/*******************************************************************************
 * fopen() with error check.
 *
 * @param char *file_name
 * @return FILE *
 ******************************************************************************/
FILE *open_file_read_mode(char *file_name)
{
  FILE *fptr;

  fptr = fopen(file_name, "r");
  if (fptr == NULL)
  {
    printf("Unable to open the file.");
    exit(1);
  }
  return fptr;
}

/*******************************************************************************
 * export_file_content will export the file_content to a file specified by file_name
 *    set file_name to NULL if overwritten is wanted.
 *
 * @param
 *    - char *file_name, (NULL if overwritting)
 *    - file_content_t *file_content
 * @return void
 ******************************************************************************/
void export_file_content(char *file_name, file_content_t *file_content)
{
  if (file_name == NULL)
    file_name = file_content->file_name;
  FILE *fptr_dest;

  fptr_dest = fopen(file_name, "w");
  if (fptr_dest == NULL)
  {
    printf("Unable to open the file.");
    exit(1);
  }
  char *export_text = file_content_to_string(file_content);
  int export_length = strlen(export_text);
  if ((export_text = realloc(export_text, sizeof(char) * (export_length + 3))) == NULL)
  {
    perror("Allocating memory for export text failed");
    exit(EXIT_FAILURE);
  }
  export_text[export_length + 1] = '\n';
  export_text[export_length + 2] = '\0';
  fprintf(fptr_dest, "%s", export_text);
  free(export_text);
  fclose(fptr_dest);
}
// TODO parse query in the client!
// "{user_name}\n{line_index}\n{action}\n{message}"
void process_query(file_content_t *file_content, char *user_name,
                   int line_index, char action, char *modified_line_text)
{
  line_t *new_line;
  switch (action)
  {
  case ACTION_DELETE:
    // Delete line
    remove_line(file_content, line_index);
    return;
  case ACTION_MODIFY:
    // Modify line
    modify_line(file_content, line_index, modified_line_text);
    return;
  default:
    // insert or apped line
    new_line = init_line_with_text(modified_line_text);
    add_line(file_content, new_line, line_index);
    return;
  }
}

void clean_file_system(FILE *fptr, file_content_t *file_content)
{
  destroy_file_content(file_content);
  fclose(fptr);
}

// int main()
// {
//   char *file_name = "Archive/f1.txt";
//   FILE *fptr = open_file_read_mode(file_name);
//   file_content_t *file_content = init_file_content_with_file(file_name, 0, "student", fptr);
//   print_file_content(file_content);

//   // print_file_content(file_content);
//   char *text = malloc(sizeof(char) * 100);
//   strcpy(text, "inserting line here");
//   process_query(file_content, "student", -1, ACTION_INSERT, text);
//   print_file_content(file_content);
//   // export_file_content("Archive/f2.txt", file_content);
//   free(text);
//   clean_file_system(fptr, file_content);

//   return 0;
// }