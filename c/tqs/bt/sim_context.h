#ifndef _SIM_CONTEXT_H
#define _SIM_CONTEXT_H

#include <unordered_map>
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

        void init(SimDataApi* dapi, DataLevel level, SimTradeApi* tapi);

        DataLevel data_level() { return m_data_level; }

        SimDataApi*  sim_dapi() { return m_dapi; }
        SimTradeApi* sim_tapi() { return m_tapi; }

        SimAccount*  get_account(const string& account_id);

        void move_to(int trading_day);
        void run_one_day(Stralet* stralet);
        void calc_next_timer_time(DateTime* dt);
        void execute_timer(Stralet*);
        void set_sim_time(const DateTime& dt);

        virtual int32_t trading_day() override;
        virtual DateTime cur_time() override;
        virtual system_clock::time_point cur_time_as_tp() override;
        virtual void post_event(const char* evt, void* data) override;

        virtual void set_timer (Stralet* stralet, int32_t id, int32_t delay, void* data) override;
        virtual void kill_timer(Stralet* stralet, int32_t id) override;

        virtual DataApi*  data_api(const char* source = nullptr) override;
        virtual TradeApi* trade_api() override;

        virtual ostream& logger(LogLevel level = LogLevel::INFO) override;

        virtual string get_parameter(const char* name, const char* def_value) override;

        virtual const string& mode() override;

        virtual void register_algo(AlgoStralet* algo) override;
        virtual void unregister_algo(AlgoStralet* algo) override;

    private:
        SimDataApi*  m_dapi;
        SimTradeApi* m_tapi;
        DataLevel    m_data_level;
        int32_t      m_trading_day;

        system_clock::time_point m_now_tp;
        DateTime m_now;

        struct TimerInfo {
            Stralet* stralet;
            int32_t  id;
            int32_t  delay;
            void*    data;
            bool     is_dead;
            system_clock::time_point trigger_time;
        };

        vector<shared_ptr<TimerInfo>> m_timers;
        vector<AlgoStralet*> m_algos;
        Stralet* m_stralet;
        string m_mode;
    };

} } }

#endif
