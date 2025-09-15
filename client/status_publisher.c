#include "../headers.h"
#include "../message/message.h"
#include "../user/user.h"
#include "client.h"

void *run_status_publisher(void *arg)
{
    char newMessage[100];

    while (isConnected())
    {
        sprintf(newMessage, "%s is connected", userId);

        send_message(newMessage, "USERS");

        sleep(10);
    }

    sprintf(newMessage, "%s is disconnected", userId);
    send_message(newMessage, "USERS");

    return NULL;
}