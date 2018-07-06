#ifndef _TQAPI_CS_H
#define _TQAPI_CS_H

#include "myutils/unicode.h"

using namespace tquant::api;

static inline const char* _T(string& str)
{
    str = utf8_to_gbk(str);
    return str.c_str();
}

#pragma pack(1)
struct AccountInfoWrap {
    const char* account_id;       // 帐号编号
    const char* broker;           // 交易商名称，如招商证券
    const char* account;          // 交易帐号
    const char* status;           // 连接状态，取值 Disconnected, Connected, Connecting
    const char* msg;              // 状态信息，如登录失败原因
    const char* account_type;     // 帐号类型，如 stock, ctp

    AccountInfo m_orig;

    AccountInfoWrap(const AccountInfo& orig) : m_orig(orig)
    {
        account_id   = _T( m_orig.account_id   );
        broker       = _T( m_orig.broker       );
        account      = _T( m_orig.account      );
        status       = _T( m_orig.status       );
        msg          = _T( m_orig.msg          );
        account_type = _T( m_orig.account_type );
    }
};

struct BalanceWrap {
    const char* account_id;       // 帐号编号
    const char* fund_account;     // 资金帐号
    double      init_balance;     // 初始化资金
    double      enable_balance;   // 可用资金
    double      margin;           // 保证金
    double      float_pnl;        // 浮动盈亏
    double      close_pnl;        // 实现盈亏

    Balance m_orig;

    BalanceWrap(const Balance& bal) : m_orig(bal)
    {
        assign();
    }
    BalanceWrap(const BalanceWrap& rhs) : m_orig(rhs.m_orig)
    {
        assign();
    }

    void assign()
    {
        account_id      = _T( m_orig.account_id   );
        fund_account    = _T( m_orig.fund_account );
        init_balance    = m_orig.init_balance      ;
        enable_balance  = m_orig.enable_balance    ;
        margin          = m_orig.margin            ;
        float_pnl       = m_orig.float_pnl         ;
        close_pnl       = m_orig.close_pnl         ;
    }
};

struct OrderWrap {
    const char* account_id;       // 帐号编号
    const char* code;             // 证券代码
    const char* name;             // 证券名称
    const char* entrust_no;       // 委托编号
    const char* entrust_action;   // 委托动作
    double      entrust_price;    // 委托价格
    int64_t     entrust_size;     // 委托数量，单位：股
    int32_t     entrust_date;     // 委托日期
    int32_t     entrust_time;     // 委托时间
    double      fill_price;       // 成交价格
    int64_t     fill_size;        // 成交数量
    const char* status;           // 订单状态：取值: OrderStatus
    const char* status_msg;       // 状态消息
    int32_t     order_id;         // 自定义订单编号

    Order m_orig;

    OrderWrap(const Order& order) : m_orig(order)
    {
        assign();
    }

    OrderWrap(const OrderWrap& rhs) : m_orig(rhs.m_orig)
    {
        assign();
    }

    void assign()
    {
        account_id     = _T( m_orig.account_id     );
        code           = _T( m_orig.code           );
        name           = _T( m_orig.name           );
        entrust_no     = _T( m_orig.entrust_no     );
        entrust_action = _T( m_orig.entrust_action );
        entrust_price  = m_orig.entrust_price         ;
        entrust_size   = m_orig.entrust_size          ;
        entrust_date   = m_orig.entrust_date          ;
        entrust_time   = m_orig.entrust_time          ;
        fill_price     = m_orig.fill_price            ;
        fill_size      = m_orig.fill_size             ;
        status         = _T( m_orig.status      )     ;
        status_msg     = _T( m_orig.status_msg  )     ;
        order_id       = m_orig.order_id              ;
    }
};

struct TradeWrap {
    const char* account_id;       // 帐号编号
    const char* code;             // 证券代码
    const char* name;             // 证券名称
    const char* entrust_no;       // 委托编号
    const char* entrust_action;   // 委托动作
    const char* fill_no;          // 成交编号
    int64_t     fill_size;        // 成交数量
    double      fill_price;       // 成交价格
    int32_t     fill_date;        // 成交日期
    int32_t     fill_time;        // 成交时间
    int32_t     order_id;         // 订单编号

    Trade m_orig;

    TradeWrap(const Trade& trade) : m_orig(trade)
    {
        assign();
    }

    TradeWrap(const TradeWrap& rhs) : m_orig(rhs.m_orig)
    {
        assign();
    }

    void assign()
    {
        account_id     = _T( m_orig.account_id     );
        code           = _T( m_orig.code           );
        name           = _T( m_orig.name           );
        entrust_no     = _T( m_orig.entrust_no     );
        entrust_action = _T( m_orig.entrust_action );
        fill_no        = _T( m_orig.fill_no        );
        fill_size      = m_orig.fill_size           ;
        fill_price     = m_orig.fill_price          ;
        fill_date      = m_orig.fill_date           ;
        fill_time      = m_orig.fill_time           ;
        order_id       = m_orig.order_id            ;
    }
};

struct PositionWrap {
    const char*   account_id;       // 帐号编号
    const char*   code;             // 证券代码
    const char*   name;             // 证券名称
    int64_t       current_size;     // 当前持仓
    int64_t       enable_size;      // 可用（可交易）持仓
    int64_t       init_size;        // 初始持仓
    int64_t       today_size;       // 今日持仓
    int64_t       frozen_size;      // 冻结持仓
    const char*   side;             // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
    double        cost;             // 成本
    double        cost_price;       // 成本价格
    double        last_price;       // 最新价格
    double        float_pnl;        // 持仓盈亏
    double        close_pnl;        // 平仓盈亏
    double        margin;           // 保证金
    double        commission;       // 手续费

    shared_ptr<Position> m_orig;

    PositionWrap(const Position& orig) :  m_orig(make_shared<Position>(orig))
    {
        assign();
    }

    PositionWrap(const PositionWrap& rhs) : m_orig(rhs.m_orig)
    {
        assign();
    }

    void assign()
    {
        account_id    = _T( m_orig->account_id );
        code          = _T( m_orig->code       );
        name          = _T( m_orig->name       );
        current_size  = m_orig->current_size         ;
        enable_size   = m_orig->enable_size          ;
        init_size     = m_orig->init_size            ;
        today_size    = m_orig->today_size           ;
        frozen_size   = m_orig->frozen_size          ;
        side          = _T( m_orig->side       );
        cost          = m_orig->cost                 ;
        cost_price    = m_orig->cost_price           ;
        last_price    = m_orig->last_price           ;
        float_pnl     = m_orig->float_pnl            ;
        close_pnl     = m_orig->close_pnl            ;
        margin        = m_orig->margin               ;
        commission    = m_orig->commission           ;
    }
};

struct OrderIDWrap {
    const char*  entrust_no;       // 订单委托号
    int32_t      order_id;         // 自定义编号
    
    shared_ptr<OrderID> m_orig;

    OrderIDWrap(const OrderID& orig): m_orig(make_shared<OrderID>(orig))
    {
        assign();
    }

    OrderIDWrap(const OrderIDWrap& rhs) : m_orig(rhs.m_orig)
    {
        assign();
    }

    void assign()
    {
        entrust_no = _T(m_orig->entrust_no);
        order_id   = m_orig->order_id;
    }
};

struct CallResultWrap {
    const char* msg;
    const void* value;
    int32_t     element_size;
    int32_t     element_count;
    int32_t     value_type;

    CallResultWrap() :
        msg(nullptr), value(nullptr), element_size(0),
        element_count(0), value_type(0)
    {}

    string  _msg;
    string text;
    shared_ptr<const BarArray>          bars;
    shared_ptr<const DailyBarArray>     daily_bars;
    shared_ptr<const MarketQuoteArray>  quotes;
    shared_ptr<const MarketQuote>       quote;
    shared_ptr<vector<PositionWrap>>    positions;
    shared_ptr<vector<OrderWrap>>       orders;
    shared_ptr<vector<TradeWrap>>       trades;
    shared_ptr<vector<AccountInfoWrap>> account_infos;
    shared_ptr<OrderIDWrap>             order_id;
    shared_ptr<BalanceWrap>             balance;
    shared_ptr<bool>                    bool_value;
};

enum CallResultValueType {
    VT_BAR_ARRAY = 1,
    VT_QUOTE_ARRAY,
    VT_DAILYBAR_ARRAY,
    VT_QUOTE,
    VT_STRING,
    VT_ACCOUNT_INFO_ARRAY,
    VT_POSITION_ARRAY,
    VT_ORDER_ARRAY,
    VT_TRADE_ARRAY,
    VT_ORDER_ID,
    VT_BOOL,
    VT_BALANCE
};

#pragma pack()

#endif
