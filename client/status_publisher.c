#include "../headers.h"
#include "../message/message.h"
#include "../user/user.h"
#include "client.h"

void *run_status_publisher(void *arg)
{
    char new_message[100];

    while (is_connected())
    {
        sprintf(new_message, "%s is connected", user_id);

        send_message(new_message, "USERS");

        sleep(10);
    }

    return NULL;
}