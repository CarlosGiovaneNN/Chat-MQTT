#include "../headers.h"

#include "../group/group.h"
#include "../message/message.h"
#include "../user/user.h"

Chat *chats = NULL;

char *create_chat(char *name, int is_group)
{
    if (!name || strlen(name) == 0)
        return;

    Chat *chat = malloc(sizeof(Chat));
    if (!chat)
        return;

    strncpy(chat->to, name, sizeof(chat->to) - 1);
    chat->to[sizeof(chat->to) - 1] = '\0';

    chat->is_group = is_group;
    chat->next = chats;
    chats = chat;

    if (is_group)
    {
        Group *g = get_group_by_name(name);
        if (g)
        {
            chat->participants = g->participants;
        }
        else
        {
            chat->participants = NULL;
        }

        strcpy(chat->topic, name);
    }
    else
    {
        chat->participants = NULL;

        FILE *file = fopen("chat/chat.txt", "a");
        if (!file)
            return;

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);

        fprintf(file, "%s_%s_%s\n", user_id, name, timestamp);

        fclose(file);

        sprintf(chat->topic, "%s_%s_%s", user_id, name, timestamp);
    }

    return chat->topic;
}

void add_chat_by_message(char *message, char *from)
{
    Chat *chat = malloc(sizeof(Chat));
    if (!chat)
        return;

    strncpy(chat->topic, message, sizeof(chat->topic) - 1);
    chat->topic[sizeof(chat->topic) - 1] = '\0';

    chat->is_group = 0;
    chat->participants = NULL;
    chat->next = chats;
    chats = chat;
}

void load_chats_from_file()
{
    FILE *file = fopen("chat/chat.txt", "r");
    if (!file)
    {
        perror("Erro ao abrir chat.txt");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\r\n")] = 0;

        if (strstr(line, user_id) != NULL)
        {
            Chat *chat = malloc(sizeof(Chat));
            if (!chat)
                continue;

            strncpy(chat->topic, line, sizeof(chat->topic) - 1);
            chat->topic[sizeof(chat->topic) - 1] = '\0';

            chat->is_group = 0;
            chat->participants = NULL;
            chat->next = chats;
            chats = chat;
        }
    }

    fclose(file);
}

Chat *find_chat(char *name, int is_group)
{
    Chat *c = chats;
    while (c)
    {
        if (c->is_group == is_group && strcmp(c->to, name) == 0)
            return c;
        c = c->next;
    }
    return NULL;
}

void show_chat_menu()
{
    printf("\nConversas disponiveis:\n");

    int count = 1;

    Users *current_user = users;

    while (current_user != NULL)
    {
        printf(" %d - %s %s\n", count, current_user->username,
               find_chat(current_user->username, 0) ? "" : "(Pedido necessario)");
        count++;
        current_user = current_user->next;
    }

    Group *current_group = groups;

    while (current_group != NULL)
    {
        if (find_chat(current_group->name, 1))
        {
            printf(" %d - %s\n", count, current_group->name);
            count++;
        }
    }

    printf("0 - Voltar\n");
    printf("---------------------------\n");
    printf("\nSelecione o número da conversa que deseja entrar:\n");

    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);
    int choice = atoi(buffer);

    if (choice == 0)
        return;

    int total = count - 1;

    if (choice < 1 || choice > total)
    {
        printf("Índice inválido.\n");
        return;
    }

    if (choice <= user_count())
    {
        Users *u = users;
        for (int i = 1; i < choice && u; i++)
            u = u->next;

        if (u)
        {
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
    }
    else
    {
        int group_index = choice - user_count();
        Group *g = groups;
        for (int i = 1; i < group_index && g; i++)
            g = g->next;

        if (g)
            printf("Você entrou no grupo: %s\n", g->name);
    }
}
