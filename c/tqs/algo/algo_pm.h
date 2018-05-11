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
        string  account_id;       // 帐号编号
        string  code;             // 证券代码
        string  name;             // 证券名称
        int64_t current_size;     // 当前持仓
                                  //int64_t enable_size;      // 可用（可交易）持仓
        int64_t init_size;        // 初始持仓
        double  cost;             // 成本
        double  cost_price;       // 成本价格
        double  last_price;       // 最新价格
                                  //double  float_pnl;        // 持仓盈亏
                                  //double  close_pnl;        // 平仓盈亏
                                  //double  margin;           // 保证金
                                  //double  commission;       // 手续费

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