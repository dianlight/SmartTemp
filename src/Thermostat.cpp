#include "Thermostat.h"
#include "EvoDebug.h"
#include "at8i2cGateway.h"


Thermostat::Thermostat() {
    _loopTicker.attach_scheduled(5,std::bind(&Thermostat::_loopThermostat, this));
}
    
void Thermostat::_loopThermostat(){

    // Automatic return to Auto
    if(myConfig.get()->returnAutoTimeout > 0 
        && (!myConfig.get()->away  || myConfig.get()->awayMode == Config::MODES::AUTO )
        && myConfig.get()->hold == Config::MODES::MANUAL 
        && millis() - _manualTime > myConfig.get()->returnAutoTimeout * 1000){
            setMode(Config::MODES::AUTO);
    } 

    // Heater control
    if(millis() - _switchLastTime > myConfig.get()->minSwitchTime * 1000){
        // Controllo Switch!
        if(_curTemp - myConfig.get()->tempPrecision < getCurrentTarget()){
            #ifdef DEBUG_EVENT
                debugD("Relay On %f < %f",at8gw.getTemperature() - myConfig.get()->tempPrecision,getCurrentTarget());
            #endif
            if(_heatingCallback != NULL)
                _heatingCallback(true);
            _switchLastTime = millis();
        } else if(_curTemp + myConfig.get()->tempPrecision > getCurrentTarget()) {
            #ifdef DEBUG_EVENT
                debugD("Relay Off %f > %f",at8gw.getTemperature() - myConfig.get()->tempPrecision,getCurrentTarget());
            #endif
            if(_heatingCallback != NULL)
                _heatingCallback(false);
            _switchLastTime = millis();
        }
    }
}




Thermostat thermostat;
        
