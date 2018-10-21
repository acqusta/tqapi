#ifndef _SIM_TAPI_H
#define _SIM_TAPI_H

#include <unordered_map>
#include <list>
#include "stralet.h"
#include "tquant_api.h"
#include "backtest.h"

namespace tquant { namespace stralet { namespace backtest {

    using namespace tquant::api;
    using namespace tquant::stralet;

    class SimStraletContext;

    struct OrderData {
        shared_ptr<Order> order;
        string            price_type;
        size_t            last_volume;
        double            last_turnover;
        size_t            volume_in_queue;
        double            volume_multiple;
        double            price_tick;
    };

    struct TradeData {
        string  account_id;
        int32_t trading_day;
        double  init_balance;
        double  avail_balance;
        double  frozen_balance;
        double  margin;
        double  frozen_margin;
        double  commission;
        unordered_map<string, shared_ptr<Position>>     positions;  // code + side -> Position
        unordered_map<string, shared_ptr<OrderData>>    orders;     // entrust_no -> order
        unordered_map<string, shared_ptr<Trade>>        trades;     // fill_no -> trade
    };

    class SimAccount {
        friend SimStraletContext;
        friend SimTradeApi;
    public:
        SimAccount(SimStraletContext* ctx, const string& account_id,
                   double init_balance,
                   const vector<Holding> & holdings);

        CallResult<const Balance>             query_balance();
        CallResult<const vector<Order>>       query_orders   (const unordered_set<string>* codes);
        CallResult<const vector<Trade>>       query_trades   (const unordered_set<string>* codes);
        CallResult<const vector<Position>>    query_positions(const unordered_set<string>* codes);
        CallResult<const OrderID>             place_order(const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id);
        CallResult<bool>                      cancel_order(const string& code, int order_id);
        CallResult<bool>                      cancel_order(const string& code, const string& entrust_no);
        CallResult<const OrderID>             validate_order(const string& code, double price, int64_t size, const string& action, const string& price_type);

        void try_match();

        void try_buy  (OrderData* order);
        void try_short(OrderData* order);
        void try_cover(OrderData* order);
        void try_sell (OrderData* order);
        void estimate_vol_in_queue(OrderData* order, const MarketQuote* q);
        
        inline bool check_quote_time(const MarketQuote* quote, const Order* order);

        bool reject_order   (Order* order, const char* msg);
        void make_trade     (Order* order, double price);
        Position* get_position(const string& code, const string& side);


        void move_to(int trading_day);
        void save_data(const string& dir);

    private:
        shared_ptr<TradeData> m_tdata;
        vector<shared_ptr<TradeData>> m_his_tdata;

        list<shared_ptr<Order>> m_ord_status_ind_list;
        list<shared_ptr<Trade>> m_trade_ind_list;

        SimStraletContext* m_ctx;

        static int g_order_id;
        static int g_fill_id;;
    };

    class SimTradeApi : public TradeApi {
        friend SimStraletContext;
    public:
        SimTradeApi(SimStraletContext* ctx, vector<SimAccount*>& accounts)
            : m_ctx(ctx)
        {
            for (auto& e : accounts)
                m_accounts[e->m_tdata->account_id] = e;
        }

        // TradeApi
        virtual CallResult<const vector<AccountInfo>>   query_account_status();
        virtual CallResult<const Balance>               query_balance  (const string& account_id) override;
        virtual CallResult<const vector<Order>>         query_orders   (const string& account_id, const unordered_set<string>* codes) override;
        virtual CallResult<const vector<Trade>>         query_trades   (const string& account_id, const unordered_set<string>* codes) override;
        virtual CallResult<const vector<Position>>      query_positions(const string& account_id, const unordered_set<string>* codes) override;
        virtual CallResult<const vector<Order>>         query_orders   (const string& account_id, const string& codes) override;
        virtual CallResult<const vector<Trade>>         query_trades   (const string& account_id, const string& codes) override;
        virtual CallResult<const vector<Position>>      query_positions(const string& account_id, const string& codes) override;
        virtual CallResult<const OrderID>               place_order    (const string& account_id, const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id) override;
        virtual CallResult<bool>                        cancel_order   (const string& account_id, const string& code, int order_id) override;
        virtual CallResult<bool>                        cancel_order   (const string& account_id, const string& code, const string& entrust_no) override;
        virtual CallResult<string>                      query          (const string& account_id, const string& command, const string& params) override;
        virtual TradeApi_Callback* set_callback(TradeApi_Callback* callback);

        SimAccount* get_account(const string& account_id) {
            auto it = m_accounts.find(account_id);
            return it != m_accounts.end() ? it->second : nullptr;
        }

        void try_match();

        void move_to(int trading_day);

        const unordered_map<string, SimAccount*> accounts() { return m_accounts; }

    private:
        SimStraletContext* m_ctx;
        unordered_map<string, SimAccount*> m_accounts;
    };

} } }

#endif

