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
#include "MyMQTT.h"
#include "MyTimeNTP.h"
#include "I2CDebug.h"
#include "Thermostat.h"
#include "Webserver.h"
Config myConfig;

AT8I2CGATEWAY at8gw(AT8_I2C_GW);

Thermostat thermostat(myConfig);

// Base structure data:
bool heating = false;
//float curTemp = 20.4f;
float curHumidity = 80.9f;

WiFiEventHandler stationConnectedHandler;

void onStationModeGotIP(const WiFiEventStationModeGotIP& evt) {

    bootConnectedDisplay();

    setupNTP();

    setupOTA();
 
    setupMQTT();

    setupWebServer();

}



void setup()
{
  Wire.begin();

  Serial.begin(115200);
  while(!Serial);

  myConfig = Config();

  setupDisplay();

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
      if(sendMQTTState()){
         sendMQTTAvail(true);
      }
    } 
  });
}   

void loop()
{
  loopDisplay();
  if(!loopOTA()){
      at8gw.i2cReader();
      curHumidity = at8gw.getHumidity();
      thermostat.setCurrentTemp(at8gw.getTemperature() + FIXED_TEMP_CORRECTION);
      heating = at8gw.getRelay();

      if (WiFi.status() == WL_CONNECTED){
          networkLoop();
          loopMQTT();
          loopWebServer();
          loopNTP();
      }
  }
}

