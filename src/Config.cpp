#include "Config.h"
#include <EEPROM.h>
#include <Arduino.h>
#include "EvoDebug.h"

static Config::StoreStruct storage;
static Config::ConfigModeStruct configMode;

Config::Config() {
  EEPROM.begin(sizeof(Config::StoreStruct));  
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(0) == CONFIG_VERSION[0] &&
      EEPROM.read(1) == CONFIG_VERSION[1] &&
      EEPROM.read(2) == CONFIG_VERSION[2]){
    for (unsigned int t=0; t<sizeof(Config::StoreStruct); t++){
      *((char*)&storage + t) = EEPROM.read(t);
      if(t ==  sizeof(CONFIG_VERSION)+ sizeof(size_t)-1 && storage.configSize != sizeof(Config::StoreStruct)){
        debugE("Illegal Config Size %d != %d!", storage.configSize, sizeof(Config::StoreStruct));
        storage.configSize = sizeof(Config::StoreStruct);
        break;
      }
    }
  } else {
    debugW("\nConfig Version UNMATCH '%s' != '%c%c%c'!\n",CONFIG_VERSION, EEPROM.read(0),EEPROM.read(1),EEPROM.read(2));
  }
}

void Config::saveConfig() {
  for (unsigned int t=0; t<sizeof(Config::StoreStruct); t++)
    EEPROM.write(t, *((char*)&storage + t));
    EEPROM.commit();
}

Config::StoreStruct *Config::get() {
    return &storage;
}

Config::ConfigModeStruct *Config::getMode() {
  return &configMode;
}

Config myConfig = Config();
