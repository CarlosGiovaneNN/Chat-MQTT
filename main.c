#include "headers.h"
#include "threads/threads.h"

#include "chat/chat.h"
#include "client/client.h"
#include "group/group.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        printf("Erro ao passar argumentos\n./main <username>\n");
        return EXIT_FAILURE;
    }

    update_user_id(argv[1]);

    init_mutexes();

    load_users_from_file();
    // printf("Users loaded\n");
    load_groups_from_file();
    // printf("Groups loaded\n");
    load_chats_from_file();
    // printf("Chats loaded\n");

    init_client();

    if (create_threads())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
