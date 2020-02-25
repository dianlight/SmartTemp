#pragma once


void setupMQTT();
void loopMQTT();
bool sendMQTTState();
void sendMQTTAvail(bool online);
