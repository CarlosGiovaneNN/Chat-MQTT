#ifndef GROUP_H
#define GROUP_H

#include "../headers.h"

// void create_group(Group **groups, char *group_name, char *leader);
// void add_participant(Group *group,  char *username);
// void save_groups(Group *groups);
int toggle_participant_status_file(Group *group, char *username);
void create_group_menu();
void list_groups();
void add_group_by_message(char *message);
void load_groups_from_file();
Group *get_group_by_name(char *group_name);
void change_participant_status(Group *group, char *username, int pending);

extern Group *groups;

#endif