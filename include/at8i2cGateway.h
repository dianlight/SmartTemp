#pragma once

//#include <Ticker.h>
#include "EvoStartableInterface.h"


class AT8I2CGATEWAY: public EvoStopable {
    public:
        AT8I2CGATEWAY(int i2caddress);
        float   getHumidity();
        float   getTemperature();
        int8_t  getEncoder();
        byte    getEncoderButton();
        bool    getRelay();
        void    setRelay(bool on);
        void    i2cReader();

        bool start();
        bool stop();

        enum CLICK_CODE {
                 OPEN = 0,
                 CLOSED = 1,
                 PRESSED = 2,
                 HELD = 3,
                 RELEASED = 4,
                 CLICKED = 5,
                 DOUBLECLICKED = 6, 
        };        
    private:
        enum __attribute__((__packed__)) COMMAND {
            RELAY = 'R',
        };
        struct __attribute__((packed, aligned(1))) sensorInValue_t {
            COMMAND cmd;
            bool    relay;
        };
        struct __attribute__((packed, aligned(1))) sensorOutValue_t {
            float   temperature;
            float   humidity;
            int8_t  encoder;
            byte    select;
            bool    relay;
        };

        union __attribute__((packed, aligned(1))) sensorValue_t {
            sensorInValue_t out;
            sensorOutValue_t in;
        };

        union  __attribute__((packed, aligned(1))) I2C_Packet_t {
            sensorValue_t sensorData;
            byte I2CPacket[sizeof(sensorValue_t)];
        };

        int _i2caddress;
        I2C_Packet_t indata;
};

extern AT8I2CGATEWAY at8gw;
