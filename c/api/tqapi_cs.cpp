#include <assert.h>
#include <iostream>
#include <unordered_set>

#include "tquant_api.h"
#include "myutils/stringutils.h"
#include "myutils/unicode.h"
#include "tqapi_cs.h"


extern "C" _TQAPI_EXPORT
void tqapi_set_params(const char* key, const char* value)
{
    set_params(key, value);
}

extern "C" _TQAPI_EXPORT
void* dapi_create(const char* addr)
{
    auto api = create_data_api(addr);
    return reinterpret_cast<void*>(api);
}

extern "C" _TQAPI_EXPORT
void dapi_destroy(void* h)
{
    auto api = reinterpret_cast<DataApi*>(h);
    if (api)
        delete api;
}

extern "C" _TQAPI_EXPORT
void* tapi_create(const char* addr)
{
    auto api = create_trade_api(addr);
    return reinterpret_cast<void*>(api);
}

extern "C" _TQAPI_EXPORT
void tapi_destroy(void* h)
{
    auto api = reinterpret_cast<TradeApi*>(h);
    if (api)
        delete api;
}

extern "C" _TQAPI_EXPORT
void destroy_callresult(void* h)
{
    auto cr = reinterpret_cast<CallResultWrap*>(h);
    delete cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* dapi_get_bar(void* h, const char* code, const char* cycle, int trading_day, bool align)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);
    
    CallResultWrap* cr = new CallResultWrap();
    auto r = dapi->bar(code, cycle, trading_day, align);
    if (r.value) {
        cr->bars = r.value;
        cr->value = reinterpret_cast<const void*>(cr->bars->data());
        cr->element_size = sizeof(RawBar);
        cr->element_count = (int32_t)cr->bars->size();
        cr->value_type = VT_BAR_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* dapi_get_daily_bar(void* h, const char* code, const char* price_adj, bool align)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);

    CallResultWrap* cr = new CallResultWrap();
    auto r = dapi->daily_bar(code, price_adj, align);
    if (r.value) {
        cr->daily_bars = r.value;
        cr->value = reinterpret_cast<const void*>(cr->daily_bars->data());
        cr->element_size = sizeof(RawDailyBar);
        cr->element_count = (int32_t)cr->daily_bars->size();
        cr->value_type = VT_DAILYBAR_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* dapi_get_tick(void* h, const char* code, int trading_day)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);

    CallResultWrap* cr = new CallResultWrap();
    auto r = dapi->tick(code, trading_day);
    if (r.value) {
        cr->quotes = r.value;
        cr->value = reinterpret_cast<const void*>(cr->quotes->data());
        cr->element_size = sizeof(RawMarketQuote);
        cr->element_count = (int32_t)cr->quotes->size();
        cr->value_type = VT_QUOTE_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* dapi_get_quote(void* h, const char* code)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);

    CallResultWrap* cr = new CallResultWrap();
    auto r = dapi->quote(code);
    if (r.value) {
        cr->quote = r.value;
        cr->value = reinterpret_cast<const void*>(cr->quote.get());
        cr->element_size = sizeof(RawMarketQuote);
        cr->element_count = 1;
        cr->value_type = VT_QUOTE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
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
        cr->value_type = VT_STRING;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
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
        cr->value_type = VT_STRING;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

typedef void(*T_on_market_quote)(const MarketQuote* quote);
typedef void(*T_on_bar)(const char*, const Bar* bar);

class MyDapiCallback : public DataApi_Callback {
public:
    T_on_market_quote m_on_quote;
    T_on_bar          m_on_bar;

    MyDapiCallback(T_on_market_quote a_on_quote, T_on_bar a_on_bar)
        : m_on_quote(a_on_quote)
        , m_on_bar(a_on_bar)
    {
    }

    virtual void on_market_quote(shared_ptr<const MarketQuote> quote) override 
    {
        if (m_on_quote) m_on_quote(quote.get());
    }

    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override 
    {
        if (m_on_bar) m_on_bar(cycle.c_str(), bar.get());
    }
};

extern "C" _TQAPI_EXPORT
void dapi_set_callback(void*h, T_on_market_quote a_on_quote, T_on_bar a_on_bar)
{
    auto dapi = reinterpret_cast<DataApi*>(h);
    assert(dapi);
    if (a_on_bar || a_on_bar) {
        MyDapiCallback* cb = new MyDapiCallback(a_on_quote, a_on_bar);
        DataApi_Callback* old_cb = dapi->set_callback(cb);
        if (old_cb)
            delete old_cb;
    }
    else {
        DataApi_Callback* old_cb = dapi->set_callback(nullptr);
        if (old_cb)
            delete old_cb;
    }
}

extern "C" _TQAPI_EXPORT
CallResultWrap* tapi_query_account_status(void* h)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_account_status();
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        auto account_infos = make_shared<vector<AccountInfoWrap>>();
        for (auto& tmp : *r.value) {
            account_infos->push_back(AccountInfoWrap(tmp));
        }
        cr->account_infos = account_infos;
        cr->value = reinterpret_cast<const void*>(cr->account_infos->data());
        cr->element_size = sizeof(AccountInfoWrap);
        cr->element_count = (int32_t)cr->account_infos->size();
        cr->value_type = VT_ACCOUNT_INFO_ARRAY;
    }
    else 
    {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* tapi_query_balance(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_balance(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->balance = make_shared<BalanceWrap>(*r.value);
        cr->value = reinterpret_cast<const void*>(cr->balance.get());
        cr->element_size = sizeof(BalanceWrap);
        cr->element_count = 1;
        cr->value_type = VT_BALANCE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* tapi_query_positions(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_positions(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {

        cr->positions = make_shared<vector<PositionWrap>>();
        for (auto& tmp : *r.value)
            cr->positions->push_back(PositionWrap(tmp));

        cr->value = reinterpret_cast<const void*>(cr->positions->data());
        cr->element_size = sizeof(PositionWrap);
        cr->element_count = (int32_t)cr->positions->size();
        cr->value_type = VT_POSITION_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* tapi_query_orders(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_orders(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->orders = make_shared<vector<OrderWrap>>();
        for (auto& tmp : *r.value)
            cr->orders->push_back(OrderWrap(tmp));

        cr->value = reinterpret_cast<const void*>(cr->orders->data());
        cr->element_size = sizeof(OrderWrap);
        cr->element_count = (int32_t)cr->orders->size();
        cr->value_type = VT_ORDER_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* tapi_query_trades(void* h, const char* account_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query_trades(account_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->trades = make_shared<vector<TradeWrap>>();
        for (auto& tmp : *r.value)
            cr->trades->push_back(TradeWrap(tmp));

        cr->value = reinterpret_cast<const void*>(cr->trades->data());
        cr->element_size = sizeof(TradeWrap);
        cr->element_count = (int32_t)cr->trades->size();
        cr->value_type = VT_TRADE_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
CallResultWrap* tapi_place_order(void* h, const char* account_id,
                                 const char* code, 
                                 double price,
                                 int64_t size,
                                 const char* action,
                                 const char* price_type,
                                 int32_t order_id)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->place_order(account_id, code, price, size, action, price_type, order_id);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->order_id = make_shared<OrderIDWrap>(*r.value);
        cr->value = reinterpret_cast<const void*>(cr->order_id.get());
        cr->element_size = sizeof(OrderID);
        cr->element_count = 1;
        cr->value_type = VT_ORDER_ID;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}


extern "C" _TQAPI_EXPORT
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
        cr->value_type = VT_BOOL;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

extern "C" _TQAPI_EXPORT
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
        cr->value_type = VT_BOOL;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}


extern "C" _TQAPI_EXPORT
CallResultWrap* tapi_query(void* h, const char* account_id, const char* command, const char* data)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);

    auto r = tapi->query(account_id, command, data);
    CallResultWrap* cr = new CallResultWrap();
    if (r.value) {
        cr->text = _T(*r.value);
        cr->value = cr->text.c_str();
        cr->value_type = VT_STRING;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

typedef void(*T_on_order_status)  (OrderWrap*       order);
typedef void(*T_on_order_trade)   (TradeWrap*       trade);
typedef void(*T_on_account_status)(AccountInfoWrap* account);

class MyTapiCallback : public TradeApi_Callback {
public:
    T_on_order_status   m_on_order;
    T_on_order_trade    m_on_trade;
    T_on_account_status m_on_account;

    MyTapiCallback(T_on_order_status on_order,
                   T_on_order_trade on_trade,
                   T_on_account_status on_account)
        : m_on_order(on_order)
        , m_on_trade(on_trade)
        , m_on_account(on_account)
    {
    }

    virtual void on_order_status(shared_ptr<Order> order) override
    {
        auto v = OrderWrap(*order);
        if (m_on_order) m_on_order(&v);
    }

    virtual void on_order_trade(shared_ptr<Trade> trade) override
    {
        auto v = TradeWrap(*trade);
        if (m_on_trade) m_on_trade(&v);
    }

    virtual void on_account_status(shared_ptr<AccountInfo> account) override
    {
        auto v = AccountInfoWrap(*account);
        if (m_on_account) m_on_account(&v);
    }
};

extern "C" _TQAPI_EXPORT
void tapi_set_callback(void*h,
    T_on_order_status on_order,
    T_on_order_trade on_trade,
    T_on_account_status on_account)
{
    auto tapi = reinterpret_cast<TradeApi*>(h);
    assert(tapi);
    if (on_order || on_trade || on_account) {
        MyTapiCallback* cb = new MyTapiCallback(on_order, on_trade, on_account);
        auto old_cb = tapi->set_callback(cb);
        if (old_cb)
            delete old_cb;
    }
    else {
        auto old_cb = tapi->set_callback(nullptr);
        if (old_cb)
            delete old_cb;
    }
}
