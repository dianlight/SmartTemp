#include "Network.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Display.h"
#include <WiFiUdp.h>
//#include <ezTime.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

//#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include "Config.h"
extern Config myConfig;

//extern Timezone myTZ;  


//callback notifying us of the need to save config
void saveConfigCallback () {
  #ifdef DEBUG_EVENT
    Serial.println("Should save config");
  #endif
  myConfig.saveConfig();
}

void apModeCallback(WiFiManager *wfmanager) {
  Serial.print("Config AP:");
  Serial.println(wfmanager->getConfigPortalSSID());
  bootAPDisplay(wfmanager->getConfigPortalSSID());

}


void setupWifi(bool captiveConfig) {
  if(!captiveConfig){
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin();
    delay(500);
    int8_t status = WiFi.waitForConnectResult();
    if(status != WL_CONNECTED){
      Serial.printf("WiFi connection error or not configured! %d\n", status);
      displayError("WiFi conncection error!");
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
      wifiManager.resetSettings();
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
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    //read updated parameters
    strcpy(myConfig.get()->mqtt_server, custom_mqtt_server.getValue());
    strcpy(myConfig.get()->mqtt_port, custom_mqtt_port.getValue());
    strcpy(myConfig.get()->mqtt_client_id,custom_mqtt_client_id.getValue());
    strcpy(myConfig.get()->mqtt_user,custom_mqtt_user.getValue());
    strcpy(myConfig.get()->mqtt_password,custom_mqtt_password.getValue());
    strcpy(myConfig.get()->mqtt_topic_prefix,custom_mqtt_topic_prefix.getValue());

    myConfig.saveConfig();

  }
  
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  bootConnectedDisplay();
  
  delay(1000);
  clearDisplay();
}


void networkLoop()
{
}


