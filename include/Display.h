#pragma once
#include <Stream.h>
#include <U8g2lib.h>
#include <Ticker.h>
#include <MQTTforHA.h>

#include "at8i2cGateway.h"  
#include "Thermostat.h"
#include "Network.h"
#include "Config.h"

#define U8_Width 128
#define U8_Height 64

const static std::string DAYSNAME[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

class Display {
    public:

        Display(Thermostat &thermostat, Config &myConfig, AT8I2CGATEWAY &at8gw, TimeNTPClient &timeNTPClient);
        void clearDisplay();
        void bootAPDisplay(String AP);
        void bootConnectedDisplay();
        void displayError(String Error);  
        void displayProgress(u8 perc,String type);

        void screeshot(Print &p);
        void screeshotbmp(Print &p);
        void loopDisplay();

        void setMQTTforHA(MQTTforHA *mqttforHA){ _mqttforHA = mqttforHA; }

    private:
        typedef byte u8g2_Bitmap[U8_Height][U8_Width];

        U8G2 _u8g2;
        MQTTforHA *_mqttforHA = NULL;
        Thermostat &thermostat;
        Config &myConfig;
        TimeNTPClient &timeNTPClient;
        AT8I2CGATEWAY &at8gw;

        Ticker sleepModeTicker;

        int8_t lastEncPosition =0;

        void sleepModeDisplay();

};
