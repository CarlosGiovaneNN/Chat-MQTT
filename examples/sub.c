#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MQTTAsync.h"

#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "ExampleClientSub"
#define TOPIC "MQTT Examples"
#define QOS 1

volatile int finished = 0;
volatile int subscribed = 0;

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
    if (cause) printf("     cause: %s\n", cause);

    printf("Reconnecting...\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleanstart = 1;
    conn_opts.onSuccess5 = onConnect;
    conn_opts.onFailure5 = onConnectFailure;
    conn_opts.context = client;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start reconnect, return code %d\n", rc);
        finished = 1;
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
    printf("Subscribing to topic %s with QoS %d...\n", TOPIC, QOS);

    opts.onSuccess5 = onSubscribe;
    opts.onFailure5 = onSubscribeFailure;
    opts.context = client;

    int rc;
    if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}

void onConnectFailure(void *context, MQTTAsync_failureData5 *response)
{
    printf("Connect failed, rc %d\n", response->code);
    finished = 1;
}

void onSubscribe(void *context, MQTTAsync_successData5 *response)
{
    printf("Subscribe succeeded\n");
    subscribed = 1;
}

void onSubscribeFailure(void *context, MQTTAsync_failureData5 *response)
{
    printf("Subscribe failed, rc %d\n", response->code);
    finished = 1;
}

int main(int argc, char *argv[])
{
    MQTTAsync client;
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    int rc;

    const char *uri = (argc > 1) ? argv[1] : ADDRESS;
    printf("Using server at %s\n", uri);

    create_opts.MQTTVersion = MQTTVERSION_5;

    if ((rc = MQTTAsync_createWithOptions(&client, uri, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        return EXIT_FAILURE;
    }

    if ((rc = MQTTAsync_setCallbacks(client, client, connlost, msgarrvd, NULL)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        return EXIT_FAILURE;
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleanstart = 1;
    conn_opts.onSuccess5 = onConnect;
    conn_opts.onFailure5 = onConnectFailure;
    conn_opts.context = client;
    conn_opts.MQTTVersion = MQTTVERSION_5;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        return EXIT_FAILURE;
    }

    // Loop principal aguardando mensagens
    while (!finished)
        usleep(100000L); // 0.1s

    MQTTAsync_destroy(&client);
    return EXIT_SUCCESS;
}
