#pragma once
#include "EvoWebserver.h"
#include "EvoDebug.h"
#include <Ticker.h>
#include <ESPAsyncWebServer.h>

#include "Display.h"
#include "Config.h"

class EvoWebserver {
    public:
        EvoWebserver(Display &display, Config &myConfig);
        void start();
        void stop();

    private:
        Display &display;
        Config &myConfig;

        Ticker webServerTicker;
        AsyncWebServer server{80};
        #ifdef EVODEBUG_REMOTE
            AsyncWebSocket ws{"/ws"};
        #endif
        //AsyncEventSource events{"/events"};

        void loopWebServer();

        void handleDisplayData(AsyncWebServerRequest *request);
        void handleDisplayBmpData(AsyncWebServerRequest *request);
        void handleLoadData(AsyncWebServerRequest *request);
        void handleLoadProgramData(AsyncWebServerRequest *request);
        void handleSaveData(AsyncWebServerRequest *request);
        void handleSaveProgramData(AsyncWebServerRequest *request);
        bool handleFileRead(AsyncWebServerRequest *request);
        void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

};

 #ifdef EVODEBUG_REMOTE
         class WebsocketEvoAppender: public EvoAppender {
            public:
                WebsocketEvoAppender(AsyncWebSocket &ws): ws(ws){}

                void begin(){
                    ws.printfAll("///// WebsocketEvoAppender start /////");
                }

                void end(){
                    ws.printfAll("///// WebsocketEvoAppender stop  /////");
                }

                void displayMessage(LEVEL level,const char *fname,const char *mname,uint line, char* message){
                    ws.printfAll(message);
                }
            
            private:
                AsyncWebSocket &ws;
        };
 #endif
