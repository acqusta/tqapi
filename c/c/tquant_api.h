#ifndef _TQUANT_API_H
#define _TQUANT_API_H

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>

namespace tquant {  namespace api {

    using namespace std;

    template<typename T> 
    class TickDataHolder : public T {
        string _code;
    public:
        TickDataHolder(const T& t, const char* a_code) : T(t), _code(a_code) {
            this->code = _code.c_str();
        }

        TickDataHolder(const TickDataHolder<T>& t) {
            *this = t;
            if (t.code) this->code = t._code.c_str();
        }

        //void set_code(const char* c) {
        //    if (c) {
        //        _code = c;
        //        code = _code.c_str();
        //    }
        //    else {
        //        _code = "";
        //        code = nullptr;
        //    }
        //}
    };

#pragma pack(1)
    // keep same with tk_schema!
    struct RawMarketQuote{
        const char*     code;
        int32_t         date;
        int32_t         time;
        int64_t         recv_time;
        int32_t         trading_day;
        double          open;
        double          high;
        double          low;
        double          close;
        double          last;
        double          high_limit;
        double          low_limit;
        double          pre_close;
        int64_t         volume;
        double          turnover;
        double          ask1;
        double          ask2;
        double          ask3;
        double          ask4;
        double          ask5;
        double          bid1;
        double          bid2;
        double          bid3;
        double          bid4;
        double          bid5;
        int64_t         ask_vol1;
        int64_t         ask_vol2;
        int64_t         ask_vol3;
        int64_t         ask_vol4;
        int64_t         ask_vol5;
        int64_t         bid_vol1;
        int64_t         bid_vol2;
        int64_t         bid_vol3;
        int64_t         bid_vol4;
        int64_t         bid_vol5;
        double          settle;
        double          pre_settle;
        int64_t         oi;
        int64_t         pre_oi;
    };

    typedef TickDataHolder<RawMarketQuote> MarketQuote;

    struct RawBar {
        const char*     code;
        int32_t         date;
        int32_t         time;
        int32_t         trading_day;
        double          open;
        double          high;
        double          low;
        double          close;
        int64_t         volume;
        double          turnover;
        int64_t         oi;
    };

    typedef TickDataHolder<RawBar> Bar;

    struct RawDailyBar {
        const char*     code;
        int32_t         date;
        double          open;
        double          high;
        double          low;
        double          close;
        int64_t         volume;
        double          turnover;
        int64_t         oi;
        double          _padding;
        //double        pre_close;
        //double        pre_settle;
    };

    typedef TickDataHolder<RawDailyBar> DailyBar;

#pragma pack()

    /**
    *  数据查询接口
    *
    *  功能：
    *      查实时行情，当天的tick, 分钟线
    *      订阅和推送行情
    */
    class DataApi_Callback {
    public:
        virtual void onMarketQuote (shared_ptr<MarketQuote> quote) = 0;
        virtual void onBar         (const char* cycle, shared_ptr<Bar> bar) = 0;
    };

    template<typename T_VALUE>
    struct CallResult {
        shared_ptr<T_VALUE> value;
        string    msg;

        CallResult(shared_ptr<T_VALUE> a_value)
            : value(a_value)
        {
        }
        CallResult(const string& a_msg)
            : msg(a_msg)
        {
        }
    };

    class DataApi {
    protected:
        virtual ~DataApi() {}
    public:   
        /**
        * 取某交易日的某个代码的 ticks
        *
        * tradingday 为0，表示当前交易日
        *
        * @param code
        * @param trading_day
        * @return
        */
        virtual CallResult<vector<MarketQuote>> tick(const char* code, int trading_day) = 0;

        /**
        * 取某个代码的Bar
        *
        * 目前只支持分钟线和日线。
        *  当 cycle == "1m"时，返回trading_day的分钟线，trading_day=0表示当前交易日。
        *
        * @param code          证券代码
        * @param cycle         "1m""
        * @param trading_day   交易日，对分钟线有意义
        * @param align         是否对齐
        * @return
        */
        virtual CallResult<vector<Bar>> bar(const char* code, const char* cycle, int trading_day, bool align) = 0;

        /**
        * 取某个代码的日线
        *
        * 目前只支持分钟线和日线。
        *  当 cycle == "1m"时，返回trading_day的分钟线，trading_day=0表示当前交易日。
        *  当 cycle == "1d"时，返回所有的日线，trading_day值无意义。
        *
        * @param code          证券代码
        * @param price_adj     价格复权，取值
        *                        back -- 后复权
        *                        forward -- 前复权
        * @param align         是否对齐
        * @return
        */
        virtual CallResult<vector<DailyBar>> daily_bar(const char* code, const char* price_adj, bool align) = 0;

        /**
        * 取当前的行情快照
        *
        * @param code
        * @return
        */
        virtual CallResult<MarketQuote> quote(const char* code) = 0;

        /**
        * 订阅行情
        *
        * codes为新增的订阅列表，返回所有已经订阅的代码,包括新增的列表。如果codes为空，可以返回已订阅列表。
        *
        * @param codes
        * @return 所有已经订阅的代码
        */
        virtual CallResult<vector<string>> subscribe(const vector<string>& codes) = 0;

        /**
        * 取消订阅
        *
        * codes为需要取消的列表，返回所有还在订阅的代码。
        * 如果需要取消所有订阅，先通过 subscribe 得到所有列表，然后使用unscribe取消

        * @param codes
        * @return
        */
        virtual CallResult<vector<string>> unsubscribe(const vector<string>& codes) = 0;

        /**
        * 设置推送行情的回调函数
        *
        * 当订阅的代码列表中有新的行情，会通过该callback通知用户。
        *
        * @param callback
        */
        virtual void setCallback(DataApi_Callback* callback) = 0;
    };

    class TradeApi {
    protected:
        virtual ~TradeApi() {}
    public:
        TradeApi() { }
    };

    class TQuantApi {
    public:
        
        virtual ~TQuantApi() {}

        /**
        * 取数据接口
        *
        * @return
        */
        virtual TradeApi* trade_api() = 0;

        /**
        *  取交易接口
        *
        * @return
        */
        virtual DataApi*  data_api() = 0;

        static TQuantApi* create(const char* addr);
    };

} }

#endif
