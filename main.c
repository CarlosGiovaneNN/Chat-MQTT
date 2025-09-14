#include "client/client.h"
#include "headers.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Erro ao passar argumentos\n./main <username> <password>\n");
        return EXIT_FAILURE;
    }

    initClient(argv[1]);

    while (!isConnected())
    {
        usleep(100000);
    }

    while (isConnected())
    {
    }

    return 0;
}