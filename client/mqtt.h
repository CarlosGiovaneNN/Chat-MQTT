#ifndef MQTT_H
#define MQTT_H

#include "MQTTAsync.h"

void onConnect(void *context, MQTTAsync_successData5 *response);
void onConnectFailure(void *context, MQTTAsync_failureData5 *response);

void onSubscribe(void *context, MQTTAsync_successData5 *response);
void onSubscribeFailure(void *context, MQTTAsync_failureData5 *response);

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);

void connlost(void *context, char *cause);

#endif