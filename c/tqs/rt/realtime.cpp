#include <iostream>
#include <unordered_map>
#include "realtime.h"
#include "myutils/timeutils.h"
#include "myutils/unicode.h"
#include "myutils/loop/MsgRunLoop.h"
#include "jsoncpp/inc/json/json.h"


namespace tquant { namespace stralet { namespace realtime {

    using namespace tquant::stralet;

    class DataApiWrap : public DataApi {
        DataApi* m_dapi;
    public:
        DataApiWrap(DataApi* dapi)
            : m_dapi(dapi)
        {}

        virtual CallResult<const vector<MarketQuote>> tick(const string& code, int trading_day) override
        {
            return m_dapi->tick(code, trading_day);
        }

        virtual CallResult<const vector<Bar>> bar(const string& code, const string& cycle, int trading_day, bool align) override
        {
            return m_dapi->bar(code, cycle, trading_day, align);
        }

        virtual CallResult<const vector<DailyBar>> daily_bar(const string& code, const string& price_adj, bool align) override
        {
            return m_dapi->daily_bar(code, price_adj, align);
        }

        virtual CallResult<const MarketQuote> quote(const string& code) override
        {
            return m_dapi->quote(code);
        }

        virtual CallResult<const vector<string>> subscribe(const vector<string>& codes)
        {
            return m_dapi->subscribe(codes);
        }

        virtual CallResult<const vector<string>> unsubscribe(const vector<string>& codes)
        {
            return m_dapi->unsubscribe(codes);
        }

        virtual DataApi_Callback* set_callback(DataApi_Callback* callback)
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

        virtual CallResult<const vector<Order>> query_orders(const string& account_id) override
        {
            return m_tapi->query_orders(account_id);
        }

        virtual CallResult<const vector<Trade>> query_trades(const string& account_id) override
        {
            return m_tapi->query_trades(account_id);
        }

        virtual CallResult<const vector<Position>> query_positions(const string& account_id) override
        {
            return m_tapi->query_positions(account_id);
        }

        virtual CallResult<const OrderID> place_order(const string& account_id, const string& code, double price, int64_t size, const string& action, int order_id) override
        {
            return m_tapi->place_order(account_id, code, price, size, action, order_id);
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
        Stralet* stralet;
        int64_t  id;
        int64_t  delay;
        void*    data;
        bool     is_dead;
    };

    class RealTimeStraletContext : public StraletContext, public DataApi_Callback, public TradeApi_Callback {
    public:
        RealTimeStraletContext(DataApi* dapi, TradeApi* tapi, Stralet* stralet)
            : m_stralet(stralet)
            , m_mode("realtime")
        {
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

        virtual void set_timer(Stralet* stralet, int64_t id, int64_t delay, void* data) override;

        virtual void kill_timer(Stralet* stralet, int64_t id) override;

        virtual DataApi*  data_api() override;

        virtual TradeApi* trade_api() override;

        virtual ostream& logger(LogLevel level = LogLevel::INFO) override;

        virtual string        get_property(const char* name, const char* def_value) override;
        virtual const string& get_properties() override;

        virtual const string& mode() override;

        virtual void register_algo(AlgoStralet* algo) override;

        virtual void unregister_algo(AlgoStralet* algo) override;

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

        std::unordered_map<string, shared_ptr<TimerInfo>> m_timers;
        vector<AlgoStralet*> m_algos;
        Stralet* m_stralet;

        loop::MessageLoop m_msgloop;
        string m_mode;
    };

    void RealTimeStraletContext::post_event(const char* evt, void* data)
    {
        string evt_s(evt);
        m_msgloop.PostTask([this, evt_s, data]() { m_stralet->on_event(evt_s, data); });
    }

    DataApi* RealTimeStraletContext::data_api()
    {
        return m_dapi_wrap;
    }

    TradeApi* RealTimeStraletContext::trade_api()
    {
        return m_tapi_wrap;
    }

    void RealTimeStraletContext::set_timer(Stralet* stralet, int64_t id, int64_t delay, void* data)
    {
        auto timer = make_shared<TimerInfo>();
        timer->stralet = stralet;
        timer->id = id;
        timer->delay = delay;
        timer->data = data;
        timer->is_dead = false;

        char str_id[100];
        uint64_t tmp = (uint64_t)stralet;
#ifdef _WIN32
        sprintf(str_id, "%I64u%I64u", tmp, id);
#else
        sprintf(str_id, "%llu%llu", tmp, id);
#endif
        auto it = m_timers.find(str_id);
        if (it != m_timers.end())
            it->second->is_dead = true;
        m_timers[str_id] = timer;
        m_msgloop.PostDelayedTask(bind(&RealTimeStraletContext::timer_tigger, this, timer), (uint32_t)timer->delay);
    }

    void RealTimeStraletContext::kill_timer(Stralet* stralet, int64_t id)
    {
        char str_id[100];
        uint64_t tmp = (uint64_t)stralet;
#ifdef _WIN32
        sprintf(str_id, "%I64u%I64u", tmp, id);
#else
        sprintf(str_id, "%llu%llu", tmp, id);
#endif
        auto it = m_timers.find(str_id);
        if (it != m_timers.end()) {
            it->second->is_dead = true;
            m_timers.erase(it);
        }
    }

    void RealTimeStraletContext::timer_tigger(shared_ptr<TimerInfo> timer)
    {
        if (!timer->is_dead) {
            m_stralet->on_timer(timer->id, timer->data);
            if (!timer->is_dead)
                m_msgloop.PostDelayedTask(bind(&RealTimeStraletContext::timer_tigger, this, timer), (uint32_t)timer->delay);
        }
    }


    ostream& RealTimeStraletContext::logger(LogLevel level)
    {
        static const char* str_level[] = {
            "I",
            "W",
            "E",
            "F"
        };

        DateTime now = cur_time();

        char label[100];

        int h = now.time / 1000 / 10000;
        int m = (now.time / 1000 / 100) % 100;
        int s = (now.time / 1000) % 100;
        sprintf(label, "%08d %02d:%02d:%02d.%03d %s| ", now.date, h, m, s, now.time % 1000, str_level[level]);
        cout << label;
        return cout;
    }

    void RealTimeStraletContext::register_algo(AlgoStralet* algo)
    {
        m_algos.push_back(algo);
    }

    void RealTimeStraletContext::unregister_algo(AlgoStralet* algo)
    {
        auto it = find(m_algos.begin(), m_algos.end(), algo);
        if (it != m_algos.end())
            m_algos.erase(it);
    }

    string RealTimeStraletContext::get_property(const char* name, const char* def_value)
    {
        return "";
    }

    const string& RealTimeStraletContext::get_properties()
    {
        return "";
    }

    const string& RealTimeStraletContext::mode()
    {
        return m_mode;
    }

    void RealTimeStraletContext::on_market_quote(shared_ptr<const MarketQuote> quote)
    {
        m_msgloop.PostTask([this, quote]() {
            for (auto& algo : m_algos) algo->on_quote(quote);
            m_stralet->on_quote(quote);
        });
    }

    void RealTimeStraletContext::on_bar(const string& cycle, shared_ptr<const Bar> bar)
    {
        m_msgloop.PostTask([this, cycle, bar]() {
            for (auto& algo : m_algos) algo->on_bar(cycle.c_str(), bar);
            m_stralet->on_bar(cycle.c_str(), bar);
        });
    }

    void RealTimeStraletContext::on_order_status(shared_ptr<Order> order)
    {
        m_msgloop.PostTask([this, order]() {
            for (auto& algo : m_algos) algo->on_order_status(order);
            m_stralet->on_order_status(order);
        });
    }

    void RealTimeStraletContext::on_order_trade(shared_ptr<Trade> trade)
    {
        m_msgloop.PostTask([this, trade]() {
            for (auto& algo : m_algos) algo->on_order_trade(trade);
            m_stralet->on_order_trade(trade);
        });
    }
    void RealTimeStraletContext::on_account_status(shared_ptr<AccountInfo> account)
    {
        m_msgloop.PostTask([this, account]() {
            for (auto& algo : m_algos) algo->on_account_status(account);
            m_stralet->on_account_status(account);
        });
    }

    void run(const RealTimeConfig & a_cfg, function<Stralet*()> creator)
    {
        RealTimeConfig cfg = a_cfg;
        if (cfg.data_api_addr.empty())   cfg.data_api_addr = "ipc://tqc_10001";
        if (cfg.trade_api_addr.empty())  cfg.trade_api_addr = "ipc://tqc_10001";
        if (cfg.output_dir.empty()) cfg.output_dir = ".";

        //cout << "run stralet: " << cfg.output_dir << endl;

        auto dapi = create_data_api(cfg.data_api_addr.c_str());
        auto tapi = create_trade_api(cfg.data_api_addr.c_str());

        auto stralet = creator();

        RealTimeStraletContext* sc = new RealTimeStraletContext(dapi, tapi, stralet);

        stralet->on_init(sc);

        loop::RunLoop run(&sc->m_msgloop);
        run.Run();

        stralet->on_fini();
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
