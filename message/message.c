#include "../headers.h"

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

void add_message(Messages **array, char payload[], char topic[], char from[])
{
    Messages *new_message = malloc(sizeof(Messages));
    if (!new_message)
        return;

    strcpy(new_message->topic, topic);
    strcpy(new_message->payload, payload);
    strcpy(new_message->from, from);
    new_message->next = NULL;

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

void add_all_received_message(MQTTAsync_message *message, char topic[], char from[])
{
    add_message(&all_received_messages, message->payload, topic, from);
}

void add_unread_message(MQTTAsync_message *message, char topic[], char from[])
{
    add_message(&unread_messages, message->payload, topic, from);
    add_all_received_message(message, topic, from);
}

void add_control_message(char topic[], char from[], char msg[])
{
    add_message(&control_messages, msg, topic, from);
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

void list_control_msg()
{
    int count = 1;
    printf("\n\n");
    for (Messages *message = control_messages; message != NULL; message = message->next)
    {
        printf("\n---------------------------\n");
        printf("%d - Mensagem de controle\n", count);
        printf("Realizada por: %s\n\n", message->from);

        printf("%s\n", message->payload);
        printf("---------------------------\n");

        count++;
    }

    printf("\nSelecione o número da mensagem que deseja responder:\n");
}

// int check_id_control(char msg[])
// {
//     int input;

//     sscanf(msg, "%d;", input);

//     if (input > 0)
//         return input;

//     return 0;
// }

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

        //printf("%s\n", (char *)message->payload);
        //printf("%d\n", check_status(msg));
        //printf("%s\n", from);

        if (check_status(msg) == 1)
        {
            change_status(from, 1);
        }
        else if (check_status(msg) == 0)
        {
            //printf("%s\n", from);
            change_status(from, 0);
        }
    }
    else if (strcmp(topic, "GROUPS") == 0)
    {
        add_group_by_message(msg);
    }
    else if (strcmp(topic, id_control) == 0)
    {
        printf("%s\n", (char *)message->payload);
        int option;
        char new_msg[128];

        sscanf(msg, "%d;", &option);

        if (option == IDCONTROL_GROUP_INVITATION)
        {
            char group_name[100];
            sscanf(msg, "%d;%s;", &option, group_name);

            sprintf(new_msg, "Convidou voce para o grupo: %s", group_name);

            add_control_message(topic, from, new_msg);
        }
        else if (option == IDCONTROL_CHAT_INVITATION)
        {
            add_control_message(topic, from, "");
        }
        else if (option == IDCONTROL_CHAT_INVITATION_ACCEPTED)
        {
            add_control_message(topic, from, "");
        }
        else if (option == IDCONTROL_CHAT_INVITATION_REJECTED)
        {
            add_control_message(topic, from, "");
        }
    }
    else
    {
        add_unread_message(message, topic, from);
    }
}