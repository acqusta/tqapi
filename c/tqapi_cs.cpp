#include <assert.h>
#include <iostream>
#include <unordered_set>

#include "tquant_api.h"
#include "../../lib/myutils/stringutils.h"

using namespace tquant::api;

struct CallResultWrap {
    const char* msg;
    const void* value;
    int32_t     element_size;
    int32_t     element_count;
    int32_t     value_type;

    CallResultWrap() :
        msg(nullptr), value(nullptr), element_size(0),
        element_count(0), value_type(0)
    {}

    string  _msg;
    string text;
    shared_ptr<const vector<Bar>> bars;
    shared_ptr<const vector<DailyBar>> daily_bars;
    shared_ptr<const vector<MarketQuote>> quotes;
    shared_ptr<const MarketQuote> quote;
    shared_ptr<const vector<Position>> positions;
    shared_ptr<const vector<Order>> orders;
    shared_ptr<const vector<Trade>> trades;
    shared_ptr<const vector<AccountInfo>> account_infos;
    shared_ptr<const OrderID>  order_id;
    shared_ptr<const Balance> balance;
    shared_ptr<bool> bool_value;
};

enum CallResultValueType {
    BAR_ARRAY = 1,
    QUOTE_ARRAY,
    DAILYBAR_ARRAY,
    QUOTE,
    STRING,
    ACCOUNT_INFO_ARRAY,
    POSITION_ARRAY,
    ORDER_ARRAY,
    TRADE_ARRAY,
    ORDER_ID,
    BOOL_VALUE,
    BALANCE_VALUE
};

extern "C" __declspec(dllexport)
void* tqapi_create(const char* addr)
{
    auto api = TQuantApi::create(addr);
    return reinterpret_cast<void*>(api);
}

extern "C" __declspec(dllexport)
void tqapi_destroy(void* h)
{
    auto api = reinterpret_cast<TQuantApi*>(h);
    if (api)
        delete api;
}

extern "C" __declspec(dllexport)
void* tqapi_get_data_api(void* h, const char* source)
{
    auto api = reinterpret_cast<TQuantApi*>(h);
    if (!api) return nullptr;

    auto dapi = api->data_api(source);
    return reinterpret_cast<void*>(dapi);
}

extern "C" __declspec(dllexport)
void* tqapi_get_trade_api(void* h)
{
    auto api = reinterpret_cast<TQuantApi*>(h);
    if (!api) return nullptr;

    auto tapi = api->trade_api();
    return reinterpret_cast<void*>(tapi);
}

extern "C" __declspec(dllexport)
void destroy_callresult(void* h)
{
    auto cr = reinterpret_cast<CallResultValueType*>(h);
    delete cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* dapi_get_bar(void* h, const char* code, const char* cycle, int trading_day, bool align)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);
    
    CallResultWrap* cr = new CallResultWrap();
    auto r = dapi->bar(code, cycle, trading_day, align);
    if (r.value) {
        cr->bars = r.value;
        cr->value = reinterpret_cast<const void*>(cr->bars->data());
        cr->element_size = sizeof(Bar);
        cr->element_count = (int32_t)cr->bars->size();
        cr->value_type = BAR_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* dapi_get_daily_bar(void* h, const char* code, const char* price_adj, bool align)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);

    CallResultWrap* cr = new CallResultWrap();
    auto r = dapi->daily_bar(code, price_adj, align);
    if (r.value) {
        cr->daily_bars = r.value;
        cr->value = reinterpret_cast<const void*>(cr->daily_bars->data());
        cr->element_size = sizeof(DailyBar);
        cr->element_count = (int32_t)cr->daily_bars->size();
        cr->value_type = DAILYBAR_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* dapi_get_tick(void* h, const char* code, int trading_day)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);

    CallResultWrap* cr = new CallResultWrap();
    auto r = dapi->tick(code, trading_day);
    if (r.value) {
        cr->quotes = r.value;
        cr->value = reinterpret_cast<const void*>(cr->quotes->data());
        cr->element_size = sizeof(MarketQuote);
        cr->element_count = (int32_t)cr->quotes->size();
        cr->value_type = QUOTE_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* dapi_subscribe(void* h, const char* codes)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);

    vector<string> ss;
    split(codes, ",", &ss);

    auto r = dapi->subscribe(ss);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        stringstream s;
        for (auto code : *r.value)
            s << code << ",";
        string str = s.str();
        if (str.size() > 0)
            str.resize(str.size() - 1);

        cr->text  = str;
        cr->value = cr->text.c_str();
        cr->value_type = STRING;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* dapi_unsubscribe(void* h, const char* codes)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);

    vector<string> ss;
    split(codes, ",", &ss);

    auto r = dapi->unsubscribe(ss);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        stringstream s;
        for (auto code : *r.value)
            s << code << ",";
        string str = s.str();
        if (str.size() > 0)
            str.resize(str.size() - 1);

        cr->text = str;
        cr->value = cr->text.c_str();
        cr->value_type = STRING;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* tapi_query_account_status(void* h)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_account_status();
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->account_infos = r.value;        
        cr->value = reinterpret_cast<const void*>(cr->account_infos->data());
        cr->element_size = sizeof(AccountInfo);
        cr->element_count = (int32_t)cr->account_infos->size();
        cr->value_type = ACCOUNT_INFO_ARRAY;
    }
    else 
    {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* tapi_query_balance(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_balance(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->balance = r.value;
        cr->value = reinterpret_cast<const void*>(cr->balance.get());
        cr->element_size = sizeof(Balance);
        cr->element_count = 1;
        cr->value_type = BALANCE_VALUE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* tapi_query_positions(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_positions(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->positions = r.value;
        cr->value = reinterpret_cast<const void*>(cr->positions->data());
        cr->element_size = sizeof(Position);
        cr->element_count = (int32_t)cr->positions->size();
        cr->value_type = POSITION_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* tapi_query_orders(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_orders(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->orders = r.value;
        cr->value = reinterpret_cast<const void*>(cr->orders->data());
        cr->element_size = sizeof(Order);
        cr->element_count = (int32_t)cr->orders->size();
        cr->value_type = ORDER_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* tapi_query_trades(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_trades(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->trades = r.value;
        cr->value = reinterpret_cast<const void*>(cr->trades->data());
        cr->element_size = sizeof(Trade);
        cr->element_count = (int32_t)cr->trades->size();
        cr->value_type = TRADE_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* tapi_place_order(void* h, const char* account_id,
                                 const char* code, 
                                 double price,
                                 int64_t size,
                                 const char* action,
                                 int32_t order_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->place_order(account_id, code, price, size, action, order_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->order_id = r.value;
        cr->value = reinterpret_cast<const void*>(cr->orders.get());
        cr->element_size = sizeof(OrderID);
        cr->element_count = 1;
        cr->value_type = ORDER_ID;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}


extern "C" __declspec(dllexport)
CallResultWrap* tapi_cancel_order1(void* h, const char* account_id,
                                   const char* code,
                                   const char* entrust_no)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->cancel_order(account_id, code, entrust_no);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->bool_value = r.value;
        cr->value = reinterpret_cast<const void*>(cr->bool_value.get());
        cr->element_size = sizeof(bool);
        cr->element_count = 1;
        cr->value_type = BOOL_VALUE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}

extern "C" __declspec(dllexport)
CallResultWrap* tapi_cancel_order2(void* h, const char* account_id,
                                   const char* code,
                                   int32_t order_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->cancel_order(account_id, code, order_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->bool_value = r.value;
        cr->value = reinterpret_cast<const void*>(cr->bool_value.get());
        cr->element_size = sizeof(bool);
        cr->element_count = 1;
        cr->value_type = BOOL_VALUE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}


extern "C" __declspec(dllexport)
CallResultWrap* tapi_query(void* h, const char* account_id, const char* command, const char* data)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query(account_id, command, data);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->text = *r.value;
        cr->value = cr->text.c_str();
        cr->value_type = STRING;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = cr->_msg.c_str();
    }
    return cr;
}
