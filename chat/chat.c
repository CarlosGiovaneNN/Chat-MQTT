#include "../headers.h"

#include "../group/group.h"
#include "../message/message.h"
#include "../user/user.h"

Chat *chats = NULL;

void add_chat_by_message(char *message, char *from);
void load_chats_by_groups();
void load_chats_from_file();
void show_chat_menu();

char *create_chat(char *name, int is_group);

Chat *find_chat(char *name, int is_group);
Chat *find_chat_by_to_and_type(const char *to, int is_group);

// DONE
void add_chat_by_message(char *message, char *from)
{
    Chat *chat = malloc(sizeof(Chat));
    if (!chat)
        return;

    strncpy(chat->topic, message, sizeof(chat->topic) - 1);
    chat->topic[sizeof(chat->topic) - 1] = '\0';

    chat->is_group = 0;
    chat->participants = NULL;

    pthread_mutex_lock(&mutex_chats);

    chat->next = chats;
    chats = chat;

    pthread_mutex_unlock(&mutex_chats);
}

// DONE
void load_chats_by_groups()
{
    pthread_mutex_lock(&mutex_groups);
    pthread_mutex_lock(&mutex_chats);

    Group *group = groups;
    while (group != NULL)
    {
        int is_member = 0;

        if (strcmp(group->leader, user_id) == 0)
        {
            is_member = 1;
        }
        else
        {
            Participant *p = group->participants;
            while (p != NULL)
            {
                if (strcmp(p->username, user_id) == 0 && p->pending == 0)
                {
                    is_member = 1;
                    break;
                }
                p = p->next;
            }
        }

        if (is_member)
        {
            Chat *existing = chats;
            int found = 0;
            while (existing)
            {
                if (existing->is_group == 1 && strcmp(existing->to, group->name) == 0)
                {
                    found = 1;
                    break;
                }
                existing = existing->next;
            }

            if (!found)
            {
                Chat *chat = malloc(sizeof(Chat));
                if (!chat)
                {
                    group = group->next;
                    continue;
                }

                strncpy(chat->topic, group->name, sizeof(chat->topic) - 1);
                chat->topic[sizeof(chat->topic) - 1] = '\0';

                strncpy(chat->to, group->name, sizeof(chat->to) - 1);
                chat->to[sizeof(chat->to) - 1] = '\0';

                chat->is_group = 1;
                chat->participants = group->participants;

                chat->next = chats;
                chats = chat;
            }
        }

        group = group->next;
    }

    pthread_mutex_unlock(&mutex_chats);
    pthread_mutex_unlock(&mutex_groups);
}

// DONE
void load_chats_from_file()
{
    FILE *file = fopen(FILE_CHATS, "r");
    if (!file)
    {
        perror("Erro ao abrir chat/chat.txt");
        return;
    }

    pthread_mutex_lock(&mutex_chats);

    char line[512];
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\r\n")] = 0;

        if (strstr(line, user_id) == NULL)
            continue;

        char user1[128] = {0}, user2[128] = {0}, timestamp[128] = {0};
        int parts = sscanf(line, "%127[^_]_%127[^_]_%127s", user1, user2, timestamp);

        if (parts != 3)
            continue;

        char *other = NULL;
        if (strcmp(user_id, user1) == 0)
            other = user2;
        else if (strcmp(user_id, user2) == 0)
            other = user1;
        else
            continue;

        if (find_chat_by_to_and_type(other, 0) != NULL)
            continue;

        Chat *chat = malloc(sizeof(Chat));
        if (!chat)
            continue;

        strncpy(chat->topic, line, sizeof(chat->topic) - 1);
        chat->topic[sizeof(chat->topic) - 1] = '\0';

        strncpy(chat->to, other, sizeof(chat->to) - 1);
        chat->to[sizeof(chat->to) - 1] = '\0';

        chat->is_group = 0;
        chat->participants = NULL;
        chat->next = chats;
        chats = chat;
    }

    fclose(file);

    pthread_mutex_unlock(&mutex_chats);

    load_chats_by_groups();
}

// DONE
void show_chat_menu()
{
    MenuItem items[256];
    int count = 1;

    printf("\n================ Conversas Disponíveis ================\n\n");

    printf("[Conversas ativas]\n");

    pthread_mutex_lock(&mutex_users);
    pthread_mutex_lock(&mutex_groups);

    Users *current_user = users;
    while (current_user != NULL)
    {
        if (find_chat(current_user->username, 0))
        {
            printf("  %2d) %s\n", count, current_user->username);
            items[count].type = ITEM_USER;
            items[count].ptr = current_user;
            count++;
        }
        current_user = current_user->next;
    }

    Group *current_group = groups;
    while (current_group != NULL)
    {
        if (find_chat(current_group->name, 1))
        {
            printf("  %2d) %s [Grupo]\n", count, current_group->name);
            items[count].type = ITEM_GROUP;
            items[count].ptr = current_group;
            count++;
        }
        current_group = current_group->next;
    }

    printf("\n[Precisam de permissão para iniciar uma conversa]\n");
    current_user = users;
    while (current_user != NULL)
    {
        if (!find_chat(current_user->username, 0))
        {
            printf("  %2d) %s\n", count, current_user->username);
            items[count].type = ITEM_USER;
            items[count].ptr = current_user;
            count++;
        }
        current_user = current_user->next;
    }

    printf("\n--------------------------------------------------------\n");
    printf("  0) Voltar\n");
    printf("========================================================\n");
    printf("Selecione o número da conversa que deseja entrar: ");

    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);
    int choice = atoi(buffer);

    if (choice == 0)
        return;

    if (choice < 1 || choice >= count)
    {
        printf("Índice inválido.\n");
        return;
    }

    MenuItem selected = items[choice];
    if (selected.type == ITEM_USER)
    {
        Users *u = (Users *)selected.ptr;
        printf("Você entrou no chat privado com: %s\n", u->username);

        if (find_chat(u->username, 0) == NULL)
        {
            char new_message[256];
            char topic[256];
            sprintf(new_message, "%d;", IDCONTROL_CHAT_INVITATION);
            sprintf(topic, "%s_CONTROL", u->username);
            send_message(new_message, topic);
            printf("Pedido para iniciar chat enviado para %s\n", u->username);
        }
    }
    else if (selected.type == ITEM_GROUP)
    {
        Group *g = (Group *)selected.ptr;
        printf("Você entrou no grupo: %s\n", g->name);
    }

    pthread_mutex_unlock(&mutex_users);
    pthread_mutex_unlock(&mutex_groups);
}

// DONE
char *create_chat(char *name, int is_group)
{
    if (!name || strlen(name) == 0)
        return NULL;

    Chat *chat = malloc(sizeof(Chat));
    if (!chat)
        return NULL;

    strncpy(chat->to, name, sizeof(chat->to) - 1);
    chat->to[sizeof(chat->to) - 1] = '\0';

    chat->is_group = is_group;

    pthread_mutex_lock(&mutex_chats);

    chat->next = chats;
    chats = chat;

    if (is_group)
    {

        pthread_mutex_lock(&mutex_groups);

        Group *g = get_group_by_name(name);
        if (g)
        {
            chat->participants = g->participants;
        }
        else
        {
            chat->participants = NULL;
        }

        pthread_mutex_unlock(&mutex_groups);

        strcpy(chat->topic, name);
    }
    else
    {
        chat->participants = NULL;

        FILE *file = fopen(FILE_CHATS, "a");
        if (!file)
            return NULL;

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);

        fprintf(file, "%s_%s_%s\n", user_id, name, timestamp);

        fclose(file);

        sprintf(chat->topic, "%s_%s_%s", user_id, name, timestamp);
    }

    pthread_mutex_unlock(&mutex_chats);

    return chat->topic;
}

// DONE
Chat *find_chat(char *name, int is_group)
{
    pthread_mutex_lock(&mutex_chats);

    Chat *c = chats;
    while (c)
    {
        if (c->is_group == is_group && strcmp(c->to, name) == 0)
        {
            pthread_mutex_unlock(&mutex_chats);

            return c;
        }
        c = c->next;
    }

    pthread_mutex_unlock(&mutex_chats);

    return NULL;
}

// DONE
Chat *find_chat_by_to_and_type(const char *to, int is_group)
{
    pthread_mutex_lock(&mutex_chats);

    Chat *c = chats;
    while (c)
    {
        if (c->is_group == is_group && strcmp(c->to, to) == 0)
        {
            pthread_mutex_unlock(&mutex_chats);

            return c;
        }
        c = c->next;
    }

    pthread_mutex_unlock(&mutex_chats);

    return NULL;
}
