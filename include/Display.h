#pragma once

void setupDisplay();
void loopDisplay();
void clearDisplay();

void  bootAPDisplay(String AP);
void  bootConnectedDisplay();

void displayError(String Error);  
void displayProgress(u8 perc);

