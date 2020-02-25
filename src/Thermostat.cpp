#include "Thermostat.h"


Thermostat::Thermostat(Config myConfig) {
    _myConfig = myConfig;
    _loopTicker.attach_scheduled(5,std::bind(&Thermostat::_loopThermostat, this));
}
    
void Thermostat::_loopThermostat(){

    // Automatic return to Auto
    if(_myConfig.get()->returnAutoTimeout > 0 
        && (!_myConfig.get()->away  || _myConfig.get()->awayMode == Config::MODES::AUTO )
        && _myConfig.get()->hold == Config::MODES::MANUAL 
        && millis() - _manualTime > _myConfig.get()->returnAutoTimeout * 1000){
            _myConfig.get()->hold = Config::MODES::AUTO;
    } 

    // Heater control
    if(millis() - _switchLastTime > _myConfig.get()->minSwitchTime * 1000){
        // Controllo Switch!
        if(_curTemp - _myConfig.get()->tempPrecision < getCurrentTarget()){
            #ifdef DEBUG_EVENT
                debugV("Relay On %f < %f",curTemp - myConfig.get()->tempPrecision,TARGET_TEMP);
            #endif
            if(_heatingCallback != NULL)
                _heatingCallback(true);
            _switchLastTime = millis();
        } else if(_curTemp + _myConfig.get()->tempPrecision > getCurrentTarget()) {
            #ifdef DEBUG_EVENT
                debugV("Relay Off %f > %f",curTemp - myConfig.get()->tempPrecision,TARGET_TEMP);
            #endif
            if(_heatingCallback != NULL)
                _heatingCallback(false);
            _switchLastTime = millis();
        } else {
            #ifdef DEBUG_EVENT
                debugV("Const Relay %s\n",heating?"On":"Off");
                at8gw.setRelay(heating);
            #endif
        }
    }
}

void Thermostat::setHeatingCallback(HeatingCallback heatingCallback){
    _heatingCallback = heatingCallback;
}

void Thermostat::setMode(Config::MODES mode){
    _myConfig.get()->mode = mode;
    if(_myConfig.get()->mode == Config::MANUAL){
        _manualTime = millis();
    }
}

void Thermostat::setHold(Config::HOLDS hold){
    _myConfig.get()->hold = hold;
    _myConfig.get()->mode = Config::MANUAL;
    _manualTime = millis();
}


float Thermostat::getCurrentTarget(){
    return (_myConfig.get()->targetTemp[CURRENT_HOLD] + 
            ( 
                _myConfig.get()->away?_myConfig.get()->awayModify:0 
            ));
}
        
