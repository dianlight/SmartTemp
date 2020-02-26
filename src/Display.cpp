#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "Display.h"
#include <Wire.h>

#include "MyTimeNTP.h"

//#define fontName u8g2_font_profont10_tf
#define fontName u8g2_font_haxrcorp4089_tr
#define fontX 8
#define fontY 10
#define offsetX 0
#define offsetY 0

//extern PubSubClient client;
//extern bool heating;
extern float curHumidity;
#define MAX_DEPTH 2

static time_t lastAction;
static bool psMode = false;

void Display::sleepModeDisplay()
{
  if (millis() - lastAction > myConfig.get()->displayPowerOff * 1000 && !myConfig.getMode()->active)
  {
    _u8g2.setPowerSave(1);
#ifdef DEBUG_REMOTE
    debugV("PowerOff Display");
#endif
    psMode = true;
  }
  else if (millis() - lastAction > myConfig.get()->displaySleep * 1000 && !myConfig.getMode()->active)
  {
    uint8_t ctn = map((millis() - lastAction) / 1000, myConfig.get()->displaySleep, myConfig.get()->displayPowerOff, myConfig.get()->displayContrast / 2, 0);
    _u8g2.setContrast(ctn);
#ifdef DEBUG_REMOTE
    debugV("Display Power %d", ctn);
#endif
    psMode = true;
  }
  else
  {
    psMode = false;
  }
}

Display::Display(Thermostat &thermostat, Config &myConfig, PubSubClient &mqttclient, AT8I2CGATEWAY &at8gw) : thermostat(thermostat),
                                                                                                             myConfig(myConfig),
                                                                                                             mqttclient(mqttclient),
                                                                                                             at8gw(at8gw)
{

  //U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
  //U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);
  // U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);
  _u8g2 = U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/SCL, /* data=*/SDA);

  _u8g2.begin();
  _u8g2.enableUTF8Print();
  _u8g2.setFont(fontName);
  lastAction = millis();
  sleepModeTicker.attach_scheduled(myConfig.get()->displaySleep / 3, std::bind(&Display::sleepModeDisplay, this));

  _u8g2.firstPage();
  do
  {
    _u8g2.drawUTF8(0, 16, "Please wait...");
  } while (_u8g2.nextPage());
}

unsigned long lastTime = 0, lastBtnTime = 0;

void Display::loopDisplay()
{
  if (at8gw.getEncoder() > lastEncPosition && !psMode && !myConfig.getMode()->active)
  {
    thermostat.setMode(Config::MODES::MANUAL);
    myConfig.get()->mode = Config::MODES::MANUAL;
    myConfig.get()->hold = (myConfig.get()->hold + 1) % Config::HOLDS::H_SIZE;
    lastEncPosition = at8gw.getEncoder();
    lastAction = millis();
    myConfig.saveConfig();
  }
  else if (at8gw.getEncoder() < lastEncPosition && !psMode && !myConfig.getMode()->active)
  {
    thermostat.setMode(Config::MODES::MANUAL);
    myConfig.get()->hold = abs(myConfig.get()->hold - 1) % Config::HOLDS::H_SIZE;
    lastEncPosition = at8gw.getEncoder();
    lastAction = millis();
    myConfig.saveConfig();
  }
  else if (at8gw.getEncoder() != lastEncPosition && !psMode && myConfig.getMode()->active)
  {
    myConfig.getMode()->encoder += (at8gw.getEncoder() - lastEncPosition);
    lastEncPosition = at8gw.getEncoder();
    lastAction = millis();
  }
  else if (at8gw.getEncoder() != lastEncPosition || at8gw.getEncoderButton() > 0)
  {
    psMode = false;
    _u8g2.setPowerSave(0);
    _u8g2.setContrast(myConfig.get()->displayContrast);
    lastAction = millis();
    lastEncPosition = at8gw.getEncoder();
  }

  if (at8gw.getEncoderButton() == AT8I2CGATEWAY::CLICKED && myConfig.get()->mode != Config::MODES::AUTO && !myConfig.getMode()->active)
  { // Click
    myConfig.get()->mode = Config::MODES::AUTO;
    lastBtnTime = millis();
    lastAction = millis();
    myConfig.saveConfig();
  }
  else if (at8gw.getEncoderButton() == AT8I2CGATEWAY::CLICKED && myConfig.getMode()->active)
  { // Click
    myConfig.getMode()->position = (myConfig.getMode()->position + 1) % Config::CONFIG_TARGET::_CONFIG_MODE_SIZE;
    myConfig.getMode()->encoder = 0;
    lastBtnTime = millis();
    lastAction = millis();
  }
  else if (at8gw.getEncoderButton() == AT8I2CGATEWAY::HELD && (millis() - lastBtnTime) > 3000 && myConfig.getMode()->active)
  { // Hold
    myConfig.getMode()->position = abs(myConfig.getMode()->position - 1) % Config::CONFIG_TARGET::_CONFIG_MODE_SIZE;
    myConfig.getMode()->encoder = 0;
    lastBtnTime = millis();
    lastAction = millis();
  }
  else if (at8gw.getEncoderButton() == AT8I2CGATEWAY::HELD && myConfig.get()->mode == Config::MODES::AUTO && (millis() - lastBtnTime) > 3000)
  { // Hold
    myConfig.get()->away = !myConfig.get()->away;
    lastBtnTime = millis();
    lastAction = millis();
    myConfig.saveConfig();
  }
  else if (at8gw.getEncoderButton() == AT8I2CGATEWAY::DOUBLECLICKED)
  { // Double click
    myConfig.getMode()->active = !myConfig.getMode()->active;
    myConfig.saveConfig();
    if (myConfig.get()->configSize == 0)
      ESP.restart();
  }

  bool blink = (millis() - lastTime) > 800;
  bool fblink = (millis() - lastTime) > 900;
  if ((millis() - lastTime) > 1600)
    lastTime = millis();

  _u8g2.firstPage();
  do
  {
    _u8g2.clearBuffer();

    // Header ( Target temp, Humidity and icons )

    if (WiFi.status() == WL_CONNECTED)
    {
      _u8g2.setFont(u8g2_font_open_iconic_embedded_1x_t);
      _u8g2.drawGlyph(128 - 8, 10, 0x50);
    }
    else if (blink)
    {
      _u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
      _u8g2.drawGlyph(128 - 8, 10, 0x4A);
    }

    if (at8gw.getRelay() && blink)
    {
      _u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
      _u8g2.drawGlyph(128 - 17, 10, 0x4E);
    }

    if (mqttclient.state() == 0)
    {
      _u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
      _u8g2.drawGlyph(128 - 28, 10, 0x46);
    }
    else if (!blink)
    {
      _u8g2.setFont(u8g2_font_open_iconic_embedded_1x_t);
      _u8g2.drawGlyph(128 - 28, 10, 0x47);
    }

    if (myConfig.get()->away)
    {
      _u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
      _u8g2.drawGlyph(0, 10, 0x41);
    }
    else
    {
      _u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
      _u8g2.drawGlyph(0, 10, 0x40);
    }

    _u8g2.setFont(fontName);
    if (myConfig.get()->mode != Config::OFF)
    {
      _u8g2.setCursor(12, 10);
      _u8g2.printf("%2.1f\xB0\x43", thermostat.getCurrentTarget());
    }
    _u8g2.setCursor(48 + 10, 10);
    _u8g2.printf("%2.0f%%", curHumidity);

    _u8g2.setFont(u8g2_font_open_iconic_thing_1x_t);
    _u8g2.drawGlyph(48, 10, 0x48);

    // Footer ( Time and Date )
    _u8g2.setFont(fontName);
    _u8g2.setCursor(0, 63);
    if (!myConfig.getMode()->active)
    {
      _u8g2.printf("%s", CURRENT_MODE_STR);
      _u8g2.setCursor(5 * fontX, 63);
      _u8g2.printf("%s", CURRENT_HOLD_STR);
      _u8g2.setCursor(128 - 47, 63);
      _u8g2.printf("%s  %.2d:%.2d", DAYSNAME[getNTPTime()->tm_wday].c_str(), getNTPTime()->tm_hour, getNTPTime()->tm_min);
    }
    else
    {
      _u8g2.printf("%s", "Config");
      _u8g2.setCursor(5 * fontX, 63);
      _u8g2.printf("%s", CONFIG_TARGET_STR);
      //          u8g2.setCursor(128-45,63);
      //          u8g2.printf("%s %.2d:%.2d",DAYSNAME[getNTPTime()->tm_wday].c_str(),getNTPTime()->tm_hour, getNTPTime()->tm_min);
    }

    // Main display
    _u8g2.drawHLine(0, 13, 128);
    _u8g2.drawHLine(0, 14 + 28, 128);
    if (!myConfig.getMode()->active)
    {
      _u8g2.setFont(u8g2_font_profont29_tr);
      _u8g2.setCursor(26, 40);
      _u8g2.printf("%2.1f", thermostat.getCurrentTemp());
      _u8g2.setFont(u8g2_font_5x7_mf);
      _u8g2.drawGlyph(90, 44 - 22, 'o');
      _u8g2.setFont(fontName);
      _u8g2.drawGlyph(90 + 5, 44 - 17, 'C');
    }
    else
    {
      switch (myConfig.getMode()->position)
      {
      case Config::CONFIG_TARGET::ECO_TEMP:
        _u8g2.setFont(u8g2_font_profont29_tr);
        _u8g2.setCursor(26, 40);
        myConfig.get()->targetTemp[0] += (myConfig.getMode()->encoder / 5.0);
        myConfig.getMode()->encoder = 0;
        if (fblink)
          _u8g2.printf("%2.1f", myConfig.get()->targetTemp[0]);
        _u8g2.setFont(u8g2_font_5x7_mf);
        _u8g2.drawGlyph(90, 44 - 22, 'o');
        _u8g2.setFont(fontName);
        _u8g2.drawGlyph(90 + 5, 44 - 17, 'C');
        break;
      case Config::CONFIG_TARGET::NORMAL_TEMP:
        _u8g2.setFont(u8g2_font_profont29_tr);
        _u8g2.setCursor(26, 40);
        myConfig.get()->targetTemp[1] += (myConfig.getMode()->encoder / 5.0);
        myConfig.getMode()->encoder = 0;
        if (fblink)
          _u8g2.printf("%2.1f", myConfig.get()->targetTemp[1]);
        _u8g2.setFont(u8g2_font_5x7_mf);
        _u8g2.drawGlyph(90, 44 - 22, 'o');
        _u8g2.setFont(fontName);
        _u8g2.drawGlyph(90 + 5, 44 - 17, 'C');
        break;
      case Config::CONFIG_TARGET::CONFORT_TEMP:
        _u8g2.setFont(u8g2_font_profont29_tr);
        _u8g2.setCursor(26, 40);
        myConfig.get()->targetTemp[2] += (myConfig.getMode()->encoder / 5.0);
        myConfig.getMode()->encoder = 0;
        if (fblink)
          _u8g2.printf("%2.1f", myConfig.get()->targetTemp[2]);
        _u8g2.setFont(u8g2_font_5x7_mf);
        _u8g2.drawGlyph(90, 44 - 22, 'o');
        _u8g2.setFont(fontName);
        _u8g2.drawGlyph(90 + 5, 44 - 17, 'C');
        break;
      case Config::CONFIG_TARGET::AWAY_DIFF:
        _u8g2.setFont(u8g2_font_profont29_tr);
        _u8g2.setCursor(26, 40);
        myConfig.get()->awayModify += (myConfig.getMode()->encoder / 10.0);
        myConfig.getMode()->encoder = 0;
        if (fblink)
          _u8g2.printf("%2.1f", myConfig.get()->awayModify);
        _u8g2.setFont(u8g2_font_5x7_mf);
        _u8g2.drawGlyph(90, 44 - 22, 'o');
        _u8g2.setFont(fontName);
        _u8g2.drawGlyph(90 + 5, 44 - 17, 'C');
        break;
      case Config::CONFIG_TARGET::MANUAL_TO_AUTO_TIME:
        _u8g2.setFont(u8g2_font_profont29_tr);
        _u8g2.setCursor(26, 40);
        myConfig.get()->returnAutoTimeout += (myConfig.getMode()->encoder);
        myConfig.getMode()->encoder = 0;
        if (fblink)
          _u8g2.printf("%d", myConfig.get()->returnAutoTimeout);
        _u8g2.setFont(fontName);
        _u8g2.setCursor(90, 44 - 17);
        _u8g2.print("sec");
        break;
      case Config::CONFIG_TARGET::AWAY_HOLDS:
        _u8g2.setFont(u8g2_font_profont29_tr);
        _u8g2.setCursor(0, 40);
        myConfig.get()->awayMode = abs((myConfig.get()->awayMode + myConfig.getMode()->encoder) % Config::AWAY_MODES::AM_SIZE);
        myConfig.getMode()->encoder = 0;
        if (fblink)
          _u8g2.printf("%s", myConfig.AWAY_MODES_NAME[myConfig.get()->awayMode]);
        break;
      case Config::CONFIG_TARGET::INFO:
        _u8g2.setFont(fontName);
        _u8g2.drawUTF8(0, 26, "Version:" _SMT_VERSION "." CONFIG_VERSION);
        //            u8g2.drawUTF8(0,36,String(String("IP:")+WiFi.localIP().toString()).c_str());
        break;
      case Config::CONFIG_TARGET::WIFI:
        _u8g2.setFont(fontName);
        _u8g2.drawUTF8(0, 26, String(String("AP:") + WiFi.SSID()).c_str());
        _u8g2.drawUTF8(0, 36, String(String("IP:") + WiFi.localIP().toString()).c_str());
        break;
      case Config::CONFIG_TARGET::FACTORY_RESET:
        _u8g2.setFont(u8g2_font_profont29_tr);
        _u8g2.setCursor(0, 40);
        myConfig.get()->version[0] = (myConfig.getMode()->encoder % 2 == 0) ? CONFIG_VERSION[0] : 'R';
        myConfig.getMode()->doFactoryReset = !(myConfig.getMode()->encoder % 2 == 0);
        if (fblink)
          _u8g2.printf("%s", myConfig.getMode()->doFactoryReset ? "No Reset" : "Reset");
        break;
      default:
        break;
      }
    }

    // Program
    if ((
            (!myConfig.get()->awayMode && myConfig.get()->mode == Config::AUTO) || (myConfig.get()->awayMode && myConfig.get()->awayMode == Config::AWAY_MODES::AS_AUTO)) &&
        !myConfig.getMode()->active)
    {
      _u8g2.drawHLine(0, 63 - fontY - 1, 128);
      for (u8 i = 1, h = 0; i < 128; i += 4, h++)
      {
        if (h == getNTPTime()->tm_hour)
        {
          for (u8 im = i + 1, dm = 0; dm < 4; dm++, im += 9)
          {
            if (dm == floor(getNTPTime()->tm_min / 15))
            {
              if (!blink)
              {
                _u8g2.drawFrame(im, 14 + 28, 9, 3 + myConfig.get()->weekProgram->hourQuarterHolds[(h * 4) + dm] * 3);
              }
              else
              {
                _u8g2.drawBox(im, 14 + 28, 9, 3 + myConfig.get()->weekProgram->hourQuarterHolds[(h * 4) + dm] * 3);
              }
              myConfig.get()->hold = myConfig.get()->weekProgram->hourQuarterHolds[(h * 4) + dm];
              im++;
            }
            else
            {
              _u8g2.drawBox(im, 14 + 28, 8, 3 + myConfig.get()->weekProgram->hourQuarterHolds[(h * 4) + dm] * 3);
            }
          }
          i += 35;
        }
        else
        {
          _u8g2.drawBox(i, 14 + 28, 3, 3 + (((myConfig.get()->weekProgram->hourQuarterHolds[(h * 4)] * 3) + (myConfig.get()->weekProgram->hourQuarterHolds[(h * 4) + 1] * 3) + (myConfig.get()->weekProgram->hourQuarterHolds[(h * 4) + 2] * 3) + (myConfig.get()->weekProgram->hourQuarterHolds[(h * 4) + 3] * 3)) / 4));
        }
      }
    }

  } while (_u8g2.nextPage());
}

void Display::bootAPDisplay(String AP)
{
  _u8g2.setFont(fontName);
  _u8g2.clearBuffer();
  _u8g2.firstPage();
  do
  {
    _u8g2.drawUTF8(0, 16, "Wifi Config Mode");
    _u8g2.drawUTF8(0, 26, String(String("AP:") + AP).c_str());
    _u8g2.drawUTF8(0, 63 - fontY, "Use a Wifi device \nto connect and configure!");
  } while (_u8g2.nextPage());
}

void Display::bootConnectedDisplay()
{
  _u8g2.setFont(fontName);
  _u8g2.clearBuffer();
  _u8g2.firstPage();
  do
  {
    _u8g2.drawUTF8(0, 16, String(String("AP:") + WiFi.SSID()).c_str());
    _u8g2.drawUTF8(0, 26, String(String("IP:") + WiFi.localIP().toString()).c_str());
  } while (_u8g2.nextPage());
}

void Display::displayError(String Error)
{
  _u8g2.setPowerSave(0);
  _u8g2.setContrast(myConfig.get()->displayContrast);
  _u8g2.setFont(fontName);
  _u8g2.clearBuffer();
  _u8g2.firstPage();
  do
  {
    _u8g2.drawUTF8(0, 18, Error.c_str());
  } while (_u8g2.nextPage());
}

void Display::clearDisplay()
{
  _u8g2.clear();
};

void Display::displayProgress(u8 perc, String type)
{
  _u8g2.setPowerSave(0);
  _u8g2.setContrast(myConfig.get()->displayContrast);
  _u8g2.setFont(fontName);
  _u8g2.clearBuffer();
  _u8g2.firstPage();
  do
  {
    _u8g2.drawUTF8(0, 12, ("OTA " + type + " Update").c_str());
    _u8g2.setCursor(52, 26);
    _u8g2.printf("%.2d%%", perc);
    _u8g2.setDrawColor(2);
    _u8g2.drawRBox(7, 15, perc + 14, 14, 3);
  } while (_u8g2.nextPage());
}

//u8g2_Bitmap _buffer;
static Print *u8g2_print_for_screenshot;

void u8g2_print_callback(const char *s)
{
  u8g2_print_for_screenshot->print(s);
}

void Display::screeshot(Print &p)
{
  u8g2_print_for_screenshot = &p;
  u8g2_WriteBufferPBM(_u8g2.getU8g2(), u8g2_print_callback);
}

static u8_t u8g2_print_for_screenshot_row, u8g2_print_for_screenshot_bit;

void u8g2_print_bmp_callback(const char *s)
{
  static byte current = 0x00;

  for (u8_t bp = 0; bp < strlen(s); bp++)
  {
    if (u8g2_print_for_screenshot_row == 0 && s[bp] == '1')
    {
      u8g2_print_for_screenshot->print("4"); // Change from P1 to P4 format (Binary)
    }
    else if (u8g2_print_for_screenshot_row == 2 && s[bp] == 0x0A)
    {
      u8g2_print_for_screenshot->print(" "); // Space as separator as BPM P4 Spec
      u8g2_print_for_screenshot_row++;
    }
    else if (u8g2_print_for_screenshot_row < 3)
    {
      u8g2_print_for_screenshot->print(s[bp]);
    }
    else if (s[bp] != 0x0A)
    {
      if (s[bp] == '0')
      {
        current = (current & ~(0x01 << (7 - u8g2_print_for_screenshot_bit))) & 0xFF;
      }
      else
      {
        current = (current | (0x01 << (7 - u8g2_print_for_screenshot_bit))) & 0xFF;
      }
      u8g2_print_for_screenshot_bit++;
      if (u8g2_print_for_screenshot_bit == 8)
      {
        u8g2_print_for_screenshot_bit = 0;
        u8g2_print_for_screenshot->printf("%c", current);
        current = 0x00;
      }
    }
    if (s[bp] == 0x0A)
      u8g2_print_for_screenshot_row++;
  }
}

void Display::screeshotbmp(Print &p)
{
  u8g2_print_for_screenshot = &p;
  u8g2_print_for_screenshot_row = 0;
  u8g2_print_for_screenshot_bit = 0;
  u8g2_WriteBufferPBM(_u8g2.getU8g2(), u8g2_print_bmp_callback);
}