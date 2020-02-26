#pragma once
#include <Ticker.h>

#include "Thermostat.h"
#include "Config.h"
#include "at8i2cGateway.h"

class MQTTforHA {
    public:
        MQTTforHA(Thermostat &thermostat,Config &myConfig,AT8I2CGATEWAY &at8gw);

        bool sendMQTTState();
        void sendMQTTAvail(bool online);

    private:

        Ticker mqttTicker;

        Thermostat &thermostat;
        Config &myConfig;
        AT8I2CGATEWAY &at8gw;

        time_t lastRegister, lastRetry = 0;

        void loopMQTT();

        void callback(char* topic, byte* payload, unsigned int length);
        bool reconnect();
};
