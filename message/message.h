#ifndef MESSAGE_H
#define MESSAGE_H

#include "MQTTAsync.h"

void print_all_received_messages();

void on_recv_message(MQTTAsync_message *message, char *topic);

void control_msg();

void read_pending_messages_control();

int send_message(char msg[], char topic[]);

#endif