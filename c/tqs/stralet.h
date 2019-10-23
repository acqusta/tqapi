#ifndef _TQUANT_STRALET_H
#define _TQUANT_STRALET_H

#include <chrono>
#include <ostream>

#include "tquant_api.h"
#include "logger.h"

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

        int cmp(const DateTime& dt) const {
            if (this->date < dt.date) return -1;
            if (this->date == dt.date) return this->time - dt.time;
            return 1;
        }

        milliseconds sub(const DateTime& dt) const;

        static DateTime from_timepoint(system_clock::time_point  tp);

        system_clock::time_point to_timepoint() const;
    };

    enum STRALET_EVENT_ID {
        ZERO_ID = 0,
        ON_INIT,
        ON_FINI,
        ON_QUOTE,
        ON_BAR,
        ON_TIMER,
        ON_EVENT,
        ON_ORDER,
        ON_TRADE,
        ON_ACCOUNT_STATUS
    };

  //  struct StraletEvent {
  //      STRALET_EVENT_ID evt_id;
  //      StraletEvent(STRALET_EVENT_ID a_id) : evt_id(a_id) {}
  //      virtual ~StraletEvent() {}

  //      template <class T_EVENT>
  //      T_EVENT* as() {
  //          return reinterpret_cast<T_EVENT*>(this);
  //      }
  //  };

  //  struct OnInit : StraletEvent {
		//StraletContext* ctx;
		//OnInit(StraletContext* c) : StraletEvent(ON_INIT), ctx(c){}
  //  };

  //  struct OnFini : StraletEvent {
  //      OnFini() : StraletEvent(ON_FINI) {}
  //  };

  //  struct OnQuote : StraletEvent {
  //      shared_ptr<const MarketQuote> quote;
  //      OnQuote(shared_ptr<const MarketQuote> q) : StraletEvent(ON_QUOTE), quote(q) {}
  //  };

  //  struct OnBar : StraletEvent {
  //      string cycle;
  //      shared_ptr<const Bar> bar;
  //      OnBar(const string& a_cycle, shared_ptr<const Bar> b) : StraletEvent(ON_BAR), cycle(a_cycle), bar(b) {}
  //  };

  //  struct OnTimer : StraletEvent {
  //      int64_t id;
  //      void*   data;
  //      OnTimer(int64_t a_id, void* a_data) : StraletEvent(ON_TIMER), id(a_id), data(a_data) {}
  //  };

  //  struct OnEvent : StraletEvent {
  //      string  name;
  //      void*   data;
  //      OnEvent(const string& a_name, void* a_data) : StraletEvent(ON_EVENT), name(a_name), data(a_data) {}
  //  };

  //  struct OnOrder : StraletEvent {
  //      shared_ptr<Order> order;
  //      OnOrder(shared_ptr<Order> a_order) : StraletEvent(ON_ORDER), order(a_order) {}
  //  };

  //  struct OnTrade : StraletEvent {
  //      shared_ptr<Trade> trade;
  //      OnTrade(shared_ptr<Trade> a_trade) : StraletEvent(ON_TRADE), trade(a_trade) {}
  //  };
  //  struct OnAccountStatus : StraletEvent {
  //      shared_ptr<AccountInfo> account;
  //      OnAccountStatus(shared_ptr<AccountInfo> a_account) : StraletEvent(ON_ACCOUNT_STATUS), account(a_account) {}
  //  };

  //  class StraletEventDispatcher {
  //  public:
		//StraletEventDispatcher()
  //      {}

  //      //void set_context(StraletContext* ctx)
  //      //{
  //      //    m_ctx = ctx;
  //      //}

  //      virtual ~StraletEventDispatcher() { }

  //      //inline StraletContext* ctx() {
  //      //    return m_ctx;
  //      //}

  //      virtual void on_event(shared_ptr<StraletEvent> evt) = 0;

  ////  protected:
  ////      StraletContext* m_ctx;
		////Stralet*        m_stralet
  //  };

	class Stralet {
	public:
		Stralet() : m_ctx(nullptr)
		{}

		void set_context(StraletContext* ctx)
		{
			m_ctx = ctx;
		}

		virtual ~Stralet() { }

		inline StraletContext* ctx() {
			return m_ctx;
		}

		virtual void on_init			() { }
		virtual void on_fini			() { }
		virtual void on_quote			(shared_ptr<const MarketQuote> quote) { }
		virtual void on_bar				(const string& cycle, shared_ptr<const Bar> bar) { }
		virtual void on_order			(shared_ptr<const Order> order) { }
		virtual void on_trade			(shared_ptr<const Trade> trade) { }
		virtual void on_timer			(int64_t id, void* data) { }
		virtual void on_event			(const string& name, void* data) { }
		virtual void on_account_status	(shared_ptr<const AccountInfo> account) { }

	protected:
		StraletContext* m_ctx;
	};

    class StraletContext {
    public:
        virtual ~StraletContext()
        {}

        virtual int32_t trading_day() = 0;
        virtual DateTime cur_time( ) = 0;
        virtual system_clock::time_point cur_time_as_tp() = 0;

        virtual void post_event(const char* evt, void* data) = 0;

        virtual void set_timer (int64_t id, int64_t delay, void* data) = 0;
        virtual void kill_timer(int64_t id) = 0;

        virtual DataApi*  data_api() = 0;
        virtual TradeApi* trade_api() = 0;

        virtual LogStream logger(LogSeverity severity = LogSeverity::INFO) = 0;

        virtual string        get_property(const char* name, const char* def_value) = 0;
        virtual const string& get_properties() = 0;

        virtual const string& mode() = 0;

        virtual void stop() = 0;
    };
} }

#endif
