/*
 * SmartTemp
 * 
 * Smart Thermostat
 */
#include "EvoDebug.h"
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#include "Config.h"
#include "Network.h"
#include "Display.h"
#include "OTA.h"
#include "at8i2cGateway.h"
#include "MQTTforHA.h"
#include "TimeNTPClient.h"
#include "I2CDebug.h"
#include "Thermostat.h"
#include "EvoWebserver.h"
//#include <PubSubClient.h>

Config myConfig;

AT8I2CGATEWAY at8gw(AT8_I2C_GW);

Thermostat thermostat(myConfig);
             
MQTTforHA *mQTTforHA;

TimeNTPClient timeNTPClient = TimeNTPClient();

Display display(thermostat, myConfig, at8gw,timeNTPClient);

OTA ota(display);

EvoWebserver evoWebserver(display,myConfig);

float curHumidity = 80.9f;

static bool _inOTA = false;

WiFiEventHandler stationConnectedHandler;

void onStationModeGotIP(const WiFiEventStationModeGotIP& evt) {

  debugI("IP %s/%s gw %s",evt.ip.toString().c_str(),evt.mask.toString().c_str(),evt.gw.toString().c_str());

  display.bootConnectedDisplay();

  debugD("Display OK..");

  ota.addOtaCallback([](OTA::OTA_EVENT event){
    switch (event)
    {
    case OTA::OTA_EVENT::START/* constant-expression */:
      _inOTA = true;
      evoWebserver.stop();
      break;
    case OTA::OTA_EVENT::STOP/* constant-expression */:
    case OTA::OTA_EVENT::ERROR/* constant-expression */:
      _inOTA = false;
      evoWebserver.start();
      break;
    }
  });
  ota.start();

  debugD("OTA OK..");


  mQTTforHA = new MQTTforHA(thermostat,myConfig,at8gw);
  display.setMQTTforHA(mQTTforHA);

  debugD("MQTT OK..");

  evoWebserver.start();

  debugD("EvoWenserver OK..");
}


void setup()
{
  Wire.begin();

  Serial.begin(115200);
  while(!Serial);

  myConfig = Config();

  stationConnectedHandler = WiFi.onStationModeGotIP(&onStationModeGotIP);

  #ifdef DEBUG_I2C_SCAN
    delay(500);
    scanI2C();
  #endif

  at8gw.i2cReader();
  delay(500);

  setupWifi(at8gw.getEncoderButton() == AT8I2CGATEWAY::CLICK_CODE::HELD); 

  delay(500);

  thermostat.setHeatingCallback([](bool isHeating){
    #ifdef DEBUG_EVENT
      debugD("Relay %s %f > %f",isHeating?"On":"Off",thermostat.getCurrentTemp(),thermostat.getCurrentTarget());
    #endif
    at8gw.setRelay(isHeating);
    if (WiFi.status() == WL_CONNECTED){
      if(mQTTforHA->sendMQTTState()){
         mQTTforHA->sendMQTTAvail(true);
      }
    } 
  });

}   

void loop()
{
  display.loopDisplay();
  if(!_inOTA){
      at8gw.i2cReader();
      curHumidity = at8gw.getHumidity();
      thermostat.setCurrentTemp(at8gw.getTemperature() + FIXED_TEMP_CORRECTION);
  }
}

