/* -*- C++ -*- */
/**************

ClickEncoderStream.h

Jun. 2016
Modified by Christophe Persoz and Rui Azevedo.
Based on keyStream.h developed by Rui Azevado.
and ClickEncoder library by Peter Dannegger.
https://github.com/christophepersoz/encoder

Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com

quick and dirty keyboard driver
metaprog keyboard driver where N is the number of keys
all keys are expected to be a pin (buttons)
we can have reverse logic (pull-ups) by entering negative pin numbers
ex: -A0 means: pin A0 normally high, low when button pushed (reverse logic)

***/


#ifndef __ClickEncoderI2CStream_h__
  #define __ClickEncoderI2CStream_h__

  #include <Arduino.h>

  #ifndef ARDUINO_SAM_DUE
    // Arduino specific libraries
    // #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega328P__)
      // #include <stdint.h>
      //#include <avr/io.h>
      //#include <avr/interrupt.h>
      //#include <ClickEncoder.h>
    // #endif

    #include <menuDefs.h>
    #include "at8i2cGateway.h"

    namespace Menu {

      //emulate a stream based on clickEncoder movement returning +/- for every 'sensivity' steps
      //buffer not needer because we have an accumulator
    class ClickEncoderI2CStream:public menuIn {
      public:
        AT8I2CGATEWAY &at8i2cgateway;
        int8_t sensivity;
        int8_t oldPos;
        int8_t pos;
        byte btn;

        inline void update() {
            pos = at8i2cgateway.getEncoder();
            btn = at8i2cgateway.getEncoderButton();
        }

        ClickEncoderI2CStream(AT8I2CGATEWAY &at8i2cgateway,int sensivity)
          :at8i2cgateway(at8i2cgateway),
          sensivity(sensivity),
          oldPos(0),
          pos(0),
          btn(0) {
            pos = at8i2cgateway.getEncoder();
          }


        inline void setSensivity(int s) {
            sensivity = s;
        }

        int available(void) {
            return peek() != -1;
        }

        int peek(void) {
          update();
          if (btn == 5)
            return options->navCodes[enterCmd].ch;//menu::enterCode;

          if (btn == 6)
            return options->navCodes[escCmd].ch;//menu::escCode;

          int d = pos - oldPos;
          if (d <= -sensivity)
              return options->navCodes[downCmd].ch;//menu::downCode;
          if (d >= sensivity)
              return options->navCodes[upCmd].ch;//menu::upCode;
          return -1;
        }

        int read()
        {
            int ch = peek();
         //   if (ch == options->navCodes[upCmd].ch)//menu::upCode)
         //       oldPos += sensivity;
         //   else if (ch == options->navCodes[downCmd].ch)//menu::downCode)
         //       oldPos -= sensivity;
            oldPos=pos;
            return ch;
        }

        void flush() {
            btn=0;//2017 clear current key
            update();
            oldPos = pos;
        }

        size_t write(uint8_t v) {
            oldPos = v;
            return 1;
        }
      };
    }//namespace Menu

  #endif

#endif /* ClickEncoderI2CStream_h */
