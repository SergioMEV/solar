#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user_info_utils.h"

user_info_t *user_infos_init()
{
    user_info_t *user_info = malloc(sizeof(user_info_t));
    user_info->fd = 0;
    user_info->user_name = NULL;
    return user_info;
}

user_infos_array_t *user_infos_array_init()
{
    user_infos_array_t *user_infos_struct = (user_infos_array_t *)malloc(sizeof(user_infos_array_t));
    user_infos_struct->size = 0;
    user_infos_struct->user_infos = NULL;
    return user_infos_struct;
}

void user_infos_add_user(user_infos_array_t *user_infos_struct, user_info_t *user_insert)
{
    user_infos_struct->size++;
    user_infos_struct->user_infos = realloc(user_infos_struct->user_infos, user_infos_struct->size * sizeof(user_info_t *));
    user_infos_struct->user_infos[user_infos_struct->size - 1] = user_insert;
}

/* Remove an user from the array */
// TODO: what should we do if user_remove is not in the array?
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

void user_info_destroy(user_info_t *user_info)
{
    free(user_info->user_name);
    free(user_info);
}

void user_infos_array_destroy(user_infos_array_t *user_infos_struct)
{
    for (size_t index = 0; index < user_infos_struct->size; index++)
    {
        free(user_infos_struct->user_infos[index]->user_name);
        free(user_infos_struct->user_infos[index]);
    }
    free(user_infos_struct);
}

// int main()
// {
//     user_info_t *u1 = malloc(sizeof(user_info_t));
//     u1->user_name = "u1";
//     u1->fd = 1;
//     user_info_t *u2 = malloc(sizeof(user_info_t));
//     u2->user_name = "u2";
//     u2->fd = 1;
//     user_info_t *u3 = malloc(sizeof(user_info_t));
//     u3->user_name = "u3";
//     u3->fd = 1;

//     user_infos_array_t *user_infos_struct = user_infos_init();
//     user_infos_add_element(user_infos_struct, u1);
//     user_infos_add_element(user_infos_struct, u2);
//     user_infos_add_element(user_infos_struct, u3);
//     printf("%zu\n", user_infos_struct->size);
//     printf("%zu\n", user_infos_struct->size);
//     user_infos_destroy(user_infos_struct);
//     return 0;
// }