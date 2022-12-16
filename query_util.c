#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "query_util.h"

// Concatinate all inputs to one string variable.
char *query_constructor(char *user_name, int line_index, char action, char *message)
{
  // "{line_num}~{action}~{user_name}~{message}"
  // we need three spaces for QUERY_SEPERATOR and one space for the null terminator at the end.
  char *combined_message = malloc(sizeof(char) * (strlen(user_name) + MAX_LINE_NUMBER_DIGITS + 1 + strlen(message) + 4));
  char *line_index_str = (char *)malloc(MAX_LINE_NUMBER_DIGITS * sizeof(char));
  sprintf(line_index_str, "%d", line_index);
  strcpy(combined_message, line_index_str);
  free(line_index_str);
  strcat(combined_message, QUERY_SEPERATOR);
  strncat(combined_message, &action, 1);
  strcat(combined_message, QUERY_SEPERATOR);
  strcat(combined_message, user_name);
  strcat(combined_message, QUERY_SEPERATOR);
  // Warning: when action is delete, for consistency reason, lets just keep the message with a specified message.
  strcat(combined_message, message);
  strcat(combined_message, "\0");

  return combined_message;
}
