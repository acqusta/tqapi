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
        MarketQuote** first;
        MarketQuote** last;
        shared_ptr<const vector<MarketQuote>> ticks;
    };

    struct BarTickCache {
        int64_t pos;
        int64_t size;
        Bar**   first;
        Bar**   last;
        shared_ptr<const vector<Bar>> bars;
    };

    class SimDataApi : public DataApi {
        friend SimStraletContext;
    public:

        SimDataApi(SimStraletContext* ctx, DataApi* dapi)
            : m_ctx(ctx)
            , m_dapi(dapi)
        {
        }

        virtual CallResult<const vector<MarketQuote>> tick       (const string& code, int trading_day) override;
        virtual CallResult<const vector<Bar>>         bar        (const string& code, const string& cycle, int trading_day, bool align) override;
        virtual CallResult<const vector<DailyBar>>    daily_bar  (const string& code, const string& price_adj, bool align) override;
        virtual CallResult<const MarketQuote>         quote      (const string& code) override;
        virtual CallResult<const vector<string>>      subscribe  (const vector<string>& codes) override;
        virtual CallResult<const vector<string>>      unsubscribe(const vector<string>& codes) override;

        virtual DataApi_Callback* set_callback(DataApi_Callback* callback) override;

        void calc_nex_time(DateTime* dt);

        shared_ptr<MarketQuote> next_quote(const string& code);
        shared_ptr<Bar> next_bar(const string & code);
        const Bar* last_bar(const string & code);

        DataApi* dapi() { return m_dapi; }

        void move_to(int trading_day);

        const unordered_set<string>& sub_codes() { return m_codes; }
    private:
        SimStraletContext* m_ctx;
        DataApi* m_dapi;
        unordered_map<string, TickCache>    m_tick_caches;
        unordered_map<string, BarTickCache> m_bar_caches;
        unordered_set<string> m_codes;
    };

} } }

#endif
