#include "../client/client.h"
#include "../headers.h"

#include "../group/group.h"
#include "../user/user.h"

void *run_shell(void *arg)
{
    while (is_connected())
    {
        printf("-------MENU-------\n");
        printf("1 - Enviar mensagem\n");
        printf("2 - Controle\n");
        printf("3 - Listar mensagens\n");
        printf("4 - Listar usuários\n");
        printf("5 - Listar grupos\n");
        printf("6 - Listar mensagens de controle\n");
        printf("7 - Criar grupo\n");
        printf("0 - Sair\n");
        printf("------------------\n");

        char buffer[256];
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "1") == 0)
        {
            // send_msg();
        }
        else if (strcmp(buffer, "2") == 0)
        {
            // control_msg();
        }
        else if (strcmp(buffer, "3") == 0)
        {
            // list_msg();
        }
        else if (strcmp(buffer, "4") == 0)
        {
            list_users();
        }
        else if (strcmp(buffer, "5") == 0)
        {
            list_groups();
        }
        else if (strcmp(buffer, "6") == 0)
        {
            // list_control_msg();
        }
        else if (strcmp(buffer, "7") == 0)
        {
            create_group_menu();
        }
        else if (strcmp(buffer, "0") == 0)
        {
            break;
        }
    }

    close_client();

    return NULL;
}