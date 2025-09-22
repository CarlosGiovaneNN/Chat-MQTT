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

void on_send(void *context, MQTTAsync_successData *response)
{
    Send_Context *ctx = (Send_Context *)context;

    if (strcmp(ctx->topic, "USERS") != 0)
        printf("Message delivered successfully\n");

    deliveredtoken = 1;
    free(ctx);
}

void on_send_failure(void *context, MQTTAsync_failureData *response)
{
    Send_Context *ctx = (Send_Context *)context;

    printf("Failed to deliver message, rc %d\n", response->code);

    deliveredtoken = 1;
    free(ctx);
}

void parse_message(const char *message, char *from, char *date, char *msg)
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

void add_message(Messages **array, char payload[], char topic[], char from[], int type)
{
    Messages *new_message = malloc(sizeof(Messages));
    if (!new_message)
        return;

    strcpy(new_message->topic, topic);
    strcpy(new_message->payload, payload);
    strcpy(new_message->from, from);
    new_message->next = NULL;
    new_message->type = type;

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
}

void add_all_received_message(char *payload, char topic[], char from[])
{
    add_message(&all_received_messages, payload, topic, from, MESSAGE_NORMAL);
}

void add_unread_message(char *payload, char topic[], char from[])
{
    add_message(&unread_messages, payload, topic, from, MESSAGE_NORMAL);
    add_all_received_message(payload, topic, from);
}

void add_control_message(char topic[], char from[], char msg[], int type)
{
    add_message(&control_messages, msg, topic, from, type);
}

void clear_unread_messages()
{
    // falta fazer correto, com o free em tds
    unread_messages = NULL;
}

void print_messages(Messages *messages)
{
    for (Messages *message = messages; message != NULL; message = message->next)
    {
        printf("From: %s\n", message->from);
        printf("Topic: %s\n", message->topic);

        printf("%s\n", message->payload);
    }
}

void print_all_received_messages()
{
    print_messages(all_received_messages);
}

void remove_control_message(int index)
{
    int count = 1;

    if (index > 0)
    {
        for (Messages *message = control_messages; message != NULL; message = message->next)
        {
            if (count == index)
            {
                Messages *aux = message->next;
                message->next = message->next->next;
                free(aux);
                return;
            }
            count++;
        }
    }
    else if (index == 0)
    {
        Messages *aux = control_messages;
        control_messages = control_messages->next;
        free(aux);
    }
}

Messages *get_control_message(int index)
{
    int count = 0;
    for (Messages *message = control_messages; message != NULL; message = message->next)
    {
        if (count == index)
            return message;
        count++;
    }
    return NULL;
}

void control_msg()
{
    int count = 1;
    printf("\n\n");
    for (Messages *message = control_messages; message != NULL; message = message->next)
    {
        printf("\n---------------------------\n");
        printf("%d - %s\n", count, message->payload);
        printf("Realizada por: %s\n", message->from);
        printf("---------------------------\n");

        count++;
    }

    printf("0 - Voltar\n");
    printf("---------------------------\n");
    printf("\nSelecione o número da mensagem que deseja responder:\n");

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

            remove_control_message(index);

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

            remove_control_message(index);
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
        remove_control_message(index);
    }
}

void on_recv_message(MQTTAsync_message *message, char *topic)
{
    char from[100], date[100], msg[100];
    parse_message((char *)message->payload, from, date, msg);

    char id_control[100];
    sprintf(id_control, "%s_CONTROL", user_id);

    // printf("%s\n", id_control);
    // printf("%s\n", topic);
    // printf("%d\n", strcmp(topic, id_control));

    if (strcmp(topic, "USERS") != 0) // dps remover
    {
        // printf("%s\n", (char *)message->payload);
    }

    if (strcmp(topic, "USERS") == 0)
    {
        add_user(from);

        // printf("%s\n", (char *)message->payload);
        // printf("%d\n", check_status(msg));
        // printf("%s\n", from);

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
        printf("ENTROU NO GROUPS\n");
        if (strncmp(msg, "Group:", 6) == 0)
        {
            printf("ENTROU NO GROUP CREATE\n");
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

                Group *group = get_group_by_name(group_name);

                change_participant_status(group, from, 0);
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
        int option;
        char new_msg[256];
        // char topic[128];

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
            // sprintf(new_msg, "Pede para voce entrar no chat", from);
            add_control_message(topic, from, "Pede para voce entrar no chat", MESSAGE_CHAT_INVITATION);
        }
        else if (option == IDCONTROL_CHAT_INVITATION_ACCEPTED)
        {
            printf("aceitou\n");
            sprintf(new_msg, "%s aceitou o convite para o chat", from);
            add_unread_message(topic, from, new_msg);

            create_chat(from, 0);
        }
        else if (option == IDCONTROL_CHAT_INVITATION_REJECTED)
        {
            sprintf(new_msg, "%s recusou o convite para o chat", from);
            add_unread_message(topic, from, new_msg);
        }
    }
    else
    {
        if (strcmp(user_id, from) == 0)
        {
            return;
        }

        add_unread_message(msg, topic, from);
    }
}