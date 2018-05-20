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
    const char* account_id;       // �ʺű��
    const char* broker;           // ���������ƣ�������֤ȯ
    const char* account;          // �����ʺ�
    const char* status;           // ����״̬��ȡֵ Disconnected, Connected, Connecting
    const char* msg;              // ״̬��Ϣ�����¼ʧ��ԭ��
    const char* account_type;     // �ʺ����ͣ��� stock, ctp

    AccountInfoWrap(const AccountInfo& orig)
    {
        m_orig = make_shared<AccountInfo>(orig);
        account_id   = _T( m_orig->account_id   );
        broker       = _T( m_orig->broker       );
        account      = _T( m_orig->account      );
        status       = _T( m_orig->status       );
        msg          = _T( m_orig->msg          );
        account_type = _T( m_orig->account_type );
    }

    shared_ptr<AccountInfo> m_orig;
};

struct BalanceWrap {
    const char* account_id;       // �ʺű��
    const char* fund_account;     // �ʽ��ʺ�
    double      init_balance;     // ��ʼ���ʽ�
    double      enable_balance;   // �����ʽ�
    double      margin;           // ��֤��
    double      float_pnl;        // ����ӯ��
    double      close_pnl;        // ʵ��ӯ��

    shared_ptr<Balance> m_orig;

    BalanceWrap(const Balance& orig)
    {
        m_orig = make_shared<Balance>(orig);
        account_id      = _T( m_orig->account_id   );
        fund_account    = _T( m_orig->fund_account );
        init_balance    = m_orig->init_balance         ;
        enable_balance  = m_orig->enable_balance       ;
        margin          = m_orig->margin               ;
        float_pnl       = m_orig->float_pnl            ;
        close_pnl       = m_orig->close_pnl            ;
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

    shared_ptr<Order> m_orig;

    OrderWrap(const Order& orig)
    {
        m_orig = make_shared<Order>(orig);
        account_id     = _T( m_orig->account_id     );
        code           = _T( m_orig->code           );
        name           = _T( m_orig->name           );
        entrust_no     = _T( m_orig->entrust_no     );
        entrust_action = _T( m_orig->entrust_action );
        entrust_price  = m_orig->entrust_price         ;
        entrust_size   = m_orig->entrust_size          ;
        entrust_date   = m_orig->entrust_date          ;
        entrust_time   = m_orig->entrust_time          ;
        fill_price     = m_orig->fill_price            ;
        fill_size      = m_orig->fill_size             ;
        status         = _T( m_orig->status      );
        status_msg     = _T( m_orig->status_msg  );
        order_id       = m_orig->order_id              ;
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

    shared_ptr<Trade> m_orig;

    TradeWrap(const Trade& orig)
    {
        m_orig = make_shared<Trade>(orig);
        account_id     = _T( m_orig->account_id     );
        code           = _T( m_orig->code           );
        name           = _T( m_orig->name           );
        entrust_no     = _T( m_orig->entrust_no     );
        entrust_action = _T( m_orig->entrust_action );
        fill_no        = _T( m_orig->fill_no        );
        fill_size      = m_orig->fill_size              ;
        fill_price     = m_orig->fill_price             ;
        fill_date      = m_orig->fill_date              ;
        fill_time      = m_orig->fill_time              ;
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

    shared_ptr<Position> m_orig;

    PositionWrap(const Position& orig)
    {
        m_orig = make_shared<Position>(orig);
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
    const char*  entrust_no;       // ����ί�к�
    int32_t      order_id;         // �Զ�����
    
    shared_ptr<OrderID> m_orig;

    OrderIDWrap(const OrderID& orig)
    {
        m_orig = make_shared<OrderID>(orig);
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
    shared_ptr<const vector<Bar>> bars;
    shared_ptr<const vector<DailyBar>> daily_bars;
    shared_ptr<const vector<MarketQuote>> quotes;
    shared_ptr<const MarketQuote> quote;
    shared_ptr<vector<PositionWrap>> positions;
    shared_ptr<vector<OrderWrap>> orders;
    shared_ptr<vector<TradeWrap>> trades;
    shared_ptr<vector<AccountInfoWrap>> account_infos;
    shared_ptr<OrderIDWrap>  order_id;
    shared_ptr<BalanceWrap> balance;
    shared_ptr<bool> bool_value;
};

enum CallResultValueType {
    BAR_ARRAY = 1,
    QUOTE_ARRAY,
    DAILYBAR_ARRAY,
    QUOTE_VALUE,
    STRING_VALUE,
    ACCOUNT_INFO_ARRAY,
    POSITION_ARRAY,
    ORDER_ARRAY,
    TRADE_ARRAY,
    ORDER_ID_VALUE,
    BOOL_VALUE,
    BALANCE_VALUE
};

#pragma pack()

#endif
