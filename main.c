#include "client/client.h"
#include "group/group.h"
#include "headers.h"
#include "threads/threads.h"
#include "user/user.h"

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

    if (create_threads())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}