// #include "user.h"
#include "../headers.h"

Users *users = NULL;
char user_id[100] = "";
char file_users[] = "user/users.txt";

Users *get_user_by_index(int index);
void remove_substring(char *str, const char *sub);
int find_user(char username[]);
void print_users();
void load_users_from_file();
int user_exists_in_file(char *username);
void save_user_to_file(char *username);
void add_user(char username[]);
void change_status(char username[], int status);
void list_users();
void update_user_id(char id[]);

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

int user_exists_in_file(char *username)
{
    FILE *f = fopen(file_users, "r");
    if (!f)
        return 0;

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, username) == 0)
        {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

void save_user_to_file(char *username)
{
    if (user_exists_in_file(username))
        return;

    FILE *f = fopen(file_users, "a");
    if (!f)
        return;

    fprintf(f, "%s\n", username);
    fclose(f);
}

void add_user(char username[])
{
    int status = 0;

    if (strcmp(username, user_id) == 0)
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

    save_user_to_file(username);
}

void change_status(char username[], int status)
{
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

int check_status(char *msg)
{
    if (strstr(msg, "disconnected") != NULL)
    {
        return 0;
    }
    else if (strstr(msg, "connected") != NULL)
    {
        return 1;
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
    strncpy(user_id, id, sizeof(user_id) - 1);
    user_id[sizeof(user_id) - 1] = '\0';
}

int user_count()
{
    Users *current = users;
    int count = 0;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }
    return count;
}