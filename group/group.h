#ifndef GROUP_H
#define GROUP_H

typedef struct Participant
{
    char username[100];
    int pending; // 1 = pendente, 0 = confirmado
    struct Participant *next;
} Participant;

typedef struct Group
{
    char name[100];
    char leader[100];
    Participant *participants;
    struct Group *next;
} Group;

// void create_group(Group **groups, char *group_name, char *leader);
// void add_participant(Group *group,  char *username);
// void save_groups(Group *groups);
int toggle_participant_status(Group *group, char *username);
void create_group_menu();
void list_groups();

#endif