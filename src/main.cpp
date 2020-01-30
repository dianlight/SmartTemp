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

// Base structure data:
bool heating = false;
float curTemp = 20.4f;
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

  setupWifi(at8gw.getEncoderButton() == 3); // Pressed

  delay(500);
}   

unsigned long checkLastTime, switchLastTime, manualTime;


void loop()
{
  loopDisplay();
  if(!loopOTA()){
      if(millis() - checkLastTime > 5000){
          // Automation
          if(myConfig.get()->returnAutoTimeout > 0 
                && !myConfig.get()->away 
                && myConfig.get()->hold == Config::MODES::MANUAL 
                && millis() - manualTime > myConfig.get()->returnAutoTimeout * 1000){
            myConfig.get()->hold = Config::MODES::AUTO;
          }

          // Relay Control      
          if(curTemp - myConfig.get()->tempPrecision < TARGET_TEMP && millis() - switchLastTime > myConfig.get()->minSwitchTime * 1000){
            #ifdef DEBUG_EVENT
              debugV("Relay On %f < %f",curTemp - myConfig.get()->tempPrecision,TARGET_TEMP);
            #endif
            at8gw.setRelay(true);
            switchLastTime = millis();
          } else if(curTemp + myConfig.get()->tempPrecision > TARGET_TEMP && millis() - switchLastTime > myConfig.get()->minSwitchTime * 1000) {
            #ifdef DEBUG_EVENT
              debugV("Relay Off %f > %f",curTemp - myConfig.get()->tempPrecision,TARGET_TEMP);
            #endif
            at8gw.setRelay(false);
            switchLastTime = millis();
          } else {
            #ifdef DEBUG_EVENT
              debugV("Const Relay %s\n",heating?"On":"Off");
              at8gw.setRelay(heating);
            #endif
          }
          if (WiFi.status() == WL_CONNECTED){
            sendMQTTAvail(true);
            sendMQTTState();
          } 
          checkLastTime = millis();
      }
      at8gw.i2cReader();
      curHumidity = at8gw.getHumidity();
      curTemp = at8gw.getTemperature() + FIXED_TEMP_CORRECTION;
      heating = at8gw.getRelay();

      if (WiFi.status() == WL_CONNECTED){
          networkLoop();
          loopMQTT();
          loopWebServer();
          loopNTP();
      }
  }
}

