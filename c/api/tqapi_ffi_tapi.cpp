#include <iostream>
#include "tquant_api.h"
#include "tqapi_ffi.h"
#include "myutils/stringutils.h"

using namespace std;

void init_corder(Order* c, const tquant::api::Order* order)
{
    c->account_id     = order->account_id.c_str();
    c->code           = order->code.c_str();
    c->name           = order->name.c_str();
    c->entrust_no     = order->entrust_no.c_str();
    c->entrust_action = order->entrust_action.c_str();
    c->entrust_price  = order->entrust_price;
    c->entrust_size   = order->entrust_size;
    c->entrust_date   = order->entrust_date;
    c->entrust_time   = order->entrust_time;
    c->fill_price     = order->fill_price;
    c->fill_size      = order->fill_size;
    c->status         = order->status.c_str();
    c->status_msg     = order->status_msg.c_str();
    c->order_id       = order->order_id;
}

void init_ctrade(Trade* c, const tquant::api::Trade* trade)
{
    c->account_id     = trade->account_id.c_str();
    c->code           = trade->code.c_str();
    c->name           = trade->name.c_str();
    c->entrust_no     = trade->entrust_no.c_str();
    c->entrust_action = trade->entrust_action.c_str();
    c->fill_no        = trade->fill_no.c_str();
    c->fill_size      = trade->fill_size;
    c->fill_price     = trade->fill_price;
    c->fill_date      = trade->fill_date;
    c->fill_time      = trade->fill_time;
    c->order_id       = trade->order_id;
}

void init_caccount(AccountInfo* c, const tquant::api::AccountInfo* account)
{
    c->account_id     = account->account_id.c_str();
    c->broker         = account->broker.c_str();
    c->account        = account->broker.c_str();
    c->status         = account->status.c_str();
    c->msg            = account->msg.c_str();
    c->account_type   = account->account_type.c_str();
}

void init_cbalance(Balance* c, const tquant::api::Balance* balance)
{
    c->account_id    = balance->account_id.c_str();
    c->fund_account  = balance->fund_account.c_str();
    c->init_balance  = balance->init_balance;
    c->enable_balance = balance->enable_balance;
    c->margin        = balance->margin;
    c->float_pnl     = balance->float_pnl;
    c->close_pnl     = balance->close_pnl;
}

void init_cposition_array(vector<Position>* c, const vector<tquant::api::Position>* positions)
{
    c->resize(positions->size());
    for (size_t i =0; i < positions->size(); i++) {
        auto p1 = &(*c)[i];
        auto p2 = &(*positions)[i];
        p1->account_id    = p2->account_id.c_str();
        p1->code          = p2->code.c_str();
        p1->name          = p2->name.c_str();
        p1->current_size  = p2->current_size;
        p1->enable_size   = p2->enable_size;
        p1->init_size     = p2->init_size;
        p1->today_size    = p2->today_size;
        p1->frozen_size   = p2->frozen_size;
        p1->side          = p2->side.c_str();
        p1->cost          = p2->cost;
        p1->cost_price    = p2->cost_price;
        p1->last_price    = p2->last_price;
        p1->float_pnl     = p2->float_pnl;
        p1->close_pnl     = p2->close_pnl;
        p1->margin        = p2->margin;
        p1->commission    = p2->commission;
    }
}

void init_corder_array(vector<Order>* c, const vector<tquant::api::Order>* orders)
{
    c->resize(orders->size());
    for (size_t i =0; i < orders->size(); i++) {
        auto p1 = &(*c)[i];
        auto p2 = &(*orders)[i];
        init_corder(p1, p2);
    }
}

void init_ctrade_array(vector<Trade>* c, const vector<tquant::api::Trade>* trades)
{
    c->resize(trades->size());
    for (size_t i =0; i < trades->size(); i++) {
        auto p1 = &(*c)[i];
        auto p2 = &(*trades)[i];
        init_ctrade(p1, p2);
    }
}

void init_caccount_array(vector<AccountInfo>* c, const vector<tquant::api::AccountInfo>* accounts)
{
    c->resize(accounts->size());
    for (size_t i =0; i < accounts->size(); i++) {
        auto p1 = &(*c)[i];
        auto p2 = &(*accounts)[i];
        init_caccount(p1, p2);
    }
}

struct TradeApi : public tquant::api::TradeApi_Callback {

    tquant::api::TradeApi* instance;
    TradeApiCallback* cb;
    bool is_owner;

    virtual void on_order_status  (shared_ptr<tquant::api::Order> order) override {
        if (cb) {
            Order c_order;
            init_corder(&c_order, order.get());
            cb->on_order(&c_order, cb->user_data);
        }
    }

    virtual void on_order_trade   (shared_ptr<tquant::api::Trade> trade) override {
        if (cb) {
            Trade c_trade;
            init_ctrade(&c_trade, trade.get());
            cb->on_trade(&c_trade, cb->user_data);
        }
    }

    virtual void on_account_status(shared_ptr<tquant::api::AccountInfo> account) override {
        if (cb) {
            AccountInfo c_account;
            init_caccount(&c_account, account.get());
            cb->on_account_status(&c_account, cb->user_data);
        }
    }
};


extern "C" {

    TradeApi* tqapi_create_trade_api (const char* addr)
    {
        auto tapi = tquant::api::create_trade_api(addr);
        auto inst = new TradeApi();
        inst->instance = tapi;
        inst->is_owner = true;
        inst->cb  = nullptr;
        return inst;
    }

    void tqapi_free_trade_api (TradeApi* tapi)
    {
        if (tapi && tapi->is_owner) {
            delete tapi->instance;
            delete tapi;
        }
    }

    TradeApi* tqapi_tapi_from(tquant::api::TradeApi* inst)
    {
        auto tapi = new TradeApi();
        tapi->instance = inst;
        tapi->is_owner = false;
        tapi->cb  = nullptr;
        return tapi;
    }

    TradeApiCallback*     tqapi_tapi_set_callback    (TradeApi* tapi, TradeApiCallback* callback)
    {
        auto old = tapi->cb;
        tapi->cb = callback;
        return old;
    }

    struct PlaceOrderResultData {
        tquant::api::CallResult<const tquant::api::OrderID> result;
        OrderID oid;
    };

    PlaceOrderResult*     tqapi_tapi_place_order     (TradeApi* tapi, const char* account_id, NewOrder* order)
    {
        auto result = new PlaceOrderResult();
        result->_data = new PlaceOrderResultData();
        result->_data->result = tapi->instance->place_order(account_id, order->code,
                    order->price, order->size,
                    order->action, order->price_type, order->order_id);

        auto& r = result->_data->result;
        if (r.value) {
            result->result = &result->_data->oid;
            result->result->entrust_no = r.value->entrust_no.c_str();
            result->result->order_id   = r.value->order_id;
            result->msg = nullptr;
        }
        else {
            result->result = nullptr;
            result->msg = result->_data->result.msg.c_str();
        }

        return result;
    }
    struct CancelOrderResultData {
        tquant::api::CallResult<bool> result;
    };

    CancelOrderResult*    tqapi_tapi_cancel_order    (TradeApi* tapi, const char* account_id, const char* code, OrderID* oid)
    {
        auto result   = new CancelOrderResult();
        result->_data = new CancelOrderResultData();
        result->_data->result = oid->order_id ?
            tapi->instance->cancel_order(account_id, code, oid->order_id)   :
            tapi->instance->cancel_order(account_id, code, oid->entrust_no);

        auto & r = result->_data->result;

        if (r.value && *r.value) {
            result->success = 1;
            result->msg = nullptr;
        }
        else {
            result->success = 0;
            result->msg = r.msg.c_str();
        }

        return result;
    }

    struct QueryBalanceResultData {
        tquant::api::CallResult<const tquant::api::Balance> result;
        Balance balance;
    };

    QueryBalanceResult*   tqapi_tapi_query_balance   (TradeApi* tapi, const char* account_id)
    {
        auto result   = new QueryBalanceResult();
        result->_data = new QueryBalanceResultData();
        result->_data->result = tapi->instance->query_balance(account_id);

        auto & r = result->_data->result;

        if (r.value) {
            init_cbalance(&result->_data->balance, r.value.get());
            result->result = &result->_data->balance;
            result->msg = nullptr;
        }
        else {
            result->result = nullptr;
            result->msg = r.msg.c_str();
        }

        return result;
    }

    struct QueryPositionsResultData {
        tquant::api::CallResult<const vector<tquant::api::Position>> result;
        vector<Position> positions;
    };

    QueryPositionsResult* tqapi_tapi_query_positions (TradeApi* tapi, const char* account_id,  const char* codes)
    {
        auto result   = new QueryPositionsResult();
        result->_data = new QueryPositionsResultData();
        result->_data->result = tapi->instance->query_positions(account_id);

        auto & r = result->_data->result;

        if (r.value) {
            init_cposition_array(&result->_data->positions, r.value.get());
            result->array = &result->_data->positions[0];
            result->array_length = result->_data->positions.size();
            result->element_size = sizeof(Position);
            result->msg = nullptr;
        }
        else {
            result->array = nullptr;
            result->array_length = 0;
            result->element_size = 0;
            result->msg = r.msg.c_str();
        }

        return result;
    }

    struct QueryOrdersResultData {
        tquant::api::CallResult<const vector<tquant::api::Order>> result;
        vector<Order> orders;
    };

    QueryOrdersResult*    tqapi_tapi_query_orders    (TradeApi* tapi, const char* account_id,  const char* codes)
    {
        auto result   = new QueryOrdersResult();
        result->_data = new QueryOrdersResultData();
        result->_data->result = tapi->instance->query_orders(account_id, codes);

        auto & r = result->_data->result;

        if (r.value) {
            init_corder_array(&result->_data->orders, r.value.get());
            result->array = &result->_data->orders[0];
            result->array_length = result->_data->orders.size();
            result->element_size = sizeof(Order);
            result->msg = nullptr;
        }
        else {
            result->array = nullptr;
            result->array_length = 0;
            result->element_size = 0;
            result->msg = r.msg.c_str();
        }

        return result;
    }

    struct QueryTradesResultData {
        tquant::api::CallResult<const vector<tquant::api::Trade>> result;
        vector<Trade> trades;
    };

    QueryTradesResult*    tqapi_tapi_query_trades    (TradeApi* tapi, const char* account_id,  const char* codes)
    {
        auto result   = new QueryTradesResult();
        result->_data = new QueryTradesResultData();
        result->_data->result = tapi->instance->query_trades(account_id, codes);

        auto & r = result->_data->result;

        if (r.value) {
            init_ctrade_array(&result->_data->trades, r.value.get());
            result->array = &result->_data->trades[0];
            result->array_length = result->_data->trades.size();
            result->element_size = sizeof(Trade);
            result->msg = nullptr;
        }
        else {
            result->array = nullptr;
            result->array_length = 0;
            result->element_size = 0;
            result->msg = r.msg.c_str();
        }

        return result;
    }

    struct QueryResultData {
        tquant::api::CallResult<string> result;
    };

    QueryResult* tqapi_tapi_query (TradeApi* tapi, const char* account_id,  const char* command, const char* params)
    {
        auto result   = new QueryResult();
        result->_data = new QueryResultData();
        result->_data->result = tapi->instance->query(account_id, command, params);

        auto & r = result->_data->result;

        if (r.value) {
            result->result = r.value->c_str();
            result->msg = nullptr;
        }
        else {
            result->result = nullptr;
            result->msg = r.msg.c_str();
        }

        return result;
    }

    struct QueryAccountsResultData {
        tquant::api::CallResult<const vector<tquant::api::AccountInfo>> result;
        vector<AccountInfo> accounts;
    };


    QueryAccountsResult*  tqapi_tapi_query_accounts  (TradeApi* tapi)
    {
        auto result   = new QueryAccountsResult();
        result->_data = new QueryAccountsResultData();
        result->_data->result = tapi->instance->query_account_status();

        auto & r = result->_data->result;

        if (r.value) {
            init_caccount_array(&result->_data->accounts, r.value.get());
            result->array = &result->_data->accounts[0];
            result->array_length = result->_data->accounts.size();
            result->element_size = sizeof(AccountInfo);
            result->msg = nullptr;
        }
        else {
            result->array = nullptr;
            result->array_length = 0;
            result->element_size = 0;
            result->msg = r.msg.c_str();
        }

        return result;

    }

    void tqapi_tapi_free_place_order_result (TradeApi* tapi, PlaceOrderResult * result)
    {
        delete result->_data;
        delete result;
    }

    void tqapi_tapi_free_cancel_order_result     (TradeApi* tapi, CancelOrderResult   * result)
    {
        delete result->_data;
        delete result;
    }

    void tqapi_tapi_free_query_accounts_result   (TradeApi* tapi, QueryAccountsResult * result)
    {
        delete result->_data;
        delete result;
    }

    void tqapi_tapi_free_query_balance_result (TradeApi* tapi, QueryBalanceResult  * result)
    {
        delete result->_data;
        delete result;
    }

    void tqapi_tapi_free_query_positions_result (TradeApi* tapi, QueryPositionsResult* result)
    {
        delete result->_data;
        delete result;
    }

    void tqapi_tapi_free_query_orders_result (TradeApi* tapi, QueryOrdersResult * result)
    {
        delete result->_data;
        delete result;
    }

    void tqapi_tapi_free_query_trades_result (TradeApi* tapi, QueryTradesResult * result)
    {
        delete result->_data;
        delete result;
    }

    void tqapi_tapi_free_query_result (TradeApi* tapi, QueryResult * result)
    {
        delete result->_data;
        delete result;
    }
}
