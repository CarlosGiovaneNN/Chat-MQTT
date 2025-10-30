#include "headers.h"
#include "threads/threads.h"

#include "chat/chat.h"
#include "client/client.h"
#include "group/group.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Erro ao passar argumentos\n./main <username>\n");
        return EXIT_FAILURE;
    }

    update_user_id(argv[1]);

    init_mutexes();

    load_users_from_file();
    load_groups_from_file();
    load_chats_from_file();

    init_client();

    if (create_threads())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
