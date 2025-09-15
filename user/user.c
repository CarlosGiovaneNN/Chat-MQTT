#include "user.h"
#include "../headers.h"

Users *users = NULL;
char userId[] = "";

void remove_substring(char *str, const char *sub)
{
    char *pos, *temp;
    size_t len = strlen(sub);

    while ((pos = strstr(str, sub)) != NULL)
    {
        temp = pos + len;
        memmove(pos, temp, strlen(temp) + 1);
    }
}

int find_user(char username[])
{
    Users *current = users;

    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            return 1;
        }
        current = current->next;
    }

    return 0;
}

void print_users()
{
    Users *current = users;

    while (current != NULL)
    {
        printf("%s\n", current->username);
        printf("%d\n", current->online);
        current = current->next;
    }
}

void add_user(char username[])
{
    int status = 0;

    if (strstr(username, "is connected"))
        status = 1;

    remove_substring(username, " is connected");
    remove_substring(username, " is disconnected");

    if (strcmp(username, userId) == 0)
        return;

    if (find_user(username))
        return;

    Users *new_user = malloc(sizeof(Users));
    if (!new_user)
        return;

    strcpy(new_user->username, username);
    new_user->online = status;
    new_user->next = users;
    users = new_user;
}

void change_status(char username[], int status)
{

    remove_substring(username, " is connected");
    remove_substring(username, " is disonnected");

    Users *current = users;

    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            current->online = status;
            return;
        }
        current = current->next;
    }
}

int check_status(const char *msg)
{
    if (strstr(msg, "is connected") != NULL)
    {
        return 1;
    }
    else if (strstr(msg, "is disconnected") != NULL)
    {
        return 0;
    }
    return -1; // nenhum dos dois
}

void listUsers()
{
    Users *current = users;

    printf("Lista de usuarios: \n");

    if (current == NULL)
    {
        printf("Nenhum usuario conectado\n\n");
        return;
    }

    while (current != NULL)
    {
        printf("%s - %s\n", current->username, current->online ? "online" : "offline");
        current = current->next;
    }

    printf("\n");
}

void updateUserId(char id[])
{
    strcpy(userId, id);
}