#ifndef GROUP_H
#define GROUP_H

#include "../headers.h"

void list_groups();

void add_group_by_message(char *message);
void add_participant(Group *group, char *username, int pending);

void load_groups_from_file();

void change_participant_status(Group *group, char *username, int pending);

void join_group_menu();

void create_group_menu();

int add_participant_to_group_file(char *group_name, char *username, Group *group, int pending);

int toggle_participant_status_file(Group *group, char *username);

Group *get_group_by_name(char *group_name);

Participant *get_participant_by_username(Group *group, char *username);

extern Group *groups;

#endif