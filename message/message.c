#include "message.h"
#include "../client/client.h"
#include "../headers.h"

volatile int deliveredtoken = 0;

Messages *unread_messages = NULL;
Messages *all_received_messages = NULL;

void onSend(void *context, MQTTAsync_successData5 *response);
void onSendFailure(void *context, MQTTAsync_failureData5 *response);

void onSend(void *context, MQTTAsync_successData5 *response)
{
    printf("Message delivered successfully\n");
    deliveredtoken = 1;
}

void onSendFailure(void *context, MQTTAsync_failureData5 *response)
{
    printf("Failed to deliver message, rc %d\n", response->code);
    deliveredtoken = 1;
}

int send_message(char msg[], char topic[])
{

    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    char payload[100];

    strcpy(payload, msg);

    pubmsg.payload = (void *)payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    opts.onSuccess5 = onSend;
    opts.onFailure5 = onSendFailure;
    opts.context = client;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void add_message(Messages **array, MQTTAsync_message *message, char topic[])
{
    Messages *new_message = malloc(sizeof(Messages));
    if (!new_message)
        return;

    strcpy(new_message->topic, topic);
    strcpy(new_message->payload, (char *)message->payload);
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

void add_all_received_message(MQTTAsync_message *message, char topic[])
{
    add_message(&all_received_messages, message, topic);
}

void add_unread_message(MQTTAsync_message *message, char topic[])
{
    add_message(&unread_messages, message, topic);
    add_all_received_message(message, topic);
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