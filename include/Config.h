#pragma once

#ifndef SMT_VERSION
    #define SMT_VERSION NdN
#endif

// ArduinoJson Size: 5=459124 6=460132 6opt=460116


#define DEBUG_REMOTE
//#define DEBUG_I2C_SCAN
//#define DEBUG_I2C_IN
//#define DEBUG_I2C_OUT
#define DEBUG_MQTT
#define DEBUG_EVENT
#define DEBUG_WEBSERVER
//#define DEBUG_SERIAL_MENU

#include <time.h>
#include <TZ.h>
#define MYTZ TZ_Europe_Rome

// Pin Definition. NodeMCU
// LCD
#define SDA         0
#define SCL         2
#define LCD_ADDRESS 0x27

// AT8GW
#define AT8_I2C_GW  0x08

// RUNNING PARAMETERS
#define FIXED_TEMP_CORRECTION -2.0


// General Config
#define CONFIG_VERSION  "027"

// 
#define CONFIG_TARGET_STR    myConfig.CONFIG_TARGET_NAME[myConfig.getMode()->position]


#include "Constants.h"

class Config {
    public:

        enum __attribute__((__packed__)) MODES {
            OFF,     
            MANUAL,  
            AUTO,

            M_SIZE    
        };
        enum __attribute__((__packed__)) HOLDS {
            ECO,     
            NORMAL,  
            CONFORT,

            H_SIZE    
        };
        enum __attribute__((__packed__)) AWAY_MODES {
            FIXED_ECO,     
            FIXED_NORMAL,  
            FIXED_CONFORT,
            AS_AUTO,

            AM_SIZE    
        };
        const char* MODES_NAME[3] = {"Off","Manual","Auto"};
        const char* MODES_MQTT_NAME[3] = {"off","heat","auto"};
        const char* HOLDS_NAME[3] = {"Eco","Normal","Confort"};
        const char* HOLDS_MQTT_NAME[3] = {"Eco","Normal","Confort"};
        const char* AWAY_MODES_NAME[4] = {"As Auto","To Eco","To Norm.","To Conf."};

        enum CONFIG_TARGET {
            ECO_TEMP = 0,
            NORMAL_TEMP,
            CONFORT_TEMP,
            MANUAL_TO_AUTO_TIME,
            AWAY_DIFF,
            AWAY_HOLDS,
            //  TIME_INTERNET,
            //  TIME_WEEKDAY,
            //  TIME_HOUR,
            //  TIME_MINUTE,
            WIFI,
            INFO,
            FACTORY_RESET,

            _CONFIG_MODE_SIZE
        };


        struct ConfigModeStruct {
            bool active = false;
            uint8_t position = ECO_TEMP;
            int8_t  encoder = 0;
            bool    doFactoryReset = false;
        };


        const char* CONFIG_TARGET_NAME[9] = {"Eco","Normal","Confort","Manual to Auto time","Away Delta","Away Holds","Info","WiFi","Factory Reset"};

        struct __attribute__((packed, aligned(1))) DayProgram {
            byte hourQuarterHolds[24*4] = {
                    ECO,     ECO,     ECO,     ECO,   // 0
                    ECO,     ECO,     ECO,     ECO,   // 1
                    ECO,     ECO,     ECO,     ECO,   // 2
                    ECO,     ECO,     ECO,     ECO,   // 3
                    ECO,     ECO,     ECO,     ECO,   // 4
                    ECO,     ECO,     ECO,     ECO,   // 5
                    ECO,  NORMAL,  NORMAL, CONFORT,   // 6
                CONFORT, CONFORT, CONFORT, CONFORT,   // 7
                CONFORT, CONFORT, CONFORT, CONFORT,   // 8
                CONFORT, CONFORT, CONFORT,  NORMAL,   // 9
                 NORMAL,  NORMAL,  NORMAL,  NORMAL,   // 10
                 NORMAL,  NORMAL,  NORMAL,  NORMAL,   // 11
                 NORMAL,  NORMAL,  NORMAL,  NORMAL,   // 12
                 NORMAL,  NORMAL,  NORMAL,  NORMAL,   // 13
                 NORMAL,  NORMAL,  NORMAL,  NORMAL,   // 14
                 NORMAL,  NORMAL,  NORMAL,  NORMAL,   // 15
                 NORMAL,  NORMAL,  NORMAL,  NORMAL,   // 16
                 NORMAL,  NORMAL,  NORMAL, CONFORT,   // 17
                CONFORT, CONFORT, CONFORT, CONFORT,   // 18
                CONFORT, CONFORT, CONFORT, CONFORT,   // 19
                CONFORT, CONFORT, CONFORT,  NORMAL,   // 20
                 NORMAL,  NORMAL,  NORMAL,     ECO,   // 21
                    ECO,     ECO,     ECO,     ECO,   // 22
                    ECO,     ECO,     ECO,     ECO,   // 23
                                            };
        };

        struct __attribute__((packed, aligned(1))) StoreStruct {
            char version[4]                 = CONFIG_VERSION;
            size_t configSize               = sizeof(StoreStruct);
            float targetTemp[HOLDS::H_SIZE] = {18.5,19.5,20.5};
            float awayModify                = -0.5;
            byte  awayMode                  = AWAY_MODES::AS_AUTO;
            float tempPrecision             = 0.2;
            uint8_t minSwitchTime           = 30;
            byte mode                       = MODES::AUTO;
            uint8_t returnAutoTimeout       = 30;
            bool away                       = false;
            byte hold                       = HOLDS::NORMAL;
            DayProgram weekProgram[7];       

            char mqtt_server[40]        = "hassio4.local";
            char mqtt_port[6]           = "1883";
            char mqtt_client_id[40]     = "SmartTemp";
            char mqtt_user[128]         = "mqttapi";
            char mqtt_password[40]      = "";
            char mqtt_topic_prefix[128] = "";    

            byte displaySleep           = 35;
            byte displayPowerOff        = 60;
            byte displayContrast        = 200;
        };

        Config();
        void saveConfig();
        StoreStruct *get();
        ConfigModeStruct *getMode();
};

