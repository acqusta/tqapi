#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "myutils/timeutils.h"
#include "myutils/unicode.h"
#include "myutils/loop/MsgRunLoop.h"
#include "json/json.h"

#ifdef ERROR
#  undef ERROR
#endif

#include "realtime.h"


namespace tquant { namespace stralet { namespace realtime {

    using namespace tquant::stralet;

    class DataApiWrap : public DataApi {
        DataApi* m_dapi;
    public:
        DataApiWrap(DataApi* dapi)
            : m_dapi(dapi)
        {}

        virtual CallResult<const MarketQuoteArray> tick(const string& code, int trading_day) override
        {
            return m_dapi->tick(code, trading_day);
        }

        virtual CallResult<const BarArray> bar(const string& code, const string& cycle, int trading_day, bool align) override
        {
            return m_dapi->bar(code, cycle, trading_day, align);
        }

        virtual CallResult<const DailyBarArray> daily_bar(const string& code, const string& price_adj, bool align) override
        {
            return m_dapi->daily_bar(code, price_adj, align);
        }

        virtual CallResult<const MarketQuote> quote(const string& code) override
        {
            return m_dapi->quote(code);
        }

        virtual CallResult<const vector<string>> subscribe(const vector<string>& codes) override
        {
            return m_dapi->subscribe(codes);
        }

        virtual CallResult<const vector<string>> unsubscribe(const vector<string>& codes) override
        {
            return m_dapi->unsubscribe(codes);
        }

        virtual DataApi_Callback* set_callback(DataApi_Callback* callback) override
        {
            return nullptr;
        }
    };

    class TradeApiWrap : public TradeApi {    
        TradeApi* m_tapi;
    public:
        TradeApiWrap(TradeApi* tapi)
            : m_tapi(tapi)
        { }

        virtual CallResult<const vector<AccountInfo>> query_account_status() override
        {
            return m_tapi->query_account_status();
        }

        virtual CallResult<const Balance> query_balance(const string& account_id) override
        {
            return m_tapi->query_balance(account_id);
        }

        virtual CallResult<const vector<Order>> query_orders(const string& account_id, const unordered_set<string>* codes) override
        {
            return m_tapi->query_orders(account_id, codes);
        }

        virtual CallResult<const vector<Order>> query_orders(const string& account_id, const string& codes) override
        {
            return m_tapi->query_orders(account_id, codes);
        }

        virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const unordered_set<string>* codes) override
        {
            return m_tapi->query_trades(account_id, codes);
        }

        virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const string& codes) override
        {
            return m_tapi->query_trades(account_id, codes);
        }

        virtual CallResult<const vector<Position>> query_positions(const string& account_id, const unordered_set<string>* codes) override
        {
            return m_tapi->query_positions(account_id, codes);
        }

        virtual CallResult<const vector<Position>> query_positions(const string& account_id, const string& codes) override
        {
            return m_tapi->query_positions(account_id, codes);
        }

        virtual CallResult<const OrderID> place_order(const string& account_id, const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id) override
        {
            return m_tapi->place_order(account_id, code, price, size, action, price_type, order_id);
        }

        virtual CallResult<bool> cancel_order(const string& account_id, const string& code, int order_id) override
        {
            return m_tapi->cancel_order(account_id, code, order_id);
        }

        virtual CallResult<bool> cancel_order(const string& account_id, const string& code, const string& entrust_no) override
        {
            return m_tapi->cancel_order(account_id, code, entrust_no);
        }

        virtual CallResult<string> query(const string& account_id, const string& command, const string& params) override
        {
            return m_tapi->query(account_id, command, params);
        }

        virtual TradeApi_Callback* set_callback(TradeApi_Callback* callback) override
        {
            return nullptr;
        }
    };

    struct TimerInfo {
        int64_t  id;
        int64_t  delay;
        void*    data;
        bool     is_dead;
    };

    class RealTimeStraletContext : public StraletContext, public DataApi_Callback, public TradeApi_Callback {
    public:
        RealTimeStraletContext(DataApi* dapi, TradeApi* tapi, Stralet* stralet, Json::Value& properties)
            : m_stralet(stralet)
            , m_mode("realtime")
        {
            m_properties = properties;
            m_properties_str = m_properties.toStyledString();

            // TODO: Should load trade calendar
            time_t now;
            now = time(&now);
            tm *t = localtime(&now);
            if (t->tm_hour >= 19) {
                now += 24 * 3600;
                t = localtime(&now);
            }
            while (t->tm_wday == 0 || t->tm_wday==6) {
                now += 24 * 3600;
                t = localtime(&now);
            }
            m_tradingday = fin_date(now);

            dapi->set_callback(this);
            tapi->set_callback(this);
            m_dapi_wrap = new DataApiWrap(dapi);
            m_tapi_wrap = new TradeApiWrap(tapi);
        }

        ~RealTimeStraletContext()
        {
            if (m_tapi_wrap) delete m_tapi_wrap;
            if (m_dapi_wrap) delete m_dapi_wrap;
        }

        virtual int32_t trading_day() override
        {
            return m_tradingday;
        }

        virtual DateTime cur_time() override 
        {
            DateTime now;
            fin_datetime(&now.date, &now.time);
            return now;
        }

        virtual system_clock::time_point cur_time_as_tp() override 
        {
            return system_clock::now();
        }

        virtual void  post_event(const char* evt, void* data) override;

        void timer_tigger(shared_ptr<TimerInfo> timer);

        virtual void set_timer(int64_t id, int64_t delay, void* data) override;

        virtual void kill_timer(int64_t id) override;

        virtual DataApi*  data_api() override;

        virtual TradeApi* trade_api() override;

        virtual LogStream logger(LogSeverity severity = LogSeverity::INFO) override;

        virtual string        get_property(const char* name, const char* def_value) override;
        virtual const string& get_properties() override;

        virtual const string& mode() override;

        // DataApi Callback
        virtual void on_market_quote(shared_ptr<const MarketQuote> quote) override;
        virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override;

        // TradeApi Callback
        virtual void on_order_status(shared_ptr<Order> order) override;
        virtual void on_order_trade(shared_ptr<Trade> trade) override;
        virtual void on_account_status(shared_ptr<AccountInfo> account) override;

    //private:
        int m_tradingday;
        //DataApi*      m_dapi;
        //TradeApi*     m_tapi;
        TradeApiWrap* m_tapi_wrap;
        DataApiWrap*  m_dapi_wrap;

        std::unordered_map<int64_t, shared_ptr<TimerInfo>> m_timers;
        Stralet* m_stralet;

        loop::MessageLoop m_msgloop;
        string            m_mode;
        Json::Value       m_properties;
        string            m_properties_str;

    };

    void RealTimeStraletContext::post_event(const char* evt, void* data)
    {
        auto on_event = make_shared<OnEvent>(evt, data);
        m_msgloop.post_task([this, on_event]() { m_stralet->on_event(on_event); });
    }

    DataApi* RealTimeStraletContext::data_api()
    {
        return m_dapi_wrap;
    }

    TradeApi* RealTimeStraletContext::trade_api()
    {
        return m_tapi_wrap;
    }

    void RealTimeStraletContext::set_timer(int64_t id, int64_t delay, void* data)
    {
        auto timer = make_shared<TimerInfo>();
        timer->id = id;
        timer->delay = delay;
        timer->data = data;
        timer->is_dead = false;

        auto it = m_timers.find(id);
        if (it != m_timers.end())
            it->second->is_dead = true;

        m_timers[id] = timer;
        m_msgloop.post_delayed_task(bind(&RealTimeStraletContext::timer_tigger, this, timer), (uint32_t)timer->delay);
    }

    void RealTimeStraletContext::kill_timer(int64_t id)
    {
        auto it = m_timers.find(id);
        if (it != m_timers.end()) {
            it->second->is_dead = true;
            m_timers.erase(it);
        }
    }

    void RealTimeStraletContext::timer_tigger(shared_ptr<TimerInfo> timer)
    {
        if (!timer->is_dead) {
            m_stralet->on_event(make_shared<OnTimer>(timer->id, timer->data));
            if (!timer->is_dead)
                m_msgloop.post_delayed_task(bind(&RealTimeStraletContext::timer_tigger, this, timer), (uint32_t)timer->delay);
        }
    }


    LogStream RealTimeStraletContext::logger(LogSeverity severity)
    {
        int date, time;
        fin_datetime(&date, &time);
        auto buf = make_shared<LogStreamBuf>(severity, date, time);
        LogStream tmp(buf);
        return tmp;
    }

    string RealTimeStraletContext::get_property(const char* name, const char* def_value)
    {
        Json::Value empty;
        Json::Value v = m_properties.get(name, empty);
        switch (v.type()) {
        case Json::ValueType::nullValue: return def_value;
        case Json::ValueType::intValue:
        case Json::ValueType::uintValue:
        case Json::ValueType::realValue:
        case Json::ValueType::stringValue:
        case Json::ValueType::booleanValue: return v.asString();
        default:
            return v.toStyledString();
        }
    }

    const string& RealTimeStraletContext::get_properties()
    {
        return m_properties_str;
    }

    const string& RealTimeStraletContext::mode()
    {
        return m_mode;
    }

    void RealTimeStraletContext::on_market_quote(shared_ptr<const MarketQuote> quote)
    {
        m_msgloop.post_task([this, quote]() {
            m_stralet->on_event(make_shared<OnQuote>(quote));
        });
    }

    void RealTimeStraletContext::on_bar(const string& cycle, shared_ptr<const Bar> bar)
    {
        auto on_bar = make_shared<OnBar>(cycle, bar);
        m_msgloop.post_task([this, on_bar]() {
            m_stralet->on_event(on_bar);
        });
    }

    void RealTimeStraletContext::on_order_status(shared_ptr<Order> order)
    {
        m_msgloop.post_task([this, order]() {
            m_stralet->on_event(make_shared<OnOrder>(order));
        });
    }

    void RealTimeStraletContext::on_order_trade(shared_ptr<Trade> trade)
    {
        m_msgloop.post_task([this, trade]() {
            m_stralet->on_event(make_shared<OnTrade>(trade));
        });
    }
    void RealTimeStraletContext::on_account_status(shared_ptr<AccountInfo> account)
    {
        m_msgloop.post_task([this, account]() {
            auto evt = make_shared<OnAccountStatus>(account);
            m_stralet->on_event(evt);
        });
    }

    void run(const RealTimeConfig & a_cfg, function<Stralet*()> creator)
    {
        RealTimeConfig cfg = a_cfg;
        if (cfg.data_api_addr.empty())   cfg.data_api_addr = "ipc://tqc_10001";
        if (cfg.trade_api_addr.empty())  cfg.trade_api_addr = "ipc://tqc_10001";
        if (cfg.output_dir.empty()) cfg.output_dir = ".";

        //cout << "run stralet: " << cfg.output_dir << endl;

        auto dapi = create_data_api (cfg.data_api_addr.c_str());
        auto tapi = create_trade_api(cfg.trade_api_addr.c_str());

        auto stralet = creator();

        Json::Value properties;
        if (cfg.properties.size()) {
            Json::Reader reader;
            if (!reader.parse(cfg.properties, properties)) {
                cerr << "parse conf failure: " << reader.getFormattedErrorMessages();
                return;
            }
        }

        RealTimeStraletContext* sc = new RealTimeStraletContext(dapi, tapi, stralet, properties);

        stralet->set_context(sc);
        stralet->on_event(make_shared<OnInit>());
        loop::RunLoop(&sc->m_msgloop).run();

        stralet->on_event(make_shared<OnFini>());

        delete stralet;
        delete sc;
        delete tapi;
        delete dapi;
    }

    void run(const char* cfg_str, function<Stralet*()> creator)
    {
        string utf8 = gbk_to_utf8(cfg_str);

        Json::Value conf;
        Json::Reader reader;
        if (!reader.parse(utf8, conf)) {
            cerr << "parse conf failure: " << reader.getFormattedErrorMessages();
            return;
        }

        RealTimeConfig cfg;
        try {
            Json::Value empty;
            Json::Value data_api_addr = conf.get("data_api_addr", empty);
            if (data_api_addr.isString()) cfg.data_api_addr = data_api_addr.asString();

            Json::Value trade_api_addr = conf.get("trade_api_addr", empty);
            if (trade_api_addr.isString()) cfg.trade_api_addr = trade_api_addr.asString();

            Json::Value output_dir = conf.get("output_dir", empty);
            if (output_dir.isString())    cfg.output_dir = output_dir.asString();

        }
        catch (exception& e) {
            cerr << "parse conf failure: " << e.what();
            return;
        }

        run(cfg, creator);
    }

} } }
