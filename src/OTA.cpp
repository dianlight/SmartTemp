#include "OTA.h"
#include <ESP8266WiFi.h>
#include <functional>
//#include "MyMQTT.h"
#include "Config.h"


//static bool inOTA = false;

// FIXME: Migrate do debug* macro


OTA::OTA(Display &display): display(display) {
    // OTA
  Serial.println("OTA Subsystem start!");

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(_SMT_HOST);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart(std::bind(&OTA::onStart, this));
  ArduinoOTA.onEnd(std::bind(&OTA::onEnd, this));
  ArduinoOTA.onProgress(std::bind(&OTA::onProgress,this,std::placeholders::_1,std::placeholders::_2));
  ArduinoOTA.onError(std::bind(&OTA::onError, this, std::placeholders::_1));
  ArduinoOTA.begin();
  checkOTATicker.attach_ms_scheduled(100,std::bind(&OTA::loopOTA,this));
}

void OTA::onStart(){
  processCallbacks(START);

//  sendMQTTAvail(false);
  if (ArduinoOTA.getCommand() == U_FLASH) {
    type = "sketch";
  } else { // U_FS
    type = "filesystem";
  }

  // NOTE: if updating FS this would be the place to unmount FS using FS.end()
  Serial.println("Start updating " + type);
}

void OTA::onEnd(){
  processCallbacks(STOP);
  Serial.println("\nEnd");
  display.displayError("Reboot..");
}

void OTA::onProgress(unsigned int progress, unsigned int total){
  Serial.print(".");
  display.displayProgress((progress / (total / 100)),type);
}

void OTA::onError(ota_error_t error){
  processCallbacks(ERROR);
  Serial.printf("Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR) {
    Serial.println("Auth Failed");
    display.displayError("Auth Failed");
  } else if (error == OTA_BEGIN_ERROR) {
    Serial.println("Begin Failed");
    display.displayError("Begin Failed");
  } else if (error == OTA_CONNECT_ERROR) {
    Serial.println("Connect Failed");
    display.displayError("Connect Failed");
  } else if (error == OTA_RECEIVE_ERROR) {
    Serial.println("Receive Failed");
    display.displayError("Receive Failed");
  } else if (error == OTA_END_ERROR) {
    Serial.println("End Failed");
    display.displayError("End Failed");
  }
}


bool OTA::loopOTA() {
  ArduinoOTA.handle();
//  return inOTA;
  return false;
}

void OTA::processCallbacks(OTA_EVENT event){
  std::list<OtpEvent> :: iterator it; 
    for(it = callbacks.begin(); it != callbacks.end(); ++it) 
        (*it)(event);
}