#include "MQTTforHA.h"
#include "EvoDebug.h"
#include <strings.h>
#include <ArduinoJson.h>
#include "at8i2cGateway.h"

void MQTTforHA::callback(char* topic, byte* payload, unsigned int length) {
  #ifdef DEBUG_MQTT 
    debugD("Message arrived [%s] %s\n",topic,payload);
  #endif 

  if(strstr(topic,"targetTempCmd") != NULL){
    myConfig.get()->targetTemp[myConfig.get()->hold] = atof((char *)payload) - (myConfig.get()->away?myConfig.get()->awayModify:0);
  } else if(strstr(topic,"thermostatModeCmd") != NULL){
    for (byte i = 0; i < Config::MODES::M_SIZE; i++){
      if(memcmp(payload,myConfig.MODES_MQTT_NAME[i],3) == 0){
        thermostat.setMode((Config::MODES)i);
        break;  
      }
    }
  } else if(strstr(topic,"holdModeCmd") != NULL){
    for (byte i = 0; i < Config::HOLDS::H_SIZE; i++){
      if(memcmp(payload,myConfig.HOLDS_MQTT_NAME[i],3) == 0){
        thermostat.setHold((Config::HOLDS)i);
        break;  
      }
    }
  } else if(strstr(topic,"awayModeCmd") != NULL){
    myConfig.get()->away = (payload[1] == 'N');
  }
  myConfig.saveConfig();
  sendMQTTState();
}

bool MQTTforHA::reconnect() {
    // Loop until we're reconnected
    #ifdef DEBUG_EVENT
      debugD("Attempting MQTT connection..[%s:%s@%s:%s]\n",
      myConfig.get()->mqtt_user,myConfig.get()->mqtt_password,myConfig.get()->mqtt_server,myConfig.get()->mqtt_port);
    #endif
    // Create the client ID
    String clientId = String(myConfig.get()->mqtt_client_id) + "-";
    clientId += String(ESP.getChipId(), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),myConfig.get()->mqtt_user,myConfig.get()->mqtt_password)) {
      #ifdef DEBUG_EVENT
        debugD("connected");
      #endif

      if(!client.publish((String(myConfig.get()->mqtt_topic_prefix) + "/outTopic" ).c_str(), "hello world")){
        debugE("Error sending message!");
      }

      // Memory pool for JSON object tree.
      //
      // Inside the brackets, 200 is the size of the pool in bytes.
      // Don't forget to change this value to match your JSON document.
      // Use arduinojson.org/assistant to compute the capacity.
      StaticJsonDocument<1183> root;

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
      JsonObject device = root.createNestedObject("device");
      device["identifiers"] = ESP.getChipId();
      device["manufacturer"] = "Dianlight Opensource";
      device["model"] = "Smart Temp";
      device["name"] = myConfig.get()->mqtt_client_id;
      device["sw_version"] = CONFIG_VERSION;

      // Add a nested array.
      //
      // It's also possible to create the array separately and add it to the
      // JsonObject but it's less efficient.
      JsonArray data = root.createNestedArray("modes");
      for (byte i = 0; i < Config::MODES::M_SIZE; i++){
        data.add(myConfig.MODES_MQTT_NAME[i]);
      }

      JsonArray holds = root.createNestedArray("hold_modes");
      for (byte i = 0; i < Config::HOLDS::H_SIZE; i++){
        holds.add(myConfig.HOLDS_MQTT_NAME[i]);
      }

      #ifdef DEBUG_MQTT
        serializeJsonPretty(root,Serial);
      #endif

      String json;
      serializeJson(root,json);

      bool c = client.publish((myConfig.get()->mqtt_topic_prefix + topic + String("/config")).c_str(), 
        json.c_str(), true
      ); 
      if(!c){
        debugE("Error sending MQTT message \n\t Topic --> %s \n\t Message --> %s \n\t Size --> %d\n",
          (myConfig.get()->mqtt_topic_prefix + topic + String("/config")).c_str(),
          json.c_str(),
          strlen(json.c_str()));
      #ifdef DEBUG_MQTT
        } else {
          debugD("Config MQTT Send successfull!\n");
      #endif
      }

      if ( sendMQTTState()){
        sendMQTTAvail(true);
      }

      client.subscribe((rtopic +"/targetTempCmd").c_str());
      client.subscribe((rtopic +"/thermostatModeCmd").c_str());
      client.subscribe((rtopic +"/awayModeCmd").c_str());
      client.subscribe((rtopic +"/holdModeCmd").c_str());
      return true;
    } else {
      debugE("MQTT failed, rc=%x",client.state());   
      switch(client.state()){
      case MQTT_CONNECTION_TIMEOUT:
        debugW("Timeout");
        break;
      case MQTT_CONNECTION_LOST:
        debugW("Lost");
        break;
      case MQTT_CONNECT_FAILED:
        debugW("Failed");
        break;
      case MQTT_DISCONNECTED:
        debugW("Disconnected");
        break;
      case MQTT_CONNECTED:
        debugW("Connected");
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        debugW("Bad Protocol");
        break;
      case MQTT_CONNECT_BAD_CLIENT_ID:
        debugW("Bad Client Id");
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        debugW("Unavailable");
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        debugW("Bad Credential");
        break;
      case MQTT_CONNECT_UNAUTHORIZED:
        debugW("Unauthorized");
        break;
      default:
        debugE("Unknown");
        break;
      }
      return false;
    }
}

MQTTforHA::MQTTforHA(Thermostat &thermostat,Config &myConfig, AT8I2CGATEWAY &at8gw): thermostat(thermostat),
 myConfig(myConfig), 
 at8gw(at8gw),
 client(espClient) 
 {
  client.setServer(myConfig.get()->mqtt_server, atoi(myConfig.get()->mqtt_port));
  client.setCallback(std::bind(&MQTTforHA::callback,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
  mqttTicker.attach_ms_scheduled(250,std::bind(&MQTTforHA::loopMQTT,this));
}

void MQTTforHA::loopMQTT() {
  if (!client.loop() && millis() - lastRetry > 30000) {
    if(reconnect())lastRegister = millis();
    lastRetry = millis();
  }
}


void MQTTforHA::sendMQTTAvail(bool online) {
   String topic = String("homeassistant/climate/")+ myConfig.get()->mqtt_client_id;
   #ifdef DEBUG_MQTT
    debugD("MQTT Available: %s\n",(online?"online":"offline"));
   #endif
   bool c = client.publish((myConfig.get()->mqtt_topic_prefix + topic + String("/available")).c_str(), 
        (online?"online":"offline")
      ); 
      if(!c){
        debugE("Error sending MQTT Avail message \n\tTopic --> %s",(myConfig.get()->mqtt_topic_prefix + topic + String("/available")).c_str());
      }
}

String oldjson;

bool MQTTforHA::sendMQTTState() {
  if (!client.connected())return false;

  // Memory pool for JSON object tree.
  //
  // Inside the brackets, 200 is the size of the pool in bytes.
  // Don't forget to change this value to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<770> root;

  // Add values in the object
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root.set<long>("time", 1351824120);
  String topic = String("homeassistant/climate/")+ myConfig.get()->mqtt_client_id;
  
  root["mode"] = CURRENT_MODE_MQTT; // "off" "heat",
  float targetTemp = thermostat.getCurrentTarget();
  root["target_temp"] = targetTemp; //TARGET_TEMP;
  root["away"] = myConfig.get()->away;
  if(!myConfig.get()->away){
    root["hold"] = CURRENT_HOLD_MQTT;
  }
  root["current_temp"] = round(thermostat.getCurrentTemp()*10)/10;
  root["current_action"] = CURRENT_ACTION_MQTT;

  String json;
  serializeJson(root,json);
  if(json.compareTo(oldjson) == 0){
     #ifdef DEBUG_MQTT
      debugD("Skip MQTT status update. Equal JSON\n");
     #endif
     return false;
  }
  oldjson = json;

  bool c = client.publish((myConfig.get()->mqtt_topic_prefix + topic + String("/state")).c_str(), 
    json.c_str()
  ); 
  if(!c){
    debugE("Error sending MQTT message \n\tTopic --> %s \n\tMessage --> %s\n",
      (myConfig.get()->mqtt_topic_prefix + topic + String("/state")).c_str(),
       json.c_str());
    #ifdef DEBUG_MQTT
      } else {
        debugD("Status MQTT Send successfull!\n");
    #endif
  }

  return true;

}


