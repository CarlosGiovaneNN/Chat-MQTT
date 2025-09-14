#include "../client/client.h"
#include "../headers.h"

volatile int deliveredtoken = 0;

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