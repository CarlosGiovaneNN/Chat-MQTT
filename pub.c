#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MQTTAsync.h"

#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "ExampleClientPub"
#define TOPIC "MQTT Examples"
#define QOS 1

volatile int deliveredtoken = 0;

void onConnect(void *context, MQTTAsync_successData5 *response);
void onConnectFailure(void *context, MQTTAsync_failureData5 *response);
void onSend(void *context, MQTTAsync_successData5 *response);
void onSendFailure(void *context, MQTTAsync_failureData5 *response);

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
    }
}

void onConnect(void *context, MQTTAsync_successData5 *response)
{
    printf("Connected successfully\n");
}

void onConnectFailure(void *context, MQTTAsync_failureData5 *response)
{
    printf("Connect failed, rc %d\n", response->code);
}

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

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *)message->payload);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

int main(int argc, char *argv[])
{
    MQTTAsync client;
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
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
    conn_opts.automaticReconnect = 1;
    conn_opts.MQTTVersion = MQTTVERSION_5;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        return EXIT_FAILURE;
    }

    // Aguarda conexão
    while (!MQTTAsync_isConnected(client))
        usleep(100000L);

    printf("Finished\n");

    // Mensagem a enviar
    const char *payload = "Hello from Publisher!";
    pubmsg.payload = (void *)payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    opts.onSuccess5 = onSend;
    opts.onFailure5 = onSendFailure;
    opts.context = client;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        return EXIT_FAILURE;
    }

    // Espera entrega da mensagem
    while (!deliveredtoken)
        usleep(100000L);

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer5;
    disc_opts.onSuccess5 = NULL;
    disc_opts.onFailure5 = NULL;
    disc_opts.context = client;

    MQTTAsync_disconnect(client, &disc_opts);
    MQTTAsync_destroy(&client);

    return EXIT_SUCCESS;
}
