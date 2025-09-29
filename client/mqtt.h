#ifndef MQTT_H
#define MQTT_H

#include "MQTTAsync.h"

void on_connect(void *context, MQTTAsync_successData *response);
void on_connect_failure(void *context, MQTTAsync_failureData *response);

void on_subscribe(void *context, MQTTAsync_successData *response);
void on_subscribe_failure(void *context, MQTTAsync_failureData *response);

void connlost(void *context, char *cause);

int msgarrvd(void *context, char *topic_name, int topicLen, MQTTAsync_message *message);
int subscribe_topic(char *topic);

#endif