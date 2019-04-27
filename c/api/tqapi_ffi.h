#ifndef _TQAPI_FFI_H
#define _TQAPI_FFI_H

#include <stdint.h>

#ifdef _WIN32
#  ifdef _TQAPI_DLL
#    define _TQAPI_EXPORT __declspec(dllimport)
#  elif defined(_TQAPI_DLL_IMPL)
#    define _TQAPI_EXPORT __declspec(dllexport)
#  else
#    define _TQAPI_EXPORT
#  endif
#else
#  define _TQAPI_EXPORT
#endif


#pragma pack(1)
    // keep same with tk_schema!
struct MarketQuote {
    const char*     code;
#if defined(WIN32) && !defined(_WIN64)
    int32_t         _padding_1;
#endif
    int32_t         date;
    int32_t         time;
    int64_t         recv_time;
    int32_t         trading_day;
    double          open;
    double          high;
    double          low;
    double          close;
    double          last;
    double          high_limit;
    double          low_limit;
    double          pre_close;
    int64_t         volume;
    double          turnover;
    double          ask1;
    double          ask2;
    double          ask3;
    double          ask4;
    double          ask5;
    double          bid1;
    double          bid2;
    double          bid3;
    double          bid4;
    double          bid5;
    int64_t         ask_vol1;
    int64_t         ask_vol2;
    int64_t         ask_vol3;
    int64_t         ask_vol4;
    int64_t         ask_vol5;
    int64_t         bid_vol1;
    int64_t         bid_vol2;
    int64_t         bid_vol3;
    int64_t         bid_vol4;
    int64_t         bid_vol5;
    double          settle;
    double          pre_settle;
    int64_t         oi;
    int64_t         pre_oi;
};

struct Bar {
    const char*     code;
#if defined(WIN32) && !defined(_WIN64)
    int32_t         _padding_1;
#endif
    int32_t         date;
    int32_t         time;
    int32_t         trading_day;
    double          open;
    double          high;
    double          low;
    double          close;
    int64_t         volume;
    double          turnover;
    int64_t         oi;
};

struct DailyBar {
    const char*     code;
#if defined(WIN32) && !defined(_WIN64)
    int32_t         _padding_1;
#endif
    int32_t         date;
    double          open;
    double          high;
    double          low;
    double          close;
    int64_t         volume;
    double          turnover;
    int64_t         oi;
    double          settle;
    double          pre_close;
    double          pre_settle;
    double          af;
};

struct GetTickResultData;
struct GetTickResult {
    GetTickResultData*   data;
    MarketQuote*         array;
    int32_t              array_length;
    int32_t              element_size;
    const char*          msg;
};

struct SubscribeResultData;
struct SubscribeResult {
    SubscribeResultData* _data;
    const char* codes;
    const char* msg;
};

struct UnSubscribeResultData;
struct UnSubscribeResult {
    UnSubscribeResultData* _data;
    const char* codes;
    const char* msg;
};

#pragma pack()

struct DataApi;

extern "C" {
    DataApi* tqapi_create_data_api(const char* addr);
    void tqapi_free_data_api(DataApi* );

    GetTickResult* tqapi_dapi_get_ticks(DataApi* dapi, const char* code, int trade_date);
    void tqapi_dapi_free_get_ticks_result(DataApi* dapi, GetTickResult* result);

    SubscribeResult*   tqapi_dapi_subscribe(DataApi* dapi, const char*codes);
    UnSubscribeResult* tqapi_dapi_unsubscribe(DataApi* dapi, const char*codes);

    void tqapi_dapi_free_subscribe_result(DataApi* dapi, SubscribeResult* result);
    void tqapi_dapi_free_unsubscribe_result(DataApi* dapi, UnSubscribeResult* result);
}


#endif
