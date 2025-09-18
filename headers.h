#ifndef HEADERS_H
#define HEADERS_H

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "MQTTAsync.h"

#define ADDRESS "tcp://localhost:1883"
#define QOS 2

#define GROUP_INVITATION_TYPE 1
#define GROUP_INVITATION_ACCEPTED_TYPE 2
#define GROUP_INVITATION_REJECTED_TYPE 3
#define USER_CHAT_INVITATION_TYPE 4
#define USER_CHAT_INVITATION_ACCEPTED_TYPE 5
#define USER_CHAT_INVITATION_REJECTED_TYPE 6

typedef struct Participant
{
    char username[100];
    int pending; // 1 = pendente, 0 = confirmado
    struct Participant *next;
} Participant;

typedef struct Group
{
    char name[100];
    char leader[100];
    Participant *participants;
    struct Group *next;
} Group;

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
} Send_Context;

typedef struct Users
{
    char username[100];
    int online;
    struct Users *next;
} Users;

extern pthread_t thread_status, thread_shell;

#endif