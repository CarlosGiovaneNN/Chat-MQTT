#include "../headers.h"
#include "../message/message.h"
#include "../threads/threads.h"
#include "../user/user.h"
#include "mqtt.h"

MQTTAsync client;

int connect_client()
{
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.automaticReconnect = 1;
    conn_opts.context = client;

    conn_opts.onSuccess = on_connect;
    conn_opts.onFailure = on_connect_failure;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int init_client()
{
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    int rc;

    if ((rc = MQTTAsync_createWithOptions(&client, ADDRESS, user_id, MQTTCLIENT_PERSISTENCE_DEFAULT,
                                          "/tmp/mqtt_persist", &create_opts)) != MQTTASYNC_SUCCESS)
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

    if (connect_client())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int is_connected()
{
    return MQTTAsync_isConnected(client);
}

void show_reconnect_options()
{
    printf("Voce deseja reconectar? (s/n)\n");

    char buffer[10];
    fgets(buffer, sizeof(buffer), stdin);

    printf("%s", buffer);

    if (buffer[0] == 's')
    {
        connect_client();
        create_threads();
        return;
    }

    MQTTAsync_destroy(&client);
}

void on_disconnect_success(void *context, MQTTAsync_successData *response)
{
    printf("Desconectado do broker.\n");
}

void close_client()
{
    send_message("disconnected", "USERS");

    pthread_cancel(thread_status);

    usleep(100000);

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    disc_opts.onSuccess = on_disconnect_success;
    disc_opts.context = client;

    int rc = MQTTAsync_disconnect(client, &disc_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Erro ao desconectar: %d\n", rc);
    }
    else
    {
        while (is_connected())
            ;
        show_reconnect_options();
    }
}
