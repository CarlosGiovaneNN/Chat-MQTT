#include "../headers.h"
#include "../message/message.h"
#include "../user/user.h"
#include "client.h"

void verify_others_connection_status();

void *run_status_publisher(void *arg)
{
    int count = 0;

    while (is_connected())
    {
        send_message("connected", "USERS");

        if (count == 1)
        {
            verify_others_connection_status();
            count = 0;
        }
        sleep(20);
        count++;
    }

    return NULL;
}

void verify_others_connection_status() // - X
{
    time_t now;
    time(&now);

    pthread_mutex_lock(&mutex_users);

    Users *current = users;
    while (current != NULL)
    {
        if (current->online == 1)
        {
            if (compare_time(current->last_seen, 120.0))
            {
                current->online = 0;
            }
        }

        current = current->next;
    }

    pthread_mutex_unlock(&mutex_users);
}