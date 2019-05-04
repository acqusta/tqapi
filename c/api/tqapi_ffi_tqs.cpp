#include <string.h>
#include <iostream>

#include "stralet.h"
#include "bt/backtest.h"
#include "rt/realtime.h"
#include "tqapi_ffi.h"

// using namespace tquant::api;
// using namespace tquant::stralet;

#ifdef _WIN32
#  define _TQS_EXPORT __declspec(dllexport)
#else
#  define _TQS_EXPORT
#endif

void init_corder      (Order* c,       const tquant::api::Order* order);
void init_ctrade      (Trade* c,       const tquant::api::Trade* trade);
void init_cposition   (Position* c,    const tquant::api::Position* position);
void init_caccount    (AccountInfo* c, const tquant::api::AccountInfo* account);

// struct CStralet {
//     void* user_data;
//     void (*on_init)          (StraletContext* ctx, void* user_data);
//     void (*on_fini)          (StraletContext* ctx, void* user_data);
//     void (*on_quote)         (StraletContext* ctx, const MarketQuote* quote, void* user_data);
//     void (*on_bar)           (StraletContext* ctx, const char* cycle, const Bar* bar, void* user_data);
//     void (*on_order)         (StraletContext* ctx, const Order* order, void* user_data);
//     void (*on_trade)         (StraletContext* ctx, const Trade* trade, void* user_data);
//     void (*on_timer)         (StraletContext* ctx, int64_t id, void* data, void* user_data);
//     void (*on_event)         (StraletContext* ctx, const char* name, void* data, void* user_data);
//     void (*on_account_status)(StraletContext* ctx, const AccountInfo* account, void* user_data);
// };

extern "C" {
    TradeApi* tqapi_tapi_from(tquant::api::TradeApi* inst);
    DataApi*  tqapi_dapi_from(tquant::api::DataApi* inst);
}
struct StraletContext {
    tquant::stralet::StraletContext* cpp_ctx;
    TradeApi* tapi;
    DataApi*  dapi;
};

extern "C" _TQS_EXPORT
int32_t tqapi_sc_trading_day(StraletContext* ctx)
{
    return ctx->cpp_ctx->trading_day();
}

// struct C_DateTime {
//     int date;
//     int time;
// };

extern "C" _TQS_EXPORT
DateTime tqapi_sc_cur_time(StraletContext* ctx)
{
    DateTime v;
    auto r = ctx->cpp_ctx->cur_time();
    v.date = r.date;
    v.time = r.time;
    return v;
}

extern "C" _TQS_EXPORT
void tqapi_sc_post_event(StraletContext* ctx, const char* evt, void* data)
{
    ctx->cpp_ctx->post_event(evt, data);
}

extern "C" _TQS_EXPORT
void tqapi_sc_set_timer(StraletContext* ctx, int64_t id, int64_t delay, void* data)
{
    ctx->cpp_ctx->set_timer(id, delay, data);
}

extern "C" _TQS_EXPORT
void tqapi_sc_kill_timer(StraletContext* ctx, int64_t id)
{
    ctx->cpp_ctx->kill_timer(id);
}

extern "C" _TQS_EXPORT
DataApi*  tqapi_sc_data_api(StraletContext* ctx)
{
    return ctx->dapi;
}

extern "C" _TQS_EXPORT
TradeApi* tqapi_sc_trade_api(StraletContext* ctx)
{
    return ctx->tapi;
}

extern "C" _TQS_EXPORT
void tqapi_sc_log(StraletContext* ctx, int32_t severity, const char* str)
{
    const char* p = str + strlen(str) - 1;
    if (*p != '\n')
        ctx->cpp_ctx->logger((tquant::stralet::LogSeverity)severity) << str << endl;
    else
        ctx->cpp_ctx->logger((tquant::stralet::LogSeverity)severity) << str;
}

extern "C" _TQS_EXPORT
const char* tqapi_sc_get_properties(StraletContext* ctx)
{
    return ctx->cpp_ctx->get_properties().c_str();
}

extern "C" _TQS_EXPORT
const char* tqapi_sc_get_mode(StraletContext* ctx)
{
    return ctx->cpp_ctx->mode().c_str();
}


struct StraletWrap : public tquant::stralet::Stralet {
public:
    ::Stralet*      m_stralet;
    StraletContext* m_cctx;
    StraletFactory* m_factory;

    StraletWrap(StraletFactory* factory)
        : m_stralet(nullptr), m_cctx(nullptr)
        , m_factory(factory)
    {
        m_stralet = factory->create(factory->obj);
    }

    ~StraletWrap() {
        delete m_cctx;
        m_factory->destroy(m_factory->obj, m_stralet);
    }

    // struct TimerWrap {
    //     int64_t id;
    //     void*   data;
    // };

    // struct EventWrap {
    //     const char* name;
    //     void*       data;
    // };

    // struct BarWrap {
    //     const char*     cycle;
    //     const RawBar*   bar;
    // };

    virtual void on_init() override {
        m_cctx = new StraletContext();
        m_cctx->cpp_ctx = m_ctx;
        m_cctx->dapi = tqapi_dapi_from(m_ctx->data_api());
        m_cctx->tapi = tqapi_tapi_from(m_ctx->trade_api());
        m_stralet->on_init(m_stralet->obj, m_cctx);
    }

    virtual void on_fini() override {
        m_stralet->on_fini(m_stralet->obj, m_cctx);
    }

    virtual void on_quote(shared_ptr<const tquant::api::MarketQuote> quote) override {
        m_stralet->on_quote(m_stralet->obj, m_cctx, (const MarketQuote*)quote.get());
    }

    virtual void on_bar(const string& cycle, shared_ptr<const tquant::api::Bar> bar) override {
        m_stralet->on_bar(m_stralet->obj, m_cctx, cycle.c_str(), (const Bar*)bar.get());
    }

    virtual void on_order(shared_ptr<const tquant::api::Order> order) override {
        Order c_order;
        init_corder(&c_order, order.get());
        m_stralet->on_order(m_stralet->obj, m_cctx, &c_order);
    }

    virtual void on_trade(shared_ptr<const tquant::api::Trade> trade) override {
        Trade c_trade;
        init_ctrade(&c_trade, trade.get());
        m_stralet->on_trade(m_stralet->obj, m_cctx, &c_trade);
    }

    virtual void on_timer(int64_t id, void* data) override {
        m_stralet->on_timer(m_stralet->obj, m_cctx, id, data);
    }

    virtual void on_event(const string& name, void* data) override {
        m_stralet->on_event(m_stralet->obj, m_cctx, name.c_str(), data);
    }

    virtual void on_account_status(shared_ptr<const tquant::api::AccountInfo> account) override {
        AccountInfo c_account;
        init_caccount(&c_account, account.get());
        m_stralet->on_account_status(m_stralet->obj, m_cctx, &c_account);
    }
};

// extern "C" _TQS_EXPORT
// StraletWrap* tqs_stralet_create(CStralet* stralet)
// {
//     return new StraletWrap(*stralet);
// }

// extern "C" _TQS_EXPORT
// void tqs_stralet_destroy(StraletWrap* stralet)
// {
//     delete stralet;
// }

extern "C" _TQS_EXPORT
void tqapi_bt_run(const char* cfg, StraletFactory* factory)
{
    tquant::stralet::backtest::run(cfg, [factory](){
        return new StraletWrap(factory);
    });
}

extern "C" _TQS_EXPORT
void tqapi_rt_run(const char* cfg, StraletFactory* factory)
{
    tquant::stralet::backtest::run(cfg, [factory](){
        return new StraletWrap(factory);
    });
}
