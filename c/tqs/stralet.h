#ifndef _TQUANT_STRALET_H
#define _TQUANT_STRALET_H

#include <chrono>
#include <ostream>

#include "tquant_api.h"

namespace tquant { namespace stralet {

    using namespace std::chrono;
    using namespace tquant::api;

    class StraletContext;

    struct DateTime {
        int date;
        int time;

        DateTime() : date(0), time(0)
        {}

        DateTime(int a_date, int a_time) : date(a_date), time(a_time)
        {}

        int cmp(const DateTime& dt) {
            if (this->date < dt.date) return -1;
            if (this->date == dt.date) return this->time - dt.time;
            return 1;
        }
    };


    class Stralet {
    public:
        virtual ~Stralet() { }

        inline StraletContext* ctx() {
            return m_ctx;
        }

        virtual void on_init(StraletContext* sc) {
            m_ctx = sc;
        }

        virtual void on_fini            () { }
        virtual void on_quote           (shared_ptr<const MarketQuote> q) { }
        virtual void on_bar             (const char* cycle, shared_ptr<const Bar> bar) { }
        virtual void on_timer           (int32_t id, void* data) { }
        virtual void on_event           (const string& evt, void* data) { }
        virtual void on_order_status    (shared_ptr<const Order> order) { }
        virtual void on_order_trade     (shared_ptr<const Trade> trade) { }
        virtual void on_account_status  (shared_ptr<const AccountInfo> account) { }
    protected:
        StraletContext* m_ctx;
    };

    class AlgoStralet : public Stralet {
    };

    enum LogLevel {
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    class StraletContext {
    public:
        virtual ~StraletContext()
        {

        }
        virtual int32_t trading_day() = 0;
        virtual DateTime cur_time( ) = 0;
        virtual system_clock::time_point cur_time_as_tp() = 0;
        virtual void post_event(const char* evt, void* data) = 0;

        virtual void set_timer (Stralet* stralet, int32_t id, int32_t delay, void* data) = 0;
        virtual void kill_timer(Stralet* stralet, int32_t id) = 0;

        virtual DataApi*  data_api(const char* source = "") = 0;
        virtual TradeApi* trade_api() = 0;

        virtual ostream& logger(LogLevel level = LogLevel::INFO) = 0;

        virtual string get_parameter(const char* name, const char* def_value) = 0;

        virtual const string& mode() = 0;

        virtual void register_algo  (AlgoStralet* algo) = 0;
        virtual void unregister_algo(AlgoStralet* algo) = 0;
    };
} }

#endif
