#include "../headers.h"
#include "../message/message.h"
#include "../user/user.h"
#include "mqtt.h"

MQTTAsync client;

int initClient(char userId[])
{
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    int rc;

    updateUserId(userId);

    create_opts.MQTTVersion = MQTTVERSION_5;

    if ((rc = MQTTAsync_createWithOptions(&client, ADDRESS, userId, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts)) !=
        MQTTASYNC_SUCCESS)
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

    return 0;
}

int isConnected()
{
    return MQTTAsync_isConnected(client);
}

void onDisconnectSuccess(void *context, MQTTAsync_successData5 *response)
{
    printf("Desconectado do broker.\n");

    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_destroy(&client);
}

void closeClient()
{
    char newMessage[256];
    sprintf(newMessage, "%s is disconnected", userId);
    send_message(newMessage, "USERS");

    pthread_cancel(thread_status);

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer5;
    disc_opts.onSuccess5 = onDisconnectSuccess;
    disc_opts.context = client;

    int rc = MQTTAsync_disconnect(client, &disc_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Erro ao desconectar: %d\n", rc);
    }
}
