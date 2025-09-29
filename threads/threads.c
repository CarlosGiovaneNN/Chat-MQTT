#include "../headers.h"

#include "client/status_publisher.h"
#include "shell/shell.h"

#include "../client/client.h"
#include "../message/message.h"

pthread_t thread_status, thread_shell;

pthread_mutex_t mutex_unread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_all_received = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_control = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_groups = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_users = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_chats = PTHREAD_MUTEX_INITIALIZER;

void init_mutexes()
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

    pthread_mutex_init(&mutex_unread, &attr);
    pthread_mutex_init(&mutex_all_received, &attr);
    pthread_mutex_init(&mutex_control, &attr);

    pthread_mutex_init(&mutex_groups, &attr);

    pthread_mutex_init(&mutex_users, &attr);

    pthread_mutex_init(&mutex_chats, &attr);

    pthread_mutexattr_destroy(&attr);
}

int create_threads()
{
    while (!is_connected())
    {
        usleep(100000);
    }

    read_pending_messages_control();

    if (pthread_create(&thread_status, NULL, &run_status_publisher, NULL) != 0)
    {
        printf("Falha ao criar a thread de publicação de status\n");
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_shell, NULL, &run_shell, NULL) != 0)
    {
        printf("Falha ao criar a thread do shell\n");
        return EXIT_FAILURE;
    }

    if (pthread_join(thread_status, NULL) != 0)
    {
        printf("Falha ao juntar a thread do shell\n");
        return EXIT_FAILURE;
    }

    if (pthread_join(thread_shell, NULL) != 0)
    {
        printf("Falha ao juntar a thread do shell\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
