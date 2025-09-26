#include "../headers.h"

#include "client/status_publisher.h"
#include "shell/shell.h"

#include "../client/client.h"
#include "../message/message.h"

pthread_t thread_status, thread_shell;

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