#include "user.h"
#include "../headers.h"

Users *users = NULL;
char user_id[] = "";
char file_users[] = "user/users.txt";

Users *get_user_by_index(int index)
{
    int count = 0;
    for (Users *current = users; current != NULL; current = current->next)
    {
        if (count == index)
        {
            return current;
        }
        count++;
    }
    return NULL;
}

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

void load_users_from_file()
{
    FILE *f = fopen(file_users, "r");
    if (!f)
        return;

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) > 0)
        {
            if (!find_user(line))
                add_user(line);
        }
    }

    fclose(f);
}

void save_user_to_file(char *username)
{
    FILE *f = fopen(file_users, "a");
    if (!f)
        return;

    fprintf(f, "%s\n", username);
    fclose(f);
}

void add_user(char username[])
{
    int status = 0;

    if (strstr(username, "is connected"))
        status = 1;

    char name[256];
    strcpy(name, username);

    remove_substring(name, " is connected");
    remove_substring(name, " is disconnected");

    if (strcmp(name, user_id) == 0)
        return;

    if (find_user(name))
        return;

    Users *new_user = malloc(sizeof(Users));
    if (!new_user)
        return;

    strcpy(new_user->username, name);
    new_user->online = status;
    new_user->next = users;
    users = new_user;

    save_user_to_file(name);
}

void change_status(char username[], int status)
{
    char name[256];
    strcpy(name, username);

    remove_substring(name, " is connected");
    remove_substring(name, " is disconnected");

    Users *current = users;

    while (current != NULL)
    {
        if (strcmp(current->username, name) == 0)
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

void list_users()
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

void update_user_id(char id[])
{
    strcpy(user_id, id);
}