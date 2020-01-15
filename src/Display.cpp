#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "Config.h"
//#include <Ticker.h>
#ifdef ENABLE_MENU
  #include <menu.h>
  #include <menuIO/u8g2Out.h>
  #include "menuIO/clickEncoderI2CIn.h"
  #include <menuIO/chainStream.h>
  #include <menuIO/serialOut.h>
  #include <menuIO/serialIn.h>
  #include <plugin/barField.h>
#else
  #include "at8i2cGateway.h"  
#endif
#include <Wire.h>

#include "MyTimeNTP.h"
//#include <ezTime.h>

#include "Thermostat.h"
#include "Network.h"

extern Config myConfig;

//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA); 
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA); 

//#define fontName u8g2_font_profont10_tf
#define fontName u8g2_font_haxrcorp4089_tr
#define fontX 8
#define fontY 10
#define offsetX 0
#define offsetY 0
#define U8_Width 128
#define U8_Height 64
#define USE_HWI2C


extern PubSubClient client;
//extern Timezone myTZ;
// Base structure data:
extern bool heating;
extern float curTemp;
extern float curHumidity;

#ifdef ENABLE_MENU
  // define menu colors --------------------------------------------------------
  //each color is in the format:
  //  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
  // this is a monochromatic color table
  const colorDef<uint8_t> colors[6] MEMMODE={
    {{0,0},{0,1,1}},//bgColor
    {{1,1},{1,0,0}},//fgColor
    {{1,1},{1,0,0}},//valColor
    {{1,1},{1,0,0}},//unitColor
    {{0,1},{0,0,1}},//cursorColor
    {{1,1},{1,0,0}},//titleColor
  };

  result doSaveConfig(eventMask e, prompt &item);
  result doWiFiCaptiveConfig(eventMask e, prompt &item);
  result doWiFiWPSConfig(eventMask e, prompt &item);


  MENU(setupMenu,"Calibration Setup",doNothing,noEvent,noStyle
    ,BARFIELD(myConfig.get()->targetTemp[Config::HOLDS::ECO], "    T Eco: ","°C",5,70,1,0.1,doSaveConfig,noEvent,wrapStyle)
    ,FIELD(myConfig.get()->targetTemp[Config::HOLDS::NORMAL], " T Normal: ","°C",5,70,1,0.1,doSaveConfig,noEvent,wrapStyle)
    ,FIELD(myConfig.get()->targetTemp[Config::HOLDS::CONFORT],"T Confort: ","°C" ,5,70,1,0.1,doSaveConfig,noEvent,wrapStyle)
    ,FIELD(myConfig.get()->awayModify,                        "Away Diff: ","°C",-5,+5,1,0.1,doSaveConfig,noEvent,wrapStyle)
    ,FIELD(myConfig.get()->tempPrecision,                     "Precision: ","°C",0,5,1,0.1,doSaveConfig,noEvent,wrapStyle)
    ,FIELD(myConfig.get()->minSwitchTime,                     "Switch rs: ","sec",10,120,10,1,doSaveConfig,noEvent,wrapStyle)
    ,OP("",doNothing,noEvent)
    ,OP("Factory Reset *WIP*",doNothing,noEvent)
    ,OP("",doNothing,noEvent)
    ,EXIT("<Back")
  );

  //char* const numbers PROGMEM="0123456789";//allowed hexadecimal digits 
  //char* const digit PROGMEM="0123456789abcdefghijklmnopqrstuvxyz";//allowed hexadecimal digits
  //char* const digit1 PROGMEM="0123456789abcdefghijklmnopqrstuvxyz._@#&%$!?";//allowed hexadecimal digits
  //char* const digitNr[] PROGMEM={digit};//validators
  //char* const digit1Nr[] PROGMEM={digit1};//validators
  //char* const numbersNr[] PROGMEM={numbers};//validators

  char *wifiOP = "  WIFI: _________________________________";
  char *mqttOP = "  MQTT: _________________________________";
  char *ipOP =   "    IP: _________________________________";

  MENU(networkMenu,"Network Setup",doNothing,noEvent,noStyle
    ,OP(  "  WIFI: ",doNothing,noEvent)
    ,OP(  "    IP: ",doNothing,noEvent)
    ,OP(  "  MQTT: ",doNothing,noEvent)
    ,OP("",doNothing,noEvent)
  //  ,EDIT("  Name: ",myConfig.get()->mqtt_client_id,digitNr,doSaveConfig,noEvent,wrapStyle)
  //  ,EDIT("Server: ",myConfig.get()->mqtt_server,digit1Nr,doSaveConfig,noEvent,wrapStyle)
  //  ,EDIT("  Port: ",myConfig.get()->mqtt_port,numbersNr,doSaveConfig,noEvent,wrapStyle)
  //  ,EDIT("  User: ",myConfig.get()->mqtt_user,digit1Nr,doSaveConfig,noEvent,wrapStyle)
  //  ,EDIT("Passwd: ",myConfig.get()->mqtt_password,digit1Nr,doSaveConfig,noEvent,wrapStyle)
  //  ,EDIT(" Topic: ",myConfig.get()->mqtt_topic_prefix,digit1Nr,doSaveConfig,noEvent,wrapStyle)
  //  ,OP("",doNothing,noEvent)
    ,OP("WiFi AP Config mode.",doWiFiCaptiveConfig,enterEvent)
    ,OP("WiFi WPS Config mode.",doWiFiWPSConfig,enterEvent)
    ,OP("",doNothing,noEvent)
    ,EXIT("<Back")
  );

  //MENU(programMenu,"Week Program *WIP*",doNothing,noEvent,noStyle
  //  ,OP("**Work in Progress**",doNothing,noEvent)
  //  ,EXIT("<Back")
  //);



  TOGGLE(myConfig.get()->mode,modeSel,"  Mode: ",doSaveConfig,noEvent,wrapStyle
      ,VALUE("Auto",Config::MODES::AUTO,doNothing,noEvent)
      ,VALUE("Manual",Config::MODES::MANUAL,doNothing,noEvent)
      ,VALUE("Off",Config::MODES::OFF,doNothing,noEvent)
  );

  TOGGLE(myConfig.get()->away,awaySel,"Preset: ",doNothing,noEvent,wrapStyle
      ,VALUE("Away",HIGH,doNothing,noEvent)
      ,VALUE("In",LOW,doNothing,noEvent)
  )

  TOGGLE(myConfig.get()->hold,holdSel,"  Hold: ",doNothing,noEvent,wrapStyle
      ,VALUE("Eco",Config::HOLDS::ECO,doNothing,noEvent)
      ,VALUE("Normal",Config::HOLDS::NORMAL,doNothing,noEvent)
      ,VALUE("Confort",Config::HOLDS::CONFORT,doNothing,noEvent)
  )

  MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
    ,SUBMENU(awaySel)
    ,SUBMENU(modeSel)
    ,SUBMENU(holdSel)
  //  ,OP("",doNothing,noEvent)
  //  ,SUBMENU(programMenu)
    ,OP("",doNothing,noEvent)
    ,SUBMENU(setupMenu)
    ,SUBMENU(networkMenu)
    ,OP("",doNothing,noEvent)
    ,EXIT("<Exit")
  );
#endif // #ifdef ENABLE_MENU

#define MAX_DEPTH 2

#ifdef DEBUG_SERIAL_MENU
  serialIn serial(Serial);
#endif

extern AT8I2CGATEWAY at8gw;

#ifdef ENABLE_MENU
  ClickEncoderI2CStream encStream(at8gw,1);
  MENU_INPUTS(in
    #ifdef DEBUG_SERIAL_MENU 
      ,&serial
    #endif
    ,&encStream
  );

  MENU_OUTPUTS(out,MAX_DEPTH
    ,U8G2_OUT(u8g2,colors,5,8,offsetX,offsetY,{0,0,U8_Width/5,U8_Height/8})
    #ifdef DEBUG_SERIAL_MENU 
      ,SERIAL_OUT(Serial)
    #else
      ,NONE  
    #endif
  );

  NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);
#endif // ENABLE_MENU


int8_t lastEncPosition =0;

#ifdef ENABLE_MENU
  //when menu is suspended
  result idle(menuOut& o,idleEvent e) {
    o.clear();    
    switch(e) {
      case idleStart:
          o.println("suspending menu!");
          lastEncPosition = at8gw.getEncoder();
          break;
      case idling:
          o.println("suspended.");
          break;
      case idleEnd:
          o.println("resuming menu.");
          networkMenu[0].disable();
          strcpy(&wifiOP[8],(WiFi.status() == WL_CONNECTED)?WiFi.SSID().c_str():"* Not Connected*");
          networkMenu[0].shadow->text = wifiOP;
          networkMenu[1].disable();
          strcpy(&ipOP[8],(client.state() == 0)?WiFi.localIP().toString().c_str():"* Not Connected*");
          networkMenu[1].shadow->text = ipOP;
          networkMenu[2].disable();
          strcpy(&mqttOP[8],(client.state() == 0)?"Connected":"* Not Connected*");
          networkMenu[2].shadow->text = mqttOP;
          break;
    }
    Serial.print(".");
    return proceed;
  }
#endif // ENABLE_MENU

void setupDisplay() {
      u8g2.begin();
      u8g2.enableUTF8Print();
      u8g2.setFont(fontName);
      u8g2.firstPage();
      do {
          u8g2.drawUTF8(0,16,"Please wait...");
      } while ( u8g2.nextPage() );   

      #ifdef DEBUG_SERIAL_MENU 
        Serial.println("Use keys + - * /");
      #endif
      #ifdef ENABLE_MENU
        nav.idleTask=idle;//point a function to be used when menu is suspended
        nav.showTitle=false;
  //      nav.sleepTask=idle;
  //      nav.idleChanged=true;
        nav.timeOut=30;
        nav.idleOn();
      #endif // ENABLE_MENU

}


unsigned long lastTime = 0, lastBtnTime = 0;

void loopDisplay() {
  #ifdef ENABLE_MENU
    nav.doInput();
    if( nav.sleepTask ){
  #endif // ENABLE_MENU  

    if( at8gw.getEncoder() > lastEncPosition){
      myConfig.get()->mode= Config::MODES::MANUAL;
      myConfig.get()->hold= (myConfig.get()->hold+1) %  Config::HOLDS::H_SIZE;
      lastEncPosition = at8gw.getEncoder();
      myConfig.saveConfig();
    } else if ( at8gw.getEncoder() < lastEncPosition){
      myConfig.get()->mode= Config::MODES::MANUAL;
      myConfig.get()->hold= abs(myConfig.get()->hold-1) %  Config::HOLDS::H_SIZE;
      lastEncPosition = at8gw.getEncoder();
      myConfig.saveConfig();
    }

    if( at8gw.getEncoderButton() == 3 && myConfig.get()->mode != Config::MODES::AUTO && ( millis() - lastBtnTime ) > 3000){ // Hold
      myConfig.get()->mode = Config::MODES::AUTO;
      lastBtnTime = millis();
      myConfig.saveConfig();
    } else if( at8gw.getEncoderButton() == 3 && myConfig.get()->mode == Config::MODES::AUTO && ( millis() - lastBtnTime ) > 3000){ // Hold
      myConfig.get()->away = !myConfig.get()->away;
      lastBtnTime = millis();
      myConfig.saveConfig();
    }

    bool blink = ( millis() - lastTime ) > 800;
    if(( millis() - lastTime ) > 1600)lastTime = millis();

    u8g2.firstPage();
    do {
        u8g2.clearBuffer();
        if (WiFi.status() == WL_CONNECTED) {
            u8g2.setFont(u8g2_font_open_iconic_embedded_1x_t);
            u8g2.drawGlyph(128-8, 10, 0x50);	
        } else if(blink) {
            u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
            u8g2.drawGlyph(128-8, 10, 0x4A);	
        }

        if(heating && blink){
            u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
            u8g2.drawGlyph(128-17, 10, 0x4E);	
        }

        if(client.state() == 0){
          u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
          u8g2.drawGlyph(128-28,10,0x46);
        } else if(!blink){
          u8g2.setFont(u8g2_font_open_iconic_embedded_1x_t);
          u8g2.drawGlyph(128-28,10,0x47);
        }

        if(myConfig.get()->away){
            u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
            u8g2.drawGlyph(0, 10, 0x41);	
        } else {
            u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
            u8g2.drawGlyph(0, 10, 0x40);	
        }



        // Temp ant time display
        u8g2.setFont(fontName);
        if(myConfig.get()->mode != Config::OFF){
            u8g2.setCursor(12,12);
            u8g2.printf("%2.1f\xB0\x43",TARGET_TEMP);
        }
        u8g2.setCursor(48+10,12);
        u8g2.printf("%2.0f%%",curHumidity);

        u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
        u8g2.drawGlyph(48, 10, 0x48);

        // Footer ( Time and Date )
        u8g2.setFont(fontName);
        u8g2.setCursor(0,63);
        u8g2.printf("%s",CURRENT_MODE_STR);
        u8g2.setCursor(5*fontX,63);
        u8g2.printf("%s",CURRENT_HOLD_STR);
        u8g2.setCursor(128-45,63);
//        u8g2.printf("%s",myTZ.dateTime(String("D H")+(blink?":":" ")+"i").c_str()); //G
        u8g2.printf("%d %d:%d",getNTPTime()->tm_wday,getNTPTime()->tm_hour, getNTPTime()->tm_min);


        // Main display
        u8g2.drawHLine(0,14,128);
        u8g2.drawHLine(0,14+28,128);
        u8g2.setFont(u8g2_font_profont29_tn);
        u8g2.setCursor(26,40);
        u8g2.printf("%2.1f",curTemp);
        u8g2.setFont(u8g2_font_5x7_mf);
        u8g2.drawGlyph(90,44-22,'o');
        u8g2.setFont(fontName);
        u8g2.drawGlyph(90+5,44-17,'C');

        // Program
        if(myConfig.get()->mode == Config::AUTO){
          u8g2.drawHLine(0,63-fontY-1,128);
          for(u8 i=1,h=0; i < 128; i+=4,h++){
//            if(h == myTZ.hour()){
            if(h == getNTPTime()->tm_hour){
              for(u8 im=i+1,dm=0; dm < 4; dm++,im+=9){
//                if(dm == floor(myTZ.minute() / 15)){
                if(dm == floor(getNTPTime()->tm_min / 15)){
                  if(!blink){
                    u8g2.drawFrame(im,14+28,9,3+myConfig.get()->weekProgram->hourQuarterHolds[(h*4)+dm]*3);
                  } else {
                    u8g2.drawBox(im,14+28,9,3+myConfig.get()->weekProgram->hourQuarterHolds[(h*4)+dm]*3);
                  }
                  myConfig.get()->hold = myConfig.get()->weekProgram->hourQuarterHolds[(h*4)+dm];
                  im++;
                } else {
                  u8g2.drawBox(im,14+28,8,3+myConfig.get()->weekProgram->hourQuarterHolds[(h*4)+dm]*3);
                }
              }
              i+=35;
            } else {
              u8g2.drawBox(i,14+28,3,3+
                ((
                   (myConfig.get()->weekProgram->hourQuarterHolds[(h*4)]*3) +
                   (myConfig.get()->weekProgram->hourQuarterHolds[(h*4)+1]*3) +
                   (myConfig.get()->weekProgram->hourQuarterHolds[(h*4)+2]*3) +
                   (myConfig.get()->weekProgram->hourQuarterHolds[(h*4)+3]*3) 
                )/4)
              );
            }
          }
        }

    } while ( u8g2.nextPage() );
  #ifdef ENABLE_MENU  
    } else if (nav.changed(0)) {
        //only draw if menu changed for gfx device
        //change checking leaves more time for other tasks
        u8g2.setFont(u8g2_font_5x7_mf);
        u8g2.firstPage();
        do nav.doOutput(); while(u8g2.nextPage());
    }
  #endif //ENABLE_MENU
}

void  bootAPDisplay(String AP){
      u8g2.setFont(fontName);
      u8g2.clearBuffer();
      u8g2.firstPage();
      do {
          u8g2.drawUTF8(0,16,"Wifi Config Mode");
          u8g2.drawUTF8(0,26,String(String("AP:") +  AP).c_str());
          u8g2.drawUTF8(0,63-fontY,"Use a Wifi device \nto connect and configure!");
      } while ( u8g2.nextPage() );    
}

void  bootConnectedDisplay(){
      u8g2.setFont(fontName);
      u8g2.clearBuffer();
      u8g2.firstPage();
      do {
          u8g2.drawUTF8(0,16,String(String("AP:")+WiFi.SSID()).c_str());
          u8g2.drawUTF8(0,26,String(String("IP:")+WiFi.localIP().toString()).c_str());
      } while ( u8g2.nextPage() );    
}

void displayError(String Error){
      u8g2.setFont(fontName);
      u8g2.clearBuffer();
      u8g2.firstPage();
      do {
          u8g2.drawUTF8(0,18,Error.c_str());
      } while ( u8g2.nextPage() );    
}

void clearDisplay() {
    u8g2.clear();
};

void displayProgress(u8 perc)
{	
    u8g2.setFont(fontName);
    u8g2.clearBuffer();
    u8g2.firstPage();
    do {
        u8g2.drawUTF8(0,12,"OTA Update");
        u8g2.setCursor(52,26);
        u8g2.printf("%.2d%%",perc);
        u8g2.setDrawColor(2);
        u8g2.drawRBox(7,15,perc+14,14,3);
    } while ( u8g2.nextPage() );    
}

#ifdef ENABLE_MENU
  result doSaveConfig(eventMask e, prompt &item){
      myConfig.saveConfig();
      return proceed;
  }

  result doWiFiCaptiveConfig(eventMask e, prompt &item){
      setupWifi(true);
      return quit;
  }

  result doWiFiWPSConfig(eventMask e, prompt &item){
      displayError("Press WPS on your router...");
      WiFi.mode(WIFI_STA);
      WiFi.beginWPSConfig();
      ESP.restart();
      return quit;
  }
#endif // NABLE_MENU
