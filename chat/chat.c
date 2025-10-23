#include "../headers.h"

#include "../group/group.h"
#include "../message/message.h"
#include "../user/user.h"

#include "../client/mqtt.h"

Chat *chats = NULL;

char selected_chat[100] = "";

void load_chats_by_groups();
void load_chats_from_file();
void show_chat_menu();
void update_selected_chat(char *name);
void load_chat();
void show_group_participants(Chat *chat);
void show_chat_topbar(char *topic);
void show_message_from_user(char *date, char *msg);
void show_message_from_other(char *from, char *date, char *msg);
void load_messages(char *topic);
void subscribe_all_chats();

int add_private_chat(char *name, char *topic);

char *create_chat(char *name, int is_group);

Chat *find_chat(char *name, int is_group);
Chat *find_chat_by_topic(char *topic);

// CARREGA OS GRUPOS NO ARRAY DE CHAT
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

// CARREGA OS CHATS DO ARQUIVO - CRIPTOGRAFADO
void load_chats_from_file()
{
    FILE *file = fopen(FILE_CHATS, "r");
    if (!file)
    {
        perror("Erro ao abrir chat/chat.txt");
        return;
    }

    pthread_mutex_lock(&mutex_chats);

    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0)
            continue;

        if (strstr((char *)line, user_id) == NULL)
            continue;

        char user1[128] = {0}, user2[128] = {0}, timestamp[128] = {0};
        int parts = sscanf((char *)line, "%127[^_]_%127[^_]_%127s", user1, user2, timestamp);

        if (parts != 3)
            continue;

        char *other = NULL;
        if (strcmp(user_id, user1) == 0)
            other = user2;
        else if (strcmp(user_id, user2) == 0)
            other = user1;
        else
            continue;

        if (find_chat(other, 0) != NULL)
            continue;

        Chat *chat = malloc(sizeof(Chat));
        if (!chat)
            continue;

        strncpy(chat->topic, (char *)line, sizeof(chat->topic) - 1);
        chat->topic[sizeof(chat->topic) - 1] = '\0';

        // printf("Chat: %s\n", chat->topic);

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

// MOSTRA AS CONVERSAS DISPONIVEIS
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
    {
        pthread_mutex_unlock(&mutex_users);
        pthread_mutex_unlock(&mutex_groups);

        return;
    }

    if (choice < 1 || choice >= count)
    {
        printf("Índice inválido.\n");

        pthread_mutex_unlock(&mutex_users);
        pthread_mutex_unlock(&mutex_groups);

        return;
    }

    MenuItem selected = items[choice];
    if (selected.type == ITEM_USER)
    {
        printf("%d\n", 1);
        Users *u = (Users *)selected.ptr;

        if (find_chat(u->username, 0) == NULL)
        {
            printf("%d\n", 2);
            char new_message[256];
            char topic[256];
            sprintf(new_message, "%d;", IDCONTROL_CHAT_INVITATION);
            sprintf(topic, "%s_CONTROL", u->username);
            send_message(new_message, topic);
            printf("Pedido para iniciar chat enviado para %s\n", u->username);

            pthread_mutex_unlock(&mutex_users);
            pthread_mutex_unlock(&mutex_groups);

            return;
        }
        else
        {
            printf("%d\n", 3);
            printf("Você entrou no chat privado com: %s\n", u->username);

            strcpy(selected_chat, find_chat(u->username, 0)->topic);
        }
    }
    else if (selected.type == ITEM_GROUP)
    {
        Group *g = (Group *)selected.ptr;
        printf("Você entrou no grupo: %s\n", g->name);

        strcpy(selected_chat, g->name);
    }

    pthread_mutex_unlock(&mutex_users);
    pthread_mutex_unlock(&mutex_groups);

    load_chat();
}

// CARREGA O CHAT PELA PRIMEIRA VEZ - FAZ A LIMPA NO CONSOLE - ESCUTA O USUARIO
void load_chat()
{

    // system("clear");

    char buffer[256];

    // printf("%s", selected_chat);

    while (1)
    {
        print_all_msgs_from_chat(selected_chat);

        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        printf("%s\n", buffer);

        if (strcmp(buffer, "/quit") == 0)
            break;

        if (send_message(buffer, selected_chat) != EXIT_SUCCESS)
        {
            printf("Erro ao enviar a mensagem.\n");
        }

        system("clear");
    }

    system("clear");

    strcpy(selected_chat, "");
}

// CARREGA AS MENSAGENS DO CHAT ( CHAMANDO A FUNCAO NO MESSAGES )
void load_messages(char *topic)
{
    pthread_mutex_lock(&mutex_chats);

    Chat *chat = find_chat_by_topic(topic);

    if (!chat)
    {
        printf("Chat inexistente.\n");
        pthread_mutex_unlock(&mutex_chats);

        return;
    }

    print_all_msgs_from_chat(chat->topic);

    pthread_mutex_unlock(&mutex_chats);
}

// MOSTRA OS PARTICIPANTES DO GRUPO
void show_group_participants(Chat *chat)
{
    pthread_mutex_lock(&mutex_chats);
    pthread_mutex_lock(&mutex_groups);

    if (!chat->is_group || !chat->participants)
    {
        pthread_mutex_unlock(&mutex_groups);
        pthread_mutex_unlock(&mutex_chats);
        return;
    }

    Group *group = get_group_by_name(chat->to);

    char leader[256];
    strncpy(leader, group->leader, sizeof(leader) - 1);
    leader[sizeof(leader) - 1] = '\0';

    int width = 50;
    printf("┌");
    for (int i = 0; i < width - 2; i++)
        printf("─");
    printf("┐\n");

    printf("│ %-2d - %-28s [Líder]      │\n", 0, leader);

    Participant *p = chat->participants;
    int count = 1;
    while (p)
    {
        if (p->pending)
        {
            printf("│ %-2d - %-28s [Convidado]  │\n", count, p->username);
        }
        else
        {

            printf("│ %-2d - %-28s [Ativo]      │\n", count, p->username);
        }

        count++;
        p = p->next;
    }

    printf("└");
    for (int i = 0; i < width - 2; i++)
        printf("─");
    printf("┘\n\n");

    pthread_mutex_unlock(&mutex_groups);
    pthread_mutex_unlock(&mutex_chats);
}

// MOSTRA A BARRA DO CHAT COM O NOME DO GRUPO OU NOME DO USUARIO
void show_chat_topbar(char *topic)
{
    pthread_mutex_lock(&mutex_chats);

    Chat *chat = find_chat_by_topic(topic);
    int width = 50;
    int len = strlen(chat->to);
    int padding = (width - len - 2) / 2;

    printf("\n┌");
    for (int i = 0; i < width - 2; i++)
        printf("─");
    printf("┐\n");

    printf("│");
    for (int i = 0; i < padding; i++)
        printf(" ");
    printf("%s", chat->to);
    for (int i = 0; i < width - 2 - padding - len; i++)
        printf(" ");
    printf("│\n");

    printf("└");
    for (int i = 0; i < width - 2; i++)
        printf("─");
    printf("┘\n\n");

    if (chat->is_group)
    {
        show_group_participants(chat);
    }

    pthread_mutex_unlock(&mutex_chats);
}

// MOSTRA AS MENSAGENS DO USUARIO DO CHAT
void show_message_from_user(char *date, char *msg)
{
    printf("[%s] Você: %s\n", date, msg);
}

// MOSTRA AS MENSAGENS DE OUTROS USUARIOS
void show_message_from_other(char *from, char *date, char *msg)
{
    printf("[%s] %s: %s\n", date, from, msg);
}

// ATUALIZA O NOME DO CHAT SELECIONADO
void update_selected_chat(char *name)
{
    strcpy(selected_chat, name);
}

// SE INSCREVE EM TODOS OS CHATS QUE O USUARIO PARTICIPA
void subscribe_all_chats()
{
    pthread_mutex_lock(&mutex_chats);

    Chat *current_chat = chats;

    while (current_chat != NULL)
    {
        printf("Subscribing to chat: %s\n", current_chat->topic);
        subscribe_topic(current_chat->topic);
        current_chat = current_chat->next;
    }

    pthread_mutex_unlock(&mutex_chats);
}

// ADICIONA UM CHAT PRIVADO
int add_private_chat(char *name, char *topic)
{
    pthread_mutex_lock(&mutex_chats);

    Chat *chat = find_chat_by_topic(topic);

    if (chat)
    {
        pthread_mutex_unlock(&mutex_chats);
        return 0;
    }

    chat = malloc(sizeof(Chat));
    if (!chat)
    {
        pthread_mutex_unlock(&mutex_chats);
        return 0;
    }

    strncpy(chat->topic, topic, sizeof(chat->topic) - 1);
    chat->topic[sizeof(chat->topic) - 1] = '\0';

    strncpy(chat->to, name, sizeof(chat->to) - 1);
    chat->to[sizeof(chat->to) - 1] = '\0';

    chat->is_group = 0;
    chat->participants = NULL;

    chat->next = chats;
    chats = chat;

    printf("Chat priva! %s\n", chat->topic);

    subscribe_topic(chat->topic);

    pthread_mutex_unlock(&mutex_chats);

    return 1;
}

// CRIA UM NOVO CHAT
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

    pthread_mutex_lock(&mutex_groups);
    pthread_mutex_lock(&mutex_chats);

    chat->next = chats;
    chats = chat;

    if (is_group)
    {
        // printf("Grupos disponíveis:\n");

        Group *g = get_group_by_name(name);
        if (g)
            chat->participants = g->participants;
        else
            chat->participants = NULL;

        strcpy(chat->topic, name);
        subscribe_topic(chat->topic);
    }
    else
    {
        // printf("Usuários disponíveis:\n");
        chat->participants = NULL;

        FILE *file = fopen(FILE_CHATS, "a");
        if (!file)
        {

            pthread_mutex_unlock(&mutex_chats);
            pthread_mutex_unlock(&mutex_groups);

            return NULL;
        }

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);

        sprintf(chat->topic, "%s_%s_%s", user_id, name, timestamp);

        fprintf(file, "%s\n", chat->topic);
        fclose(file);

        subscribe_topic(chat->topic);
        // printf("Chat criado com sucesso! %s\n", chat->topic);
    }

    pthread_mutex_unlock(&mutex_chats);
    pthread_mutex_unlock(&mutex_groups);

    return chat->topic;
}

// ENCONTRA UM CHAT PELO NOME
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

// ENCONTRA UM CHAT PELO TOPIC
Chat *find_chat_by_topic(char *topic)
{
    pthread_mutex_lock(&mutex_chats);

    Chat *c = chats;
    while (c)
    {
        if (strcmp(c->topic, topic) == 0)
        {
            pthread_mutex_unlock(&mutex_chats);

            return c;
        }
        c = c->next;
    }

    pthread_mutex_unlock(&mutex_chats);

    return NULL;
}