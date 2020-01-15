#include <Arduino.h>
#include "MyTimeNTP.h"
#include "Config.h"
#include <TZ.h>
#include <sntp.h>                       // sntp_servermode_dhcp()
#include <PolledTimeout.h>
#include <coredecls.h>                  // settimeofday_cb()


static timeval tv;
static time_t now;

static esp8266::polledTimeout::periodicMs showTimeNow(60000);
static int time_machine_days = 0; // 0 = now
static bool time_machine_running = false;

void time_is_set_scheduled() {
  // everything is allowed in this function

  if (time_machine_days == 0) {
    time_machine_running = !time_machine_running;
  }

  // time machine demo
  if (time_machine_running) {
    if (time_machine_days == 0)
      Serial.printf("---- settimeofday() has been called - possibly from SNTP\n"
                    "     (starting time machine demo to show libc's automatic DST handling)\n\n");
    now = time(nullptr);
    const tm* tm = localtime(&now);
    Serial.printf("future=%3ddays: DST=%s - ",
                  time_machine_days,
                  tm->tm_isdst ? "true " : "false");
    Serial.print(ctime(&now));
    gettimeofday(&tv, nullptr);
    constexpr int days = 30;
    time_machine_days += days;
    if (time_machine_days > 360) {
      tv.tv_sec -= (time_machine_days - days) * 60 * 60 * 24;
      time_machine_days = 0;
    } else {
      tv.tv_sec += days * 60 * 60 * 24;
    }
    settimeofday(&tv, nullptr);
  } 
}

void setupNTP(){
  // install callback - called when settimeofday is called (by SNTP or us)
  // once enabled (by DHCP), SNTP is updated every hour
  settimeofday_cb(time_is_set_scheduled);

  // NTP servers may be overriden by your DHCP server for a more local one
  // (see below)
  configTime(MYTZ, "pool.ntp.org");

}

void loopNTP(){

}

tm* getNTPTime(){
    if (showTimeNow) {
        gettimeofday(&tv, nullptr);
        now = time(nullptr);
    }
    return localtime(&now);
}

