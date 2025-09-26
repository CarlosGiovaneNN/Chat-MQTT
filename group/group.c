#include "../headers.h"

#include "../chat/chat.h"
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

    create_chat(group_name, 1);
}

void add_participant(Group *group, char *username, int pending)
{
    Participant *p = malloc(sizeof(Participant));
    if (!p)
        return;

    strcpy(p->username, username);
    p->pending = pending;
    p->next = NULL;

    if (!group->participants)
    {
        group->participants = p;
    }
    else
    {
        Participant *current = group->participants;
        while (current->next)
        {
            current = current->next;
        }
        current->next = p;
    }
}

int add_participant_to_group_file(char *group_name, char *username, Group *group, int pending)
{
    FILE *in = fopen(file_groups, "r");
    if (!in)
    {
        perror("Erro ao abrir group.txt");
        return 0;
    }

    FILE *out = fopen("tmp.txt", "w");
    if (!out)
    {
        perror("Erro ao criar tmp.txt");
        fclose(in);
        return 0;
    }

    char line[256];
    int inside_group = 0;
    int group_found = 0;

    while (fgets(line, sizeof(line), in))
    {
        fputs(line, out);

        if (strncmp(line, "Group: ", 7) == 0)
        {
            char current_group[100];
            sscanf(line + 7, "%[^\n]", current_group);

            if (strcmp(current_group, group_name) == 0)
            {
                inside_group = 1;
                group_found = 1;
            }
            else
            {
                inside_group = 0;
            }
        }

        if (inside_group && strncmp(line, "Participants:", 12) == 0)
        {
            // DO NOTHING
        }
        else if (inside_group && strncmp(line, "Group: ", 7) == 0)
        {
            fprintf(out, "- %s (%s)\n", username, pending ? "pending" : "active");
            inside_group = 0;
        }
    }

    if (inside_group)
    {
        fprintf(out, "- %s (%s)\n", username, pending ? "pending" : "active");
    }

    fclose(in);
    fclose(out);

    add_participant(group, username, pending);

    remove(file_groups);
    rename("tmp.txt", file_groups);

    return group_found;
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

int toggle_participant_status_file(Group *group, char *username)
{
    if (!group)
        return 0;

    Participant *p = group->participants;
    while (p)
    {
        if (strcmp(p->username, username) == 0)
        {
            p->pending = !p->pending;

            FILE *in = fopen(file_groups, "r");
            if (!in)
                return 0;

            FILE *out = fopen("tmp.txt", "w");
            if (!out)
            {
                fclose(in);
                return 0;
            }

            char line[256];

            while (fgets(line, sizeof(line), in))
            {
                // remove \n ou \r\n do final
                line[strcspn(line, "\r\n")] = 0;

                if (strstr(line, username) && strstr(line, "(pending)"))
                {
                    fprintf(out, "- %s (active)\n", username);
                }
                else if (strstr(line, username) && strstr(line, "(active)"))
                {
                    fprintf(out, "- %s (pending)\n", username);
                }
                else
                {
                    fprintf(out, "%s\n", line);
                }
            }

            fclose(in);
            fclose(out);

            remove(file_groups);
            rename("tmp.txt", file_groups);

            return 1;
        }
        p = p->next;
    }

    return 0;
}

void change_participant_status(Group *group, char *username, int pending)
{
    Participant *p = group->participants;
    while (p)
    {
        if (strcmp(p->username, username) == 0)
        {
            p->pending = pending;
            return;
        }
        p = p->next;
    }
}

void add_group_by_message(char *message)
{
    if (!message || strlen(message) == 0)
        return;

    Group *group = malloc(sizeof(Group));
    if (!group)
        return;
    group->participants = NULL;

    char *ptr = strstr(message, "Group:<");
    if (ptr)
    {
        ptr += strlen("Group:<");
        char *end = strstr(ptr, ">;");
        if (end)
        {
            while (end > ptr && isspace(*(end - 1)))
                end--;
            size_t len = end - ptr;
            strncpy(group->name, ptr, len);
            group->name[len] = 0;
        }
    }

    ptr = strstr(message, "Leader:<");
    if (ptr)
    {
        ptr += strlen("Leader:<");
        char *end = strstr(ptr, ">;");
        if (end)
        {
            while (end > ptr && isspace(*(end - 1)))
                end--;
            size_t len = end - ptr;
            strncpy(group->leader, ptr, len);
            group->leader[len] = 0;
        }
    }

    ptr = strstr(message, "Participants:[");
    if (ptr)
    {
        ptr += strlen("Participants:[");
        char *end = strchr(ptr, ']');
        if (end)
        {
            *end = 0;
            char *token = strtok(ptr, ";");
            while (token)
            {
                while (*token && isspace(*token))
                    token++;
                char *tend = token + strlen(token) - 1;
                while (tend > token && isspace(*tend))
                {
                    *tend = 0;
                    tend--;
                }

                char username[100];

                strcpy(username, token);

                add_participant(group, username, 1);
                token = strtok(NULL, ";");
            }
        }
    }

    group->next = groups;
    groups = group;
}

void create_group_message()
{
    char group_name[254];
    snprintf(group_name, sizeof(group_name), "Group:<%s>;", groups->name);

    char leader[254];
    snprintf(leader, sizeof(leader), "Leader:<%s>;Participants:", groups->leader);

    char participants[1024] = "[";
    for (Participant *p = groups->participants; p != NULL; p = p->next)
    {
        strncat(participants, p->username, sizeof(participants) - strlen(participants) - 1);
        strncat(participants, ";", sizeof(participants) - strlen(participants) - 1);
    }
    strncat(participants, "];", sizeof(participants) - strlen(participants) - 1);

    size_t len = strlen(group_name) + strlen(leader) + strlen(participants) + 1;
    char *message = malloc(len);
    if (!message)
        return;

    snprintf(message, len, "%s%s%s", group_name, leader, participants);

    send_message(message, "GROUPS");
    free(message);
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

            sprintf(new_message, "%d;%s;", IDCONTROL_GROUP_INVITATION, group_name);
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

    create_group_message();
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

Group *get_group_by_name(char *group_name)
{
    for (Group *g = groups; g != NULL; g = g->next)
    {
        if (strcmp(g->name, group_name) == 0)
        {
            return g;
        }
    }

    return NULL;
}

void aks_to_join_group(Group *group)
{
    char message[256];
    char topic[256];
    snprintf(message, sizeof(message), "%d;%s;", IDCONTROL_GROUP_ASK_TO_JOIN, group->name);
    snprintf(topic, sizeof(topic), "%s_CONTROL", group->leader);

    send_message(message, topic);
}

void join_group_menu()
{
    printf("\nGrupos disponíveis para entrar:\n");
    printf("-------------------------------\n");

    int count = 1;
    Group *current_group = groups;

    while (current_group != NULL)
    {
        int is_member = 0;

        if (strcmp(current_group->leader, user_id) == 0)
        {
            is_member = 1;
        }

        Participant *p = current_group->participants;
        while (p != NULL)
        {
            if (strcmp(p->username, user_id) == 0)
            {
                is_member = 1;
                break;
            }
            p = p->next;
        }

        if (!is_member)
        {
            printf(" %d - %s\n", count, current_group->name);
            count++;
        }

        current_group = current_group->next;
    }

    if (count == 1)
    {
        printf("Nenhum grupo disponível.\n");
    }

    printf("0 - Voltar\n");
    printf("-------------------------------\n");
    printf("\nSelecione o número do grupo que deseja entrar:\n");

    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);

    if (buffer[0] == '0')
        return;

    int index = atoi(buffer) - 1;

    Group *group = get_group_by_index(index);

    if (group != NULL)
    {
        aks_to_join_group(group);
    }
}

Participant *get_participant_by_username(Group *group, char *username)
{
    Participant *p = group->participants;
    while (p != NULL)
    {
        if (strcmp(p->username, username) == 0)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}