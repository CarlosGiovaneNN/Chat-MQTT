#include "../headers.h"

#include "../chat/chat.h"
#include "../client/client.h"
#include "../group/group.h"
#include "../user/user.h"

Messages *unread_messages = NULL;
Messages *all_received_messages = NULL;
Messages *control_messages = NULL;

void on_send(void *context, MQTTAsync_successData *response);
void on_send_failure(void *context, MQTTAsync_failureData *response);

void add_message(Messages **array, pthread_mutex_t *mtx, char payload[], char topic[], char from[], int type,
                 char time[]);
void add_all_received_message(char *payload, char topic[], char from[], char time[]);
void add_unread_message(char *payload, char topic[], char from[], char time[]);
void add_control_message(char topic[], char from[], char msg[], int type, char time[]);
void clear_unread_messages();
void remove_control_message(int index);
void parse_message(char *message, char *from, char *date, char *msg);
void print_messages(Messages *messages, pthread_mutex_t *mtx);
void print_all_received_messages();
void print_unread_messages();
void read_pending_messages_control();
void control_msg();
void on_recv_message(MQTTAsync_message *message, char *topic);
void print_all_msgs_from_chat(char *topic);

char *format_message();

int send_message(char msg[], char topic[]);
int compare_time(char time1_str[], double limit_time);

Messages *get_control_message(int index);

// NAO ALTERADO
void on_send(void *context, MQTTAsync_successData *response)
{
    Send_Context *ctx = (Send_Context *)context;

    // if (strcmp(ctx->topic, "USERS") != 0)
    //  printf("Message delivered successfully\n");

    free(ctx);
}

// NAO ALTERADO
void on_send_failure(void *context, MQTTAsync_failureData *response)
{
    Send_Context *ctx = (Send_Context *)context;

    // printf("Failed to deliver message, rc %d\n", response->code);

    free(ctx);
}

// ADICIONA A MSG NO ARRAY - { ARRAY DE MENSAGENS, MUTEX DA ARRAY, MENSAGEM, TOPICO, NOME DO USUARIO, TIPO }
void add_message(Messages **array, pthread_mutex_t *mtx, char payload[], char topic[], char from[], int type,
                 char time[])
{
    Messages *new_message = malloc(sizeof(Messages));
    if (!new_message)
        return;

    strcpy(new_message->topic, topic);
    strcpy(new_message->payload, payload);
    strcpy(new_message->from, from);
    strcpy(new_message->time, time);

    new_message->next = NULL;
    new_message->type = type;

    pthread_mutex_lock(mtx);

    if (*array == NULL)
    {
        *array = new_message;
    }
    else
    {
        Messages *current = *array;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_message;
    }

    pthread_mutex_unlock(mtx);
}

// ADICIONA A MSG NO ARRAY DE MENSAGENS RECEBIDAS
void add_all_received_message(char *payload, char topic[], char from[], char time[])
{
    add_message(&all_received_messages, &mutex_all_received, payload, topic, from, MESSAGE_NORMAL, time);
}

// ADICIONA A MSG NO ARRAY DE MENSAGENS NAO LIDAS
void add_unread_message(char *payload, char topic[], char from[], char time[])
{
    add_message(&unread_messages, &mutex_unread, payload, topic, from, MESSAGE_NORMAL, time);
    add_all_received_message(payload, topic, from, time);
}

// ADICIONA A MSG NO ARRAY DE MENSAGENS DE CONTROLE
void add_control_message(char topic[], char from[], char msg[], int type, char time[])
{
    add_message(&control_messages, &mutex_control, msg, topic, from, type, time);
}

// LIMPA O ARRAY DE MENSAGENS NAO LIDAS
void clear_unread_messages()
{
    pthread_mutex_lock(&mutex_unread);

    Messages *curr = unread_messages;
    while (curr)
    {
        Messages *tmp = curr->next;

        free(curr);

        curr = tmp;
    }

    unread_messages = NULL;

    pthread_mutex_unlock(&mutex_unread);
}

// REMOVE A MSG NO ARRAY DE MENSAGENS DE CONTROLE
void remove_control_message(int index)
{
    pthread_mutex_lock(&mutex_control);

    if (control_messages == NULL)
    {
        pthread_mutex_unlock(&mutex_control);
        return;
    }

    if (index == 0)
    {
        Messages *aux = control_messages;
        control_messages = control_messages->next;
        free(aux);
        pthread_mutex_unlock(&mutex_control);
        return;
    }

    int count = 0;
    Messages *prev = NULL;
    Messages *cur = control_messages;
    while (cur)
    {
        if (count == index)
        {
            if (prev)
                prev->next = cur->next;
            free(cur);
            pthread_mutex_unlock(&mutex_control);
            return;
        }
        prev = cur;
        cur = cur->next;
        count++;
    }

    pthread_mutex_unlock(&mutex_control);
}

/* DIVIDE A MENSAGEM EM 3 PARTES -
{
    MSG DIRETA DO TOPICO,
    ENDERECO DO LOCAL PARA SALVAR:
    [
        NOME DO USUARIO,
        DATA,
        MENSAGEM
    ]
}
*/
void parse_message(char *message, char *from, char *date, char *msg)
{
    from[0] = date[0] = msg[0] = '\0';

    if (!message)
        return;

    char buffer[1024];
    strncpy(buffer, message, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token = strtok(buffer, "-");
    if (token)
        strncpy(from, token, 99);
    from[99] = '\0';

    token = strtok(NULL, "-");
    if (token)
        strncpy(date, token, 99);
    date[99] = '\0';

    token = strtok(NULL, "-");
    if (token)
        strncpy(msg, token, 99);
    msg[99] = '\0';
}

// PRINTA AS MENSAGENS NO FORMATO CORRETO - { ARRAY DE MENSAGENS, MUTEX DA ARRAY }
void print_messages(Messages *messages, pthread_mutex_t *mtx)
{
    pthread_mutex_lock(mtx);

    for (Messages *message = messages; message != NULL; message = message->next)
    {
        printf("====================================\n");
        printf("De:    %s\n", message->from);
        printf("Tópico: %s\n", message->topic);
        printf("------------------------------------\n");
        printf("%s\n", message->payload);
        printf("====================================\n\n");
    }

    if (messages == NULL)
    {
        printf("Nenhuma mensagem para exibir.\n");
    }

    pthread_mutex_unlock(mtx);
}

// PRINTA AS MENSAGENS RECEBIDAS
void print_all_received_messages()
{
    print_messages(all_received_messages, &mutex_all_received);
}

// PRINTA AS MENSAGENS NAO LIDAS
void print_unread_messages()
{
    print_messages(unread_messages, &mutex_unread);

    clear_unread_messages();
}

/*
    LE OS PARICIPANTES DE CADA GRUPO E VERIFICA SE EXISTE MENSAGENS DE CONTROLE PENDENTES
    MENSAGENS DE CONTROLE PENDENTES:
    SERIA A MENSAGEM PERDIDA PELO USUARIO QUE DECIDIU NAO RESPONDER AO RECEBIMENTO
*/
void read_pending_messages_control()
{
    pthread_mutex_lock(&mutex_groups);
    pthread_mutex_lock(&mutex_control);

    Group *current_group = groups;

    while (current_group != NULL)
    {
        Participant *p = get_participant_by_username(current_group, user_id);
        if (p && p->pending == 1)
        {

            Messages *msg_node = get_control_message(0);
            int found = 0;

            while (msg_node != NULL)
            {

                char *group_name = strstr(msg_node->payload, "grupo: ");

                if (group_name)
                {
                    group_name += strlen("grupo: ");
                }

                if (msg_node->type == MESSAGE_GROUP_INVITATION && strcmp(group_name, current_group->name) == 0)
                {
                    found = 1;
                    break;
                }
                msg_node = msg_node->next;
            }

            if (!found)
            {
                char new_msg[256];

                sprintf(new_msg, "Convidou voce para o grupo: %s", current_group->name);
                add_control_message(current_group->name, user_id, new_msg, MESSAGE_GROUP_INVITATION, "NULL");
            }
        }

        current_group = current_group->next;
    }

    pthread_mutex_unlock(&mutex_control);
    pthread_mutex_unlock(&mutex_groups);
}

// LISTA MENSAGENS DE CONTROLE E PERGUNTA QUAL DESEJA RESPONDER
void control_msg()
{
    int count = 1;

    pthread_mutex_lock(&mutex_control);

    if (control_messages == NULL)
    {
        printf("Nenhuma mensagem de controle pendente\n");
        pthread_mutex_unlock(&mutex_control);
        return;
    }

    printf("\n");

    for (Messages *message = control_messages; message != NULL; message = message->next)
    {
        printf("============================================\n");
        printf("Mensagem %d:\n", count);
        printf("%s\n", message->payload);
        printf("Enviada por: %s\n", message->from);
        printf("============================================\n\n");

        count++;
    }

    printf("0 - Voltar\n");
    printf("=======================================\n");
    printf("Selecione o número da mensagem que deseja responder:\n");

    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);

    if (buffer[0] == '0')
        return;

    int index = atoi(buffer) - 1;

    Messages *msg = get_control_message(index);

    if (msg == NULL)
    {
        printf("Mensagem não encontrada\n");
        return;
    }

    printf("\nVoce deseja aceitar essa mensagem? (s/n):\n");
    fgets(buffer, sizeof(buffer), stdin);
    char message[512];
    char topic[512];

    char *group_name = strstr(msg->payload, "grupo: ");

    if (group_name)
    {
        group_name += strlen("grupo: ");
    }

    if (buffer[0] == 's')
    {
        if (msg->type == MESSAGE_GROUP_INVITATION)
        {
            sprintf(message, "%d;%s;0;", GROUP_INVITATION_ACCEPTED, group_name);

            send_message(message, "GROUPS");

            create_chat(group_name, 1);

            if (!toggle_participant_status_file(get_group_by_name(group_name), user_id))
            {
                printf("Erro ao alterar status no arquivo de grupos\n");
            }
        }
        else if (msg->type == MESSAGE_CHAT_INVITATION)
        {
            sprintf(message, "%d;", IDCONTROL_CHAT_INVITATION_ACCEPTED);
            sprintf(topic, "%s_CONTROL", msg->from);

            send_message(message, topic);
        }
        else if (msg->type == MESSAGE_GROUP_ASK_TO_JOIN)
        {
            sprintf(message, "%d;%s;1;", GROUP_INVITATION_ACCEPTED, group_name);

            send_message(message, "GROUPS");

            if (!add_participant_to_group_file(group_name, msg->from, get_group_by_name(group_name), 0))
            {
                printf("Erro ao adicionar participante no arquivo de grupos\n");
            }
        }
    }
    else
    {
        if (msg->type == MESSAGE_GROUP_INVITATION)
        {
            sprintf(message, "%d;%s;", GROUP_INVITATION_REJECTED, group_name);

            remove_participant_from_group_file(group_name, user_id);

            pthread_mutex_lock(&mutex_groups);

            remove_participant_from_group(get_group_by_name(group_name), user_id);

            pthread_mutex_unlock(&mutex_groups);

            send_message(message, "GROUPS");
        }
        else if (msg->type == MESSAGE_CHAT_INVITATION)
        {
            sprintf(message, "%d;", IDCONTROL_CHAT_INVITATION_REJECTED);
            sprintf(topic, "%s_CONTROL", msg->from);

            send_message(message, topic);
        }
    }

    remove_control_message(index);

    pthread_mutex_unlock(&mutex_control);
}

// AO RECEBER MENSAGEM DE QUALQUER TOPICO
void on_recv_message(MQTTAsync_message *message, char *topic)
{
    // printf("%s\n", (char *)message->payload);
    char from[100] = {0}, date[100] = {0}, msg[100] = {0};
    parse_message((char *)message->payload, from, date, msg);

    if (strlen(from) == 0 || strlen(date) == 0 || strlen(msg) == 0)
        return;

    if (compare_time(date, 30) && (!strcmp(topic, "USERS") || !strcmp(topic, "GROUPS")))
    {
        return;
    }

    char id_control[100] = {0};
    sprintf(id_control, "%s_CONTROL", user_id);

    if (strcmp(topic, "USERS") == 0)
    {
        if (strcmp(user_id, from) == 0)
            return;

        add_user(from);

        if (check_status(msg) == 1)
        {
            change_status(from, 1, date);
        }
        else if (check_status(msg) == 0)
        {
            change_status(from, 0, NULL);
        }
    }
    else if (strcmp(topic, "GROUPS") == 0)
    {

        if (strcmp(user_id, from) == 0)
        {
            return;
        }

        if (strncmp(msg, "Group:", 6) == 0)
        {
            add_group_by_message(msg);
        }
        else
        {
            int option;
            char new_msg[256] = {0};
            char group_name[128] = {0};

            sscanf(msg, "%d;", &option);

            if (option == GROUP_INVITATION_ACCEPTED)
            {

                int fromAsk = 0;
                sscanf(msg, "%d;%[^;];%d;", &option, group_name, &fromAsk);
                sprintf(new_msg, "Aceitou o convite para o grupo: %s", group_name);

                add_unread_message(new_msg, topic, from, date);

                pthread_mutex_lock(&mutex_groups);

                Group *group = get_group_by_name(group_name);

                if (get_participant_by_username(group, from) == NULL || fromAsk == 1)
                {
                    add_participant(group, fromAsk ? user_id : from, 0);
                }
                else
                {
                    change_participant_status(group, from, 0);
                }

                pthread_mutex_unlock(&mutex_groups);
            }
            else if (option == GROUP_INVITATION_REJECTED)
            {
                sscanf(msg, "%d;%[^;];", &option, group_name);
                sprintf(new_msg, "Recusou o convite para o grupo: %s", group_name);
                add_unread_message(topic, from, new_msg, date);

                pthread_mutex_lock(&mutex_groups);

                Group *group = get_group_by_name(group_name);

                if (remove_participant_from_group(group, from) == 0)
                {
                    printf("Erro ao remover participante do grupo\n");
                }

                pthread_mutex_unlock(&mutex_groups);
            }
        }
    }
    else if (strcmp(topic, id_control) == 0)
    {

        if (strcmp(user_id, from) == 0)
            return;

        int option;
        char new_msg[256] = {0};

        sscanf(msg, "%d;", &option);

        if (option == IDCONTROL_GROUP_INVITATION)
        {
            char group_name[100];
            sscanf(msg, "%d;%[^;];", &option, group_name);

            sprintf(new_msg, "Convidou voce para o grupo: %s", group_name);

            add_control_message(topic, from, new_msg, MESSAGE_GROUP_INVITATION, date);
        }
        else if (option == IDCONTROL_CHAT_INVITATION)
        {
            add_control_message(topic, from, "Pede para voce entrar no chat", MESSAGE_CHAT_INVITATION, date);
        }
        else if (option == IDCONTROL_CHAT_INVITATION_ACCEPTED)
        {
            sprintf(new_msg, "%s aceitou o convite para o chat", from);
            add_unread_message(topic, from, new_msg, date);

            char topic[128] = {0};
            char new_chat[100] = {0};

            strcpy(new_chat, create_chat(from, 0));

            sprintf(new_msg, "%d;%s;", IDCONTROL_ADD_PRIVATE_CHAT, new_chat);

            sprintf(topic, "%s_CONTROL", from);

            send_message(new_msg, topic);
        }
        else if (option == IDCONTROL_CHAT_INVITATION_REJECTED)
        {
            sprintf(new_msg, "%s recusou o convite para o chat", from);
            add_unread_message(topic, from, new_msg, date);
        }
        else if (option == IDCONTROL_GROUP_ASK_TO_JOIN)
        {
            char group_name[100] = {0};
            printf("Group name: antes\n");
            sscanf(msg, "%d;%[^;];", &option, group_name);
            printf("Group name: %s\n", group_name);

            sprintf(new_msg, "Pede permissao para entrar no grupo: %s", group_name);

            add_control_message(topic, from, new_msg, MESSAGE_GROUP_ASK_TO_JOIN, date);
        }
        else if (option == IDCONTROL_ADD_PRIVATE_CHAT)
        {
            char new_private_chat_topic[128];

            sscanf(msg, "%d;%[^;];", &option, new_private_chat_topic);

            add_private_chat(from, new_private_chat_topic);
        }
    }
    else
    {
        if (strcmp(topic, selected_chat) != 0)
        {
            if (strcmp(user_id, from) == 0)
                return;

            add_unread_message(msg, topic, from, date);
        }
        else
        {
            add_all_received_message(msg, topic, from, date);

            if (strcmp(user_id, from) == 0)
                return;

            show_message_from_other(from, date, msg);
        }
    }
}

// PRINTA TODAS AS MENSAGENS DO CHAT
void print_all_msgs_from_chat(char *topic)
{
    pthread_mutex_lock(&mutex_all_received);

    Messages *current = all_received_messages;

    show_chat_topbar(topic);

    while (current != NULL)
    {
        if (strcmp(current->topic, topic) == 0)
        {
            if (strcmp(current->from, user_id) == 0)
            {
                show_message_from_user(current->time, current->payload);
            }
            else
            {
                show_message_from_other(current->from, current->time, current->payload);
            }
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex_all_received);
}

// FORMATA A MENSAGEM COM O FORMATO CORRETO { USUARIO - DD/MM/AAAA HH:MM:SS - MENSAGEM }
char *format_message()
{
    time_t now;
    struct tm *time_info;

    // 01/01/1970
    time(&now);
    time_info = localtime(&now);

    char *message = malloc(128);
    if (!message)
        return NULL;

    sprintf(message, "%s-%02d/%02d/%04d %02d:%02d:%02d-", user_id, time_info->tm_mday, time_info->tm_mon + 1,
            time_info->tm_year + 1900, time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

    return message;
}

// ENVIA A MENSAGEM
int send_message(char msg[], char topic[])
{
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    char *prefix = format_message();
    if (!prefix)
        return EXIT_FAILURE;

    // --- COMECO DO GPT ---
    size_t total_len = strlen(prefix) + strlen(msg) + 1;

    char *payload = malloc(total_len);
    if (!payload)
    {
        free(prefix);
        // printf("Erro ao alocar payload para send_message \n");
        return EXIT_FAILURE;
    }

    strcpy(payload, prefix);
    strcat(payload, msg);

    free(prefix);

    // --- FIM DO GPT ---

    unsigned char encrypted_payload[1024];
    int enc_len = aes_encrypt((unsigned char *)payload, strlen(payload), encrypted_payload);

    free(payload);

    if (enc_len > sizeof(encrypted_payload))
    {
        enc_len = sizeof(encrypted_payload);
    }

    size_t flag_payload_len = 4 + enc_len;
    char *flaged_payload = malloc(flag_payload_len + 1);
    if (!flaged_payload)
    {
        // printf("Erro ao alocar flaged_payload\n");
        return EXIT_FAILURE;
    }

    memcpy(flaged_payload, "ENC:", 4);
    memcpy(flaged_payload + 4, encrypted_payload, enc_len);
    flaged_payload[flag_payload_len] = '\0';

    pubmsg.payload = flaged_payload;
    pubmsg.payloadlen = 4 + enc_len;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    Send_Context *ctx = malloc(sizeof(Send_Context));
    ctx->client = client;
    strcpy(ctx->topic, topic);

    opts.onSuccess = on_send;
    opts.onFailure = on_send_failure;
    opts.context = ctx;

    if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        free(ctx);
        free(flaged_payload);
        return EXIT_FAILURE;
    }

    free(flaged_payload);

    return EXIT_SUCCESS;
}

// COMPARA O TEMPPO ENVIADO COM O DE AGORA
int compare_time(char time1_str[], double limit_time)
{
    time_t time2_now = time(NULL);

    struct tm last_seen_tm = {0};
    int day, mon, year, hour, min, sec;

    if (sscanf(time1_str, "%d/%d/%d %d:%d:%d", &day, &mon, &year, &hour, &min, &sec) == 6)
    {
        last_seen_tm.tm_mday = day;
        last_seen_tm.tm_mon = mon - 1;
        last_seen_tm.tm_year = year - 1900;
        last_seen_tm.tm_hour = hour;
        last_seen_tm.tm_min = min;
        last_seen_tm.tm_sec = sec;
        last_seen_tm.tm_isdst = -1;

        time_t last_seen_time_t = mktime(&last_seen_tm);

        if (last_seen_time_t == (time_t)-1)
        {
            return 0;
        }

        double difference = difftime(time2_now, last_seen_time_t);

        if (difference > limit_time)
        {
            return 1;
        }
    }
    return 0;
}

// OBTEM A MENSAGEM DE CONTROLE PELO INDICE
Messages *get_control_message(int index)
{
    pthread_mutex_lock(&mutex_control);

    int count = 0;

    for (Messages *message = control_messages; message != NULL; message = message->next)
    {
        if (count == index)
        {
            pthread_mutex_unlock(&mutex_control);
            return message;
        }
        count++;
    }

    pthread_mutex_unlock(&mutex_control);

    return NULL;
}
