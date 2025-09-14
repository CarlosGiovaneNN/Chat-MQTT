#include "../client/client.h"
#include "../headers.h"

void *run_shell(void *arg)
{
    while (isConnected())
    {
        printf("-------MENU-------\n");
        printf("1 - Enviar mensagem\n");
        printf("2 - Listar mensagens\n");
        printf("3 - Listar usuários\n");
        printf("4 - Listar grupos\n");
        printf("0 - Sair\n");
        printf("------------------\n");

        char buffer[256];

        scanf("%[^\n]", buffer);

        if (strcmp(buffer, "1") == 0)
        {
            // sendMsg();
        }
        else if (strcmp(buffer, "2") == 0)
        {
            // listMsg();
        }
        else if (strcmp(buffer, "3") == 0)
        {
            // listUsers();
        }
        else if (strcmp(buffer, "4") == 0)
        {
            // listGroups();
        }
        else if (strcmp(buffer, "0") == 0)
        {
            break;
        }
    }

    closeClient();

    return NULL;
}