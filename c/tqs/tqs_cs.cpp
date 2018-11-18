#include <string.h>
#include "stralet.h"
#include "bt/backtest.h"
#include "rt/realtime.h"
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
void tqs_sc_set_timer(void* h, int64_t id, int64_t delay, void* data)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->set_timer(id, delay, data);
}

extern "C" _TQS_EXPORT
void tqs_sc_kill_timer(void * h, int64_t id)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    ctx->kill_timer(id);
}

extern "C" _TQS_EXPORT
DataApi*  tqs_sc_data_api(void*h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->data_api();
}

extern "C" _TQS_EXPORT
TradeApi* tqs_sc_trade_api(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->trade_api();
}

extern "C" _TQS_EXPORT
void tqs_sc_log(void* h, int32_t severity, const char* str)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    const char* p = str + strlen(str) - 1;
    if (*p != '\n')
        ctx->logger((LogSeverity)severity) << str << endl;
    else
        ctx->logger((LogSeverity)severity) << str;
}

extern "C" _TQS_EXPORT
const char* tqs_sc_get_properties(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->get_properties().c_str();
}

extern "C" _TQS_EXPORT
const char* tqs_sc_mode(void* h)
{
    StraletContext* ctx = reinterpret_cast<StraletContext*>(h);
    return ctx->mode().c_str();
}

struct DotNetStalet {
    void (*on_init) (StraletContext* ctx);
    void (*on_fini) ();
    void (*on_quote)(const MarketQuote* quote);
    void (*on_bar)  (const char* cycle, const Bar* bar);
    void (*on_order)(const OrderWrap* order);
    void (*on_trade)(const TradeWrap* trade);
    void (*on_timer)(int64_t id, void* data);
    void (*on_event)(const char* name, void* data);
    void (*on_account_status)(const AccountInfoWrap* account);
};

class StraletWrap : public Stralet {
    DotNetStalet m_stralet;
public:
    StraletWrap(DotNetStalet& s) : m_stralet(s)
    {}

    ~StraletWrap() {
        //m_stralet.on_destroy();
    }

    struct TimerWrap {
        int64_t id;
        void*   data;
    };

    struct EventWrap {
        const char* name;
        void*       data;
    };

    struct BarWrap {
        const char*     cycle;
        const RawBar*   bar;
    };

    virtual void on_init() {
        m_stralet.on_init(m_ctx);
    }

    virtual void on_fini() { 
        m_stralet.on_fini();
    }

    virtual void on_quote(shared_ptr<const MarketQuote> quote) {
        m_stralet.on_quote(quote.get());
    }

    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) {
        m_stralet.on_bar(cycle.c_str(), bar.get());
    }

    virtual void on_order(shared_ptr<const Order> order) { 
        OrderWrap wrap(order.get());
        m_stralet.on_order(&wrap);
    }

    virtual void on_trade(shared_ptr<const Trade> trade) { 
        TradeWrap wrap(trade.get());
        m_stralet.on_trade(&wrap);
    }

    virtual void on_timer(int64_t id, void* data) { 
        m_stralet.on_timer(id, data);
    }

    virtual void on_event(const string& name, void* data) { 
        m_stralet.on_event(name.c_str(), data);
    }

    virtual void on_account_status(shared_ptr<const AccountInfo> account) { 
        AccountInfoWrap wrap(*account);
        m_stralet.on_account_status(&wrap);
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
    backtest::run(cfg, create_stralet);
}

extern "C" _TQS_EXPORT
void tqs_rt_run(const char* cfg, Stralet* (*create_stralet)())
{
    realtime::run(cfg, create_stralet);
}
