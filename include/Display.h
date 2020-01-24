#pragma once

const static std::string DAYSNAME[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

#define U8_Width 128
#define U8_Height 64

typedef byte u8g2_Bitmap[U8_Height][U8_Width];

void setupDisplay();
void loopDisplay();
void clearDisplay();

void  bootAPDisplay(String AP);
void  bootConnectedDisplay();

void displayError(String Error);  
void displayProgress(u8 perc,String type);

void screeshot(Print &p);

