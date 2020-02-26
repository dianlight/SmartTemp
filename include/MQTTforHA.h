#pragma once
#include <Ticker.h>
#include <WiFiClient.h>
#include <PubSubClient.h>


#include "Thermostat.h"
#include "Config.h"
#include "at8i2cGateway.h"
#include "TimeNTPClient.h"

class MQTTforHA {
    public:
        MQTTforHA(Thermostat &thermostat,Config &myConfig,AT8I2CGATEWAY &at8gw);

        bool sendMQTTState();
        void sendMQTTAvail(bool online);

        int state(){ return client.state();};

    private:

        Ticker mqttTicker;

        Thermostat &thermostat;
        Config &myConfig;
        AT8I2CGATEWAY &at8gw;

        time_t lastRegister, lastRetry = 0;

        WiFiClient espClient;
        PubSubClient client;

        void loopMQTT();

        void callback(char* topic, byte* payload, unsigned int length);
        bool reconnect();
};
