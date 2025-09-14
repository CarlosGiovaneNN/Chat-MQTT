#ifndef CLIENT_H
#define CLIENT_H

#include "MQTTAsync.h"

int initClient(char userId[]);

int isConnected();

void closeClient();

extern MQTTAsync client;

#endif