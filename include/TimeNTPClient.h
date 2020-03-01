#pragma once
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval


class TimeNTPClient {
    public:
        TimeNTPClient();
        tm* getNTPTime();

    private:
        void time_is_set_scheduled();
};

extern TimeNTPClient timeNTPClient;
