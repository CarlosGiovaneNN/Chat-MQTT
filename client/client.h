#ifndef CLIENT_H
#define CLIENT_H

#include "MQTTAsync.h"

int init_client();

int is_connected();

void close_client();

extern MQTTAsync client;

#endif