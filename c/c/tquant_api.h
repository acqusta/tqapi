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

    // TradeApi
    struct AccountInfo {
        string account_id;       // 帐号编号
        string broker;           // 交易商名称，如招商证券
        string account;          // 交易帐号
        string status;           // 连接状态，取值 Disconnected, Connected, Connecting
        string msg;              // 状态信息，如登录失败原因
        string account_type;     // 帐号类型，如 stock, ctp
    };

    struct Balance {
        string account_id;       // 帐号编号
        string fund_account;     // 资金帐号
        double init_balance;     // 初始化资金
        double enable_balance;   // 可用资金
        double margin;           // 保证金
        double float_pnl;        // 浮动盈亏
        double close_pnl;        // 实现盈亏

        Balance() : init_balance(0.0), enable_balance(0.0), margin(0.0)
            , float_pnl(0.0), close_pnl(0.0)
        {}            
    };

    //struct OrderStatus {
        static const char* OS_New       = "New";
        static const char* OS_Accepted  = "Accepted";
        static const char* OS_Filled    = "Filled";
        static const char* OS_Rejected  = "Rejected";
        static const char* OS_Cancelled = "Cancelled";
    //}

    //class EntrustAction {
        static const char* EA_Buy            = "Buy";
        static const char* EA_Short          = "Sell";
        static const char* EA_Cover          = "Cover";
        static const char* EA_Sell           = "Sell";
        static const char* EA_CoverToday     = "CoverToday";
        static const char* EA_CoverYesterday = "CoverYesterday";
        static const char* EA_SellToday      = "SellToday";
        static const char* EA_SellYesterday  = "SellYesterday";
    //}

    struct Order {
        string  account_id;       // 帐号编号
        string  code;             // 证券代码
        string  name;             // 证券名称
        string  entrust_no;       // 委托编号
        string  entrust_action;   // 委托动作
        double  entrust_price;    // 委托价格
        int64_t entrust_size;     // 委托数量，单位：股
        int32_t entrust_date;     // 委托日期
        int32_t entrust_time;     // 委托时间
        double  fill_price;       // 成交价格
        int64_t fill_size;        // 成交数量
        string  status;           // 订单状态：取值: OrderStatus
        string  status_msg;       // 状态消息
        int32_t order_id;         // 自定义订单编号

        Order() 
            : entrust_price(0.0), entrust_size(0), entrust_date(0), entrust_time(0)
            , fill_price(0.0), fill_size(0), order_id(0)
        {}
    };

    struct Trade {
        string  account_id;       // 帐号编号
        string  code;             // 证券代码
        string  name;             // 证券名称
        string  entrust_no;       // 委托编号
        string  entrust_action;   // 委托动作
        string  fill_no;          // 成交编号
        int64_t fill_size;        // 成交数量
        double  fill_price;       // 成交价格
        int32_t fill_date;        // 成交日期
        int32_t fill_time;        // 成交时间

        Trade() : fill_size(0), fill_price(0.0), fill_date(0), fill_time(0)
        {}
    };

    // Side {
    static const char* SD_Long  = "Long";
    static const char* SD_Short = "Short";
    //}

    struct Position {
        string  account_id;       // 帐号编号
        string  code;             // 证券代码
        string  name;             // 证券名称
        int64_t current_size;     // 当前持仓
        int64_t enable_size;      // 可用（可交易）持仓
        int64_t init_size;        // 初始持仓
        int64_t today_size;       // 今日持仓
        int64_t frozen_size;      // 冻结持仓
        string  side;             // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
        double  cost;             // 成本
        double  cost_price;       // 成本价格
        double  last_price;       // 最新价格
        double  float_pnl;        // 持仓盈亏
        double  close_pnl;        // 平仓盈亏
        double  margin;           // 保证金
        double  commission;       // 手续费

        Position()
            : current_size(0), enable_size(0), init_size(0), today_size(0), frozen_size(0)
            , cost(0.0), cost_price(0.0), last_price(0.0), float_pnl(0.0), close_pnl(0.0)
            , margin(0.0), commission(0.0)
        {
        }
    };

    struct OrderID {
        string entrust_no;       // 订单委托号
        string order_id;         // 自定义编号
    };

    class TradeApi_Callback{
    public:
        virtual void onOrderStatus  (shared_ptr<Order> order) = 0;
        virtual void onOrderTrade   (shared_ptr<Trade> trade) = 0;
        virtual void onAccountStatus(shared_ptr<AccountInfo> account) = 0;
    };

    class TradeApi {
    protected:
        virtual ~TradeApi() {}
    public:
        TradeApi() { }

        /**
        * 查询帐号连接状态。
        *
        * @return
        */
        virtual CallResult<vector<AccountInfo>> query_account_status() = 0;

        /**
        * 查询某个帐号的资金使用情况
        *
        * @param account_id
        * @return
        */
        virtual CallResult<Balance> query_balance(const char* account_id) = 0;

        /**
        * 查询某个帐号的当天的订单
        *
        * @param account_id
        * @return
        */
        virtual CallResult<vector<Order>> query_orders(const char* account_id) = 0;

        /**
        * 查询某个帐号的当天的成交
        *
        * @param account_id
        * @return
        */
        virtual CallResult<vector<Trade>> query_trades(const char* account_id) = 0;

        /**
        * 查询某个帐号的当天的持仓
        *
        * @param account_id
        * @return
        */
        virtual CallResult<vector<Position>> query_positions(const char* account_id) = 0;

        /**
        * 下单
        *
        * 股票通道为同步下单模式，即必须下单成功必须返回委托号 entrust_no。
        *
        * CTP交易通道为异步下单模式，下单后立即返回自定义编号order_id。当交易所接受订单，生成委托号好，通过 Callback.onOrderStatus通知
        * 用户。用户可以通过order_id匹配。如果订单没有被接收，onOrderStatus回调函数中entrust_no为空，状态为Rejected。
        * 当参数order_id不为0，表示用户自己对订单编号，这时用户必须保证编号的唯一性。如果交易通道不支持order_id，该函数返回错误代码。
        *
        * @param account_id    帐号编号
        * @param code          证券代码
        * @param price         委托价格
        * @param size          委托数量
        * @param action        委托动作
        * @param order_id      自定义订单编号，不为0表示有值
        * @return OrderID      订单ID
        */
        virtual CallResult<OrderID> place_order(const char* account_id, const char* code, double price, long size, const char* action, int order_id) = 0;

        /**
        * 根据订单号撤单
        *
        * security 不能为空
        *
        * @param account_id    帐号编号
        * @param code          证券代码
        * @param order_id      订单号
        * @return 是否成功
        */
        virtual CallResult<bool> cancel_order(const char* account_id, const char* code, int order_id) = 0;

        /**
        * 根据委托号撤单
        *
        * security 不能为空
        *
        * @param account_id    帐号编号
        * @param code          证券代码
        * @param entrust_no    委托编号
        * @return 是否成功
        */
        virtual CallResult<bool> cancel_order(const char* account_id, const char* code, const char* entrust_no) = 0;

        /**
        * 通用查询接口
        *
        * 用于查询交易通道特有的信息。如查询 CTP的代码表 command="ctp_codetable".
        * 返回字符串。
        *
        * @param account_id
        * @param command
        * @param params
        * @return
        */
        virtual CallResult<string> query(const char* account_id, const char* command, const char* params) = 0;
        /**
        * 设置 TradeApi.Callback
        *
        * @param callback
        */
        virtual void set_callback(TradeApi_Callback* callback) = 0;
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
