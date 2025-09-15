#include "../headers.h"
#include "../message/message.h"
#include "../user/user.h"
#include "mqtt.h"

MQTTAsync client;

int init_client(char user_id[])
{
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    int rc;

    update_user_id(user_id);

    create_opts.MQTTVersion = MQTTVERSION_5;

    if ((rc = MQTTAsync_createWithOptions(&client, ADDRESS, user_id, MQTTCLIENT_PERSISTENCE_NONE, NULL,
                                          &create_opts)) != MQTTASYNC_SUCCESS)
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
    conn_opts.onSuccess5 = on_connect;
    conn_opts.onFailure5 = on_connect_failure;
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

int is_connected()
{
    return MQTTAsync_isConnected(client);
}

void on_disconnect_success(void *context, MQTTAsync_successData5 *response)
{
    printf("Desconectado do broker.\n");

    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_destroy(&client);
}

void close_client()
{
    char new_message[256];
    sprintf(new_message, "%s is disconnected", user_id);
    send_message(new_message, "USERS");

    pthread_cancel(thread_status);

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer5;
    disc_opts.onSuccess5 = on_disconnect_success;
    disc_opts.context = client;

    int rc = MQTTAsync_disconnect(client, &disc_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Erro ao desconectar: %d\n", rc);
    }
}
