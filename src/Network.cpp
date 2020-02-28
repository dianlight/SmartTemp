#include "Network.h"
#include "EvoDebug.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <WiFiUdp.h>

//needed for library
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#define NO_EXTERN_AsyncWebSocket   // WiFi manager incompatibility!
#include "Config.h"
extern Config myConfig;

#include "Display.h"
extern Display display;

// FIXME: Migrate do debug* macro


//callback notifying us of the need to save config
void saveConfigCallback () {
  #ifdef DEBUG_EVENT
    debugD("Should save config");
  #endif
  myConfig.saveConfig();
}

void apModeCallback(WiFiManager *wfmanager) {
  debugD("Config AP:%s",wfmanager->getConfigPortalSSID().c_str());
  display.bootAPDisplay(wfmanager->getConfigPortalSSID());

}


void setupWifi(bool captiveConfig) {
  if(!captiveConfig){
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin();
    delay(500);
    int8_t status = WiFi.waitForConnectResult();
    if(status != WL_CONNECTED){
      switch (status)
      {
      case WL_IDLE_STATUS:
        debugW("WiFi connection error or not configured! IDLE");
        display.displayError("WiFi conncection error! IDLE");
        break;
      case WL_NO_SSID_AVAIL:
        debugW("WiFi connection error or not configured! NO_SSD");
        display.displayError("WiFi conncection error! NO_SSD");
        break;
      case WL_CONNECT_FAILED:
        debugW("WiFi connection error or not configured! FAIL");
        display.displayError("WiFi conncection error! FAIL");
        break;
      case WL_CONNECTION_LOST:
        debugW("WiFi connection error or not configured! LOST");
        display.displayError("WiFi conncection error! LOST");
        break;
      case WL_DISCONNECTED:
        debugW("WiFi connection error or not configured! DICONNECTED");
        display.displayError("WiFi conncection error! DICONNECTED");
        break;
      
      case WL_SCAN_COMPLETED:
      default:
        debugW("WiFi connection error or not configured! Status=0x%x", status);
        display.displayError("WiFi conncection error!");
        break;
      }
    }
  } else {


    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", myConfig.get()->mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", myConfig.get()->mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_client_id("name", "mqtt client id", myConfig.get()->mqtt_client_id, 40);
    WiFiManagerParameter custom_mqtt_user("user", "mqtt user", myConfig.get()->mqtt_user, 128);
    WiFiManagerParameter custom_mqtt_password("password", "mqtt password", myConfig.get()->mqtt_password, 40);
    WiFiManagerParameter custom_mqtt_topic_prefix("prefix", "mqtt topic prefix", myConfig.get()->mqtt_topic_prefix, 128);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(apModeCallback);
    wifiManager.setBreakAfterConfig(true);
  
    //set static ip
    //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    
    //add all your parameters here
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_client_id);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_mqtt_topic_prefix);
    
    //reset settings - for testing
  //  if(myConfig.get()->resetConfig){
  //    wifiManager.resetSettings();
  //    myConfig.get()->resetConfig = false;
  //  }

    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();
    
    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    wifiManager.setTimeout(120);

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    
    if (!wifiManager.autoConnect()) {
      debugW("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    debugI("connected...yeey :)");

    //read updated parameters
    strcpy(myConfig.get()->mqtt_server, custom_mqtt_server.getValue());
    strcpy(myConfig.get()->mqtt_port, custom_mqtt_port.getValue());
    strcpy(myConfig.get()->mqtt_client_id,custom_mqtt_client_id.getValue());
    strcpy(myConfig.get()->mqtt_user,custom_mqtt_user.getValue());
    strcpy(myConfig.get()->mqtt_password,custom_mqtt_password.getValue());
    strcpy(myConfig.get()->mqtt_topic_prefix,custom_mqtt_topic_prefix.getValue());
  
    myConfig.saveConfig();

  }
  
//  debugI("AP: %s", WiFi.SSID().c_str());
//  debugI("Local IP %s",WiFi.localIP().toString().c_str());
//  display.bootConnectedDisplay();
  
//  delay(1000);
//  display.clearDisplay();
}


