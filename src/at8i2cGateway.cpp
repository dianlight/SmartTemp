#include <Wire.h>
#include <Arduino.h>
#include "at8i2cGateway.h"



AT8I2CGATEWAY::AT8I2CGATEWAY(int i2caddress) {
    Wire.begin();
    _i2caddress = i2caddress;
}

float AT8I2CGATEWAY::getHumidity() {   
    return indata.sensorData.in.humidity;
}

float AT8I2CGATEWAY::getTemperature() {
    return indata.sensorData.in.temperature;
}

int8_t AT8I2CGATEWAY::getEncoder(){
    return indata.sensorData.in.encoder;
}

byte  AT8I2CGATEWAY::getEncoderButton(){
    return indata.sensorData.in.select;
}

bool AT8I2CGATEWAY::getRelay(){
    return indata.sensorData.in.relay;
} 

void AT8I2CGATEWAY::setRelay(bool on){
    #ifdef DEBUG_I2C_OUT
        Serial.printf("I2C<--");
        Serial.print(_i2caddress,HEX);
        Serial.printf("<--");
        Serial.print(sizeof(I2C_Packet_t));
    #endif
    I2C_Packet_t data;
    data.sensorData.out.cmd = RELAY;
    data.sensorData.out.relay = on;
    Wire.beginTransmission(_i2caddress);
    size_t writed = Wire.write(data.I2CPacket, sizeof(I2C_Packet_t));
    Wire.endTransmission();
    #ifdef DEBUG_I2C_OUT
     for(size_t i=0; i < writed; i++){
        Serial.write(data.I2CPacket[i]);            
        Serial.print("[0x"); // print the character
        Serial.print(data.I2CPacket[i],HEX);         // print the character
        Serial.print("] "); // print the character
     }
     Serial.println();
    #endif
}

void AT8I2CGATEWAY::i2cReader(){
    #ifdef DEBUG_I2C_IN
        Serial.printf("I2C-->");
        Serial.print(_i2caddress,HEX);
        Serial.printf("-->");
        Serial.print(sizeof(I2C_Packet_t));
    #endif
    Wire.requestFrom(_i2caddress,sizeof(I2C_Packet_t));
    #ifdef DEBUG_I2C_IN
        Serial.print("-->");
        Serial.print(Wire.available());
        Serial.print("-->");
    #endif

    delay(100);
    Wire.setTimeout(500);
    size_t readed = Wire.readBytes(indata.I2CPacket,sizeof(I2C_Packet_t));
    if(readed != sizeof(I2C_Packet_t)){
        Serial.printf("\n*ERR* Truncated Message or Timeout! %d != %d\n",readed,sizeof(I2C_Packet_t));
    }
    #ifdef DEBUG_I2C_IN
     for(i=0; i < readed; i++){
        Serial.write(indata.I2CPacket[i]);            
        Serial.print("[0x"); // print the character
        Serial.print(indata.I2CPacket[i],HEX);         // print the character
        Serial.print("] "); // print the character
     }
     Serial.println();
    #endif
}