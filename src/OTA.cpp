#include "OTA.h"
#include "EvoDebug.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <functional>
#include "Config.h"


//static bool inOTA = false;

// FIXME: Migrate do debug* macro


OTA::OTA(Display &display): display(display) {
    // OTA
  debugD("OTA Subsystem start!");

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart(std::bind(&OTA::onStart, this));
  ArduinoOTA.onEnd(std::bind(&OTA::onEnd, this));
  ArduinoOTA.onProgress(std::bind(&OTA::onProgress,this,std::placeholders::_1,std::placeholders::_2));
  ArduinoOTA.onError(std::bind(&OTA::onError, this, std::placeholders::_1));

  ArduinoOTA.setHostname(_SMT_HOST);
  ArduinoOTA.begin();

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
  debugI("Start updating %s",type.c_str());
}

void OTA::onEnd(){
  processCallbacks(STOP);
  debugI("\nEnd");
  display.displayError("Reboot..");
}

void OTA::onProgress(unsigned int progress, unsigned int total){
  debugI("%d/%d",progress,total);
  display.displayProgress((progress / (total / 100)),type);
}

void OTA::onError(ota_error_t error){
  processCallbacks(ERROR);
  debugE("Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR) {
    debugE("Auth Failed");
    display.displayError("Auth Failed");
  } else if (error == OTA_BEGIN_ERROR) {
    debugE("Begin Failed");
    display.displayError("Begin Failed");
  } else if (error == OTA_CONNECT_ERROR) {
    debugE("Connect Failed");
    display.displayError("Connect Failed");
  } else if (error == OTA_RECEIVE_ERROR) {
    debugE("Receive Failed");
    display.displayError("Receive Failed");
  } else if (error == OTA_END_ERROR) {
    debugE("End Failed");
    display.displayError("End Failed");
  }
}


void OTA::loopOTA() {
  if (WiFi.status() == WL_CONNECTED){
    ArduinoOTA.handle();
  }
}

void OTA::processCallbacks(OTA_EVENT event){
  std::list<OtpEvent> :: iterator it; 
    for(it = callbacks.begin(); it != callbacks.end(); ++it) 
        (*it)(event);
}

void OTA::start(){
  MDNS.end();
  MDNS.begin(ArduinoOTA.getHostname().c_str());
  MDNS.enableArduino(8266);  
  checkOTATicker.attach_ms_scheduled(150,std::bind(&OTA::loopOTA,this));
}

void OTA::stop(){
  checkOTATicker.detach();
}
