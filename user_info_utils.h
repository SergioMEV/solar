typedef struct user_info
{
    char *user_name;
    int fd;
} user_info_t;

typedef struct user_infos_array
{
    size_t size;
    user_info_t **user_infos;
} user_infos_array_t;

user_info_t *user_infos_init();
user_infos_array_t *user_infos_array_init();
void user_infos_add_user(user_infos_array_t *user_infos_struct, user_info_t *user_insert);
void user_infos_remove_user(user_infos_array_t *user_infos_struct, user_info_t *user_remove_name);
int is_duplicate_user_name(user_infos_array_t *user_infos_struct, char *user_name);
void user_info_destroy(user_info_t *user_info);
void user_infos_array_destroy(user_infos_array_t *user_infos_struct);
