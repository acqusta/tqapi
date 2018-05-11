#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <string.h>
#include <list>
#include <assert.h>
#include "stralet.h"
#include "algo.h"


namespace tquant { namespace stra {

    using namespace tquant::api;

    struct NetPosition {
        string  account_id;       // �ʺű��
        string  code;             // ֤ȯ����
        string  name;             // ֤ȯ����
        int64_t current_size;     // ��ǰ�ֲ�
                                  //int64_t enable_size;      // ���ã��ɽ��ף��ֲ�
        int64_t init_size;        // ��ʼ�ֲ�
        double  cost;             // �ɱ�
        double  cost_price;       // �ɱ��۸�
        double  last_price;       // ���¼۸�
                                  //double  float_pnl;        // �ֲ�ӯ��
                                  //double  close_pnl;        // ƽ��ӯ��
                                  //double  margin;           // ��֤��
                                  //double  commission;       // ������

        NetPosition()
            : current_size(0), init_size(0)
            , cost(0.0), cost_price(0.0), last_price(0.0)
        {
        }
    };


    class PorfolioManagerAlgoImpl;

    class PorfolioManagerAlgo : public AlgoStralet {
    public:
        PorfolioManagerAlgo();
        virtual ~PorfolioManagerAlgo();

        // interface AlgoStralet
        virtual void on_init            (StraletContext* sc) override;
        virtual void on_timer           (int32_t id, void* data) override;
        virtual void on_order_status    (shared_ptr<const Order> order) override;
        virtual void on_order_trade     (shared_ptr<const Trade> trade) override;
        virtual void on_account_status  (shared_ptr<const AccountInfo> account) override;

        void init(const string& account_id, const vector<string>& codes);

        CallResult<const vector<NetPosition>> query_net_position();
        CallResult<const vector<Position>>    query_position();

        void set_target(const vector<NetPosition>& target);
        void stop_target();
        bool is_stopped();

        // return task_id, not entrust_id
        CallResult<int64_t>  place_order  (const string& code, double price, int64_t inc_size);
        CallResult<bool>     cancel_order (const string& code, int64_t task_id);

    private:
        PorfolioManagerAlgoImpl* m_impl;
    };

} }