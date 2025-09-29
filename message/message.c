#include "../headers.h"

#include "../chat/chat.h"
#include "../client/client.h"
#include "../group/group.h"
#include "../user/user.h"

volatile int deliveredtoken = 0;

Messages *unread_messages = NULL;
Messages *all_received_messages = NULL;
Messages *control_messages = NULL;

void on_send(void *context, MQTTAsync_successData *response);
void on_send_failure(void *context, MQTTAsync_failureData *response);

void add_message(Messages **array, pthread_mutex_t *mtx, char payload[], char topic[], char from[], int type);
void add_all_received_message(char *payload, char topic[], char from[]);
void add_unread_message(char *payload, char topic[], char from[]);
void add_control_message(char topic[], char from[], char msg[], int type);
void clear_unread_messages();
void remove_control_message(int index);
void parse_message(char *message, char *from, char *date, char *msg);
void print_messages(Messages *messages, pthread_mutex_t *mtx);
void print_all_received_messages();
void read_pending_messages_control();
void control_msg();
void on_recv_message(MQTTAsync_message *message, char *topic);
void print_all_msgs_from_chat(char *topic);

char *format_message();

int send_message(char msg[], char topic[]);

Messages *get_control_message(int index);

// NAO ALTEREI AINDA
void on_send(void *context, MQTTAsync_successData *response)
{
    Send_Context *ctx = (Send_Context *)context;

    if (strcmp(ctx->topic, "USERS") != 0)
        printf("Message delivered successfully\n");

    deliveredtoken = 1;
    free(ctx);
}

// NAO ALTEREI AINDA
void on_send_failure(void *context, MQTTAsync_failureData *response)
{
    Send_Context *ctx = (Send_Context *)context;

    printf("Failed to deliver message, rc %d\n", response->code);

    deliveredtoken = 1;
    free(ctx);
}

// ADICIONA A MSG NO ARRAY - { ARRAY DE MENSAGENS, MUTEX DA ARRAY, MENSAGEM, TOPICO, NOME DO USUARIO, TIPO }
void add_message(Messages **array, pthread_mutex_t *mtx, char payload[], char topic[], char from[], int type)
{
    Messages *new_message = malloc(sizeof(Messages));
    if (!new_message)
        return;

    strcpy(new_message->topic, topic);
    strcpy(new_message->payload, payload);
    strcpy(new_message->from, from);
    new_message->next = NULL;
    new_message->type = type;

    pthread_mutex_lock(mtx);

    if (*array == NULL)
    {
        *array = new_message;
    }
    else
    {
        Messages *current = *array;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_message;
    }

    pthread_mutex_unlock(mtx);
}

// ADICIONA A MSG NO ARRAY DE MENSAGENS RECEBIDAS
void add_all_received_message(char *payload, char topic[], char from[])
{
    add_message(&all_received_messages, &mutex_all_received, payload, topic, from, MESSAGE_NORMAL);
}

// ADICIONA A MSG NO ARRAY DE MENSAGENS NAO LIDAS
void add_unread_message(char *payload, char topic[], char from[])
{
    add_message(&unread_messages, &mutex_unread, payload, topic, from, MESSAGE_NORMAL);
    add_all_received_message(payload, topic, from);
}

// ADICIONA A MSG NO ARRAY DE MENSAGENS DE CONTROLE
void add_control_message(char topic[], char from[], char msg[], int type)
{
    add_message(&control_messages, &mutex_control, msg, topic, from, type);
}

// TO DO
void clear_unread_messages()
{
    // falta fazer correto, com o free em tds
    unread_messages = NULL;
}

// REMOVE A MSG NO ARRAY DE MENSAGENS DE CONTROLE
void remove_control_message(int index)
{
    pthread_mutex_lock(&mutex_control);

    if (control_messages == NULL)
    {
        pthread_mutex_unlock(&mutex_control);
        return;
    }

    if (index == 0)
    {
        Messages *aux = control_messages;
        control_messages = control_messages->next;
        free(aux);
        pthread_mutex_unlock(&mutex_control);
        return;
    }

    int count = 0;
    Messages *prev = NULL;
    Messages *cur = control_messages;
    while (cur)
    {
        if (count == index)
        {
            if (prev)
                prev->next = cur->next;
            free(cur);
            pthread_mutex_unlock(&mutex_control);
            return;
        }
        prev = cur;
        cur = cur->next;
        count++;
    }

    pthread_mutex_unlock(&mutex_control);
}

/* DIVIDE A MENSAGEM EM 3 PARTES -
{
    MSG DIRETA DO TOPICO,
    ENDERECO DO LOCAL PARA SALVAR:
    [
        NOME DO USUARIO,
        DATA,
        MENSAGEM
    ]
}
*/
void parse_message(char *message, char *from, char *date, char *msg)
{
    char buffer[200];
    strncpy(buffer, message, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // garante null terminator

    char *token = strtok(buffer, "-");
    if (token)
        strcpy(from, token);

    token = strtok(NULL, "-");
    if (token)
        strcpy(date, token);

    token = strtok(NULL, "-");
    if (token)
        strcpy(msg, token);
}

// PRINTA AS MENSAGENS NO FORMATO CORRETO - { ARRAY DE MENSAGENS, MUTEX DA ARRAY }
void print_messages(Messages *messages, pthread_mutex_t *mtx)
{
    int count = 1;

    pthread_mutex_lock(mtx);

    for (Messages *message = messages; message != NULL; message = message->next)
    {
        printf("====================================\n");
        printf("Mensagem %d\n", count);
        printf("------------------------------------\n");
        printf("De:    %s\n", message->from);
        printf("Tópico: %s\n", message->topic);
        printf("------------------------------------\n");
        printf("%s\n", message->payload);
        printf("====================================\n\n");

        count++;
    }

    pthread_mutex_unlock(mtx);

    if (count == 1)
    {
        printf("Nenhuma mensagem para exibir.\n");
    }
}

// PRINTA AS MENSAGENS RECEBIDAS
void print_all_received_messages()
{
    print_messages(all_received_messages, &mutex_all_received);
}

/*
    LE OS PARICIPANTES DE CADA GRUPO E VERIFICA SE EXISTE MENSAGENS DE CONTROLE PENDENTES
    MENSAGENS DE CONTROLE PENDENTES:
    SERIA A MENSAGEM PERDIDA PELO USUARIO QUE DECIDIU NAO RESPONDER AO RECEBIMENTO
*/
void read_pending_messages_control()
{
    pthread_mutex_lock(&mutex_groups);
    pthread_mutex_lock(&mutex_control);

    Group *current_group = groups;

    while (current_group != NULL)
    {
        Participant *p = get_participant_by_username(current_group, user_id);
        if (p && p->pending == 1)
        {

            Messages *msg_node = get_control_message(0);
            int found = 0;

            while (msg_node != NULL)
            {

                char *group_name = strstr(msg_node->payload, "grupo: ");

                if (group_name)
                {
                    group_name += strlen("grupo: ");
                }

                if (msg_node->type == MESSAGE_GROUP_INVITATION && strcmp(group_name, current_group->name) == 0)
                {
                    found = 1;
                    break;
                }
                msg_node = msg_node->next;
            }

            if (!found)
            {
                char new_msg[256];

                sprintf(new_msg, "Convidou voce para o grupo: %s", current_group->name);
                add_control_message(current_group->name, user_id, new_msg, MESSAGE_GROUP_INVITATION);
            }
        }

        current_group = current_group->next;
    }

    pthread_mutex_unlock(&mutex_control);
    pthread_mutex_unlock(&mutex_groups);
}

// LISTA MENSAGENS DE CONTROLE E PERGUNTA QUAL DESEJA RESPONDER
void control_msg()
{
    int count = 1;
    printf("\n");

    pthread_mutex_lock(&mutex_control);

    for (Messages *message = control_messages; message != NULL; message = message->next)
    {
        printf("============================================\n");
        printf("Mensagem %d:\n", count);
        printf("%s\n", message->payload);
        printf("Enviada por: %s\n", message->from);
        printf("============================================\n\n");

        count++;
    }

    printf("0 - Voltar\n");
    printf("=======================================\n");
    printf("Selecione o número da mensagem que deseja responder:\n");

    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);

    if (buffer[0] == '0')
        return;

    int index = atoi(buffer) - 1;

    Messages *msg = get_control_message(index);

    if (msg == NULL)
    {
        printf("Mensagem não encontrada\n");
        return;
    }

    printf("\nVoce deseja aceitar essa mensagem? (s/n):\n");
    fgets(buffer, sizeof(buffer), stdin);
    char message[512];
    char topic[512];

    char *group_name = strstr(msg->payload, "grupo: ");

    if (group_name)
    {
        group_name += strlen("grupo: ");
    }

    if (buffer[0] == 's')
    {
        printf("\n%d\n", msg->type);
        if (msg->type == MESSAGE_GROUP_INVITATION)
        {
            sprintf(message, "%d;%s;", GROUP_INVITATION_ACCEPTED, group_name);

            send_message(message, "GROUPS");

            create_chat(group_name, 1);

            if (!toggle_participant_status_file(get_group_by_name(group_name), user_id))
            {
                printf("Erro ao alterar status no arquivo de grupos\n");
            }
        }
        else if (msg->type == MESSAGE_CHAT_INVITATION)
        {
            printf("entrou\n");
            sprintf(message, "%d;", IDCONTROL_CHAT_INVITATION_ACCEPTED);
            sprintf(topic, "%s_CONTROL", msg->from);

            send_message(message, topic);
        }
        else if (msg->type == MESSAGE_GROUP_ASK_TO_JOIN)
        {
            printf("entrou 2\n");

            sprintf(message, "%d;%s;", GROUP_INVITATION_ACCEPTED, group_name);

            send_message(message, "GROUPS");

            if (!add_participant_to_group_file(group_name, msg->from, get_group_by_name(group_name), 0))
            {
                printf("Erro ao adicionar participante no arquivo de grupos\n");
            }
        }
    }
    else
    {
        if (msg->type == MESSAGE_GROUP_INVITATION)
        {
            sprintf(message, "%d;%s;", GROUP_INVITATION_REJECTED, group_name);

            send_message(message, "GROUPS");
        }
        else if (msg->type == MESSAGE_CHAT_INVITATION)
        {
            sprintf(message, "%d;", IDCONTROL_CHAT_INVITATION_REJECTED);
            sprintf(topic, "%s_CONTROL", msg->from);

            send_message(message, topic);
        }
    }

    remove_control_message(index);

    pthread_mutex_unlock(&mutex_control);
}

// AO RECEBER MENSAGEM DE QUALQUER TOPICO
void on_recv_message(MQTTAsync_message *message, char *topic)
{
    char from[100], date[100], msg[100];
    parse_message((char *)message->payload, from, date, msg);

    if (strlen(from) == 0 || strlen(date) == 0 || strlen(msg) == 0)
        return;

    char id_control[100];
    sprintf(id_control, "%s_CONTROL", user_id);

    // printf("%s\n", id_control);
    // printf("%s\n", topic);
    // printf("%d\n", strcmp(topic, id_control));

    if (strcmp(topic, "USERS") != 0) // dps remover
    {
        printf("%s\n", (char *)message->payload);
    }

    if (strcmp(topic, "USERS") == 0)
    {

        if (strcmp(user_id, from) == 0)
            return;

        add_user(from);

        // printf("%s\n", (char *)message->payload);
        // printf("%s\n", from);
        //  printf("%s\n", from);

        if (check_status(msg) == 1)
        {
            change_status(from, 1);
        }
        else if (check_status(msg) == 0)
        {
            // printf("%s\n", from);
            change_status(from, 0);
        }
    }
    else if (strcmp(topic, "GROUPS") == 0)
    {

        if (strcmp(user_id, from) == 0)
            return;

        // printf("ENTROU NO GROUPS\n");
        if (strncmp(msg, "Group:", 6) == 0)
        {
            // printf("ENTROU NO GROUP CREATE\n");
            add_group_by_message(msg);
        }
        else
        {
            int option;
            char new_msg[256];
            char group_name[128];

            sscanf(msg, "%d;", &option);

            printf("%s\n", (char *)message->payload);

            if (option == GROUP_INVITATION_ACCEPTED)
            {
                sscanf(msg, "%d;%[^;];", &option, group_name);
                sprintf(new_msg, "Aceitou o convite para o grupo: %s", group_name);

                add_unread_message(new_msg, topic, from);

                pthread_mutex_lock(&mutex_groups);

                Group *group = get_group_by_name(group_name);

                if (get_participant_by_username(group, from) == NULL)
                {
                    printf("ENTROU NO ADD PARTICIPANT\n");
                    add_participant(group, from, 0);
                }
                else
                {
                    change_participant_status(group, from, 0);
                }

                pthread_mutex_unlock(&mutex_groups);
            }
            else if (option == GROUP_INVITATION_REJECTED)
            {
                sscanf(msg, "%d;%[^;];", &option, group_name);
                sprintf(new_msg, "Recusou o convite para o grupo: %s", group_name);
                add_unread_message(topic, from, new_msg);

                // Group *group = get_group_by_name(group_name);

                // change_participant_status(group, from, -1);
            }
        }
    }
    else if (strcmp(topic, id_control) == 0)
    {

        if (strcmp(user_id, from) == 0)
            return;

        int option;
        char new_msg[256];

        sscanf(msg, "%d;", &option);

        if (option == IDCONTROL_GROUP_INVITATION)
        {
            char group_name[100];
            sscanf(msg, "%d;%[^;];", &option, group_name);

            sprintf(new_msg, "Convidou voce para o grupo: %s", group_name);

            add_control_message(topic, from, new_msg, MESSAGE_GROUP_INVITATION);
        }
        else if (option == IDCONTROL_CHAT_INVITATION)
        {
            add_control_message(topic, from, "Pede para voce entrar no chat", MESSAGE_CHAT_INVITATION);
        }
        else if (option == IDCONTROL_CHAT_INVITATION_ACCEPTED)
        {
            printf("aceitou\n");
            sprintf(new_msg, "%s aceitou o convite para o chat", from);
            add_unread_message(topic, from, new_msg);

            char topic[128];
            char new_chat[100];

            strcpy(new_chat, create_chat(from, 0));

            sprintf(topic, "%s_CONTROL", from);

            send_message(new_chat, topic);
        }
        else if (option == IDCONTROL_CHAT_INVITATION_REJECTED)
        {
            sprintf(new_msg, "%s recusou o convite para o chat", from);
            add_unread_message(topic, from, new_msg);
        }
        else if (option == IDCONTROL_GROUP_ASK_TO_JOIN)
        {
            printf("ENTROU NO IDCONTROL_GROUP_ASK_TO_JOIN\n");

            printf("%s\n", (char *)message->payload);

            add_control_message(topic, from, "Pede permissao para entrar no grupo", MESSAGE_GROUP_ASK_TO_JOIN);
        }
        else
        {

            printf("%s", (char *)message->payload);
        }
    }
    else
    {
        // printf("ENTROU NO ELSE\n");

        if (strcmp(topic, selected_chat) != 0)
        {
            // printf("ENTROU NO IF\n");
            if (strcmp(user_id, from) == 0)
                return;

            add_unread_message(msg, topic, from);
        }
        else
        {
            // printf("ENTROU NO ELSE\n");

            add_all_received_message(msg, topic, from);

            if (strcmp(user_id, from) == 0)
                return;

            show_message_from_other(from, topic, msg);
        }
    }
}

void print_all_msgs_from_chat(char *topic)
{
    pthread_mutex_lock(&mutex_all_received);

    Messages *current = all_received_messages;

    while (current != NULL)
    {
        printf("ENTROU NO WHILE\n");
        if (strcmp(current->topic, topic) == 0)
        {
            printf("ENTROU NO IF\n");
            if (strcmp(current->from, user_id) == 0)
            {
                printf("ENTROU NO IF\n");
                show_message_from_user(current->topic, current->payload);
            }
            else
            {
                printf("ENTROU NO ELSE\n");
                show_message_from_other(current->from, current->topic, current->payload);
            }
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex_all_received);
}

// FORMATA A MENSAGEM COM O FORMATO CORRETO { USUARIO - DD/MM/AAAA HH:MM:SS - MENSAGEM }
char *format_message()
{

    time_t agora;
    struct tm *infoTempo;

    // 01/01/1970
    time(&agora);
    infoTempo = localtime(&agora);

    char *message = malloc(128);
    if (!message)
        return NULL;

    sprintf(message, "%s-%02d/%02d/%04d %02d:%02d:%02d-", user_id, infoTempo->tm_mday, infoTempo->tm_mon + 1,
            infoTempo->tm_year + 1900, infoTempo->tm_hour, infoTempo->tm_min, infoTempo->tm_sec);

    return message;
}

// ENVIA A MENSAGEM
int send_message(char msg[], char topic[])
{

    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    char payload[526];
    char *prefix = format_message();
    if (!prefix)
        return EXIT_FAILURE;

    strcpy(payload, prefix);
    free(prefix);

    strcat(payload, msg);

    pubmsg.payload = (void *)payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    Send_Context *ctx = malloc(sizeof(Send_Context));
    ctx->client = client;
    strcpy(ctx->topic, topic);

    opts.onSuccess = on_send;
    opts.onFailure = on_send_failure;
    opts.context = ctx;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        free(ctx);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// OBTEM A MENSAGEM DE CONTROLE PELO INDICE
Messages *get_control_message(int index)
{
    pthread_mutex_lock(&mutex_control);

    int count = 0;

    for (Messages *message = control_messages; message != NULL; message = message->next)
    {
        if (count == index)
        {
            pthread_mutex_unlock(&mutex_control);
            return message;
        }
        count++;
    }

    pthread_mutex_unlock(&mutex_control);

    return NULL;
}
