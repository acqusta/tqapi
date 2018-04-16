#include <assert.h>
#include <iostream>
#include <unordered_set>

#include "tquant_api.h"
#include "myutils/stringutils.h"
#include "myutils/unicode.h"

using namespace tquant::api;

static inline const char* _T(string& str)
{
    str = utf8_to_gbk(str);
    return str.c_str();
}

#pragma pack(1)
struct AccountInfoWrap {
    const char* account_id;       // 帐号编号
    const char* broker;           // 交易商名称，如招商证券
    const char* account;          // 交易帐号
    const char* status;           // 连接状态，取值 Disconnected, Connected, Connecting
    const char* msg;              // 状态信息，如登录失败原因
    const char* account_type;     // 帐号类型，如 stock, ctp

    AccountInfoWrap(const AccountInfo& orig)
    {
        m_orig = make_shared<AccountInfo>(orig);
        account_id   = _T( m_orig->account_id   );
        broker       = _T( m_orig->broker       );
        account      = _T( m_orig->account      );
        status       = _T( m_orig->status       );
        msg          = _T( m_orig->msg          );
        account_type = _T( m_orig->account_type );
    }

    shared_ptr<AccountInfo> m_orig;
};

struct BalanceWrap {
    const char* account_id;       // 帐号编号
    const char* fund_account;     // 资金帐号
    double      init_balance;     // 初始化资金
    double      enable_balance;   // 可用资金
    double      margin;           // 保证金
    double      float_pnl;        // 浮动盈亏
    double      close_pnl;        // 实现盈亏

    shared_ptr<Balance> m_orig;

    BalanceWrap(const Balance& orig)
    {
        m_orig = make_shared<Balance>(orig);
        account_id      = _T( m_orig->account_id   );
        fund_account    = _T( m_orig->fund_account );
        init_balance    = m_orig->init_balance         ;
        enable_balance  = m_orig->enable_balance       ;
        margin          = m_orig->margin               ;
        float_pnl       = m_orig->float_pnl            ;
        close_pnl       = m_orig->close_pnl            ;
    }
};

struct OrderWrap {
    const char* account_id;       // 帐号编号
    const char* code;             // 证券代码
    const char* name;             // 证券名称
    const char* entrust_no;       // 委托编号
    const char* entrust_action;   // 委托动作
    double      entrust_price;    // 委托价格
    int64_t     entrust_size;     // 委托数量，单位：股
    int32_t     entrust_date;     // 委托日期
    int32_t     entrust_time;     // 委托时间
    double      fill_price;       // 成交价格
    int64_t     fill_size;        // 成交数量
    const char* status;           // 订单状态：取值: OrderStatus
    const char* status_msg;       // 状态消息
    int32_t     order_id;         // 自定义订单编号

    shared_ptr<Order> m_orig;

    OrderWrap(const Order& orig)
    {
        m_orig = make_shared<Order>(orig);
        account_id     = _T( m_orig->account_id     );
        code           = _T( m_orig->code           );
        name           = _T( m_orig->name           );
        entrust_no     = _T( m_orig->entrust_no     );
        entrust_action = _T( m_orig->entrust_action );
        entrust_price  = m_orig->entrust_price         ;
        entrust_size   = m_orig->entrust_size          ;
        entrust_date   = m_orig->entrust_date          ;
        entrust_time   = m_orig->entrust_time          ;
        fill_price     = m_orig->fill_price            ;
        fill_size      = m_orig->fill_size             ;
        status         = _T( m_orig->status      );
        status_msg     = _T( m_orig->status_msg  );
        order_id       = m_orig->order_id              ;
    }
};

struct TradeWrap {
    const char* account_id;       // 帐号编号
    const char* code;             // 证券代码
    const char* name;             // 证券名称
    const char* entrust_no;       // 委托编号
    const char* entrust_action;   // 委托动作
    const char* fill_no;          // 成交编号
    int64_t     fill_size;        // 成交数量
    double      fill_price;       // 成交价格
    int32_t     fill_date;        // 成交日期
    int32_t     fill_time;        // 成交时间

    shared_ptr<Trade> m_orig;

    TradeWrap(const Trade& orig)
    {
        m_orig = make_shared<Trade>(orig);
        account_id     = _T( m_orig->account_id     );
        code           = _T( m_orig->code           );
        name           = _T( m_orig->name           );
        entrust_no     = _T( m_orig->entrust_no     );
        entrust_action = _T( m_orig->entrust_action );
        fill_no        = _T( m_orig->fill_no        );
        fill_size      = m_orig->fill_size              ;
        fill_price     = m_orig->fill_price             ;
        fill_date      = m_orig->fill_date              ;
        fill_time      = m_orig->fill_time              ;
    }
};

struct PositionWrap {
    const char*   account_id;       // 帐号编号
    const char*   code;             // 证券代码
    const char*   name;             // 证券名称
    int64_t       current_size;     // 当前持仓
    int64_t       enable_size;      // 可用（可交易）持仓
    int64_t       init_size;        // 初始持仓
    int64_t       today_size;       // 今日持仓
    int64_t       frozen_size;      // 冻结持仓
    const char*   side;             // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
    double        cost;             // 成本
    double        cost_price;       // 成本价格
    double        last_price;       // 最新价格
    double        float_pnl;        // 持仓盈亏
    double        close_pnl;        // 平仓盈亏
    double        margin;           // 保证金
    double        commission;       // 手续费

    shared_ptr<Position> m_orig;

    PositionWrap(const Position& orig)
    {
        m_orig = make_shared<Position>(orig);
        account_id    = _T( m_orig->account_id );
        code          = _T( m_orig->code       );
        name          = _T( m_orig->name       );
        current_size  = m_orig->current_size         ;
        enable_size   = m_orig->enable_size          ;
        init_size     = m_orig->init_size            ;
        today_size    = m_orig->today_size           ;
        frozen_size   = m_orig->frozen_size          ;
        side          = _T( m_orig->side       );
        cost          = m_orig->cost                 ;
        cost_price    = m_orig->cost_price           ;
        last_price    = m_orig->last_price           ;
        float_pnl     = m_orig->float_pnl            ;
        close_pnl     = m_orig->close_pnl            ;
        margin        = m_orig->margin               ;
        commission    = m_orig->commission           ;
    }
};

struct OrderIDWrap {
    const char*  entrust_no;       // 订单委托号
    int32_t      order_id;         // 自定义编号
    
    shared_ptr<OrderID> m_orig;

    OrderIDWrap(const OrderID& orig)
    {
        m_orig = make_shared<OrderID>(orig);
        entrust_no = _T(m_orig->entrust_no);
        order_id   = m_orig->order_id;
    }
};

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
    shared_ptr<vector<PositionWrap>> positions;
    shared_ptr<vector<OrderWrap>> orders;
    shared_ptr<vector<TradeWrap>> trades;
    shared_ptr<vector<AccountInfoWrap>> account_infos;
    shared_ptr<OrderIDWrap>  order_id;
    shared_ptr<BalanceWrap> balance;
    shared_ptr<bool> bool_value;
};

enum CallResultValueType {
    BAR_ARRAY = 1,
    QUOTE_ARRAY,
    DAILYBAR_ARRAY,
    QUOTE_VALUE,
    STRING_VALUE,
    ACCOUNT_INFO_ARRAY,
    POSITION_ARRAY,
    ORDER_ARRAY,
    TRADE_ARRAY,
    ORDER_ID_VALUE,
    BOOL_VALUE,
    BALANCE_VALUE
};

#pragma pack()

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
        cr->msg = _T(cr->_msg);
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
        cr->msg = _T(cr->_msg);
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
        cr->msg = _T(cr->_msg);
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
        cr->value_type = STRING_VALUE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
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
        cr->value_type = STRING_VALUE;
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

extern "C" __declspec(dllexport)
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

extern "C" __declspec(dllexport)
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
        cr->value_type = ACCOUNT_INFO_ARRAY;
    }
    else 
    {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
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
        cr->balance = make_shared<BalanceWrap>(*r.value);
        cr->value = reinterpret_cast<const void*>(cr->balance.get());
        cr->element_size = sizeof(BalanceWrap);
        cr->element_count = 1;
        cr->value_type = BALANCE_VALUE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
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

        cr->positions = make_shared<vector<PositionWrap>>();
        for (auto& tmp : *r.value)
            cr->positions->push_back(PositionWrap(tmp));

        cr->value = reinterpret_cast<const void*>(cr->positions->data());
        cr->element_size = sizeof(PositionWrap);
        cr->element_count = (int32_t)cr->positions->size();
        cr->value_type = POSITION_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
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
        cr->orders = make_shared<vector<OrderWrap>>();
        for (auto& tmp : *r.value)
            cr->orders->push_back(OrderWrap(tmp));

        cr->value = reinterpret_cast<const void*>(cr->orders->data());
        cr->element_size = sizeof(OrderWrap);
        cr->element_count = (int32_t)cr->orders->size();
        cr->value_type = ORDER_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
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
        cr->trades = make_shared<vector<TradeWrap>>();
        for (auto& tmp : *r.value)
            cr->trades->push_back(TradeWrap(tmp));

        cr->value = reinterpret_cast<const void*>(cr->trades->data());
        cr->element_size = sizeof(TradeWrap);
        cr->element_count = (int32_t)cr->trades->size();
        cr->value_type = TRADE_ARRAY;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
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
        cr->order_id = make_shared<OrderIDWrap>(*r.value);
        cr->value = reinterpret_cast<const void*>(cr->orders.get());
        cr->element_size = sizeof(OrderID);
        cr->element_count = 1;
        cr->value_type = ORDER_ID_VALUE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
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
        cr->msg = _T(cr->_msg);
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
        cr->msg = _T(cr->_msg);
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
        cr->text = _T(*r.value);
        cr->value = cr->text.c_str();
        cr->value_type = STRING_VALUE;
    }
    else {
        cr->_msg = r.msg;
        cr->msg = _T(cr->_msg);
    }
    return cr;
}

typedef void(*T_on_order_status)  (Order*       order);
typedef void(*T_on_order_trade)   (Trade*       trade);
typedef void(*T_on_account_status)(AccountInfo* account);

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
        if (m_on_order) m_on_order(order.get());
    }
    virtual void on_order_trade(shared_ptr<Trade> trade) override
    {
        if (m_on_trade) m_on_trade(trade.get());
    }
    virtual void on_account_status(shared_ptr<AccountInfo> account) override
    {
        if (m_on_account) m_on_account(account.get());
    }
};

extern "C" __declspec(dllexport)
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

