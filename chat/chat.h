#ifndef CHAT_H
#define CHAT_H

void show_chat_menu();
void load_chats_from_file();
void show_message_from_user(char *date, char *msg);
void show_message_from_other(char *from, char *date, char *msg);
void subscribe_all_chats();
void show_chat_topbar(char *topic);

int add_private_chat(char *name, char *topic);

char *create_chat(char *name, int is_group);

extern char selected_chat[100];

#endif