#ifndef _TQUANT_API_H
#define _TQUANT_API_H

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <exception>
#include <cstring>
#include <unordered_set>

#ifdef _WIN32
#  ifdef _TQAPI_DLL
#    define _TQAPI_EXPORT __declspec(dllimport)
#  elif defined(_TQAPI_DLL_IMPL)
#    define _TQAPI_EXPORT __declspec(dllexport)
#  else
#    define _TQAPI_EXPORT
#  endif
#else
#  define _TQAPI_EXPORT
#endif

namespace tquant {  namespace api {

    using namespace std;

    template<typename T>
    class TickDataHolder : public T {
        string _code;
    public:
        TickDataHolder() {
            std::memset(this, 0, sizeof(T));
        }

        TickDataHolder(const T& t, const string& a_code) : T(t), _code(a_code) {
            this->code = _code.c_str();
        }

        TickDataHolder(const T& t) : T(t), _code(t.code) {
            this->code = _code.c_str();
        }

        TickDataHolder(const TickDataHolder<T>& t) {
            *this = t;
            if (t.code){
                this->_code = t.code;
                this->code = this->_code.c_str();
            }
        }

        void assign(const T& t, const char* code = nullptr) {
            *(T*)this = t;
            if (code) {
                _code = code;
                this->code = _code.c_str();
            }
        }

        void set_code(const string& a_code) {
            _code = a_code;
            this->code = _code.c_str();
        }
    };

    struct TickArray {
        TickArray(size_t type_size, size_t max_size)
            : _data(nullptr)
            , _type_size((int)type_size)
            , _size(0)
        {
            if (max_size)
                _data = new uint8_t[type_size* max_size];
        }

        TickArray()
            : _data(nullptr)
            , _type_size(0)
        {}

        ~TickArray() {
            if (_data)
                delete[] _data;
        }

        const uint8_t*  data() const        { return _data; }
        size_t          type_size() const   { return _type_size; }
        size_t          size() const        { return _size; }
        const string&   code() const        { return _code; }

        void set_code(const string& code)   { _code = code; }

        // Be careful!
        void set_size(int size) { _size = size; }

        void assign(const char* code, uint8_t* data, int type_size, int size) {
            if (this->_data) delete[] this->_data;
            this->_data      = data;
            this->_type_size = type_size;
            this->_size      = size;
            this->_code      = code;

            uint8_t* p = this->_data;
            for (int i = 0; i < _size; i++) {
                *((const char**)p) = _code.c_str();
                p += _type_size;
            }
        }
    //protected:
        uint8_t*    _data;
        int         _type_size;
        int         _size;
        string      _code;
    };

    template <class T>
    struct _TickArray : public TickArray {

        _TickArray(const string& code, size_t max_size)
            : TickArray(sizeof(T), max_size)
        {
            set_code(code);
        }

        T& operator[] (size_t i) const {
            if (i < this->_size)
                return *reinterpret_cast<T*>(_data + _type_size*i);
            else
                throw std::runtime_error("wrong index");
        }
        T&  at(size_t i) const {
            return *reinterpret_cast<T*>(_data + _type_size*i);
        }

        void push_back(const T& t) {
            auto t2 = reinterpret_cast<T*>(_data + _type_size * _size);
            *t2 = t;
            t2->code = _code.c_str();
            _size++;
        }
    };

#pragma pack(1)
    // keep same with tk_schema!
    struct RawMarketQuote{
        const char*     code;
#if defined(WIN32) && !defined(_WIN64)
        int32_t         _padding_1;
#endif
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
#if defined(WIN32) && !defined(_WIN64)
        int32_t         _padding_1;
#endif
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
#if defined(WIN32) && !defined(_WIN64)
        int32_t         _padding_1;
#endif
        int32_t         date;
        double          open;
        double          high;
        double          low;
        double          close;
        int64_t         volume;
        double          turnover;
        int64_t         oi;
        double          settle;
        double          pre_close;
        double          pre_settle;
        double          af;
    };

    typedef TickDataHolder<RawDailyBar> DailyBar;

#pragma pack()

    typedef _TickArray<RawMarketQuote> MarketQuoteArray;
    typedef _TickArray<RawBar>         BarArray;
    typedef _TickArray<RawDailyBar>    DailyBarArray;


    /**
    *  ���ݲ�ѯ�ӿ�
    *
    *  ���ܣ�
    *      ��ʵʱ���飬�����tick, ������
    *      ���ĺ���������
    */
    class DataApi_Callback {
    public:
        virtual ~DataApi_Callback()  { }
        virtual void on_market_quote (shared_ptr<const MarketQuote> quote) = 0;
        virtual void on_bar          (const string& cycle, shared_ptr<const Bar> bar) = 0;
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
        CallResult()
        {
        }
    };

    class DataApi {
    public:
        DataApi() {}

        virtual ~DataApi() {}

        /**
        * ȡĳ�����յ�ĳ������� ticks
        *
        * ��tradingdayΪ0����ʾ��ǰ������
        *
        * @param code
        * @param trading_day
        * @return
        */
        virtual CallResult<const MarketQuoteArray> tick(const string& code, int trading_day) = 0;

        /**
        * ȡĳ�������Bar
        *
        * Ŀǰֻ֧�ַ�����
        *  �� cycle == "1m"ʱ������trading_day�ķ����ߣ�trading_day=0��ʾ��ǰ�����ա�
        *
        * @param code          ֤ȯ����
        * @param cycle         "1m""
        * @param trading_day   ������
        * @param align         �Ƿ����
        * @return
        */
        virtual CallResult<const BarArray> bar(const string& code, const string& cycle, int trading_day, bool align) = 0;

        /**
        * ȡĳ�����������
        *
        *
        * @param code          ֤ȯ����
        * @param price_adj     �۸�Ȩ��ȡֵ
        *                        back -- ��Ȩ
        *                        forward -- ǰ��Ȩ
        * @param align         �Ƿ����
        * @return
        */
        virtual CallResult<const DailyBarArray> daily_bar(const string& code, const string& price_adj, bool align) = 0;

        /**
        * ȡ��ǰ���������
        *
        * @param code
        * @return
        */
        virtual CallResult<const MarketQuote> quote(const string& code) = 0;

        /**
        * ��������
        *
        * codesΪ�����Ķ����б������������Ѿ����ĵĴ���,�����������б������codesΪ�գ����Է����Ѷ����б���
        *
        * @param codes
        * @return �����Ѿ����ĵĴ���
        */
        virtual CallResult<const vector<string>> subscribe(const vector<string>& codes) = 0;

        /**
        * ȡ������
        *
        * codesΪ��Ҫȡ�����б����������л��ڶ��ĵĴ��롣
        * �����Ҫȡ�����ж��ģ���ͨ�� subscribe �õ������б���Ȼ��ʹ��unscribeȡ��

        * @param codes
        * @return
        */
        virtual CallResult<const vector<string>> unsubscribe(const vector<string>& codes) = 0;

        /**
        * ������������Ļص�����
        *
        * �����ĵĴ����б������µ����飬��ͨ����callback֪ͨ�û���
        *
        * @param callback
        */
        virtual DataApi_Callback* set_callback(DataApi_Callback* callback) = 0;
    };


    // TradeApi

    struct AccountInfo {
        string account_id;       // �ʺű��
        string broker;           // ���������ƣ�������֤ȯ
        string account;          // �����ʺ�
        string status;           // ����״̬��ȡֵ Disconnected, Connected, Connecting
        string msg;              // ״̬��Ϣ�����¼ʧ��ԭ��
        string account_type;     // �ʺ����ͣ��� stock, ctp
    };

    struct Balance {
        string account_id;       // �ʺű��
        string fund_account;     // �ʽ��ʺ�
        double init_balance;     // ��ʼ���ʽ�
        double enable_balance;   // �����ʽ�
        double margin;           // ��֤��
        double float_pnl;        // ����ӯ��
        double close_pnl;        // ʵ��ӯ��

        Balance() : init_balance(0.0), enable_balance(0.0), margin(0.0)
            , float_pnl(0.0), close_pnl(0.0)
        {}
    };

    //struct OrderStatus {
#define OS_New        "New"
#define OS_Accepted   "Accepted"
#define OS_Filled     "Filled"
#define OS_Rejected   "Rejected"
#define OS_Cancelled  "Cancelled"
    //}

    //class EntrustAction {
#define EA_Buy             "Buy"
#define EA_Short           "Short"
#define EA_Cover           "Cover"
#define EA_Sell            "Sell"
#define EA_CoverToday      "CoverToday"
#define EA_CoverYesterday  "CoverYesterday"
#define EA_SellToday       "SellToday"
#define EA_SellYesterday   "SellYesterday"
    //}

    struct Order {
        string  account_id;       // �ʺű��
        string  code;             // ֤ȯ����
        string  name;             // ֤ȯ����
        string  entrust_no;       // ί�б��
        string  entrust_action;   // ί�ж���
        double  entrust_price;    // ί�м۸�
        int64_t entrust_size;     // ί����������λ����
        int32_t entrust_date;     // ί������
        int32_t entrust_time;     // ί��ʱ��
        double  fill_price;       // �ɽ��۸�
        int64_t fill_size;        // �ɽ�����
        string  status;           // ����״̬��ȡֵ: OrderStatus
        string  status_msg;       // ״̬��Ϣ
        int32_t order_id;         // �Զ��嶩�����

        Order()
            : entrust_price(0.0), entrust_size(0), entrust_date(0), entrust_time(0)
            , fill_price(0.0), fill_size(0), order_id(0)
        {}
    };

    struct Trade {
        string  account_id;       // �ʺű��
        string  code;             // ֤ȯ����
        string  name;             // ֤ȯ����
        string  entrust_no;       // ί�б��
        string  entrust_action;   // ί�ж���
        string  fill_no;          // �ɽ����
        int64_t fill_size;        // �ɽ�����
        double  fill_price;       // �ɽ��۸�
        int32_t fill_date;        // �ɽ�����
        int32_t fill_time;        // �ɽ�ʱ��
        int32_t order_id;         // �Զ��嶩�����

        Trade() : fill_size(0), fill_price(0.0), fill_date(0), fill_time(0), order_id(0)
        {}
    };

    // Side {
#define SD_Long "Long"
#define SD_Short "Short"
    //}

    struct Position {
        string  account_id;       // �ʺű��
        string  code;             // ֤ȯ����
        string  name;             // ֤ȯ����
        int64_t current_size;     // ��ǰ�ֲ�
        int64_t enable_size;      // ���ã��ɽ��ף��ֲ�
        int64_t init_size;        // ��ʼ�ֲ�
        int64_t today_size;       // ���ճֲ�
        int64_t frozen_size;      // ����ֲ�
        string  side;             // �ֲַ��򣬹�Ʊ�ĳֲַ���Ϊ Long, �ڻ��� Long, Short
        double  cost;             // �ɱ�
        double  cost_price;       // �ɱ��۸�
        double  last_price;       // ���¼۸�
        double  float_pnl;        // �ֲ�ӯ��
        double  close_pnl;        // ƽ��ӯ��
        double  margin;           // ��֤��
        double  commission;       // ������

        Position()
            : current_size(0), enable_size(0), init_size(0), today_size(0), frozen_size(0)
            , cost(0.0), cost_price(0.0), last_price(0.0), float_pnl(0.0), close_pnl(0.0)
            , margin(0.0), commission(0.0)
        {
        }
    };

    struct OrderID {
        string  entrust_no;       // ����ί�к�
        int32_t order_id;         // �Զ�����
    };


    class TradeApi_Callback{
    public:
        virtual ~TradeApi_Callback() { }
        virtual void on_order_status  (shared_ptr<Order> order) = 0;
        virtual void on_order_trade   (shared_ptr<Trade> trade) = 0;
        virtual void on_account_status(shared_ptr<AccountInfo> account) = 0;
    };

    class TradeApi {
    public:
        TradeApi() { }

        virtual ~TradeApi() {}

        /**
        * ��ѯ�ʺ�����״̬��
        *
        * @return
        */
        virtual CallResult<const vector<AccountInfo>> query_account_status() = 0;

        /**
        * ��ѯĳ���ʺŵ��ʽ�ʹ�����
        *
        * @param account_id
        * @return
        */
        virtual CallResult<const Balance> query_balance(const string& account_id) = 0;

        /**
        * ��ѯĳ���ʺŵĵ���Ķ���
        *
        * @param account_id
        * @return
        */
        virtual CallResult<const vector<Order>> query_orders(const string& account_id, const unordered_set<string>* codes = nullptr) = 0;
        virtual CallResult<const vector<Order>> query_orders(const string& account_id, const string& codes) = 0;

        /**
        * ��ѯĳ���ʺŵĵ���ĳɽ�
        *
        * @param account_id
        * @return
        */
        virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const unordered_set<string>* codes = nullptr) = 0;
        virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const string& codes) = 0;

        /**
        * ��ѯĳ���ʺŵĵ���ĳֲ�
        *
        * @param account_id
        * @return
        */
        virtual CallResult<const vector<Position>> query_positions(const string& account_id, const unordered_set<string>* codes = nullptr) = 0;
        virtual CallResult<const vector<Position>> query_positions(const string& account_id, const string& codes) = 0;

        /**
        * �µ�
        *
        * ��Ʊͨ��Ϊͬ���µ�ģʽ���������µ��ɹ����뷵��ί�к� entrust_no��
        *
        * CTP����ͨ��Ϊ�첽�µ�ģʽ���µ������������Զ�����order_id�������������ܶ���������ί�кźã�ͨ�� Callback.on_order_status֪ͨ
        * �û����û�����ͨ��order_idƥ�䡣�������û�б����գ�on_order_status�ص�������entrust_noΪ�գ�״̬ΪRejected��
        * ������order_id��Ϊ0����ʾ�û��Լ��Զ�����ţ���ʱ�û����뱣֤��ŵ�Ψһ�ԡ��������ͨ����֧��order_id���ú������ش�����롣
        *
        * @param account_id    �ʺű��
        * @param code          ֤ȯ����
        * @param price         ί�м۸�
        * @param size          ί������
        * @param action        ί�ж���
        * @param price_type    �۸����ͣ�ȱʡΪ�޼۵�(LimitPrice)
        * @param order_id      �Զ��嶩����ţ���Ϊ0��ʾ��ֵ
        * @return OrderID      ����ID
        */
        virtual CallResult<const OrderID> place_order(const string& account_id, const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id) = 0;

        /**
        * ���ݶ����ų���
        *
        * security ����Ϊ��
        *
        * @param account_id    �ʺű��
        * @param code          ֤ȯ����
        * @param order_id      ������
        * @return �Ƿ�ɹ�
        */
        virtual CallResult<bool> cancel_order(const string& account_id, const string& code, int order_id) = 0;

        /**
        * ����ί�кų���
        *
        * security ����Ϊ��
        *
        * @param account_id    �ʺű��
        * @param code          ֤ȯ����
        * @param entrust_no    ί�б��
        * @return �Ƿ�ɹ�
        */
        virtual CallResult<bool> cancel_order(const string& account_id, const string& code, const string& entrust_no) = 0;

        /**
        * ͨ�ò�ѯ�ӿ�
        *
        * ���ڲ�ѯ����ͨ�����е���Ϣ�����ѯ CTP�Ĵ���� command="ctp_codetable".
        * �����ַ�����
        *
        * @param account_id
        * @param command
        * @param params
        * @return
        */
        virtual CallResult<string> query(const string& account_id, const string& command, const string& params) = 0;

        /**
        * ���� TradeApi.Callback
        *
        * @param callback
        */
        virtual TradeApi_Callback* set_callback(TradeApi_Callback* callback) = 0;
    };

    _TQAPI_EXPORT DataApi*  create_data_api (const string& addr);

    _TQAPI_EXPORT TradeApi* create_trade_api(const string& addr);

    _TQAPI_EXPORT void set_params(const string& key, const string& value);

    typedef DataApi*  (*T_create_data_api)(const char* str_params);
    typedef TradeApi* (*T_create_trade_api)(const char* str_params);


    _TQAPI_EXPORT void register_trade_api_factory(T_create_trade_api factory);

} }

#endif
