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

#ifdef DEBUG_REMOTE
  #include <RemoteDebug.h>
  RemoteDebug Debug;
#endif

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


  #ifdef DEBUG_REMOTE // Only for development

    // Initialize RemoteDebug

    Debug.begin("SmartTemp",Debug.VERBOSE); // Initialize the WiFi server

    //Debug.setPassword("r3m0t0."); // Password for WiFi client connection (telnet or webapp)  ?

    Debug.setResetCmdEnabled(true); // Enable the reset command

    Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)

    Debug.showColors(true); // Colors

    // Debug.setSerialEnabled(true); // if you wants serial echo - only recommended if ESP is plugged in USB

    // Project commands
/*
    String helpCmd = "bench1 - Benchmark 1\n";
    helpCmd.concat("bench2 - Benchmark 2");

    Debug.setHelpProjectsCmds(helpCmd);
    Debug.setCallBackProjectCmds(&processCmdRemoteDebug);
    */

    // End of setup - show IP

    Serial.println("* Arduino RemoteDebug Library");
    Serial.println("*");
    Serial.print("* WiFI connected. IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("*");
    Serial.println("* Please use the telnet client (telnet for Mac/Unix or putty and others for Windows)");
    Serial.println("* or the RemoteDebugApp (in browser: http://joaolopesf.net/remotedebugapp)");
    Serial.println("*");
    Serial.println("* This sample will send messages of debug in all levels.");
    Serial.println("*");
    Serial.println("* Please try change debug level in client (telnet or web app), to see how it works");
    Serial.println("*");

		debugV("* This is a message of debug level VERBOSE");
		debugD("* This is a message of debug level DEBUG");
		debugI("* This is a message of debug level INFO");
		debugW("* This is a message of debug level WARNING");
		debugE("* This is a message of debug level ERROR");

  #endif


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
              Serial.printf("Relay On %f < %f",curTemp - myConfig.get()->tempPrecision,TARGET_TEMP);
            #endif
            at8gw.setRelay(true);
            switchLastTime = millis();
          } else if(curTemp + myConfig.get()->tempPrecision > TARGET_TEMP && millis() - switchLastTime > myConfig.get()->minSwitchTime * 1000) {
            #ifdef DEBUG_EVENT
              Serial.printf("Relay Off %f > %f",curTemp - myConfig.get()->tempPrecision,TARGET_TEMP);
            #endif
            at8gw.setRelay(false);
            switchLastTime = millis();
          } else {
            #ifdef DEBUG_EVENT
              Serial.printf("Const Relay %s\n",heating?"On":"Off");
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

          #ifdef DEBUG_REMOTE
            // RemoteDebug handle (for WiFi connections)
            Debug.handle();
          #endif
      }
  }
}

