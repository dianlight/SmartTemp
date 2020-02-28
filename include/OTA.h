#pragma once
#include <Ticker.h>
#include <list> 
#include <ArduinoOTA.h>

#include "Display.h"


class OTA {
    public:
        enum __attribute__((__packed__)) OTA_EVENT {
            START,
            STOP,
            ERROR
        };

        typedef void (*OtpEvent)(OTA_EVENT event);

        OTA(Display &display);
        void addOtaCallback(OtpEvent otaEventCallback){ callbacks.push_back(otaEventCallback); };

        void start();
        void stop();
    
    private:
        Display &display;

        Ticker checkOTATicker;

        std::list<OtpEvent> callbacks;

        String type;

        void loopOTA();
        void onStart();
        void onEnd();
        void onProgress(unsigned int progress, unsigned int total);
        void onError(ota_error_t error);

        void processCallbacks(OTA_EVENT event);

};