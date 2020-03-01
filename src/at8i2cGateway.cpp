#include <Wire.h>
#include <Arduino.h>
#include "at8i2cGateway.h"
#include "Config.h"
#include "EvoDebug.h"

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
        debugI("I2C<--%x<--%d",_i2caddress,sizeof(I2C_Packet_t));
    #endif
    I2C_Packet_t data;
    data.sensorData.out.cmd = RELAY;
    data.sensorData.out.relay = on;
    Wire.beginTransmission(_i2caddress);
    Wire.write(data.I2CPacket, sizeof(I2C_Packet_t));
    Wire.endTransmission();
}

void AT8I2CGATEWAY::i2cReader(){
    #ifdef DEBUG_I2C_IN
        debugI("I2C-->%02x-->%d",_i2caddress,sizeof(I2C_Packet_t));
    #endif
    Wire.requestFrom(_i2caddress,sizeof(I2C_Packet_t));
    #ifdef DEBUG_I2C_IN
        debugI("-->%d-->",Wire.available());
    #endif

    delay(100);
    Wire.setTimeout(500);
    size_t readed = Wire.readBytes(indata.I2CPacket,sizeof(I2C_Packet_t));
    if(readed != sizeof(I2C_Packet_t)){
        if(readed == 0){
            debugW("I2C Timeout");
        } else {
            debugE("I2C Truncated Message!  %d != %d",readed,sizeof(I2C_Packet_t));
        }
    }
}

// TODO: Im plement start and stop functions
bool AT8I2CGATEWAY::start(){return true;}
bool AT8I2CGATEWAY::stop(){return false;}


AT8I2CGATEWAY at8gw(AT8_I2C_GW);
