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

struct GetDailyBarResultData;
struct GetDailyBarResult {
    GetDailyBarResultData*  data;
    DailyBar*               array;
    int32_t                 array_length;
    int32_t                 element_size;
    const char*             msg;
};

struct GetBarResultData;
struct GetBarResult {
    GetBarResultData*    data;
    Bar*                 array;
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

struct GetQuoteResultData;
struct GetQuoteResult {
    GetQuoteResultData* _data;
    const MarketQuote* quote;
    const char* msg;
};

struct DataApiCallback {
    void* obj;
    void  (*on_quote ) (void* obj, const MarketQuote* quote);
    void  (*on_bar   ) (void* obj, const char* cycle, const Bar* bar);
};


struct AccountInfo {
    const char* account_id;
    const char* broker;
    const char* account;
    const char* status;
    const char* msg;
    const char* account_type;
};

struct Balance {
    const char* account_id;
    const char* fund_account;
    double      init_balance;
    double      enable_balance;
    double      margin;
    double      float_pnl;
    double      close_pnl;
};

struct Order {
    const char*  account_id;
    const char*  code;
    const char*  name;
    const char*  entrust_no;
    const char*  entrust_action;
    double       entrust_price;
    int64_t      entrust_size;
    int32_t      entrust_date;
    int32_t      entrust_time;
    double       fill_price;
    int64_t      fill_size;
    const char*  status;
    const char*  status_msg;
    int32_t      order_id;
};

struct Trade {
    const char*  account_id;
    const char*  code;
    const char*  name;
    const char*  entrust_no;
    const char*  entrust_action;
    const char*  fill_no;
    int64_t      fill_size;
    double       fill_price;
    int32_t      fill_date;
    int32_t      fill_time;
    int32_t      order_id;
};

struct Position {
    const char*    account_id;
    const char*    code;
    const char*    name;
    int64_t        current_size;
    int64_t        enable_size;
    int64_t        init_size;
    int64_t        today_size;
    int64_t        frozen_size;
    const char*    side;
    double         cost;
    double         cost_price;
    double         last_price;
    double         float_pnl;
    double         close_pnl;
    double         margin;
    double         commission;
};

struct OrderID {
    const char*  entrust_no;
    int32_t      order_id;
};

struct NewOrder {
    const char* action     ;
    const char* code       ;
    int64_t     size       ;
    double      price      ;
    const char* price_type ;
    int32_t     order_id   ;
};

struct PlaceOrderResultData;
struct PlaceOrderResult {
    PlaceOrderResultData* _data;
    OrderID*    result;
    const char* msg;
};

// struct QueryAccountsResultData;
// struct QueryAccountsResult {
//     QueryAccountsResultData* _data;
//     AccountInfo*         array;
//     int32_t              array_length;
//     int32_t              element_size;
//     const char*          msg;
// };

struct QueryPositionsResultData;
struct QueryPositionsResult {
    QueryPositionsResultData* _data;
    Position*            array;
    int32_t              array_length;
    int32_t              element_size;
    const char*          msg;
};

struct QueryOrdersResultData;
struct QueryOrdersResult {
    QueryOrdersResultData* _data;
    Order*               array;
    int32_t              array_length;
    int32_t              element_size;
    const char*          msg;
};

struct QueryTradesResultData;
struct QueryTradesResult {
    QueryTradesResultData* _data;
    Trade*               array;
    int32_t              array_length;
    int32_t              element_size;
    const char*          msg;
};

struct QueryAccountsResultData;
struct QueryAccountsResult {
    QueryAccountsResultData* _data;
    AccountInfo*         array;
    int32_t              array_length;
    int32_t              element_size;
    const char*          msg;
};

struct QueryBalanceResultData;
struct QueryBalanceResult {
    QueryBalanceResultData* _data;
    Balance*        result;
    const char*     msg;
};

struct QueryResultData;
struct QueryResult {
    QueryResultData* _data;
    const char*     result;
    const char*     msg;
};

struct CancelOrderResultData;
struct CancelOrderResult {
    CancelOrderResultData* _data;
    int                  success;
    const char*          msg;
};

struct TradeApiCallback {
    void* user_data;
    void  (*on_order)(const Order* order, void* user_data);
    void  (*on_trade)(const Trade* trade, void* user_data);
    void  (*on_account_status)(const AccountInfo* account, void* user_data);
};

struct StraletContext;
struct Stralet {
    void* obj;
    void (*on_init)           (void* obj, StraletContext* ctx);
    void (*on_fini)           (void* obj, StraletContext* ctx);
    void (*on_quote)          (void* obj, StraletContext* ctx, const MarketQuote* quote);
    void (*on_bar)            (void* obj, StraletContext* ctx, const char* cycle, const Bar* bar);
    void (*on_order)          (void* obj, StraletContext* ctx, const Order* order);
    void (*on_trade)          (void* obj, StraletContext* ctx, const Trade* trade);
    void (*on_timer)          (void* obj, StraletContext* ctx, int64_t id, void* data);
    void (*on_event)          (void* obj, StraletContext* ctx, const char* name, void* data);
    void (*on_account_status) (void* obj, StraletContext* ctx, const AccountInfo* account);
};

struct DateTime {
    int date;
    int time;
};


// struct CStraletContext {
//     tquant::stralet::StraletContext* cpp_ctx;
//     TradeApi* tapi;
//     DataApi*  dapi;
// };

struct StraletFactory {
    void*    obj;
    Stralet* (*create) (void* obj);
    void     (*destroy)(void* obj, Stralet* stralet);
};

#pragma pack()

struct DataApi;
struct TradeApi;

extern "C" {
    _TQAPI_EXPORT DataApi* tqapi_create_data_api(const char* addr);
    _TQAPI_EXPORT void     tqapi_free_data_api(DataApi* );

    _TQAPI_EXPORT DataApiCallback*   tqapi_dapi_set_callback    (DataApi* dapi, DataApiCallback* callback);

    _TQAPI_EXPORT GetTickResult*     tqapi_dapi_get_ticks       (DataApi* dapi, const char* code, int trade_date, int number);
    _TQAPI_EXPORT GetBarResult*      tqapi_dapi_get_bars        (DataApi* dapi, const char* code, const char* cycle, int trade_date, int align, int number);
    _TQAPI_EXPORT GetDailyBarResult* tqapi_dapi_get_dailybars   (DataApi* dapi, const char* code, const char* price_type, int align, int number);
    _TQAPI_EXPORT GetQuoteResult*    tqapi_dapi_get_quote       (DataApi* dapi, const char* code);
    _TQAPI_EXPORT SubscribeResult*   tqapi_dapi_subscribe       (DataApi* dapi, const char*codes);
    _TQAPI_EXPORT UnSubscribeResult* tqapi_dapi_unsubscribe     (DataApi* dapi, const char*codes);

    _TQAPI_EXPORT void tqapi_dapi_free_subscribe_result         (DataApi* dapi, SubscribeResult* result);
    _TQAPI_EXPORT void tqapi_dapi_free_unsubscribe_result       (DataApi* dapi, UnSubscribeResult* result);
    _TQAPI_EXPORT void tqapi_dapi_free_get_ticks_result         (DataApi* dapi, GetTickResult* result);
    _TQAPI_EXPORT void tqapi_dapi_free_get_bars_result          (DataApi* dapi, GetBarResult* result);
    _TQAPI_EXPORT void tqapi_dapi_free_get_dailybars_result     (DataApi* dapi, GetDailyBarResult* result);
    _TQAPI_EXPORT void tqapi_dapi_free_get_quote_result         (DataApi* dapi, GetQuoteResult* result);

    _TQAPI_EXPORT TradeApi*             tqapi_create_trade_api     (const char* addr);
    _TQAPI_EXPORT void                  tqapi_free_trade_api       (TradeApi* tapi);
    _TQAPI_EXPORT TradeApiCallback*     tqapi_tapi_set_callback    (TradeApi* tapi, TradeApiCallback* callback);

    _TQAPI_EXPORT PlaceOrderResult*     tqapi_tapi_place_order     (TradeApi* tapi, const char* account_id, NewOrder* order);
    _TQAPI_EXPORT CancelOrderResult*    tqapi_tapi_cancel_order    (TradeApi* tapi, const char* account_id, const char* code, OrderID* oid);
    _TQAPI_EXPORT QueryBalanceResult*   tqapi_tapi_query_balance   (TradeApi* tapi, const char* account_id);
    _TQAPI_EXPORT QueryPositionsResult* tqapi_tapi_query_positions (TradeApi* tapi, const char* account_id,  const char* codes);
    _TQAPI_EXPORT QueryOrdersResult*    tqapi_tapi_query_orders    (TradeApi* tapi, const char* account_id,  const char* codes);
    _TQAPI_EXPORT QueryTradesResult*    tqapi_tapi_query_trades    (TradeApi* tapi, const char* account_id,  const char* codes);
    _TQAPI_EXPORT QueryResult*          tqapi_tapi_query           (TradeApi* tapi, const char* account_id,  const char* command, const char* params);
    _TQAPI_EXPORT QueryAccountsResult*  tqapi_tapi_query_accounts  (TradeApi* tapi);

    _TQAPI_EXPORT void tqapi_tapi_free_place_order_result      (TradeApi* tapi, PlaceOrderResult    * result);
    _TQAPI_EXPORT void tqapi_tapi_free_cancel_order_result     (TradeApi* tapi, CancelOrderResult   * result);
    _TQAPI_EXPORT void tqapi_tapi_free_query_accounts_result   (TradeApi* tapi, QueryAccountsResult * result);
    _TQAPI_EXPORT void tqapi_tapi_free_query_balance_result    (TradeApi* tapi, QueryBalanceResult  * result);
    _TQAPI_EXPORT void tqapi_tapi_free_query_positions_result  (TradeApi* tapi, QueryPositionsResult* result);
    _TQAPI_EXPORT void tqapi_tapi_free_query_orders_result     (TradeApi* tapi, QueryOrdersResult   * result);
    _TQAPI_EXPORT void tqapi_tapi_free_query_trades_result     (TradeApi* tapi, QueryTradesResult   * result);
    _TQAPI_EXPORT void tqapi_tapi_free_query_result            (TradeApi* tapi, QueryResult         * result);

    _TQAPI_EXPORT int32_t     tqapi_sc_trading_day     (StraletContext* ctx);
    _TQAPI_EXPORT DateTime    tqapi_sc_cur_time        (StraletContext* ctx);
    _TQAPI_EXPORT void        tqapi_sc_post_event      (StraletContext* ctx, const char* evt, void* data);
    _TQAPI_EXPORT void        tqapi_sc_set_timer       (StraletContext* ctx, int64_t id, int64_t delay, void* data);
    _TQAPI_EXPORT void        tqapi_sc_kill_timer      (StraletContext* ctx, int64_t id);
    _TQAPI_EXPORT DataApi*    tqapi_sc_data_api        (StraletContext* ctx);
    _TQAPI_EXPORT TradeApi*   tqapi_sc_trade_api       (StraletContext* ctx);
    _TQAPI_EXPORT void        tqapi_sc_log             (StraletContext* ctx, int32_t severity, const char* str);
    _TQAPI_EXPORT const char* tqapi_sc_get_properties  (StraletContext* ctx);
    _TQAPI_EXPORT const char* tqapi_sc_get_mode        (StraletContext* ctx);
    _TQAPI_EXPORT void        tqapi_bt_run             (const char* cfg, StraletFactory* factory);
    _TQAPI_EXPORT void        tqapi_rt_run             (const char* cfg, StraletFactory* factory);


}


#endif
