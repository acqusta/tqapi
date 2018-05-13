#include "stralet.h"
//CallResultWrap* dapi_subscribe(void* h, const char* codes)

using namespace tquant::api;
using namespace tquant::stralet;


extern "C" _TQAPI_EXPORT
int32_t tqs_sc_trading_day(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->trading_day();
}

extern "C" _TQAPI_EXPORT
DateTime tqs_sc_cur_time(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->cur_time();
}

extern "C" _TQAPI_EXPORT
void tqs_sc_post_event(void* h, const char* evt, void* data)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->post_event(evt, data);
}

extern "C" _TQAPI_EXPORT
void tqs_sc_set_timer(void* h, Stralet* stralet, int32_t id, int32_t delay, void* data)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->set_timer(stralet, id, delay, data);
}

extern "C" _TQAPI_EXPORT
void tqs_sc_kill_timer(void * h, Stralet* stralet, int32_t id)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->kill_timer(stralet, id);
}

extern "C" _TQAPI_EXPORT
DataApi*  tqs_sc_data_api(void*h, const char* source)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->data_api(source);
}

extern "C" _TQAPI_EXPORT
TradeApi* tqs_sc_trade_api(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->trade_api();
}

extern "C" _TQAPI_EXPORT
void tqs_sc_log(void* h, int32_t level, const char* str)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    const char* p = str + strlen(str) - 1;
    if (*p != '\n')
        ctx->logger((LogLevel)level) << str << endl;
    else
        ctx->logger((LogLevel)level) << str;
}

extern "C" _TQAPI_EXPORT
const char* tqs_get_parameter(void* h, const char* name, const char* def_value)
{
    return nullptr;
}

extern "C" _TQAPI_EXPORT
const char* tqs_sc_mode(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    //FIXME
    return ctx->mode().c_str();
}

extern "C" _TQAPI_EXPORT
void tqs_sc_register_algo(void* h, AlgoStralet* algo)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->register_algo(algo);
}

extern "C" _TQAPI_EXPORT
void tqs_sc_unregister_algo(void* h, AlgoStralet* algo)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->unregister_algo(algo);
}

struct DotNetStalet {
    //StraletContext* (*ctx)();
    void (*on_init)          (StraletContext* sc);
    void (*on_fini)          ();
    void (*on_quote)         (const MarketQuote* q);
    void (*on_bar)           (const char* cycle, const Bar* bar);
    void (*on_timer)         (int32_t id, void* data);
    void (*on_event)         (const char* evt, void* data);
    void (*on_order_status)  (const Order* order);
    void (*on_order_trade)   (const Trade* trade);
    void (*on_account_status)(const AccountInfo* account);
};

class StraletWrap : public Stralet {
    DotNetStalet m_stralet;
public:
    StraletWrap(DotNetStalet& s) : m_stralet(s)
    {}

    virtual void on_init(StraletContext* sc) {
        Stralet::on_init(sc);
        m_stralet.on_init(sc);
    }

    virtual void on_fini() override {
        m_stralet.on_fini();
    }

    virtual void on_quote(shared_ptr<const MarketQuote> q) override {
        m_stralet.on_quote(q.get());
    }

    virtual void on_bar(const char* cycle, shared_ptr<const Bar> bar) override {
        m_stralet.on_bar(cycle, bar.get());
    }

    virtual void on_timer(int32_t id, void* data) override {
        m_stralet.on_timer(id, data);
    }

    virtual void on_event(const string& evt, void* data) override {
        m_stralet.on_event(evt.c_str(), data);
    }

    virtual void on_order_status(shared_ptr<const Order> order) override {
        m_stralet.on_order_status(order.get());
    }

    virtual void on_order_trade(shared_ptr<const Trade> trade) override {
        m_stralet.on_order_trade(trade.get());
    }

    virtual void on_account_status(shared_ptr<const AccountInfo> account) override {
        m_stralet.on_account_status(account.get());
    }
};

extern "C" _TQAPI_EXPORT
void* tqs_create_stralet(DotNetStalet* wrap)
{
    return new StraletWrap(*wrap);
}

extern "C" _TQAPI_EXPORT
void tqs_destroy_stralet(void* h)
{
    StraletWrap* stralet = reinterpret_cast<StraletWrap*>(h);
    delete stralet;
}