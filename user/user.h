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
void listUsers();
void updateUserId(char id[]);

extern Users *users;

extern char userId[];

#endif