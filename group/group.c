#include "group.h"
#include "../headers.h"
#include "../message/message.h"
#include "../user/user.h"

Group *groups = NULL;

char file_groups[] = "group/groups.txt";

Group *get_group_by_index(int index)
{
    int count = 0;
    for (Group *current = groups; current != NULL; current = current->next)
    {
        if (count == index)
            return current;
        count++;
    }
    return NULL;
}

void create_group(char *group_name, char *leader)
{
    Group *new_group = malloc(sizeof(Group));

    if (!new_group)
        return;

    strcpy(new_group->name, group_name);
    strcpy(new_group->leader, leader);
    new_group->participants = NULL;
    new_group->next = groups;
    groups = new_group;
}

void add_participant(Group *group, char *username, int pending)
{
    Participant *p = malloc(sizeof(Participant));

    if (!p)
        return;

    strcpy(p->username, username);
    p->pending = pending;
    p->next = group->participants;
    group->participants = p;
}

void save_group(Group *g)
{
    FILE *f = fopen(file_groups, "a");
    if (!f)
        return;

    fprintf(f, "Group: %s\n", g->name);
    fprintf(f, "Leader: %s\n", g->leader);
    fprintf(f, "Participants:\n");
    for (Participant *p = g->participants; p != NULL; p = p->next)
    {
        fprintf(f, "- %s (%s)\n", p->username, p->pending ? "pending" : "active");
    }
    fprintf(f, "\n");

    fclose(f);
}

int toggle_participant_status(Group *group, char *username)
{
    if (!group)
        return 0;

    Participant *p = group->participants;
    while (p)
    {
        if (strcmp(p->username, username) == 0)
        {
            p->pending = !p->pending;
            // printf("User %s is now %s\n", p->username,
            //        p->pending ? "pending" : "active");
            return 1;
        }
        p = p->next;
    }

    return 0;
}

void create_group_menu()
{
    printf("\nDigite o nome do grupo: ");
    char group_name[100];
    if (!fgets(group_name, sizeof(group_name), stdin))
        return;

    group_name[strcspn(group_name, "\n")] = 0;

    create_group(group_name, user_id);

    printf("\nDigite o numero dos participantes(de enter para cancelar): ");
    printf("\nSepare-os por espaços (ex: 1 2 3)\n");

    int participant_number = 0;

    if (users == NULL)
    {
        printf("Nenhum usuario encontrado\n\n\n");
        return;
    }

    for (Users *current = users; current != NULL; current = current->next)
    {
        printf("%d - %s\n", participant_number, current->username);
        participant_number++;
    }

    char line[256];
    if (!fgets(line, sizeof(line), stdin))
        return;

    line[strcspn(line, "\n")] = 0;

    char *token = strtok(line, " ");
    while (token != NULL)
    {
        int index = atoi(token);

        printf("Você escolheu o usuário de índice: %d\n", index);

        if (index < 0 || index >= participant_number)
        {
            printf("Indice invalido\n");
            return;
        }

        Users *participant = get_user_by_index(index);

        printf("Usuário escolhido: %s\n", participant->username);

        if (participant)
        {
            Group *new_group = get_group_by_index(0);

            if (!new_group)
                return;

            add_participant(new_group, participant->username, 1);

            char new_message[256];
            char topic[256];

            sprintf(new_message, "%s ask to join %s", user_id, group_name);
            sprintf(topic, "%s_CONTROL", participant->username);

            send_message(new_message, topic);
        }
        else
        {
            printf("Usuário nao encontrado\n");
        }

        token = strtok(NULL, " ");
    }

    save_group(get_group_by_index(0));
    printf("\nGrupo criado com sucesso!\n\n\n");
}

void load_groups_from_file()
{
    FILE *f = fopen(file_groups, "r");
    if (!f)
        return;

    char line[256];
    Group *current_group = NULL;

    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "Group:", 6) == 0)
        {
            Group *new_g = malloc(sizeof(Group));
            if (!new_g)
                continue;

            strcpy(new_g->name, line + 7);
            new_g->participants = NULL;
            new_g->next = groups;

            groups = new_g;
            current_group = new_g;
        }
        else if (strncmp(line, "Leader:", 7) == 0 && current_group)
        {
            strcpy(current_group->leader, line + 8);
        }
        else if (strncmp(line, "- ", 2) == 0 && current_group)
        {
            char username[100], status[20];
            if (sscanf(line, "- %s (%[^)])", username, status) == 2)
            {
                int pending = (strcmp(status, "pending") == 0) ? 1 : 0;
                add_participant(current_group, username, pending);
            }
        }
    }

    fclose(f);
}

void list_groups()
{
    if (groups == NULL)
    {
        printf("Nenhum grupo criado.\n\n");
        return;
    }

    printf("Lista de grupos:\n\n");

    for (Group *g = groups; g != NULL; g = g->next)
    {
        printf("Group: %s\n", g->name);
        printf("Leader: %s\n", g->leader);
        printf("Participants:\n");

        if (g->participants == NULL)
        {
            printf("  Nenhum participante.\n");
        }
        else
        {
            for (Participant *p = g->participants; p != NULL; p = p->next)
            {
                printf("  - %s (%s)\n", p->username, p->pending ? "pending" : "active");
            }
        }

        printf("\n");
    }
}
