#ifndef _SIM_CONTEXT_H
#define _SIM_CONTEXT_H

#include <list>
#include <unordered_map>
#include "jsoncpp/inc/json/json.h"
#include "stralet.h"

namespace tquant { namespace stralet { namespace backtest {

    using namespace tquant::api;
    using namespace tquant::stralet;

    enum DataLevel {
        BT_TICK,
        BT_BAR1M,
        BT_BAR1D
    };

    inline int HMS(int h, int m, int s = 0, int ms = 0) { return h * 10000000 + m * 100000 + s * 1000; }


    class SimDataApi;
    class SimTradeApi;
    class SimAccount;

    class SimStraletContext : public StraletContext {
    public:
        SimStraletContext();

        void init(SimDataApi* dapi,SimTradeApi* tapi,
            DataLevel level,
            Json::Value& properties,
            const vector<int>& calendar
            );

        DataLevel data_level() { return m_data_level; }

        SimDataApi*  sim_dapi() { return m_dapi; }
        SimTradeApi* sim_tapi() { return m_tapi; }

        SimAccount*  get_account(const string& account_id);

        void move_to(int trading_day);
        void run_one_day(Stralet* stralet);
        void calc_next_timer_time(DateTime* dt);
        void execute_timer();
        void set_sim_time(const DateTime& dt);
        void init_sim_time();

        bool is_trading_day(int date);

        virtual int32_t trading_day() override;
        virtual DateTime cur_time() override;
        virtual system_clock::time_point cur_time_as_tp() override;
        virtual void post_event(const char* evt, void* data) override;

        virtual void set_timer (int64_t id, int64_t delay, void* data) override;
        virtual void kill_timer(int64_t id) override;

        virtual DataApi*  data_api() override;
        virtual TradeApi* trade_api() override;

        virtual LogStream logger(LogSeverity severity = LogSeverity::INFO) override;

        virtual string get_property(const char* name, const char* def_value) override;
        virtual const string& get_properties() override;

        virtual const string& mode() override;
    private:
        SimDataApi*  m_dapi;
        SimTradeApi* m_tapi;
        DataLevel    m_data_level;
        int32_t      m_trading_day;

        system_clock::time_point m_now_tp;
        DateTime m_now;

        struct TimerInfo {
            //Stralet* stralet;
            int64_t  id;
            int64_t  delay;
            void*    data;
            bool     is_dead;
            system_clock::time_point trigger_time;
        };

        struct EventData {
            string name;
            void* data;
        };

        Stralet*                      m_stralet;
        unordered_map<int64_t, shared_ptr<TimerInfo>> m_timers;
        list<shared_ptr<EventData>>   m_events;
        string                        m_mode;
        Json::Value                   m_properties;
        string                        m_properties_str;

        unordered_set<int> m_calendar;
    };

} } }

#endif
