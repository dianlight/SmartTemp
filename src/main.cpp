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
#include "EvoWifi.h"
#include "Display.h"
#include "OTA.h"
#include "at8i2cGateway.h"
#include "TimeNTPClient.h"
#include "I2CDebug.h"
#include "Thermostat.h"
#include "EvoWebserver.h"
#include "MQTTforHA.h"

             
float curHumidity = 80.9f;

static bool _inOTA = false;

WiFiEventHandler stationConnectedHandler,stationModeDisconnectedHandler;

void onStationModeGotIP(const WiFiEventStationModeGotIP& evt) {

  debugI("IP %s/%s gw %s",evt.ip.toString().c_str(),evt.mask.toString().c_str(),evt.gw.toString().c_str());

  display.bootConnectedDisplay();
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

  mQTTforHA.start();

  evoWebserver.start();
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt ){
  debugI("Station Mode disconnected!");

  ota.stop();
  mQTTforHA.stop();
}

void setup()
{
  stationConnectedHandler = WiFi.onStationModeGotIP(&onStationModeGotIP);
  stationModeDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationModeDisconnected);

  Wire.begin();

  Serial.begin(74880);

  //myConfig = Config();

  #ifdef DEBUG_I2C_SCAN
    delay(500);
    scanI2C();
  #endif

  at8gw.i2cReader();
  delay(500);

  if(at8gw.getEncoderButton() == AT8I2CGATEWAY::CLICK_CODE::HELD){
    debugD("AP config portal requested!");
    evoWifi.doAPConnect();
  } else if(!evoWifi.doSTAConnect()){
      debugW("Unable to connet to WiFi. Start in AP mode");
      evoWifi.doAPConnect();
  }

//  delay(500);

  thermostat.setHeatingCallback([](bool isHeating){
    #ifdef DEBUG_EVENT
      debugD("Relay %s %f > %f",isHeating?"On":"Off",thermostat.getCurrentTemp(),thermostat.getCurrentTarget());
    #endif
    at8gw.setRelay(isHeating);
    if (WiFi.status() == WL_CONNECTED){
      if(mQTTforHA.isRunning() && mQTTforHA.sendMQTTState()){
         mQTTforHA.sendMQTTAvail(true);
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

