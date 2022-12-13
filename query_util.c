#include "constants.h"
#include "query_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Concatinate all inputs to one string variable.
char* string_concatenate(char* user_name, char* line_num, char action, char* message) {
  char combined_message[strlen(user_name) + strlen(line_num) + strlen(action) + strlen(message) + 5];
  strcpy(combined_message, line_num);
  strcat(combined_message, SEPERATOR);
  strcat(combined_message, action);
  strcat(combined_message, SEPERATOR);
  strcat(combined_message, user_name);
  strcat(combined_message, SEPERATOR);
  // Don't include the message if the action is to delete.
  if (action != ACTION_DELETE) {
    strcat(combined_message, message);
    strcat(combined_message, SEPERATOR);
  }

  return combined_message;
}
