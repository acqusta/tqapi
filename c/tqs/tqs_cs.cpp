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
    void (*on_destroy)       ();
    void (*set_context)      (StraletContext* sc);
    void (*on_event)         (int evt_id, void* data);
};

class StraletWrap : public Stralet {
    DotNetStalet m_stralet;
public:
    StraletWrap(DotNetStalet& s) : m_stralet(s)
    {}


    ~StraletWrap() {
        m_stralet.on_destroy();
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

    virtual void on_event(shared_ptr<StraletEvent> evt)
    {
        switch (evt->evt_id) {
        case STRALET_EVENT_ID::ON_INIT:
            m_stralet.set_context(m_ctx);
            m_stralet.on_event(evt->evt_id, nullptr);
            break;
        case STRALET_EVENT_ID::ON_FINI:
            m_stralet.on_event(evt->evt_id, nullptr);
            break;
        case STRALET_EVENT_ID::ON_QUOTE: {
            auto on_quote = reinterpret_cast<OnQuote*>(evt.get());
            m_stralet.on_event(evt->evt_id, (void*)on_quote->quote.get());
            break;
        }
        case STRALET_EVENT_ID::ON_BAR: {
            auto on_bar = reinterpret_cast<OnBar*>(evt.get());
            BarWrap bar;
            bar.cycle = on_bar->cycle.c_str();
            bar.bar   = on_bar->bar.get();
            m_stralet.on_event(evt->evt_id, (void*)&bar);
            break;
        }
        case STRALET_EVENT_ID::ON_TIMER: {
            auto on_timer = reinterpret_cast<OnTimer*>(evt.get());
            TimerWrap timer;
            timer.id = on_timer->id;
            timer.data = on_timer->data;

            m_stralet.on_event(evt->evt_id, &timer);
            break;
        }
        case STRALET_EVENT_ID::ON_EVENT: {
            auto on_event = reinterpret_cast<OnEvent*>(evt.get());
            EventWrap event;
            event.name = on_event->name.c_str();
            event.data = on_event->data;

            m_stralet.on_event(evt->evt_id, &event);
            break;
        }
        case STRALET_EVENT_ID::ON_ORDER: {
            auto on_order = reinterpret_cast<OnOrder*>(evt.get());
            OrderWrap order(*on_order->order);
            m_stralet.on_event(evt->evt_id, &order);
            break;
        }
        case STRALET_EVENT_ID::ON_TRADE: {
            auto on_trade = reinterpret_cast<OnTrade*>(evt.get());
            TradeWrap trade(*on_trade->trade);
            m_stralet.on_event(evt->evt_id, &trade);
            break;
        }
        case STRALET_EVENT_ID::ON_ACCOUNT_STATUS: {
            auto on_account = reinterpret_cast<OnAccountStatus*>(evt.get());
            AccountInfoWrap account(*on_account->account);
            m_stralet.on_event(evt->evt_id, &account);
            break;
        }
        }
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
