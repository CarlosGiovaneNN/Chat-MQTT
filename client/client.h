#ifndef CLIENT_H
#define CLIENT_H

#include "MQTTAsync.h"

int is_connected();

int init_client();

void close_client();

extern MQTTAsync client;

#endif