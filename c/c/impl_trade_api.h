#ifndef _IMPL_TRADE_API_H
#define _IMPL_TRADE_API_H

#include "myutils/stringutils.h"
#include "myutils/jsonrpc.h"
#include "tquant_api.h"
#include "impl_tquant_api.h"

namespace tquant { namespace api { namespace impl {

    using namespace ::jsonrpc;
    using namespace ::tquant::api;

    static inline bool convert_orderstatus(msgpack_object& obj, Order* order)
    {
        if (!is_map(obj)) return false;

        msgpack_object_kv* p = obj.via.map.ptr;
        msgpack_object_kv* p_end = p + obj.via.map.size;
        for (; p < p_end; p++) {
            if (p->key.type != MSGPACK_OBJECT_STR) continue;

            string str(p->key.via.str.ptr, p->key.via.str.size);

            if      (str == "account_id")       mp_get(p->val, &order->account_id);
            else if (str == "code")             mp_get(p->val, &order->code);
            else if (str == "name")             mp_get(p->val, &order->name);
            else if (str == "entrust_no")       mp_get(p->val, &order->entrust_no);
            else if (str == "entrust_action")   mp_get(p->val, &order->entrust_action);
            else if (str == "entrust_price")    mp_get(p->val, &order->entrust_price);
            else if (str == "entrust_size")     mp_get(p->val, &order->entrust_size);
            else if (str == "entrust_date")     mp_get(p->val, &order->entrust_date);
            else if (str == "entrust_time")     mp_get(p->val, &order->entrust_time);
            else if (str == "fill_price")       mp_get(p->val, &order->fill_price);
            else if (str == "fill_size")        mp_get(p->val, &order->fill_size);
            else if (str == "status")           mp_get(p->val, &order->status);
            else if (str == "status_msg")       mp_get(p->val, &order->status_msg);
            else if (str == "order_id")         mp_get(p->val, &order->order_id);
        }

        return true;
    }

    static bool convert_ordertrade(msgpack_object& obj, Trade* trd)
    {
        if (!is_map(obj)) return false;

        msgpack_object_kv* p = obj.via.map.ptr;
        msgpack_object_kv* p_end = p + obj.via.map.size;
        for (; p < p_end; p++) {
            if (p->key.type != MSGPACK_OBJECT_STR) continue;

            string str(p->key.via.str.ptr, p->key.via.str.size);

            if      (str == "account_id")       mp_get(p->val, &trd->account_id);
            else if (str == "code")             mp_get(p->val, &trd->code);
            else if (str == "name")             mp_get(p->val, &trd->name);
            else if (str == "entrust_no")       mp_get(p->val, &trd->entrust_no);
            else if (str == "entrust_action")   mp_get(p->val, &trd->entrust_action);
            else if (str == "fill_no")          mp_get(p->val, &trd->fill_no);
            else if (str == "fill_size")        mp_get(p->val, &trd->fill_size);
            else if (str == "fill_price")       mp_get(p->val, &trd->fill_price);
            else if (str == "fill_date")        mp_get(p->val, &trd->fill_date);
            else if (str == "fill_time")        mp_get(p->val, &trd->fill_time);
        }

        return true;
    }

    static inline bool  convert_account(msgpack_object& obj, AccountInfo* act)
    {
        if (!is_map(obj)) return false;

        get_map_field_str(obj, "account_id",   &act->account_id);
        get_map_field_str(obj, "broker",       &act->broker);
        get_map_field_str(obj, "account",      &act->account);
        get_map_field_str(obj, "status",       &act->status);
        get_map_field_str(obj, "msg",          &act->msg);
        get_map_field_str(obj, "account_type", &act->account_type);

        return true;
    }

    class TradeApiImpl : public TradeApi {
        JsonRpcClient*        m_client;
        unordered_set<string> m_sub_codes;
        uint64_t              m_sub_hash;
        TradeApi_Callback*    m_callback;
    public:
        TradeApiImpl(JsonRpcClient* client)
            : m_client(client)
            , m_sub_hash(0)
            , m_callback(nullptr)
        {}

        virtual ~TradeApiImpl() override
        {}

        virtual CallResult<vector<AccountInfo>> query_account_status() override
        {
            MsgPackPacker pk;
            pk.pack_map(0);

            auto rsp = m_client->call("tapi.account_status", pk.sb.data, pk.sb.size);
            if (!is_arr(rsp->result))
                return CallResult<vector<AccountInfo>>(builld_errmsg(rsp->err_code, rsp->err_msg));
    
            auto accounts = make_shared<vector<AccountInfo>>();

            for (size_t i = 0; i < rsp->result.via.array.size; i++) {
                auto& obj = rsp->result.via.array.ptr[i];
                AccountInfo act;
                if (convert_account(obj, &act))
                    accounts->push_back(act);
            }

            return CallResult<vector<AccountInfo>>(accounts);
        }

        virtual CallResult<Balance> query_balance(const char* account_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(1);
            pk.pack_map_item("account_id", account_id);

            auto rsp = m_client->call("tapi.query_balance", pk.sb.data, pk.sb.size);
            if (!is_arr(rsp->result))
                return CallResult<Balance>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_map(rsp->result)) return CallResult<Balance>("-1,wrong data format");
            
            auto bal = make_shared<Balance>();
            get_map_field_str    (rsp->result, "account_id",    &bal->account_id);
            get_map_field_str    (rsp->result, "fund_account",  &bal->fund_account);
            get_map_field_double (rsp->result, "init_balance",  &bal->init_balance);
            get_map_field_double (rsp->result, "enable_balance",&bal->enable_balance);
            get_map_field_double (rsp->result, "margin",        &bal->margin);
            get_map_field_double (rsp->result, "float_pnl",     &bal->float_pnl);
            get_map_field_double (rsp->result, "close_pnl",     &bal->close_pnl);

            return CallResult<Balance>(bal);
        }

        virtual CallResult<vector<Order>> query_orders(const char* account_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(1);
            pk.pack_map_item("account_id", account_id);

            auto rsp = m_client->call("tapi.query_orders", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<vector<Order>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_arr(rsp->result)) return CallResult<vector<Order>>("-1,wrong data format");

            auto orders = make_shared<vector<Order>>();

            for (size_t i = 0; i < rsp->result.via.array.size; i++) {
                auto& obj = rsp->result.via.array.ptr[i];
                Order ord;
                if (convert_orderstatus(obj, &ord))
                    orders->push_back(ord);
            }

            return CallResult<vector<Order>>(orders);
        }

        virtual CallResult<vector<Trade>> query_trades(const char* account_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(1);
            pk.pack_map_item("account_id", account_id);

            auto rsp = m_client->call("tapi.query_trades", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<vector<Trade>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_arr(rsp->result)) return CallResult<vector<Trade>>("-1,wrong data format");

            auto trades = make_shared<vector<Trade>>();

            for (size_t i = 0; i < rsp->result.via.array.size; i++) {
                auto& obj = rsp->result.via.array.ptr[i];
                Trade trd;
                if (convert_ordertrade(obj, &trd))
                    trades->push_back(trd);
            }

            return CallResult<vector<Trade>>(trades);
        }

        virtual CallResult<vector<Position>> query_positions(const char* account_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(1);
            pk.pack_map_item("account_id", account_id);

            auto rsp = m_client->call("tapi.query_positions", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<vector<Position>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_arr(rsp->result)) return CallResult<vector<Position>>("-1,wrong data format");

            auto positions = make_shared<vector<Position>>();

            for (size_t i = 0; i < rsp->result.via.array.size; i++) {
                auto& obj = rsp->result.via.array.ptr[i];
                if (!is_map(obj)) continue;
                Position pos;
                msgpack_object_kv* p = obj.via.map.ptr;
                msgpack_object_kv* p_end = p + obj.via.map.size;
                for (; p < p_end; p++) {
                    if (p->key.type != MSGPACK_OBJECT_STR) continue;

                    string str(p->key.via.str.ptr, p->key.via.str.size);

                    if      (str == "account_id")   mp_get (p->val, &pos.account_id);
                    else if (str == "code")         mp_get (p->val, &pos.code);
                    else if (str == "name")         mp_get (p->val, &pos.name);
                    else if (str == "current_size") mp_get (p->val, &pos.current_size);
                    else if (str == "enable_size")  mp_get (p->val, &pos.enable_size);
                    else if (str == "init_size")    mp_get (p->val, &pos.init_size);
                    else if (str == "today_size")   mp_get (p->val, &pos.today_size);
                    else if (str == "frozen_size")  mp_get (p->val, &pos.frozen_size);
                    else if (str == "side")         mp_get (p->val, &pos.side);
                    else if (str == "cost")         mp_get (p->val, &pos.cost);
                    else if (str == "cost_price")   mp_get (p->val, &pos.cost_price);
                    else if (str == "last_price")   mp_get (p->val, &pos.last_price);
                    else if (str == "float_pnl")    mp_get (p->val, &pos.float_pnl);
                    else if (str == "close_pnl")    mp_get (p->val, &pos.close_pnl);
                    else if (str == "margin")       mp_get (p->val, &pos.margin);
                    else if (str == "commission")   mp_get (p->val, &pos.commission);
                }
                positions->push_back(pos);
            }

            return CallResult<vector<Position>>(positions);
        }

        virtual CallResult<OrderID> place_order(const char* account_id, const char* code, double price, long size, const char* action, int order_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(6);
            pk.pack_map_item("account_id",  account_id);
            pk.pack_map_item("code",        code);
            pk.pack_map_item("price",       price);
            pk.pack_map_item("size",        size);
            pk.pack_map_item("action",      action);
            pk.pack_map_item("order_id",    order_id);

            auto rsp = m_client->call("tapi.place_order", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<OrderID>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_map(rsp->result)) return CallResult<OrderID>("-1,wrong data format");

            auto orderid = make_shared<OrderID>();
            msgpack_object_kv* p     = rsp->result.via.map.ptr;
            msgpack_object_kv* p_end = p + rsp->result.via.map.size;
            for (; p < p_end; p++) {
                if (p->key.type != MSGPACK_OBJECT_STR) continue;

                string str(p->key.via.str.ptr, p->key.via.str.size);

                if      (str == "entrust_no")   mp_get(p->val, &orderid->entrust_no);
                else if (str == "order_id")     mp_get(p->val, &orderid->order_id);
            }

            return CallResult<OrderID>(orderid);
        }

        virtual CallResult<bool> cancel_order(const char* account_id, const char* code, int order_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("order_id",   order_id);
            pk.pack_map_item("code", code);

            auto rsp = m_client->call("tapi.cancel_order", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<bool>(builld_errmsg(rsp->err_code, rsp->err_msg));

            bool r = false;
            if (mp_get(rsp->result, &r)) {
                return CallResult<bool>(make_shared<bool>(r));
            }
            else {
                return CallResult<bool>("-1,wrong data format");
            }
        }

        virtual CallResult<bool> cancel_order(const char* account_id, const char* code, const char* entrust_no)
        {
            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("entrust_no", entrust_no);
            pk.pack_map_item("code", code);

            auto rsp = m_client->call("tapi.cancel_order", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<bool>(builld_errmsg(rsp->err_code, rsp->err_msg));

            bool r = false;
            if (mp_get(rsp->result, &r)) {
                return CallResult<bool>(make_shared<bool>(r));
            }
            else {
                return CallResult<bool>("-1,wrong data format");
            }
        }

        virtual CallResult<string> query(const char* account_id, const char* command, const char* params)
        {
            return CallResult<string>("-1,to be implemented");
        }

        virtual void set_callback(TradeApi_Callback* callback) override
        {
            m_callback = callback;
        }


        void on_notification(shared_ptr<JsonRpcMessage> rpcmsg)
        {
            if (!m_callback) return;

            if (rpcmsg->method == "tapi.order_status_ind") {
                auto order = make_shared<Order>();
                if (convert_orderstatus(rpcmsg->params, order.get()))
                    m_callback->onOrderStatus(order);
            }
            else if (rpcmsg->method == "tapi.order_trade_ind") {
                auto trade = make_shared<Trade>();
                if (convert_ordertrade(rpcmsg->params, trade.get()))
                    m_callback->onOrderTrade(trade);
            }
            else if (rpcmsg->method == "tapi.account_status_ind") {
                auto act = make_shared<AccountInfo>();
                if (convert_account(rpcmsg->params, act.get()))
                    m_callback->onAccountStatus(act);
            }
        }
    };

} } }

#endif
