#ifndef HEADERS_H
#define HEADERS_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "MQTTAsync.h"

#define ADDRESS "tcp://localhost:1883"
#define QOS 1

extern pthread_t thread_status, thread_shell;

#endif