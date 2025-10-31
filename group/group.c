#include "../headers.h"

#include "../chat/chat.h"
#include "../message/message.h"
#include "../user/user.h"

Group *groups = NULL;

void create_group(char *group_name, char *leader);
void add_participant(Group *group, char *username, int pending);
void save_group(Group *g);
void change_participant_status(Group *group, char *username, int pending);
void add_group_by_message(char *message);
void create_group_menu();
void load_groups_from_file();
void list_groups();
void aks_to_join_group(Group *group);
void join_group_menu();

int toggle_participant_status_file(Group *group, char *username);
int add_participant_to_group_file(char *group_name, char *username, Group *group, int pending);
int remove_participant_from_group_file(char *group_name, char *username);
int remove_participant_from_group(Group *group, char *username);

Group *get_group_by_index(int index);
Group *get_group_by_name(char *group_name);

Participant *get_participant_by_username(Group *group, char *username);

// CRIA UM NOVO GRUPO E JA ADICIONA AO CHAT - X
void create_group(char *group_name, char *leader)
{
    Group *new_group = malloc(sizeof(Group));

    if (!new_group)
        return;

    strcpy(new_group->name, group_name);
    strcpy(new_group->leader, leader);
    new_group->participants = NULL;

    printf("ficou aqui 3\n");

    // LOCK Q ACONTECE O LOCK
    pthread_mutex_lock(&mutex_groups);

    printf("ficou aqui 4\n");

    new_group->next = groups;
    groups = new_group;

    pthread_mutex_unlock(&mutex_groups);

    create_chat(group_name, 1);
}

// ADICIONA UM PARTICIPANTE AO GRUPO - X
void add_participant(Group *group, char *username, int pending)
{
    Participant *p = malloc(sizeof(Participant));
    if (!p)
        return;

    strcpy(p->username, username);
    p->pending = pending;
    p->next = NULL;

    pthread_mutex_lock(&mutex_groups);

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

    if (strcmp(user_id, username) == 0)
    {
        create_chat(group->name, 1);
    }

    pthread_mutex_unlock(&mutex_groups);
}

// SALVA O GRUPO NO ARQUIVO - X
void save_group(Group *g)
{
    FILE *f = fopen(FILE_GROUPS, "a");
    if (!f)
        return;

    fprintf(f, "Group: %s\n", g->name);
    fprintf(f, "Leader: %s\n", g->leader);
    fprintf(f, "Participants:\n");

    pthread_mutex_lock(&mutex_groups);

    for (Participant *p = g->participants; p != NULL; p = p->next)
    {
        fprintf(f, "- %s (%s)\n", p->username, p->pending ? "pending" : "active");
    }

    pthread_mutex_unlock(&mutex_groups);

    fprintf(f, "\n");

    fclose(f);
}

// ALTERA O STATUS DO PARTICIPANTE - X
void change_participant_status(Group *group, char *username, int pending)
{
    pthread_mutex_lock(&mutex_groups);

    Participant *p = group->participants;
    while (p)
    {
        if (strcmp(p->username, username) == 0)
        {
            p->pending = pending;

            pthread_mutex_unlock(&mutex_groups);

            return;
        }
        p = p->next;
    }

    pthread_mutex_unlock(&mutex_groups);
}

// ADICIONA UM GRUPO NA LISTA A PARTIR DE UMA MENSAGEM NO TOPICO 'GROUPS' - X
void add_group_by_message(char *message)
{
    if (!message || strlen(message) == 0)
        return;

    Group *group = malloc(sizeof(Group));
    if (!group)
        return;

    pthread_mutex_lock(&mutex_groups);
    group->participants = NULL;

    char *ptr = strstr(message, "Group:");
    if (ptr)
    {
        ptr += strlen("Group:");
        char *end = strstr(ptr, ";");
        if (end)
        {
            while (end > ptr && isspace(*(end - 1)))
                end--;
            size_t len = end - ptr;
            strncpy(group->name, ptr, len);
            group->name[len] = 0;
        }
    }

    ptr = strstr(message, "Leader:");
    if (ptr)
    {
        ptr += strlen("Leader:");
        char *end = strstr(ptr, ";");
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

    pthread_mutex_unlock(&mutex_groups);
}

// CRIA A MENSAGEM DO NOVO GRUPO CRIADO PARA O TOPICO 'GROUPS' - X
void create_group_message(Group *group)
{

    pthread_mutex_lock(&mutex_groups);

    char group_name[254];
    snprintf(group_name, sizeof(group_name), "Group:%s;", group->name);

    char leader[254];
    snprintf(leader, sizeof(leader), "Leader:%s;Participants:", group->leader);

    char participants[1024] = "[";
    for (Participant *p = group->participants; p != NULL; p = p->next)
    {
        strncat(participants, p->username, sizeof(participants) - strlen(participants) - 1);
        strncat(participants, ";", sizeof(participants) - strlen(participants) - 1);
    }
    strncat(participants, "];", sizeof(participants) - strlen(participants) - 1);

    pthread_mutex_unlock(&mutex_groups);

    size_t len = strlen(group_name) + strlen(leader) + strlen(participants) + 1;
    char *message = malloc(len);
    if (!message)
        return;

    snprintf(message, len, "%s%s%s", group_name, leader, participants);

    send_message(message, "GROUPS");
    free(message);
}

// MENU PARA CRIAR UM NOVO GRUPO - x
void create_group_menu()
{
    printf("\nDigite o nome do grupo: ");

    char group_name[100];

    if (!fgets(group_name, sizeof(group_name), stdin))
        return;

    group_name[strcspn(group_name, "\n")] = 0;

    if (strlen(group_name) == 0)
    {
        printf("Erro: O nome do grupo não pode ser vazio.\n");

        return;
    }

    for (int i = 0; group_name[i] != '\0'; i++)
    {
        if (group_name[i] == ' ')
        {
            group_name[i] = '_';
        }
    }

    char *prohibited_chars = ";:[]";
    if (strpbrk(group_name, prohibited_chars) != NULL)
    {
        printf("Erro: O nome do grupo não pode conter os caracteres ';', ':', '[', ']'.\n");

        return;
    }

    char *prohibited_words[] = {
        "group",
        "leader",
        "participant",
    };

    int num_prohibited = sizeof(prohibited_words) / sizeof(prohibited_words[0]);

    for (int i = 0; i < num_prohibited; i++)
    {
        if (strcasecmp(group_name, prohibited_words[i]) == 0)
        {
            printf("Erro: '%s' é um nome de grupo proibido.\n", prohibited_words[i]);

            return;
        }
    }

    pthread_mutex_lock(&mutex_groups);

    if (get_group_by_name(group_name))
    {
        printf("\nO grupo %s ja existe\n\n", group_name);

        pthread_mutex_unlock(&mutex_groups);

        return;
    }

    pthread_mutex_unlock(&mutex_groups);

    printf("\nDigite o numero dos participantes (Digite qualquer tecla para cancelar - menos numero): ");
    printf("\nSepare-os por espaços (ex: 1 2 3)\n");

    int participant_number = 0;

    pthread_mutex_lock(&mutex_users);

    if (users == NULL)
    {
        printf("Nenhum usuario encontrado\n\n\n");

        pthread_mutex_unlock(&mutex_users);

        return;
    }

    for (Users *current = users; current != NULL; current = current->next)
    {
        printf("%d - %s\n", participant_number, current->username);
        participant_number++;
    }

    pthread_mutex_unlock(&mutex_users);

    char line[256];
    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        return;
    }

    line[strcspn(line, "\n")] = 0;

    const char *caracteres_permitidos = "0123456789 ";

    size_t len_permitida = strspn(line, caracteres_permitidos);
    size_t len_total = strlen(line);

    if (len_permitida != len_total)
    {
        return;
    }

    printf("ficou aqui 1\n");

    pthread_mutex_lock(&mutex_chats);
    pthread_mutex_lock(&mutex_groups);

    printf("ficou aqui 2\n");

    create_group(group_name, user_id);

    pthread_mutex_unlock(&mutex_groups);
    pthread_mutex_unlock(&mutex_chats);

    pthread_mutex_lock(&mutex_groups);
    pthread_mutex_lock(&mutex_users);

    printf("possou aqui\n");

    char *token = strtok(line, " ");
    while (token != NULL)
    {
        int index = atoi(token);

        if (index < 0 || index >= participant_number)
        {
            pthread_mutex_unlock(&mutex_users);
            pthread_mutex_unlock(&mutex_groups);

            printf("Indice invalido\n");
            return;
        }

        Users *participant = get_user_by_index(index);

        if (participant)
        {
            Group *new_group = get_group_by_name(group_name);

            if (!new_group)
            {
                pthread_mutex_unlock(&mutex_users);
                pthread_mutex_unlock(&mutex_groups);

                return;
            }

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

    save_group(get_group_by_name(group_name));
    printf("\nGrupo criado com sucesso!\n\n\n");

    create_group_message(get_group_by_name(group_name));

    pthread_mutex_unlock(&mutex_users);
    pthread_mutex_unlock(&mutex_groups);
}

// CARREGA OS GRUPOS DO ARQUIVO - x
void load_groups_from_file()
{
    FILE *f = fopen(FILE_GROUPS, "r");
    if (!f)
        return;

    char line[256];
    Group *current_group = NULL;

    pthread_mutex_lock(&mutex_groups);

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

    pthread_mutex_lock(&mutex_groups);

    fclose(f);
}

// LISTA OS GRUPOS - X
void list_groups()
{
    if (groups == NULL)
    {
        printf("Nenhum grupo criado.\n\n");
        return;
    }

    printf("Lista de grupos:\n\n");

    pthread_mutex_lock(&mutex_groups);

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

    pthread_mutex_unlock(&mutex_groups);
}

// MONTA MENSAGEM DE PEDIDO DE ACESSO AO GRUPO - X
void aks_to_join_group(Group *group)
{
    char message[256];
    char topic[256];
    snprintf(message, sizeof(message), "%d;%s;", IDCONTROL_GROUP_ASK_TO_JOIN, group->name);
    snprintf(topic, sizeof(topic), "%s_CONTROL", group->leader);

    send_message(message, topic);
}

// MENU PARA PEDIR ACESSO AO GRUPO - X
void join_group_menu()
{
    printf("\nGrupos disponíveis para entrar:\n");
    printf("-------------------------------\n");

    Group *available_groups[256];
    int count = 1;

    pthread_mutex_lock(&mutex_groups);

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
            printf("  %d - %s\n", count, current_group->name);

            available_groups[count] = current_group;

            count++;
        }

        current_group = current_group->next;
    }

    pthread_mutex_unlock(&mutex_groups);

    if (count == 1)
    {
        printf("Nenhum grupo disponível.\n");
    }

    printf("\n0 - Voltar\n");
    printf("-------------------------------\n");
    printf("\nSelecione o número do grupo que deseja entrar:\n");

    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);

    int choice = atoi(buffer);

    if (choice == 0)
    {
        return;
    }

    if (choice < 1 || choice >= count)
    {
        printf("Índice inválido.\n");
        return;
    }

    Group *group = available_groups[choice];

    if (group != NULL)
    {
        aks_to_join_group(group);
        printf("Pedido enviado para se juntar ao grupo '%s'.\n", group->name);
    }
}

// ALTERA O STATUS DO PARTICIPANTE NO ARQUIVO E NA LISTA - X
int toggle_participant_status_file(Group *group, char *username)
{
    if (!group)
        return 0;

    pthread_mutex_lock(&mutex_groups);

    Participant *p = group->participants;
    while (p)
    {
        if (strcmp(p->username, username) == 0)
        {
            p->pending = !p->pending;

            FILE *in = fopen(FILE_GROUPS, "r");
            if (!in)
            {
                pthread_mutex_unlock(&mutex_groups);

                return 0;
            }

            FILE *out = fopen("tmpGpStatus.txt", "w");
            if (!out)
            {
                pthread_mutex_unlock(&mutex_groups);

                fclose(in);
                return 0;
            }

            char line[256];

            while (fgets(line, sizeof(line), in))
            {
                line[strcspn(line, "\r\n")] = 0;

                if (strncmp(line, "- ", 2) == 0)
                {
                    char name_in_line[100];
                    char status[20];

                    if (sscanf(line, "- %99s (%19[^)])", name_in_line, status) == 2)
                    {
                        if (strcmp(name_in_line, username) == 0)
                        {
                            if (strcmp(status, "pending") == 0)
                            {
                                fprintf(out, "- %s (active)\n", username);
                            }
                            else if (strcmp(status, "active") == 0)
                            {
                                fprintf(out, "- %s (pending)\n", username);
                            }
                            else
                            {
                                fprintf(out, "%s\n", line);
                            }
                            continue;
                        }
                    }
                }

                fprintf(out, "%s\n", line);
            }

            fclose(in);
            fclose(out);

            remove(FILE_GROUPS);
            rename("tmpGpStatus.txt", FILE_GROUPS);

            pthread_mutex_unlock(&mutex_groups);

            return 1;
        }
        p = p->next;
    }

    pthread_mutex_unlock(&mutex_groups);

    return 0;
}

// ADICIONA PARTICIPANTE AO GRUPO NO ARQUIVO - RETORNA 1 SE O PARTICIPANTE FOI ADICIONADO - X
int add_participant_to_group_file(char *group_name, char *username, Group *group, int pending)
{
    FILE *in = fopen(FILE_GROUPS, "r");
    if (!in)
    {
        perror("Erro ao abrir group.txt");
        return 0;
    }

    FILE *out = fopen("tmpGroup.txt", "w");
    if (!out)
    {
        perror("Erro ao criar tmpGroup.txt");
        fclose(in);
        return 0;
    }

    char line[256];
    int inside_target_group = 0;
    int user_added = 0;
    int group_found = 0;
    int found_participants_line = 0;

    while (fgets(line, sizeof(line), in))
    {
        if (strncmp(line, "Group: ", 7) == 0)
        {
            if (inside_target_group && !user_added)
            {
                fprintf(out, "- %s (%s)\n", username, pending ? "pending" : "active");
                user_added = 1;
            }

            inside_target_group = 0;
            found_participants_line = 0;

            char current_group[100];
            sscanf(line + 7, "%[^\n]", current_group);
            if (strcmp(current_group, group_name) == 0)
            {
                inside_target_group = 1;
                group_found = 1;
            }
        }
        else if (inside_target_group && strncmp(line, "Participants:", 12) == 0)
        {
            found_participants_line = 1;
        }
        else if (inside_target_group && found_participants_line && strncmp(line, "- ", 2) != 0)
        {
            if (!user_added)
            {
                fprintf(out, "- %s (%s)\n", username, pending ? "pending" : "active");
                user_added = 1;
            }
            found_participants_line = 0;
        }

        fputs(line, out);
    }

    if (inside_target_group && !user_added)
    {
        fprintf(out, "- %s (%s)\n", username, pending ? "pending" : "active");
        user_added = 1;
    }

    fclose(in);
    fclose(out);

    add_participant(group, username, pending);

    remove(FILE_GROUPS);
    rename("tmpGroup.txt", FILE_GROUPS);

    return group_found;
}

// REMOVE PARTICIPANTE DO GRUPO NO ARQUIVO - X
int remove_participant_from_group_file(char *group_name, char *username)
{
    FILE *in = fopen(FILE_GROUPS, "r");
    if (!in)
    {
        perror("Erro ao abrir group.txt");
        return 0;
    }

    FILE *out = fopen("tmpGroupRemovePartc.txt", "w");
    if (!out)
    {
        perror("Erro ao criar tmpGroupRemovePartc.txt");
        fclose(in);
        return 0;
    }

    char line[256];
    int inside_group = 0;
    int group_found = 0;

    while (fgets(line, sizeof(line), in))
    {
        // remove \n ou \r\n do final
        line[strcspn(line, "\r\n")] = 0;

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

        if (inside_group && strstr(line, username))
        {
            continue;
        }

        fprintf(out, "%s\n", line);
    }

    fclose(in);
    fclose(out);

    remove(FILE_GROUPS);
    rename("tmpGroupRemovePartc.txt", FILE_GROUPS);

    return group_found;
}

// REMOVE PARTICIPANTE DO GRUPO - X
int remove_participant_from_group(Group *group, char *username)
{
    pthread_mutex_lock(&mutex_groups);

    Participant *p = group->participants;
    Participant *prev = NULL;
    while (p)
    {
        if (strcmp(p->username, username) == 0)
        {
            if (prev)
            {
                prev->next = p->next;
            }
            else
            {
                group->participants = p->next;
            }

            free(p);
            pthread_mutex_unlock(&mutex_groups);

            return 1;
        }

        prev = p;
        p = p->next;
    }

    pthread_mutex_unlock(&mutex_groups);

    return 0;
}

// RETORNA O GRUPO POR INDICE - X
Group *get_group_by_index(int index)
{
    int count = 0;

    pthread_mutex_lock(&mutex_groups);

    for (Group *current = groups; current != NULL; current = current->next)
    {
        if (count == index)
        {
            pthread_mutex_unlock(&mutex_groups);

            return current;
        }
        count++;
    }

    pthread_mutex_unlock(&mutex_groups);

    return NULL;
}

// RETORNA O GRUPO POR NOME - X
Group *get_group_by_name(char *group_name)
{
    pthread_mutex_lock(&mutex_groups);

    for (Group *g = groups; g != NULL; g = g->next)
    {
        if (strcmp(g->name, group_name) == 0)
        {

            pthread_mutex_unlock(&mutex_groups);

            return g;
        }
    }

    pthread_mutex_unlock(&mutex_groups);

    return NULL;
}

// RETORNA O PARTICIPANTE DO GRUPO POR NOME - X
Participant *get_participant_by_username(Group *group, char *username)
{
    pthread_mutex_lock(&mutex_groups);

    Participant *p = group->participants;
    while (p != NULL)
    {
        if (strcmp(p->username, username) == 0)
        {
            pthread_mutex_unlock(&mutex_groups);

            return p;
        }
        p = p->next;
    }

    pthread_mutex_unlock(&mutex_groups);

    return NULL;
}
