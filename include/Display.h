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

void setupDisplay();
void loopDisplay();
void clearDisplay();

void  bootAPDisplay(String AP);
void  bootConnectedDisplay();

void displayError(String Error);  
void displayProgress(u8 perc,String type);

