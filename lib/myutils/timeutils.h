#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H

#include <time.h>
#include <string.h>
#include <chrono>

static inline int human_date(time_t t)
{
    struct tm tm = *localtime(&t);
    //char datetime[20];
    //strftime(datetime, 15, "%Y%m%d", &tm);
    //return atoi(datetime);
    return (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
}

static inline int human_time_seconds(time_t t)
{
    struct tm tm = *localtime(&t);
    return tm.tm_hour * 10000 + tm.tm_min * 100 + tm.tm_sec;
}

static void human_datetime(int* date, int* time)
{
   std:: chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm *t = localtime(&tnow);
    t->tm_hour = 0;
    t->tm_min = 0;
    t->tm_sec = 0;
    auto midnight = std::chrono::system_clock::from_time_t(mktime(t));

    *date = (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
    *time = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - midnight).count();
}

static inline int human_today()
{
    time_t t = time(NULL);
    return human_date(t);
}

static inline int human_nextday(int day)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mday = day % 100;
    tm.tm_mon = (day/100)%100 - 1;
    tm.tm_year = (day/10000) - 1900;
    time_t t = mktime(&tm) + 3600 * 24;
    return human_date(t);
}

static inline int human_preday(int day)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mday = day % 100;
    tm.tm_mon = (day/100)%100 - 1;
    tm.tm_year = (day/10000) - 1900;
    time_t t = mktime(&tm) - 3600 * 24;
    return human_date(t);
}

static inline int human_now_seconds()
{
    time_t t = time(NULL);
    return human_time_seconds(t);
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
