#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user_info_utils.h"

/*******************************************************************************
 * user_infos_init function creates user_info struct and initializes its fields.
 *
 * @param void
 * @return user_info_t*
 ******************************************************************************/
user_info_t *user_infos_init()
{
  user_info_t *user_info = malloc(sizeof(user_info_t));
  user_info->fd = 0;
  user_info->user_name = NULL;
  return user_info;
}

/*******************************************************************************
 * user_infos_array_init function creates user_infos struct and initializes its fields.
 *
 * @param void
 * @return user_infos_array_t*
 ******************************************************************************/
user_infos_array_t *user_infos_array_init()
{
  user_infos_array_t *user_infos_struct = (user_infos_array_t *)malloc(sizeof(user_infos_array_t));
  user_infos_struct->size = 0;
  user_infos_struct->user_infos = NULL;
  return user_infos_struct;
}

/*******************************************************************************
 * user_infos_add_user function adds user info to the user_infos array.
 *
 * @param
 *    - user_infos_array_t *user_infos_struct,
 *    - user_info_t *user_insert
 * @return user_infos_array_t*
 ******************************************************************************/
void user_infos_add_user(user_infos_array_t *user_infos_struct, user_info_t *user_insert)
{
  user_infos_struct->size++;
  user_infos_struct->user_infos = realloc(user_infos_struct->user_infos, user_infos_struct->size * sizeof(user_info_t *));
  user_infos_struct->user_infos[user_infos_struct->size - 1] = user_insert;
}

/*******************************************************************************
 * user_infos_remove_user function removes an user from the array.
 *
 * @param
 *    - user_infos_array_t *user_infos_struct,
 *    - user_info_t *user_remove
 * @return int
 ******************************************************************************/
void user_infos_remove_user(user_infos_array_t *user_infos_struct, user_info_t *user_remove)
{
  for (size_t index = 0; index < user_infos_struct->size; index++)
  {
    if (strcmp(user_remove->user_name, user_infos_struct->user_infos[index]->user_name) == 0)
    {
      memmove(user_infos_struct->user_infos + index, user_infos_struct->user_infos + index + 1,
              (user_infos_struct->size - index - 1) * sizeof(user_info_t *));
      user_infos_struct->size--;
      user_infos_struct->user_infos = realloc(user_infos_struct->user_infos,
                                              user_infos_struct->size * sizeof(user_info_t *));
    }
  }
}

/*******************************************************************************
 * is_duplicate_user_name function checks if there is a duplicate username in the array.
 *
 * @param
 *    - user_infos_array_t *user_infos_struct,
 *    - char *user_name
 * @return int
 ******************************************************************************/
int is_duplicate_user_name(user_infos_array_t *user_infos_struct, char *user_name)
{
  for (size_t index = 0; index < user_infos_struct->size; index++)
  {
    if (strcmp(user_name, user_infos_struct->user_infos[index]->user_name) == 0)
    {
      return 1;
    }
  }
  return 0;
}

/*******************************************************************************
 * user_info_destroy frees user info.
 *
 * @param user_info_t *user_info
 * @return void
 ******************************************************************************/
void user_info_destroy(user_info_t *user_info)
{
  free(user_info->user_name);
  free(user_info);
}

/*******************************************************************************
 * user_infos_array_destroy function destroys the array and frees the relevant contents.
 *
 * @param user_infos_array_t *user_infos_struct
 * @return void
 ******************************************************************************/
void user_infos_array_destroy(user_infos_array_t *user_infos_struct)
{
  for (size_t index = 0; index < user_infos_struct->size; index++)
  {
    free(user_infos_struct->user_infos[index]->user_name);
    free(user_infos_struct->user_infos[index]);
  }
  free(user_infos_struct);
}
