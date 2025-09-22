#ifndef CHAT_H
#define CHAT_H

void show_chat_menu();
char *create_chat(char *name, int is_group);
void load_chats_from_file();

#endif