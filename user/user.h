#ifndef USER_H
#define USER_H

typedef struct Users
{
    char username[100];
    int online;
    struct Users *next;
} Users;

void add_user(char username[]);
int check_status(const char *msg);
void change_status(char username[], int status);
void list_users();
void update_user_id(char id[]);
void load_users_from_file();
Users *get_user_by_index(int index);

extern Users *users;

extern char user_id[];

#endif