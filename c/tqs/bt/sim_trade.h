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

    enum TradeType {
        MARGIN_TRADE,
        CASH_TRADE
    };

    enum TradeRule {
        TR_T0,
        TR_T1
    };
    struct CodeInfo {
        string code;
        string name;
        string mkt;
        string product_id;
        string product_class; // "Futurs", "Options" ???
        double price_multiple;
        double price_tick;
        double margin_ratio;
        TradeRule trade_rule;
    };

    shared_ptr<CodeInfo> get_code_info(const string& code);


    struct OrderData {
        shared_ptr<Order> order;
        string            price_type;
        size_t            last_volume;
        double            last_turnover;
        size_t            volume_in_queue;
        double            volume_multiple;
        double            price_tick;
    };

    struct PositionData {
        shared_ptr<Position> position;
        TradeType  trade_type;
        double  price_multiple = 1.0;
        double  margin_ratio = 1.0;
        bool  is_t0 = false;
    };

    struct TradeData {
        string  account_id;
        int32_t trading_day = 0;
        double  init_balance = 0.0;
        double  avail_balance = 0.0;
        double  frozen_balance = 0.0;
        double  margin = 0.0;
        double  frozen_margin = 0.0;
        double  commission = 0.0;
        double  stock_float_pnl = 0.0;
        double  future_float_pnl = 0.0;

        unordered_map<string, shared_ptr<PositionData>> positions;  // code + side -> Position
        unordered_map<string, shared_ptr<OrderData>>    orders;     // entrust_no -> order
        unordered_map<string, shared_ptr<Trade>>        trades;     // fill_no -> trade

        double avail() {
            return avail_balance - margin - frozen_margin - frozen_balance + (future_float_pnl <0 ? future_float_pnl : 0);
        }
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
        CallResult<const OrderID>             place_order    (const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id);
        //CallResult<bool>                      cancel_order   (const string& code, int order_id);
        CallResult<bool>                      cancel_order   (const string& code, const string& entrust_no);
        CallResult<const OrderID>             validate_and_freeze  (const string& code, double price, int64_t size, const string& action, const string& price_type);

        void try_match();

        void try_buy  (OrderData* order);
        void try_short(OrderData* order);
        void try_cover(OrderData* order);
        void try_sell (OrderData* order);
        void estimate_vol_in_queue(OrderData* order, const MarketQuote* q);
        
        inline bool check_quote_time(const MarketQuote* quote, const Order* order);

        bool reject_order   (Order* order, const char* msg);
        void make_trade     (Order* order, double price, string status_msg="");
        shared_ptr<PositionData> get_position(const string& code, const string& side);

        void update_last_prices();
        void update_float_pnl();

        void settle();
        void move_to(int trading_day);
        void save_data(const string& dir);

        void release_cash               (shared_ptr<PositionData> pd, double price, int64_t size);
        bool freeze_cash_if_avail       (shared_ptr<PositionData> pd, double price, int64_t size);
        void update_margin_if           (shared_ptr<PositionData> pos);
        void update_float_pnl           (shared_ptr<PositionData> pos);
        void update_cash_after_open  (shared_ptr<PositionData> pd, double inc_bal, double commission);
        void update_balance_after_close (shared_ptr<PositionData> pd, double inc_bal, double commission);

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
        virtual CallResult<const vector<AccountInfo>>   query_account_status() override;
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
        virtual TradeApi_Callback* set_callback(TradeApi_Callback* callback) override;

        SimAccount* get_account(const string& account_id) {
            auto it = m_accounts.find(account_id);
            return it != m_accounts.end() ? it->second : nullptr;
        }

        void try_match();
        void update_last_prices();

        void move_to(int trading_day);
        void settle();

        const unordered_map<string, SimAccount*> accounts() { return m_accounts; }

    private:
        SimStraletContext* m_ctx;
        unordered_map<string, SimAccount*> m_accounts;
    };

} } }

#endif

