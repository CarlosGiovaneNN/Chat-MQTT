#include "../headers.h"
#include "../message/message.h"
#include "../threads/threads.h"
#include "../user/user.h"
#include "mqtt.h"

MQTTAsync client;

void show_reconnect_options();
void on_disconnect_success(void *context, MQTTAsync_successData *response);
void close_client();

int connect_client();
int init_client();
int is_connected();

// MOSTRA AS OPCOES DE RECONEXAO
void show_reconnect_options()
{
    printf("Voce deseja reconectar? (s/n)\n");

    char buffer[10];
    fgets(buffer, sizeof(buffer), stdin);

    if (buffer[0] == 's')
    {

        init_mutexes();
        connect_client();
        create_threads();
        return;
    }

    MQTTAsync_destroy(&client);
}

// MENSAGEM DE DESCONEXAO
void on_disconnect_success(void *context, MQTTAsync_successData *response)
{
    printf("Desconectado do broker.\n");
}

// DESCONECTA O CLIENTE
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

// CONECTA O CLIENTE
int connect_client()
{
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 0;
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

// INICIALIZA O CLIENTE
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

// VERIFICA SE O CLIENTE ESTA CONECTADO
int is_connected()
{
    return MQTTAsync_isConnected(client);
}
