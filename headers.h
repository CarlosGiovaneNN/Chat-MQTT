#ifndef HEADERS_H
#define HEADERS_H

#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "MQTTAsync.h"

#include "./AES/aes.h"

#define ADDRESS "tcp://localhost:1883"
#define QOS 1

// AES
#define AES_KEY ((unsigned char *)"12345678901234567890123456789012")
#define AES_IV ((unsigned char *)"abcdefghijklmnop")

// Types of messages
#define MESSAGE_NORMAL 0
#define MESSAGE_GROUP_INVITATION 1
#define MESSAGE_CHAT_INVITATION 2
#define MESSAGE_GROUP_ASK_TO_JOIN 3

// Topic = ID_CONTROL
#define IDCONTROL_GROUP_INVITATION 1
#define IDCONTROL_CHAT_INVITATION 2
#define IDCONTROL_CHAT_INVITATION_ACCEPTED 3
#define IDCONTROL_CHAT_INVITATION_REJECTED 4
#define IDCONTROL_GROUP_ASK_TO_JOIN 5
#define IDCONTROL_ADD_PRIVATE_CHAT 6

// Topic = GROUPS
#define GROUP_INVITATION_ACCEPTED 1
#define GROUP_INVITATION_REJECTED 2

// Files
#define FILE_CHATS "chat/chat.txt"
#define FILE_USERS "user/users.txt"
#define FILE_GROUPS "group/groups.txt"

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
    int type;
    char time[100];
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

typedef struct Chat
{
    char topic[100];
    int is_group;              // 0 = private, 1 = group
    char to[100];              // private = username, group = group name
    Participant *participants; // users in the group
    struct Chat *next;
} Chat;

typedef enum
{
    ITEM_USER,
    ITEM_GROUP
} ItemType;

typedef struct MenuItem
{
    ItemType type;
    void *ptr;
} MenuItem;

// THREADS
extern pthread_t thread_status, thread_shell;

// MUTEXES
extern pthread_mutex_t mutex_unread, mutex_all_received, mutex_control;
extern pthread_mutex_t mutex_groups;
extern pthread_mutex_t mutex_users;
extern pthread_mutex_t mutex_chats;

#endif