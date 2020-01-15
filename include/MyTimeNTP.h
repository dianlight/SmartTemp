#pragma once
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval


void setupNTP();
void loopNTP();
tm* getNTPTime();
