#ifndef MESSAGE_H
#define MESSAGE_H

#include "MQTTAsync.h"

typedef struct Messages
{
    char from[256];
    char topic[256];
    char payload[256];
    struct Messages *next;
} Messages;

typedef struct
{
    MQTTAsync client;
    char topic[100];
} SendContext;

int send_message(char msg[], char topic[]);
void add_unread_message(MQTTAsync_message *message, char topic[]);
void add_all_received_message(MQTTAsync_message *message, char topic[]);
void print_all_received_messages();

#endif