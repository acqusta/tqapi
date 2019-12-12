#ifndef _TQAPI_CS_H
#define _TQAPI_CS_H

#include "myutils/unicode.h"

using namespace tquant::api;

static inline const char* _T(const string& src, string& dst)
{
    utf8_to_utf16(src, &dst);
    return dst.c_str();
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
        account_id   = _T( m_orig.account_id   , m_orig.account_id  );
        broker       = _T( m_orig.broker       , m_orig.broker      );
        account      = _T( m_orig.account      , m_orig.account     );
        status       = _T( m_orig.status       , m_orig.status      );
        msg          = _T( m_orig.msg          , m_orig.msg         );
        account_type = _T( m_orig.account_type , m_orig.account_type);
    }

	AccountInfoWrap(const AccountInfoWrap& rhs)
	{
		m_orig = rhs.m_orig;
        account_id   = m_orig.account_id  .c_str();
        broker       = m_orig.broker      .c_str();
        account      = m_orig.account     .c_str();
        status       = m_orig.status      .c_str();
        msg          = m_orig.msg         .c_str();
        account_type = m_orig.account_type.c_str();
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
        account_id      = _T( m_orig.account_id  , m_orig.account_id);
        fund_account    = _T( m_orig.fund_account, m_orig.account_id);
        init_balance    = m_orig.init_balance      ;
        enable_balance  = m_orig.enable_balance    ;
        margin          = m_orig.margin            ;
        float_pnl       = m_orig.float_pnl         ;
        close_pnl       = m_orig.close_pnl         ;
    }

    BalanceWrap(const BalanceWrap& rhs) : m_orig(rhs.m_orig)
    {
        account_id      = m_orig.account_id   .c_str();
        fund_account    = m_orig.fund_account .c_str();
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

	string _account_id    ;
	string _code          ;
	string _name          ;
	string _entrust_no    ;
	string _entrust_action;
	string _status        ;
	string _status_msg    ;

    OrderWrap(const Order* order)
    {
        account_id     = _T( order->account_id     , _account_id    );
        code           = _T( order->code           , _code          );
        name           = _T( order->name           , _name          );
        entrust_no     = _T( order->entrust_no     , _entrust_no    );
        entrust_action = _T( order->entrust_action , _entrust_action);
        entrust_price  = order->entrust_price         ;
        entrust_size   = order->entrust_size          ;
        entrust_date   = order->entrust_date          ;
        entrust_time   = order->entrust_time          ;
        fill_price     = order->fill_price            ;
        fill_size      = order->fill_size             ;
        status         = _T( order->status      , _status     )     ;
        status_msg     = _T( order->status_msg  , _status_msg )     ;
        order_id       = order->order_id              ;
    }

	OrderWrap(const OrderWrap& rhs)
    {
		_account_id     = rhs._account_id     ;
		_code           = rhs._code           ;
		_name           = rhs._name           ;
		_entrust_no     = rhs._entrust_no     ;
		_entrust_action = rhs._entrust_action ;
		_status         = rhs._status         ;
		_status_msg     = rhs._status_msg     ;

        account_id     = _account_id    .c_str()  ;
        code           = _code          .c_str()  ;
        name           = _name          .c_str()  ;
        entrust_no     = _entrust_no    .c_str()  ;
        entrust_action = _entrust_action.c_str()  ;
        entrust_price  = rhs.entrust_price            ;
        entrust_size   = rhs.entrust_size             ;
        entrust_date   = rhs.entrust_date             ;
        entrust_time   = rhs.entrust_time             ;
        fill_price     = rhs.fill_price               ;
        fill_size      = rhs.fill_size                ;
        status         = _status      .c_str()    ;
        status_msg     = _status_msg  .c_str()    ;
        order_id       = rhs.order_id                 ;
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

	string _account_id     ;
	string _code		   ;
	string _name		   ;
	string _entrust_no	   ;
	string _entrust_action ;
	string _fill_no		   ;

    TradeWrap(const Trade* trade)
    {
        account_id     = _T( trade->account_id    , _account_id     );
        code           = _T( trade->code          , _code           );
        name           = _T( trade->name          , _name           );
        entrust_no     = _T( trade->entrust_no    , _entrust_no     );
        entrust_action = _T( trade->entrust_action, _entrust_action );
        fill_no        = _T( trade->fill_no       , _fill_no        );
        fill_size      = trade->fill_size           ;
        fill_price     = trade->fill_price          ;
        fill_date      = trade->fill_date           ;
        fill_time      = trade->fill_time           ;
        order_id       = trade->order_id            ;
    }

    TradeWrap(const TradeWrap& rhs)
    {
        _account_id     = rhs._account_id      ;
        _code           = rhs._code            ;
        _name           = rhs._name            ;
        _entrust_no     = rhs._entrust_no      ;
        _entrust_action = rhs._entrust_action  ;
        _fill_no        = rhs._fill_no         ;


		account_id     = _account_id     .c_str() ;
        code           = _code           .c_str() ;
        name           = _name           .c_str() ;
        entrust_no     = _entrust_no     .c_str() ;
        entrust_action = _entrust_action .c_str() ;
        fill_no        = _fill_no        .c_str() ;
        fill_size      = rhs.fill_size        ;
        fill_price     = rhs.fill_price       ;
        fill_date      = rhs.fill_date        ;
        fill_time      = rhs.fill_time        ;
        order_id       = rhs.order_id         ;
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

	string _account_id;
	string _code;
	string _name;
	string _side;

    PositionWrap(const Position* pos)
    {
        account_id    = _T( pos->account_id , _account_id);
        code          = _T( pos->code       , _code      );
        name          = _T( pos->name       , _name      );
        current_size  = pos->current_size         ;
        enable_size   = pos->enable_size          ;
        init_size     = pos->init_size            ;
        today_size    = pos->today_size           ;
        frozen_size   = pos->frozen_size          ;
        side          = _T( pos->side       , _side);
        cost          = pos->cost                 ;
        cost_price    = pos->cost_price           ;
        last_price    = pos->last_price           ;
        float_pnl     = pos->float_pnl            ;
        close_pnl     = pos->close_pnl            ;
        margin        = pos->margin               ;
        commission    = pos->commission           ;
    }

    PositionWrap(const PositionWrap& rhs)
    {
        _account_id   = rhs._account_id ;
        _code         = rhs._code       ;
        _name         = rhs._name       ;
		_side		  = rhs._side       ;

		account_id    = _account_id .c_str();
        code          = _code       .c_str();
        name          = _name       .c_str();
        current_size  = rhs.current_size         ;
        enable_size   = rhs.enable_size          ;
        init_size     = rhs.init_size            ;
        today_size    = rhs.today_size           ;
        frozen_size   = rhs.frozen_size          ;
        side          = _side.c_str();
        cost          = rhs.cost                 ;
        cost_price    = rhs.cost_price           ;
        last_price    = rhs.last_price           ;
        float_pnl     = rhs.float_pnl            ;
        close_pnl     = rhs.close_pnl            ;
        margin        = rhs.margin               ;
        commission    = rhs.commission           ;
    }
};

struct OrderIDWrap {
    const char*  entrust_no;       // 订单委托号
    int32_t      order_id;         // 自定义编号
    
	string _entrust_no;

    OrderIDWrap(const OrderID* orig)
    {
        entrust_no = _T(orig->entrust_no, _entrust_no);
        order_id   = orig->order_id;
    }

    OrderIDWrap(const OrderIDWrap& rhs)
    {
		_entrust_no = rhs._entrust_no;
		entrust_no  = _entrust_no.c_str();
        order_id    = rhs.order_id;
    }
};

template<class T_WRAP>
struct _WrapArray {
	vector<T_WRAP*> wraps;
	
	~_WrapArray() {
		for (auto& t : wraps) delete t;
	}
};

typedef _WrapArray<OrderWrap>    OrderWrapArray;
typedef _WrapArray<TradeWrap>    TradeWrapArray;
typedef _WrapArray<PositionWrap> PositionWrapArray;

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
    shared_ptr<PositionWrapArray>       positions;
    shared_ptr<OrderWrapArray>          orders;
    shared_ptr<TradeWrapArray>          trades;
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
