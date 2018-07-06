#include "stralet.h"

namespace tquant {
    namespace stralet {

        milliseconds DateTime::sub(const DateTime& dt) const
        {
            return duration_cast<milliseconds>(this->to_timepoint() - dt.to_timepoint());
        }

        system_clock::time_point DateTime::to_timepoint() const
        {
            int y = date / 10000;
            int m = (date / 100) % 100;
            int d = date % 100;
            int H = time / 10000000;
            int M = (time / 100000) % 100;
            int S = (time / 1000) % 100;
            int MS = time % 1000;

            struct tm t;
            t.tm_year = y - 1900;
            t.tm_mon = m - 1;
            t.tm_mday = d;
            t.tm_hour = H;
            t.tm_min = M;
            t.tm_sec = S;

            return system_clock::from_time_t(mktime(&t)) + milliseconds(MS);
        }

        DateTime DateTime::from_timepoint(system_clock::time_point tp)
        {
            // localtime or gmtime?
            time_t tmp = system_clock::to_time_t(tp);
            tm t = *localtime(&tmp);

            int64_t ms = duration_cast<milliseconds>(tp.time_since_epoch()).count();

            DateTime dt;
            dt.date = (t.tm_year + 1900) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
            dt.time = t.tm_hour * 10000 + t.tm_min * 100 + t.tm_sec;
            dt.time *= 1000;
            dt.time += ms % 1000;
            return dt;
        }
    }
}
