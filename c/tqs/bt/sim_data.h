#ifndef _SIM_DATA_H
#define _SIM_DATA_H

#include <assert.h>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "tquant_api.h"
#include "stralet.h"

namespace tquant { namespace stralet { namespace backtest {

    using namespace std;
    using namespace tquant::api;
    using namespace tquant::stralet;

    class SimStraletContext;

    struct TickCache {
        int64_t       pos;
        int64_t       size;
        shared_ptr<const MarketQuoteArray> ticks;
    };

    struct BarCache {
        int64_t pos;
        int64_t size;
        shared_ptr<const BarArray> bars;
    };

    struct DailyBarCache {
        int64_t pos;
        shared_ptr<const DailyBarArray> daily_bars;
    };

    class SimDataApi : public DataApi {
        friend SimStraletContext;
    public:

        SimDataApi(SimStraletContext* ctx, DataApi* dapi)
            : m_ctx(ctx)
            , m_dapi(dapi)
            , m_is_EOD(false)
        {
        }

        virtual CallResult<const MarketQuoteArray> tick       (const string& code, int trading_day) override;
        virtual CallResult<const BarArray>         bar        (const string& code, const string& cycle, int trading_day, bool align) override;
        virtual CallResult<const DailyBarArray>    daily_bar  (const string& code, const string& price_adj, bool align) override;
        virtual CallResult<const MarketQuote>      quote      (const string& code) override;
        virtual CallResult<const vector<string>>   subscribe  (const vector<string>& codes) override;
        virtual CallResult<const vector<string>>   unsubscribe(const vector<string>& codes) override;

        virtual DataApi_Callback* set_callback(DataApi_Callback* callback) override;

        void calc_nex_time(DateTime* dt);

        shared_ptr<MarketQuote> next_quote(const string& code);
        shared_ptr<Bar>         next_bar(const string & code);
        const RawBar*           last_bar(const string & code);
        const RawDailyBar*      cur_daily_bar(const string & code);

        void set_data_to_curtime();
        void set_end_of_day();

        DataApi* dapi() { return m_dapi; }

        void move_to(int trading_day);

        const unordered_set<string>& sub_codes() { return m_codes; }
        void preload_bar        (const vector<string>& codes);
        void preload_daily_bar  (const vector<string>& codes);
        void preload_tick       (const vector<string>& codes);

        void pin_code    (const string& code);
        void unpin_code  (const string& code);

    private:
        SimStraletContext* m_ctx;
        DataApi* m_dapi;
        unordered_map<string, shared_ptr<TickCache>    > m_tick_caches;
        unordered_map<string, shared_ptr<BarCache>     > m_bar_caches;
        unordered_map<string, shared_ptr<DailyBarCache>> m_dailybar_caches;
        unordered_set<string> m_codes;
        unordered_set<string> m_pinned_codes;
        bool m_is_EOD;
    };

} } }

#endif
