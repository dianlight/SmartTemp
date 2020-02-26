/*
 * SmartTemp
 * 
 * Smart Thermostat
 */

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
#include "Webserver.h"
#include <PubSubClient.h>

Config myConfig;

AT8I2CGATEWAY at8gw(AT8_I2C_GW);

Thermostat thermostat(myConfig);
             
OTA *ota;

MQTTforHA *mQTTforHA;

TimeNTPClient timeNTPClient = TimeNTPClient();

Display display(thermostat, myConfig, at8gw,timeNTPClient);

// Base structure data:
//bool heating = false;
//float curTemp = 20.4f;
float curHumidity = 80.9f;

static bool _inOTA = false;

WiFiEventHandler stationConnectedHandler;

void onStationModeGotIP(const WiFiEventStationModeGotIP& evt) {

  display.bootConnectedDisplay();

  //setupNTP();

  ota = new OTA(display);
  ota->addOtaCallback([](OTA::OTA_EVENT event){
    switch (event)
    {
    case OTA::OTA_EVENT::START/* constant-expression */:
      _inOTA = true;
      break;
    case OTA::OTA_EVENT::STOP/* constant-expression */:
    case OTA::OTA_EVENT::ERROR/* constant-expression */:
      _inOTA = false;
      break;
    }
  });


//    setupOTA();
 
//  setupMQTT();
  mQTTforHA = new MQTTforHA(thermostat,myConfig,at8gw);
  display.setMQTTforHA(mQTTforHA);

  setupWebServer();

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
      debugV("Relay %s %f > %f",isHeating?"On":"Off",curTemp,TARGET_TEMP);
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
//  if(!loopOTA()){
  if(!_inOTA){
      at8gw.i2cReader();
      curHumidity = at8gw.getHumidity();
      thermostat.setCurrentTemp(at8gw.getTemperature() + FIXED_TEMP_CORRECTION);
//      heating = at8gw.getRelay();

      if (WiFi.status() == WL_CONNECTED){
          networkLoop();
//          loopMQTT();
          loopWebServer();
//          loopNTP();
      }
  }
}

