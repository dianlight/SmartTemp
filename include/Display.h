#pragma once
#include <Stream.h>
#include <U8g2lib.h>
#include <Ticker.h>
#include <MQTTforHA.h>


#define U8_Width 128
#define U8_Height 64

const static std::string DAYSNAME[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

class Display {
    public:

        Display();
        void clearDisplay();
        void bootAPDisplay(String AP);
        void bootConnectedDisplay();
        void displayError(String Error);  
        void displayProgress(u8 perc,String type);

//        void screeshot(Print &p);
        void screeshotbmp(Print &p);
        void loopDisplay();

    private:
        typedef byte u8g2_Bitmap[U8_Height][U8_Width];

        U8G2 _u8g2;
        
        Ticker sleepModeTicker;

        int8_t lastEncPosition =0;

        void sleepModeDisplay();

};

extern Display display;

