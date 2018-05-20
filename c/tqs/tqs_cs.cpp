#include <string.h>
#include "stralet.h"
#include "bt/backtest.h"
#include "tqapi_cs.h"

using namespace tquant::api;
using namespace tquant::stralet;

#ifdef _WIN32
#  define _TQS_EXPORT __declspec(dllexport)
#else
#  define _TQS_EXPORT
#endif

extern "C" _TQS_EXPORT
int32_t tqs_sc_trading_day(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->trading_day();
}

struct C_DateTime {
    int date;
    int time;
};

extern "C" _TQS_EXPORT
C_DateTime tqs_sc_cur_time(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    C_DateTime v;
    auto r = ctx->cur_time();
    v.date = r.date;
    v.time = r.time;
    return v;
}

extern "C" _TQS_EXPORT
void tqs_sc_post_event(void* h, const char* evt, void* data)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->post_event(evt, data);
}

extern "C" _TQS_EXPORT
void tqs_sc_set_timer(void* h, Stralet* stralet, int32_t id, int32_t delay, void* data)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->set_timer(stralet, id, delay, data);
}

extern "C" _TQS_EXPORT
void tqs_sc_kill_timer(void * h, Stralet* stralet, int32_t id)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->kill_timer(stralet, id);
}

extern "C" _TQS_EXPORT
DataApi*  tqs_sc_data_api(void*h, const char* source)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->data_api(source);
}

extern "C" _TQS_EXPORT
TradeApi* tqs_sc_trade_api(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->trade_api();
}

extern "C" _TQS_EXPORT
void tqs_sc_log(void* h, int32_t level, const char* str)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    const char* p = str + strlen(str) - 1;
    if (*p != '\n')
        ctx->logger((LogLevel)level) << str << endl;
    else
        ctx->logger((LogLevel)level) << str;
}

extern "C" _TQS_EXPORT
const char* tqs_sc_get_parameter(void* h, const char* name, const char* def_value)
{
    return nullptr;
}

extern "C" _TQS_EXPORT
const char* tqs_sc_mode(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    //FIXME
    return ctx->mode().c_str();
}

extern "C" _TQS_EXPORT
void tqs_sc_register_algo(void* h, AlgoStralet* algo)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->register_algo(algo);
}

extern "C" _TQS_EXPORT
void tqs_sc_unregister_algo(void* h, AlgoStralet* algo)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->unregister_algo(algo);
}

struct DotNetStalet {
    //StraletContext* (*ctx)();
    //void (*on_create)        ();
    void (*on_destroy)       ();
    void (*on_init)          (StraletContext* sc);
    void (*on_fini)          ();
    void (*on_quote)         (const MarketQuote* q);
    void (*on_bar)           (const char* cycle, const Bar* bar);
    void (*on_timer)         (int32_t id, void* data);
    void (*on_event)         (const char* evt, void* data);
    void (*on_order_status)  (const OrderWrap* order);
    void (*on_order_trade)   (const TradeWrap* trade);
    void (*on_account_status)(const AccountInfoWrap* account);
};

class StraletWrap : public Stralet {
    DotNetStalet m_stralet;
public:
    StraletWrap(DotNetStalet& s) : m_stralet(s)
    {}


    ~StraletWrap() {
        m_stralet.on_destroy();
    }

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
        auto v = OrderWrap(*order);
        m_stralet.on_order_status(&v);
    }

    virtual void on_order_trade(shared_ptr<const Trade> trade) override {
        auto v = TradeWrap(*trade);
        m_stralet.on_order_trade(&v);
    }

    virtual void on_account_status(shared_ptr<const AccountInfo> account) override {
        auto v = AccountInfoWrap(*account);
        m_stralet.on_account_status(&v);
    }
};

extern "C" _TQS_EXPORT
void* tqs_stralet_create(DotNetStalet* wrap)
{
    return new StraletWrap(*wrap);
}

extern "C" _TQS_EXPORT
void tqs_stralet_destroy(void* h)
{
    StraletWrap* stralet = reinterpret_cast<StraletWrap*>(h);
    delete stralet;
}

extern "C" _TQS_EXPORT
void tqs_bt_run(const char* cfg, Stralet* (*create_stralet)())
{
    bt_run(cfg, create_stralet);
}
