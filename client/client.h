#ifndef CLIENT_H
#define CLIENT_H

#include "MQTTAsync.h"

int init_client(char user_id[]);

int is_connected();

void close_client();

extern MQTTAsync client;

#endif