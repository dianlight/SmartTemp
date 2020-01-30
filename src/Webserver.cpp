#include <DNSServer.h>
#include <ESP8266mDNS.h>
//#include <FS.h>
#include <ArduinoJson.h>
#include <strings.h>
#include <U8g2lib.h>
#include "Display.h"
#include <LoopbackStream.h>
#include <AsyncJson.h>

#include "web_static/web_server_static_files.h"

#define NO_EXTERN_AsyncWebSocket
#include "Config.h"
extern Config myConfig;
#include "Thermostat.h"

/**
 * @brief Based on A Beginner's Guide to the ESP8266 -  Pieter P, 08-03-2017 
 * https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html
 * 
 */

#include <ESPAsyncWebServer.h>

// SKETCH BEGIN
AsyncWebServer server(80);
#ifdef DEBUG_REMOTE
  AsyncWebSocket ws("/ws");
#endif
//AsyncEventSource events("/events");

//ESP8266WebServer server(80);
//EspHtmlTemplateProcessor templateProcessor(&server);

//LoopbackStream _buffer(9*1024);

void handleDisplayData(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("image/x‑portable‑bitmap",10240U);
  screeshot(*response);
  request->send(response);
}

void handleDisplayBmpData(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("image/x‑portable‑bitmap");
  response->addHeader("Content-Disposition","attachment; filename=\"screen.pbm\"");
  screeshotbmp(*response);
  request->send(response);
}

void handleLoadData(AsyncWebServerRequest *request){

    StaticJsonDocument<1024> root;
    root["teco"] = myConfig.get()->targetTemp[0];
    root["tnorm"] = myConfig.get()->targetTemp[1];
    root["tconf"] = myConfig.get()->targetTemp[2];
    root["aautotimeout"] = myConfig.get()->returnAutoTimeout;
    root["dsleep"] = myConfig.get()->displaySleep;
    root["doff"] = myConfig.get()->displayPowerOff;
    root["tadelta"] = myConfig.get()->awayModify;
    root["tamode"] = myConfig.get()->awayMode;
    root["tprec"] = myConfig.get()->tempPrecision;
    root["sres"] = myConfig.get()->minSwitchTime;
    root["nname"] = myConfig.get()->mqtt_client_id;
    root["saddress"] = myConfig.get()->mqtt_server;
    root["port"] = myConfig.get()->mqtt_port;
    root["user"] = myConfig.get()->mqtt_user;
    root["password"] = myConfig.get()->mqtt_password;
    root["tprefix"] = myConfig.get()->mqtt_topic_prefix;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(root,*response);
    request->send(response);
}

void handleLoadProgramData(AsyncWebServerRequest *request){
  AsyncJsonResponse *response = new AsyncJsonResponse(true);
  JsonArray program = response->getRoot();
  AsyncWebParameter* day = request->getParam("day");
  for(u8_t h=0; h < (24*4); h++){
    program.add(myConfig.get()->weekProgram[day->value().toInt()].hourQuarterHolds[h]);
  }
  response->setLength();
  request->send(response);
}

void handleSaveData(AsyncWebServerRequest *request){
        debugV("Ricevuta chiamata di Save!");
        debugV("Ricevuti %d parametri",request->args());
        for(size_t i=0; i< request->args(); i++){
            debugV("%s=%s",request->argName(i).c_str(), request->arg(i).c_str());
        }
        myConfig.get()->targetTemp[0] = request->arg("teco").toFloat();
        myConfig.get()->targetTemp[1] = request->arg("tnorm").toFloat();
        myConfig.get()->targetTemp[2] = request->arg("tconf").toFloat();
        myConfig.get()->returnAutoTimeout = request->arg("aautotimeout").toInt();
        myConfig.get()->displaySleep = request->arg("dsleep").toInt();
        myConfig.get()->displayPowerOff = request->arg("doff").toInt();
        myConfig.get()->awayModify = request->arg("tadelta").toFloat();
        myConfig.get()->awayMode = request->arg("tamode").toFloat();
        myConfig.get()->tempPrecision = request->arg("tprec").toFloat();
        myConfig.get()->minSwitchTime = request->arg("sres").toFloat();
        strcpy(myConfig.get()->mqtt_client_id,request->arg("nname").c_str());
        strcpy(myConfig.get()->mqtt_server,request->arg("saddress").c_str());
        strcpy(myConfig.get()->mqtt_port,request->arg("port").c_str());
        strcpy(myConfig.get()->mqtt_user,request->arg("user").c_str());
        strcpy(myConfig.get()->mqtt_password,request->arg("password").c_str());
        strcpy(myConfig.get()->mqtt_topic_prefix,request->arg("tprefix").c_str());

        myConfig.saveConfig();

        request->send(204);

}

void handleSaveProgramData(AsyncWebServerRequest *request){
        debugV("Ricevuta chiamata di Save Program!");
        debugV("Ricevuti %d parametri",request->args());
        for(size_t i=0; i< request->args(); i++){
            if(request->argName(i).startsWith("plain"))continue;
            debugV("%s=%s",request->argName(i).c_str(), request->arg(i).c_str());
            u8_t day = atoi(&request->argName(i).c_str()[4]);
            debugV("Working on day %d '%s'",day,request->arg(i).c_str());
            StaticJsonDocument<1536> doc;
            DeserializationError err = deserializeJson(doc, request->arg(i).c_str());
            if (err) {
              debugV("deserializeJson() failed with code: %s ",err.c_str());
              request->send(500,err.c_str());
            } else {
              for(u8_t hq=0; hq < 24*4; hq++){
                debugV("day:%d hq:%d = %d",day,hq,doc[hq].as<u8_t>());
                myConfig.get()->weekProgram[day].hourQuarterHolds[hq]=doc[hq].as<u8_t>();
              }
            }
        }
        myConfig.saveConfig();

        request->send(204);
}

//String indexKeyProcessor(const String& key)
//{
//  if (key == "VERSION") return _SMT_VERSION "." CONFIG_VERSION;
//  else if (key == "CURRENT_MODE") return CURRENT_MODE_STR;
//  else if (key == "COPYRIGHT") return COPYRIGHT;
//
//  return "&#x1F534;" + key + "&#x1F534;";
//}

bool handleFileRead(AsyncWebServerRequest *request){  // send the right file to the client (if it exists)
  String path = request->url();
  debugV("handleFileRead: %s",path.c_str());
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  for(byte i=0; i < STATIC_FILES_SIZE; i++){
    if(staticFiles[i].filename == path || staticFiles[i].filename == path+".gz"){
      AsyncWebServerResponse *response = request->beginResponse_P(200, staticFiles[i].mimetype, staticFiles[i].data, staticFiles[i].length);
      if(staticFiles[i].filename.endsWith(".gz"))response->addHeader("Content-Encoding", "gzip");
      request->send(response);
      return true;
    } 
  }

  debugV("File Not Found: %s",path.c_str());
  return false;                                          // If the file doesn't exist, return false
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}



void setupWebServer(){

    MDNS.addService("http", "tcp", 80);   // Web server

//    SPIFFS.begin();                           // Start the SPI Flash Files System

//    server.serveStatic("/", SPIFFS, "/")
//      .setDefaultFile("index.html")
//      .setTemplateProcessor(indexKeyProcessor);

    server.on("/load",HTTP_GET, handleLoadData);
    server.on("/save",HTTP_POST, handleSaveData);
    server.on("/loadP",HTTP_GET, handleLoadProgramData);
    server.on("/saveP",HTTP_POST, handleSaveProgramData);
    server.on("/screen",HTTP_GET, handleDisplayData);
    server.on("/screenpbm",HTTP_GET, handleDisplayBmpData);
   
    server.onNotFound([](AsyncWebServerRequest *request) {                              // If the client requests any URI
        if (!handleFileRead(request))                  // send it if it exists
        request->send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

    #ifdef DEBUG_REMOTE
      ws.onEvent(onWsEvent);
      server.addHandler(&ws);
    #endif

    server.begin();                           // Actually start the server
    debugV("HTTP server started");
}

void stopWebServer(){
    server.end();
//    SPIFFS.end();
}


void loopWebServer(){
  ws.cleanupClients();
}