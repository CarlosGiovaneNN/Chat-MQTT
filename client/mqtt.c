#include "../headers.h"

#include "../group/group.h"
#include "../message/message.h"
#include "../user/user.h"

#include "client.h"

char *default_topics[] = {"USERS", "GROUPS"};

void on_connect(void *context, MQTTAsync_successData *response);
void on_connect_failure(void *context, MQTTAsync_failureData *response);
void on_subscribe(void *context, MQTTAsync_successData *response);
void on_subscribe_failure(void *context, MQTTAsync_failureData *response);

void connlost(void *context, char *cause)
{
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    int rc;

    printf("\nConnection lost\n");
    if (cause)
        printf("     cause: %s\n", cause);

    printf("Reconnecting...\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleanstart = 0;
    conn_opts.onSuccess = on_connect;
    conn_opts.onFailure = on_connect_failure;
    conn_opts.context = client;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start reconnect, return code %d\n", rc);
    }
}

int msgarrvd(void *context, char *topic_name, int topicLen, MQTTAsync_message *message)
{

    on_recv_message(message, topic_name);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topic_name);

    return 1;
}

void on_connect(void *context, MQTTAsync_successData *response)
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    printf("Connected successfully\n");

    opts.onSuccess = on_subscribe;
    opts.onFailure = on_subscribe_failure;
    opts.context = client;

    int rc;

    for (int i = 0; i < sizeof(default_topics) / sizeof(default_topics[0]); i++)
    {
        const char *topic = default_topics[i];
        if ((rc = MQTTAsync_subscribe(client, topic, QOS, &opts)) != MQTTASYNC_SUCCESS)
        {
            printf("Failed to start subscribe for topic %s, return code %d\n", topic, rc);
        }
    }

    char newTopic[100];
    sprintf(newTopic, "%s_CONTROL", user_id);
    if ((rc = MQTTAsync_subscribe(client, newTopic, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe for topic %s, return code %d\n", newTopic, rc);
    }

    printf("Subscribed successfully\n");
}

void on_connect_failure(void *context, MQTTAsync_failureData *response)
{
    printf("Connect failed, rc %d\n", response->code);
}

void on_subscribe(void *context, MQTTAsync_successData *response)
{
}

void on_subscribe_failure(void *context, MQTTAsync_failureData *response)
{
    printf("Subscribe failed, rc %d\n", response->code);
}
