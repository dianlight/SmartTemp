#pragma once
#include <Ticker.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "EvoStartableInterface.h"



class MQTTforHA:public EvoStopable {
    public:
        MQTTforHA();

        bool sendMQTTState();
        void sendMQTTAvail(bool online);

        int state(){ return client.state();};

        bool start();
        bool stop();

    private:

        Ticker mqttTicker;

        time_t lastRegister, lastRetry = 0;

        WiFiClient espClient;
        PubSubClient client{espClient};

        void loopMQTT();

        void callback(char* topic, byte* payload, unsigned int length);
        bool reconnect();
};

extern MQTTforHA mQTTforHA;
