#ifndef CHAT_H
#define CHAT_H

void show_chat_menu();
void load_chats_from_file();

char *create_chat(char *name, int is_group);

#endif