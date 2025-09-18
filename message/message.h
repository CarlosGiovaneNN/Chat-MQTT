#ifndef MESSAGE_H
#define MESSAGE_H

#include "MQTTAsync.h"
// #include "../headers.h"

int send_message(char msg[], char topic[]);
void add_unread_message(MQTTAsync_message *message, char topic[]);
void add_all_received_message(MQTTAsync_message *message, char topic[]);
void print_all_received_messages();

#endif