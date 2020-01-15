#include "OTA.h"
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "Display.h"
#include "MyMQTT.h"


static bool inOTA = false;



void setupOTA() {
    // OTA
  Serial.println("OTA Subsystem start!");

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("smarttemp");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    sendMQTTAvail(false);
    inOTA=true;
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    displayError("Reboot..");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.print(".");
    displayProgress((progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
      displayError("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
      displayError("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
      displayError("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
      displayError("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
      displayError("End Failed");
    }
  });
  ArduinoOTA.begin();

}


bool loopOTA() {
  ArduinoOTA.handle();
  return inOTA;
}