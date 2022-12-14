#include "query_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Concatinate all inputs to one string variable.
char *query_constructor(char *user_name, char *line_num, char *action, char *message)
{
  // "{line_num}\n{action}\n{user_name}\n{message}"
  // TODO explain why +4: we need three spaces for QUERY_SEPERATOR and one space for the null terminator at the end.
  char *combined_message = malloc(sizeof(char) * (strlen(user_name) + strlen(line_num) + strlen(action) + strlen(message) + 4));
  strcpy(combined_message, line_num);
  strcat(combined_message, QUERY_SEPERATOR);
  strcat(combined_message, action);
  strcat(combined_message, QUERY_SEPERATOR);
  strcat(combined_message, user_name);
  strcat(combined_message, QUERY_SEPERATOR);
  // Warning: when action is delete, for consistency reason, lets just keep the message with a specified message.
  strcat(combined_message, message);

  return combined_message;
}
