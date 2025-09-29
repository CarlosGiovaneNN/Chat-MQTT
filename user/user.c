#include "../headers.h"

Users *users = NULL;
char user_id[100] = "";

void print_users();
void load_users_from_file();
void save_user_to_file(char *username);
void add_user(char username[]);
void change_status(char username[], int status);
void list_users();
void update_user_id(char id[]);

int user_exists_in_file(char *username);
int find_user(char username[]);
int user_count();
int check_status(char *msg);

Users *get_user_by_index(int index);

// PRINTA OS USERS
void print_users()
{
    pthread_mutex_lock(&mutex_users);

    Users *current = users;

    while (current != NULL)
    {
        printf("%s\n", current->username);
        printf("%d\n", current->online);
        current = current->next;
    }

    pthread_mutex_unlock(&mutex_users);
}

// CARREGA OS USERS DO ARQUIVO
void load_users_from_file()
{
    FILE *f = fopen(FILE_USERS, "r");
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

// SALVA O USER NO ARQUIVO
void save_user_to_file(char *username)
{
    if (user_exists_in_file(username))
        return;

    FILE *f = fopen(FILE_USERS, "a");
    if (!f)
        return;

    fprintf(f, "%s\n", username);
    fclose(f);
}

// ADICIONA O USER
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

    pthread_mutex_lock(&mutex_users);

    new_user->next = users;
    users = new_user;

    pthread_mutex_unlock(&mutex_users);

    save_user_to_file(username);
}

// MUDA DE STATUS O USER ( ONLINE / OFFLINE )
void change_status(char username[], int status)
{
    pthread_mutex_lock(&mutex_users);

    Users *current = users;

    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            current->online = status;

            pthread_mutex_unlock(&mutex_users);

            return;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex_users);
}

// LISTA OS USERS
void list_users()
{
    pthread_mutex_lock(&mutex_users);

    Users *current = users;

    printf("Lista de usuarios: \n");

    if (current == NULL)
    {
        printf("Nenhum usuario conectado\n\n");

        pthread_mutex_unlock(&mutex_users);

        return;
    }

    while (current != NULL)
    {
        printf("%s - %s\n", current->username, current->online ? "online" : "offline");
        current = current->next;
    }

    pthread_mutex_unlock(&mutex_users);

    printf("\n");
}

// ATUALIZA O USER ID DO USER LOCAL (QUE ESTA CONECTADO NO TERMINAL)
void update_user_id(char id[])
{
    strncpy(user_id, id, sizeof(user_id) - 1);
    user_id[sizeof(user_id) - 1] = '\0';
}

// TOTAL DE USERS
int user_count()
{
    pthread_mutex_lock(&mutex_users);

    Users *current = users;
    int count = 0;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }

    pthread_mutex_unlock(&mutex_users);

    return count;
}

// VIZUALIZA O STATUS DO USER ( ONLINE(1) / OFFLINE(0) / NENHUM(-1) ) - PARA A MSG STATUS PUBLISHER
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

// VERIFICA QUE O USER ESTA SALVO NO ARQUIVO
int user_exists_in_file(char *username)
{
    FILE *f = fopen(FILE_USERS, "r");
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

// VERIFICA SE O USUARIO EXISTE, SE EXISTIR RETORNA 1
int find_user(char username[])
{
    pthread_mutex_lock(&mutex_users);

    Users *current = users;

    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            pthread_mutex_unlock(&mutex_users);
            return 1;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex_users);

    return 0;
}

// RETORNA O USER POR INDEX
Users *get_user_by_index(int index)
{
    int count = 0;

    pthread_mutex_lock(&mutex_users);

    for (Users *current = users; current != NULL; current = current->next)
    {
        if (count == index)
        {
            pthread_mutex_unlock(&mutex_users);

            return current;
        }
        count++;
    }

    pthread_mutex_unlock(&mutex_users);

    return NULL;
}
