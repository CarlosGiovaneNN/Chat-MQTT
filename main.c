#include "client/client.h"
#include "client/status_publisher.h"
#include "group/group.h"
#include "headers.h"
#include "shell/shell.h"
#include "user/user.h"

pthread_t thread_status, thread_shell;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Erro ao passar argumentos\n./main <username> <password>\n");
        return EXIT_FAILURE;
    }

    init_client(argv[1]);
    load_users_from_file();
    load_groups_from_file();

    while (!is_connected())
    {
        usleep(100000);
    }

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