#include <Arduino.h>
#include "TimeNTPClient.h"
#include "EvoDebug.h"
#include "Config.h"
#include <TZ.h>
#include <sntp.h>                       // sntp_servermode_dhcp()
#include <PolledTimeout.h>
#include <coredecls.h>                  // settimeofday_cb()

// FIXME: Migrate do debug* macro

static timeval tv;
static time_t now;

static esp8266::polledTimeout::periodicMs showTimeNow(60000);
static int time_machine_days = 0; // 0 = now
static bool time_machine_running = false;

void TimeNTPClient::time_is_set_scheduled() {
  // everything is allowed in this function

  if (time_machine_days == 0) {
    time_machine_running = !time_machine_running;
  }

  if (time_machine_running) {
    if (time_machine_days == 0)
      debugD("settimeofday() has been called - possibly from SNTP (starting time machine to show libc's automatic DST handling)");
    now = time(nullptr);
    const tm* tm = localtime(&now);
    debugD("future=%3ddays: DST=%s - ctime=%lld",time_machine_days, (tm->tm_isdst ? "true " : "false"),ctime(&now));

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

TimeNTPClient::TimeNTPClient(){
  // install callback - called when settimeofday is called (by SNTP or us)
  // once enabled (by DHCP), SNTP is updated every hour
  settimeofday_cb(std::bind(&TimeNTPClient::time_is_set_scheduled,this));

  // NTP servers may be overriden by your DHCP server for a more local one
  // (see below)
  configTime(MYTZ, "pool.ntp.org");

}

tm* TimeNTPClient::getNTPTime(){
    if (showTimeNow) {
        gettimeofday(&tv, nullptr);
        now = time(nullptr);
    }
    return localtime(&now);
}

TimeNTPClient timeNTPClient = TimeNTPClient();