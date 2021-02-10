#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H

#include <time.h>
#include <string.h>
#include <chrono>
#include <string>

#if defined(_WIN32)
static inline struct tm*  _localtime_r(const time_t *_Time, struct tm *_Tm) {
  return localtime_s(_Tm, _Time) ? NULL : _Tm;
}
#else
#define _localtime_r localtime_r
#endif

static inline int fin_hms(int h, int m = 0, int s = 0) 
{
    return h * 10000000 + m * 100000 + s * 1000;
}

static inline int fin_date(time_t t)
{
    struct tm tm;
    _localtime_r(&t, &tm);

    return (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
}

static inline int fin_time_seconds(time_t t)
{
    struct tm tm = *localtime(&t);
    return tm.tm_hour * 10000 + tm.tm_min * 100 + tm.tm_sec;
}

static void fin_datetime(int* date, int* time_ms)
{
    time_t t; 
    time(&t);

    struct tm tm;
    _localtime_r(&t, &tm);

    *date = (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
    *time_ms = (tm.tm_hour * 10000 + tm.tm_min * 100 + tm.tm_sec)*1000;

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    *time_ms += millis % 1000;
}

static inline int fin_today()
{
    time_t t = time(NULL);
    return fin_date(t);
}

static inline int fin_nextday(int day, int offset = 1)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mday = day % 100;
    tm.tm_mon = (day/100)%100 - 1;
    tm.tm_year = (day/10000) - 1900;
    tm.tm_isdst = 0;
    time_t t = mktime(&tm) + 3600 * 24 * offset;
    return fin_date(t);
}

static inline int fin_preday(int day, int offset = 1)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mday = day % 100;
    tm.tm_mon = (day/100)%100 - 1;
    tm.tm_year = (day/10000) - 1900;
    tm.tm_isdst = 0;
    time_t t = mktime(&tm) - 3600 * 24 * offset;
    return fin_date(t);
}

static inline int fin_now_seconds()
{
    time_t t = time(NULL);
    return fin_time_seconds(t);
}

static inline std::string today_str()
{
    char buf[64];
    time_t t = time(NULL);
    struct tm tm; 
    _localtime_r(&t, &tm);

    strftime(buf, 64, "%Y-%m-%d", &tm);

    return buf;
}

static inline std::string now_str()
{
    char buf[64];
    time_t t = time(NULL);
    struct tm tm;
    _localtime_r(&t, &tm);

    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &tm);

    return buf;
}

static inline uint64_t now_micro()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

static inline uint64_t now_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

#endif
