#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <strings.h>
#include "Display.h"
#include <EspHtmlTemplateProcessor.h>

#include "Config.h"
extern Config myConfig;
#include "Thermostat.h"

#ifdef DEBUG_REMOTE
  #include <RemoteDebug.h>
  extern RemoteDebug Debug;
#endif


/**
 * @brief Based on A Beginner's Guide to the ESP8266 -  Pieter P, 08-03-2017 
 * https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html
 * 
 */

ESP8266WebServer server(8080);
EspHtmlTemplateProcessor templateProcessor(&server);

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void handleLoadData(){

    StaticJsonDocument<770> root;
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



    String json="";
    serializeJson(root,json);
    #ifdef DEBUG_WEBSERVER
      debugV("Response: %s",json.c_str());
    #endif
    server.send(200,"application/json",json.c_str());
}

void handleSaveData(){
        debugV("Ricevuta chiamata di Save!");
        debugV("Ricevuti %d parametri",server.args());
        for(int i=0; i< server.args(); i++){
            debugV("%s=%s",server.argName(i).c_str(), server.arg(i).c_str());
        }
//        if(server.hasArg("teco")){
//            debugV("Ricevuto teco=%s",server.arg("nname").c_str());
//        } else {
//            displayError("Invalid POST. No param!");
//        }
        myConfig.get()->targetTemp[0] = server.arg("teco").toFloat();
        myConfig.get()->targetTemp[1] = server.arg("tnorm").toFloat();
        myConfig.get()->targetTemp[2] = server.arg("tconf").toFloat();
        myConfig.get()->returnAutoTimeout = server.arg("aautotimeout").toInt();
        myConfig.get()->displaySleep = server.arg("dsleep").toInt();
        myConfig.get()->displayPowerOff = server.arg("doff").toInt();
        myConfig.get()->awayModify = server.arg("tadelta").toFloat();
        myConfig.get()->awayMode = server.arg("tamode").toFloat();
        myConfig.get()->tempPrecision = server.arg("tprec").toFloat();
        myConfig.get()->minSwitchTime = server.arg("sres").toFloat();
        strcpy(myConfig.get()->mqtt_client_id,server.arg("nname").c_str());
        strcpy(myConfig.get()->mqtt_server,server.arg("saddress").c_str());
        strcpy(myConfig.get()->mqtt_port,server.arg("port").c_str());
        strcpy(myConfig.get()->mqtt_user,server.arg("user").c_str());
        strcpy(myConfig.get()->mqtt_password,server.arg("password").c_str());
        strcpy(myConfig.get()->mqtt_topic_prefix,server.arg("tprefix").c_str());

        myConfig.saveConfig();

        server.send(204,"");

}

String indexKeyProcessor(const String& key)
{
  if (key == "VERSION") return SMT_VERSION "." CONFIG_VERSION;
  else if (key == "CURRENT_MODE") return CURRENT_MODE_STR;
  else if (key == "COPYRIGHT") return COPYRIGHT;

  return "&#x1F534;" + key + "&#x1F534;";
}

bool handleFileRead(String path){  // send the right file to the client (if it exists)
  debugV("handleFileRead: %s",path.c_str());
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    if(contentType.equals("text/html") && !SPIFFS.exists(pathWithGz)){  // Do not support tempalte gzip -- for now --
      templateProcessor.processAndSend(path,indexKeyProcessor);
    } else {
      File file = SPIFFS.open(path, "r");                    // Open the file
      size_t sent = server.streamFile(file, contentType);    // Send it to the client
      file.close();                                          // Close the file again
      debugV("Sent file: %s (%d)",path.c_str(),sent);
    }
    return true;
  }
  debugV("File Not Found: %s",path.c_str());
  return false;                                          // If the file doesn't exist, return false
}


void setupWebServer(){

    MDNS.addService("http", "tcp", 8080);   // Web server

    SPIFFS.begin();                           // Start the SPI Flash Files System

    server.on("/load",HTTP_GET, handleLoadData);
    server.on("/save",HTTP_POST, handleSaveData);

    server.onNotFound([]() {                              // If the client requests any URI
        if (!handleFileRead(server.uri()))                  // send it if it exists
        server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

    server.begin();                           // Actually start the server
    debugV("HTTP server started");
}


void loopWebServer(){
    server.handleClient();
}