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
    const char* account_id;       // �ʺű��
    const char* broker;           // ���������ƣ�������֤ȯ
    const char* account;          // �����ʺ�
    const char* status;           // ����״̬��ȡֵ Disconnected, Connected, Connecting
    const char* msg;              // ״̬��Ϣ�����¼ʧ��ԭ��
    const char* account_type;     // �ʺ����ͣ��� stock, ctp

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
    const char* account_id;       // �ʺű��
    const char* fund_account;     // �ʽ��ʺ�
    double      init_balance;     // ��ʼ���ʽ�
    double      enable_balance;   // �����ʽ�
    double      margin;           // ��֤��
    double      float_pnl;        // ����ӯ��
    double      close_pnl;        // ʵ��ӯ��

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
    const char* account_id;       // �ʺű��
    const char* code;             // ֤ȯ����
    const char* name;             // ֤ȯ����
    const char* entrust_no;       // ί�б��
    const char* entrust_action;   // ί�ж���
    double      entrust_price;    // ί�м۸�
    int64_t     entrust_size;     // ί����������λ����
    int32_t     entrust_date;     // ί������
    int32_t     entrust_time;     // ί��ʱ��
    double      fill_price;       // �ɽ��۸�
    int64_t     fill_size;        // �ɽ�����
    const char* status;           // ����״̬��ȡֵ: OrderStatus
    const char* status_msg;       // ״̬��Ϣ
    int32_t     order_id;         // �Զ��嶩�����

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
    const char* account_id;       // �ʺű��
    const char* code;             // ֤ȯ����
    const char* name;             // ֤ȯ����
    const char* entrust_no;       // ί�б��
    const char* entrust_action;   // ί�ж���
    const char* fill_no;          // �ɽ����
    int64_t     fill_size;        // �ɽ�����
    double      fill_price;       // �ɽ��۸�
    int32_t     fill_date;        // �ɽ�����
    int32_t     fill_time;        // �ɽ�ʱ��
    int32_t     order_id;         // �������

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
    const char*   account_id;       // �ʺű��
    const char*   code;             // ֤ȯ����
    const char*   name;             // ֤ȯ����
    int64_t       current_size;     // ��ǰ�ֲ�
    int64_t       enable_size;      // ���ã��ɽ��ף��ֲ�
    int64_t       init_size;        // ��ʼ�ֲ�
    int64_t       today_size;       // ���ճֲ�
    int64_t       frozen_size;      // ����ֲ�
    const char*   side;             // �ֲַ��򣬹�Ʊ�ĳֲַ���Ϊ Long, �ڻ��� Long, Short
    double        cost;             // �ɱ�
    double        cost_price;       // �ɱ��۸�
    double        last_price;       // ���¼۸�
    double        float_pnl;        // �ֲ�ӯ��
    double        close_pnl;        // ƽ��ӯ��
    double        margin;           // ��֤��
    double        commission;       // ������

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
    const char*  entrust_no;       // ����ί�к�
    int32_t      order_id;         // �Զ�����
    
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
