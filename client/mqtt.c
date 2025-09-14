#include "../headers.h"
#include "../message/message.h"

char *defaultTopics[] = {"USERS", "GROUPS"};
char userId[] = "";

void onConnect(void *context, MQTTAsync_successData5 *response);
void onConnectFailure(void *context, MQTTAsync_failureData5 *response);
void onSubscribe(void *context, MQTTAsync_successData5 *response);
void onSubscribeFailure(void *context, MQTTAsync_failureData5 *response);

void connlost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    int rc;

    printf("\nConnection lost\n");
    if (cause)
        printf("     cause: %s\n", cause);

    printf("Reconnecting...\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleanstart = 1;
    conn_opts.onSuccess5 = onConnect;
    conn_opts.onFailure5 = onConnectFailure;
    conn_opts.context = client;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start reconnect, return code %d\n", rc);
    }
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *)message->payload);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void onConnect(void *context, MQTTAsync_successData5 *response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    printf("Connected successfully\n");

    opts.onSuccess5 = onSubscribe;
    opts.onFailure5 = onSubscribeFailure;
    opts.context = client;

    int rc;

    for (int i = 0; i < sizeof(defaultTopics) / sizeof(defaultTopics[0]); i++)
    {
        const char *topic = defaultTopics[i];
        if ((rc = MQTTAsync_subscribe(client, topic, QOS, &opts)) != MQTTASYNC_SUCCESS)
        {
            printf("Failed to start subscribe for topic %s, return code %d\n", topic, rc);
        }
    }

    char newTopic[100];
    sprintf(newTopic, "%s_CONTROL", userId);
    if ((rc = MQTTAsync_subscribe(client, newTopic, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe for topic %s, return code %d\n", newTopic, rc);
    }

    printf("Subscribed successfully\n");
}

void onConnectFailure(void *context, MQTTAsync_failureData5 *response)
{
    printf("Connect failed, rc %d\n", response->code);
}

void onSubscribe(void *context, MQTTAsync_successData5 *response)
{
}

void onSubscribeFailure(void *context, MQTTAsync_failureData5 *response)
{
    printf("Subscribe failed, rc %d\n", response->code);
}

void updateUserId(char id[])
{
    strcpy(userId, id);
}