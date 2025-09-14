#ifndef CLIENT_H
#define CLIENT_H

#include "MQTTAsync.h"

int initClient(char userId[]);

int isConnected();

extern MQTTAsync client;

#endif