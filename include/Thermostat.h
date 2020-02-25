#pragma once
#include <string.h>
#include <Ticker.h>
#include "Config.h"

/**
 * @brief 
 * 
 */

#define CURRENT_MODE_STR     myConfig.MODES_NAME[myConfig.get()->mode]
#define CURRENT_MODE_MQTT    myConfig.MODES_MQTT_NAME[myConfig.get()->mode]

#define CURRENT_HOLD_STR     myConfig.HOLDS_NAME[thermostat.getHold()]
#define CURRENT_HOLD_MQTT    myConfig.HOLDS_MQTT_NAME[thermostat.getHold()]
#define CURRENT_HOLD         _myConfig.get()->away? \
                                (_myConfig.get()->awayMode == Config::AWAY_MODES::AS_AUTO?_myConfig.get()->hold:_myConfig.get()->awayMode) \
                                : _myConfig.get()->hold  

#define CURRENT_ACTION_MQTT  heating?"heating":((myConfig.get()->mode == Config::MODES::OFF)?"off":"idle"); // idle, cooling, heating, drying, or off


class Thermostat {
    public:
        typedef void (*HeatingCallback)(bool isHeating);

        Thermostat(Config myConfig);

        void setHeatingCallback(HeatingCallback heatingCallback);
        void setMode(Config::MODES mode);

        float getCurrentTarget();

        /** Getter and Setter **/
        void  setCurrentTemp(float curTemp){_curTemp=curTemp;}
        float getCurrentTemp(){ return _curTemp;}
        void  setHold(Config::HOLDS hold);
        byte  getHold(){return CURRENT_HOLD;}

    private:
        Config _myConfig;    
        HeatingCallback _heatingCallback = NULL;
        unsigned long _switchLastTime, _manualTime;
        Ticker _loopTicker;
        float _curTemp = 20.4f; 
        

        void _loopThermostat();

};
