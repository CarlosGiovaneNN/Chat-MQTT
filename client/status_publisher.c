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

void verify_others_connection_status()
{
    time_t now;
    time(&now);

    double two_minutes_limit = 120.0;

    pthread_mutex_lock(&mutex_users);

    Users *current = users;
    while (current != NULL)
    {
        if (current->online == 1)
        {

            char datetime_str[30];

            strncpy(datetime_str, current->last_seen, sizeof(datetime_str) - 1);
            datetime_str[sizeof(datetime_str) - 1] = '\0';

            struct tm last_seen_tm = {0};
            int day, mon, year, hour, min, sec;

            if (sscanf(datetime_str, "%d/%d/%d %d:%d:%d", &day, &mon, &year, &hour, &min, &sec) == 6)
            {
                last_seen_tm.tm_mday = day;
                last_seen_tm.tm_mon = mon - 1;
                last_seen_tm.tm_year = year - 1900;
                last_seen_tm.tm_hour = hour;
                last_seen_tm.tm_min = min;
                last_seen_tm.tm_sec = sec;
                last_seen_tm.tm_isdst = -1;

                time_t last_seen_time_t = mktime(&last_seen_tm);

                double difference = difftime(now, last_seen_time_t);

                if (difference > two_minutes_limit)
                {
                    current->online = 0;
                }
            }
            else
            {
                printf("Data inválida: %s\n", datetime_str);
            }
        }

        current = current->next;
    }

    pthread_mutex_unlock(&mutex_users);
}