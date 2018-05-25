#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H

#include <time.h>
#include <string.h>
#include <chrono>

static inline int fin_date(time_t t)
{
    struct tm tm = *localtime(&t);
    //char datetime[20];
    //strftime(datetime, 15, "%Y%m%d", &tm);
    //return atoi(datetime);
    return (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
}

static inline int fin_time_seconds(time_t t)
{
    struct tm tm = *localtime(&t);
    return tm.tm_hour * 10000 + tm.tm_min * 100 + tm.tm_sec;
}

static void fin_datetime(int* date, int* time)
{
   std:: chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm *t = localtime(&tnow);
    t->tm_hour = 0;
    t->tm_min = 0;
    t->tm_sec = 0;
    auto midnight = std::chrono::system_clock::from_time_t(mktime(t));

    *date = (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
    int tmp = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - midnight).count();
    int ms = tmp % 1000;  tmp /= 1000;
    int s = tmp % 60; tmp /= 60;
    int m = tmp % 60; tmp /= 60;
    int h = tmp % 60;
    *time = (h * 10000 + m * 100 + s) * 1000 + ms;
}

static inline int fin_today()
{
    time_t t = time(NULL);
    return fin_date(t);
}

static inline int fin_nextday(int day)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mday = day % 100;
    tm.tm_mon = (day/100)%100 - 1;
    tm.tm_year = (day/10000) - 1900;
    time_t t = mktime(&tm) + 3600 * 24;
    return fin_date(t);
}

static inline int fin_preday(int day)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mday = day % 100;
    tm.tm_mon = (day/100)%100 - 1;
    tm.tm_year = (day/10000) - 1900;
    time_t t = mktime(&tm) - 3600 * 24;
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
    struct tm tm = *localtime(&t);
    strftime(buf, 64, "%Y-%m-%d", &tm);

    return buf;
}

static inline std::string now_str()
{
    char buf[64];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
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
