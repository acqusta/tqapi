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
    *  ���ݲ�ѯ�ӿ�
    *
    *  ���ܣ�
    *      ��ʵʱ���飬�����tick, ������
    *      ���ĺ���������
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
        * ȡĳ�����յ�ĳ������� ticks
        *
        * tradingday Ϊ0����ʾ��ǰ������
        *
        * @param code
        * @param trading_day
        * @return
        */
        virtual CallResult<vector<MarketQuote>> tick(const char* code, int trading_day) = 0;

        /**
        * ȡĳ�������Bar
        *
        * Ŀǰֻ֧�ַ����ߺ����ߡ�
        *  �� cycle == "1m"ʱ������trading_day�ķ����ߣ�trading_day=0��ʾ��ǰ�����ա�
        *
        * @param code          ֤ȯ����
        * @param cycle         "1m""
        * @param trading_day   �����գ��Է�����������
        * @param align         �Ƿ����
        * @return
        */
        virtual CallResult<vector<Bar>> bar(const char* code, const char* cycle, int trading_day, bool align) = 0;

        /**
        * ȡĳ�����������
        *
        * Ŀǰֻ֧�ַ����ߺ����ߡ�
        *  �� cycle == "1m"ʱ������trading_day�ķ����ߣ�trading_day=0��ʾ��ǰ�����ա�
        *  �� cycle == "1d"ʱ���������е����ߣ�trading_dayֵ�����塣
        *
        * @param code          ֤ȯ����
        * @param price_adj     �۸�Ȩ��ȡֵ
        *                        back -- ��Ȩ
        *                        forward -- ǰ��Ȩ
        * @param align         �Ƿ����
        * @return
        */
        virtual CallResult<vector<DailyBar>> daily_bar(const char* code, const char* price_adj, bool align) = 0;

        /**
        * ȡ��ǰ���������
        *
        * @param code
        * @return
        */
        virtual CallResult<MarketQuote> quote(const char* code) = 0;

        /**
        * ��������
        *
        * codesΪ�����Ķ����б����������Ѿ����ĵĴ���,�����������б����codesΪ�գ����Է����Ѷ����б�
        *
        * @param codes
        * @return �����Ѿ����ĵĴ���
        */
        virtual CallResult<vector<string>> subscribe(const vector<string>& codes) = 0;

        /**
        * ȡ������
        *
        * codesΪ��Ҫȡ�����б��������л��ڶ��ĵĴ��롣
        * �����Ҫȡ�����ж��ģ���ͨ�� subscribe �õ������б�Ȼ��ʹ��unscribeȡ��

        * @param codes
        * @return
        */
        virtual CallResult<vector<string>> unsubscribe(const vector<string>& codes) = 0;

        /**
        * ������������Ļص�����
        *
        * �����ĵĴ����б������µ����飬��ͨ����callback֪ͨ�û���
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
        * ȡ���ݽӿ�
        *
        * @return
        */
        virtual TradeApi* trade_api() = 0;

        /**
        *  ȡ���׽ӿ�
        *
        * @return
        */
        virtual DataApi*  data_api() = 0;

        static TQuantApi* create(const char* addr);
    };

} }

#endif
