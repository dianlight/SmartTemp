#include "MyMQTT.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
//#include <ezTime.h>
#include <strings.h>
#include <ArduinoJson.h>
#include "at8i2cGateway.h"
#include "Thermostat.h"

#include "Config.h"
extern Config myConfig;

WiFiClient espClient;
PubSubClient client(espClient);

time_t lastRegister, lastRetry = 0;
extern bool heating;
extern float curTemp;
extern unsigned long manualTime;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\nMessage arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strstr(topic,"targetTempCmd") != NULL){
    myConfig.get()->targetTemp[myConfig.get()->hold] = atof((char *)payload) - (myConfig.get()->away?myConfig.get()->awayModify:0);
  } else if(strstr(topic,"thermostatModeCmd") != NULL){
    for (byte i = 0; i < Config::MODES::M_SIZE; i++){
      if(memcmp(payload,myConfig.MODES_MQTT_NAME[i],3) == 0){
        myConfig.get()->mode = i;
        if(myConfig.get()->mode == Config::MANUAL){
          manualTime = millis();
        }
        break;  
      }
    }
  } else if(strstr(topic,"holdModeCmd") != NULL){
    for (byte i = 0; i < Config::HOLDS::H_SIZE; i++){
      if(memcmp(payload,myConfig.HOLDS_MQTT_NAME[i],3) == 0){
        myConfig.get()->hold = i;
        myConfig.get()->mode = Config::MANUAL;
        manualTime = millis();
        break;  
      }
    }
  } else if(strstr(topic,"awayModeCmd") != NULL){
    myConfig.get()->away = (payload[1] == 'N');
  }
  myConfig.saveConfig();
  sendMQTTState();
}

bool reconnect() {
    // Loop until we're reconnected
    #ifdef DEBUG_EVENT
      Serial.print(String("Attempting MQTT connection..[") +  myConfig.get()->mqtt_user + ":"+ myConfig.get()->mqtt_password + "@" +myConfig.get()->mqtt_server +":" + myConfig.get()->mqtt_port +"] ..");
    #endif
    // Create the client ID
    String clientId = String(myConfig.get()->mqtt_client_id) + "-";
    clientId += String(ESP.getChipId(), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),myConfig.get()->mqtt_user,myConfig.get()->mqtt_password)) {
      #ifdef DEBUG_EVENT
        Serial.println("connected");
      #endif

      if(!client.publish((String(myConfig.get()->mqtt_topic_prefix) + "/outTopic" ).c_str(), "hello world")){
        Serial.println("Error sending message!");
      }

      // Memory pool for JSON object tree.
      //
      // Inside the brackets, 200 is the size of the pool in bytes.
      // Don't forget to change this value to match your JSON document.
      // Use arduinojson.org/assistant to compute the capacity.
      StaticJsonBuffer<1183> jsonBuffer;

      // StaticJsonBuffer allocates memory on the stack, it can be
      // replaced by DynamicJsonBuffer which allocates in the heap.
      //
      // DynamicJsonBuffer  jsonBuffer(200);

      // Create the root of the object tree.
      //
      // It's a reference to the JsonObject, the actual bytes are inside the
      // JsonBuffer with all the other nodes of the object tree.
      // Memory is freed when jsonBuffer goes out of scope.
      JsonObject& root = jsonBuffer.createObject();

      // Add values in the object
      //
      // Most of the time, you can rely on the implicit casts.
      // In other case, you can do root.set<long>("time", 1351824120);
      String topic = String("homeassistant/climate/")+ myConfig.get()->mqtt_client_id;
      String rtopic = String("dianlight/smarttemp/")+ myConfig.get()->mqtt_client_id;

      root["name"] = myConfig.get()->mqtt_client_id,
      root["mode_cmd_t"] = rtopic +"/thermostatModeCmd",
      root["mode_stat_t"] = topic + "/state",
      root["mode_stat_tpl"] = "{{value_json.mode}}",
      root["avty_t"] = topic + "/available",
      root["pl_avail"] = "online",
      root["pl_not_avail"] = "offline",
      root["temp_cmd_t"] = rtopic +"/targetTempCmd",
      root["temp_stat_t"] = topic +"/state",
      root["temp_stat_tpl"] = "{{value_json.target_temp}}",
      root["curr_temp_t"] = topic +"/state",
      root["curr_temp_tpl"] = "{{value_json.current_temp}}",
      root["action_topic"] = topic + "/state",
      root["action_template"] = "{{value_json.current_action}}",
      root["away_mode_stat_t"] = topic + "/state",
      root["away_mode_stat_tpl"] = "{{value_json.away}}",
      root["away_mode_cmd_t"] = rtopic + "/awayModeCmd",
      root["hold_stat_t"] = topic + "/state",
      root["hold_stat_tpl"] = "{{value_json.hold}}",
      root["hold_cmd_t"] = rtopic + "/holdModeCmd",
      root["min_temp"] = "10",
      root["max_temp"] = "35",
      root["temp_step"] = "0.1";
      JsonObject &device = root.createNestedObject("device");
      device["identifiers"] = ESP.getChipId();
      device["manufacturer"] = "Dianlight Opensource";
      device["model"] = "Smart Temp";
      device["name"] = myConfig.get()->mqtt_client_id;
      device["sw_version"] = CONFIG_VERSION;

      // Add a nested array.
      //
      // It's also possible to create the array separately and add it to the
      // JsonObject but it's less efficient.
      JsonArray& data = root.createNestedArray("modes");
      for (byte i = 0; i < Config::MODES::M_SIZE; i++){
        data.add(myConfig.MODES_MQTT_NAME[i]);
      }

      JsonArray& holds = root.createNestedArray("hold_modes");
      for (byte i = 0; i < Config::HOLDS::H_SIZE; i++){
        holds.add(myConfig.HOLDS_MQTT_NAME[i]);
      }

      #ifdef DEBUG_MQTT
        root.prettyPrintTo(Serial);
      #endif

      String json = "";
      root.printTo(json);

      bool c = client.publish((myConfig.get()->mqtt_topic_prefix + topic + String("/config")).c_str(), 
        json.c_str(), true
      ); 
      if(!c){
        Serial.println("\nError sending MQTT message");
        Serial.print("Topic --> ");
        Serial.println((myConfig.get()->mqtt_topic_prefix + topic + String("/config")).c_str());
        Serial.print("Message --> ");
        Serial.println(json.c_str());
        Serial.print("Size --> ");
        Serial.println(strlen(json.c_str()));
      #ifdef DEBUG_MQTT
        } else {
          Serial.println("Config MQTT Send successfull!");
      #endif
      }

      sendMQTTAvail(true);
      sendMQTTState();

      client.subscribe((rtopic +"/targetTempCmd").c_str());
      client.subscribe((rtopic +"/thermostatModeCmd").c_str());
      client.subscribe((rtopic +"/awayModeCmd").c_str());
      client.subscribe((rtopic +"/holdModeCmd").c_str());
      return true;
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(client.state());   
      Serial.print(" --> "); 
      switch(client.state()){
      case MQTT_CONNECTION_TIMEOUT:
        Serial.println("Timeout");
        break;
      case MQTT_CONNECTION_LOST:
        Serial.println("Lost");
        break;
      case MQTT_CONNECT_FAILED:
        Serial.println("Failed");
        break;
      case MQTT_DISCONNECTED:
        Serial.println("Disconnected");
        break;
      case MQTT_CONNECTED:
        Serial.println("Connected");
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        Serial.println("Bad Protocol");
        break;
      case MQTT_CONNECT_BAD_CLIENT_ID:
        Serial.println("Bad Client Id");
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        Serial.println("Unavailable");
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        Serial.println("Bad Credential");
        break;
      case MQTT_CONNECT_UNAUTHORIZED:
        Serial.println("Unauthorized");
        break;
      default:
        Serial.println("Unknown");
        break;
      }
      return false;
    }
}

void setupMQTT() {
  client.setServer(myConfig.get()->mqtt_server, atoi(myConfig.get()->mqtt_port));
  client.setCallback(callback);
}

void loopMQTT() {
  if(lastRegister !=0 && millis() - lastRegister > 60000*30){  // 30min autoregister mtqq
    client.disconnect();
  }
  if (!client.loop() && millis() - lastRetry > 30000) {
    if(reconnect())lastRegister = millis();
    lastRetry = millis();
  }
}


void sendMQTTAvail(bool online) {
   String topic = String("homeassistant/climate/")+ myConfig.get()->mqtt_client_id;
   #ifdef DEBUG_MQTT
    Serial.printf("MQTT Available: %s\n",(online?"online":"offline"));
   #endif
   bool c = client.publish((myConfig.get()->mqtt_topic_prefix + topic + String("/available")).c_str(), 
        (online?"online":"offline")
      ); 
      if(!c){
        Serial.println("\nError sending MQTT Avail message");
        Serial.print("Topic --> ");
        Serial.println((myConfig.get()->mqtt_topic_prefix + topic + String("/available")).c_str());
      #ifdef DEBUG_MQTT
        } else {
          Serial.println("Available MQTT Send successfull!");
      #endif
      }
}

String oldjson;

void sendMQTTState() {
  if (!client.connected())return;

  // Memory pool for JSON object tree.
  //
  // Inside the brackets, 200 is the size of the pool in bytes.
  // Don't forget to change this value to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<770> jsonBuffer;

  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  //
  // DynamicJsonBuffer  jsonBuffer(200);

  // Create the root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.
  JsonObject& root = jsonBuffer.createObject();

  // Add values in the object
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root.set<long>("time", 1351824120);
  String topic = String("homeassistant/climate/")+ myConfig.get()->mqtt_client_id;
  
  #ifdef DEBUG_MQTT
      Serial.println("Preparing Status Message:");
  #endif

  root["mode"] = CURRENT_MODE_MQTT; // "off" "heat",
  float targetTemp = TARGET_TEMP;
  root["target_temp"] = targetTemp; //TARGET_TEMP;
  root["away"] = myConfig.get()->away;
  if(!myConfig.get()->away){
    root["hold"] = CURRENT_HOLD_MQTT;
  }
  root["current_temp"] = round(curTemp*10)/10;
  root["current_action"] = CURRENT_ACTION_MQTT;

  #ifdef DEBUG_MQTT
    root.prettyPrintTo(Serial);
  #endif

  String json = "";
  root.printTo(json);
  if(json.compareTo(oldjson) == 0){
     #ifdef DEBUG_MQTT
      Serial.println("Skip MQTT status update. Equal JSON");
     #endif
     return;
  }
  oldjson = json;

  bool c = client.publish((myConfig.get()->mqtt_topic_prefix + topic + String("/state")).c_str(), 
    json.c_str()
  ); 
  if(!c){
    Serial.println("\nError sending MQTT message");
    Serial.print("Topic --> ");
    Serial.println((myConfig.get()->mqtt_topic_prefix + topic + String("/state")).c_str());
    Serial.print("Message --> ");
    Serial.println(json.c_str());
    #ifdef DEBUG_MQTT
      } else {
        Serial.println("Status MQTT Send successfull!");
    #endif
  }

}

