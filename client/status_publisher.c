#include "../headers.h"
#include "../message/message.h"
#include "../user/user.h"
#include "client.h"

void *run_status_publisher(void *arg)
{

    while (is_connected())
    {
        send_message("connected", "USERS");

        sleep(20);
    }

    return NULL;
}