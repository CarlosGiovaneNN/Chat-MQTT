#ifndef USER_H
#define USER_H

#include "../headers.h"

void list_users();

void load_users_from_file();

void add_user(char username[]);

void change_status(char username[], int status);
void update_user_id(char id[]);

int check_status(char *msg);
int user_count();

Users *get_user_by_index(int index);

extern Users *users;

extern char user_id[];

#endif