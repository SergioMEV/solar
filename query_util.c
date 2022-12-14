#include "constants.h"
#include "query_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Concatinate all inputs to one string variable.
char *string_concatenate(char *user_name, char *line_num, char action, char *message)
{
  // TODO explain why +5
  char *combined_message = malloc(sizeof(char) * (strlen(user_name) + strlen(line_num) + strlen(action) + strlen(message) + 5));
  strcpy(combined_message, line_num);
  strcat(combined_message, SEPERATOR);
  strcat(combined_message, action);
  strcat(combined_message, SEPERATOR);
  strcat(combined_message, user_name);
  strcat(combined_message, SEPERATOR);
  // Warning: when action is delete, for consistency reason, lets just keep the message with a specified message.
  strcat(combined_message, message);

  return combined_message;
}
