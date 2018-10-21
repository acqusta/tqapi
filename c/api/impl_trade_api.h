#ifndef _IMPL_TRADE_API_H
#define _IMPL_TRADE_API_H

#include "tquant_api.h"

#include "myutils/stringutils.h"
#include "myutils/mprpc.h"
#include "myutils/unicode.h"
#include "impl_tquant_api.h"

namespace tquant { namespace api { namespace impl {

    using namespace ::tquant::api;
    using namespace ::mprpc;            

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

        mp_map_get(obj, "account_id",   &act->account_id);
        mp_map_get(obj, "broker",       &act->broker);
        mp_map_get(obj, "account",      &act->account);
        mp_map_get(obj, "status",       &act->status);
        mp_map_get(obj, "msg",          &act->msg);
        mp_map_get(obj, "account_type", &act->account_type);

        return true;
    }

    class MpRpcTradeApiImpl : public TradeApi, public MpRpcClient_Callback {
        MpRpc_Connection*     m_conn;
        TradeApi_Callback*    m_callback;
    public:
        MpRpcTradeApiImpl()
            : m_conn(nullptr)
            , m_callback(nullptr)
        {}

        virtual ~MpRpcTradeApiImpl() override
        {}

        bool init(MpRpc_Connection* conn, const unordered_map<string, string> properties)
        {
            m_conn = conn;
            m_conn->set_callback(this);
            return true;
        }

        virtual CallResult<const vector<AccountInfo>> query_account_status() override
        {
            MsgPackPacker pk;
            pk.pack_map(0);

            auto rsp = m_conn->m_client->call("tapi.account_status", pk.sb.data, pk.sb.size);
            if (!is_arr(rsp->result))
                return CallResult<const vector<AccountInfo>>(builld_errmsg(rsp->err_code, rsp->err_msg));
    
            auto accounts = make_shared<vector<AccountInfo>>();

            for (size_t i = 0; i < rsp->result.via.array.size; i++) {
                auto& obj = rsp->result.via.array.ptr[i];
                AccountInfo act;
                if (convert_account(obj, &act))
                    accounts->push_back(act);
            }

            return CallResult<const vector<AccountInfo>>(accounts);
        }

        virtual CallResult<const Balance> query_balance(const string& account_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(1);
            pk.pack_map_item("account_id", account_id);

            auto rsp = m_conn->m_client->call("tapi.query_balance", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<const Balance>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_map(rsp->result)) return CallResult<const Balance>("-1,wrong data format");
            
            auto bal = make_shared<Balance>();
            mp_map_get (rsp->result, "account_id",    &bal->account_id);
            mp_map_get (rsp->result, "fund_account",  &bal->fund_account);
            mp_map_get (rsp->result, "init_balance",  &bal->init_balance);
            mp_map_get (rsp->result, "enable_balance",&bal->enable_balance);
            mp_map_get (rsp->result, "margin",        &bal->margin);
            mp_map_get (rsp->result, "float_pnl",     &bal->float_pnl);
            mp_map_get (rsp->result, "close_pnl",     &bal->close_pnl);

            return CallResult<const Balance>(bal);
        }

        static string inline union_str(const unordered_set<string>& strs, char sep=',')
        {
            if (strs.empty()) return "";

            stringstream ss;
            int i = 0;
            for (auto& s : strs) {
                ss << s;
                if (i++ != strs.size() - 1)
                    ss << sep;
            }

            return ss.str();
        }

        virtual CallResult<const vector<Order>> query_orders(const string& account_id, const unordered_set<string>* codes = nullptr) override
        {
            return query_orders(account_id, codes ? union_str(*codes) : "");
        }

        virtual CallResult<const vector<Order>> query_orders(const string& account_id, const string& codes = nullptr) override
        {
            MsgPackPacker pk;
            pk.pack_map(2);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("codes",      codes);

            auto rsp = m_conn->m_client->call("tapi.query_orders", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<const vector<Order>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_arr(rsp->result)) return CallResult<const vector<Order>>("-1,wrong data format");

            auto orders = make_shared<vector<Order>>();

            for (size_t i = 0; i < rsp->result.via.array.size; i++) {
                auto& obj = rsp->result.via.array.ptr[i];
                Order ord;
                if (convert_orderstatus(obj, &ord))
                    orders->push_back(ord);
            }

            return CallResult<const vector<Order>>(orders);
        }

        virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const unordered_set<string>* codes = nullptr) override
        {
            return query_trades(account_id, codes ? union_str(*codes) : "");
        }

        virtual CallResult<const vector<Trade>> query_trades(const string& account_id, const string& codes = nullptr) override
        {
            MsgPackPacker pk;
            pk.pack_map(2);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("codes",      codes);

            auto rsp = m_conn->m_client->call("tapi.query_trades", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<const vector<Trade>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_arr(rsp->result)) return CallResult<const vector<Trade>>("-1,wrong data format");

            auto trades = make_shared<vector<Trade>>();

            for (size_t i = 0; i < rsp->result.via.array.size; i++) {
                auto& obj = rsp->result.via.array.ptr[i];
                Trade trd;
                if (convert_ordertrade(obj, &trd))
                    trades->push_back(trd);
            }

            return CallResult<const vector<Trade>>(trades);
        }

        virtual CallResult<const vector<Position>> query_positions(const string& account_id, const unordered_set<string>* codes = nullptr) override
        {
            return query_positions(account_id, codes ? union_str(*codes) : "");
        }

        virtual CallResult<const vector<Position>> query_positions(const string& account_id, const string& codes) override
        {
            MsgPackPacker pk;
            pk.pack_map(2);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("codes",      codes);

            auto rsp = m_conn->m_client->call("tapi.query_positions", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<const vector<Position>>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_arr(rsp->result)) return CallResult<const vector<Position>>("-1,wrong data format");

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

            return CallResult<const vector<Position>>(positions);
        }

        virtual CallResult<const OrderID> place_order(const string& account_id, const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(7);
            pk.pack_map_item("account_id",  account_id);
            pk.pack_map_item("code",        code);
            pk.pack_map_item("price",       price);
            pk.pack_map_item("size",        size);
            pk.pack_map_item("action",      action);
            pk.pack_map_item("price_type",  price_type);
            pk.pack_map_item("order_id",    order_id);

            auto rsp = m_conn->m_client->call("tapi.place_order", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<const OrderID>(builld_errmsg(rsp->err_code, rsp->err_msg));

            if (!is_map(rsp->result)) return CallResult<const OrderID>("-1,wrong data format");

            auto orderid = make_shared<OrderID>();
            msgpack_object_kv* p     = rsp->result.via.map.ptr;
            msgpack_object_kv* p_end = p + rsp->result.via.map.size;
            for (; p < p_end; p++) {
                if (p->key.type != MSGPACK_OBJECT_STR) continue;

                string str(p->key.via.str.ptr, p->key.via.str.size);

                if      (str == "entrust_no")   mp_get(p->val, &orderid->entrust_no);
                else if (str == "order_id")     mp_get(p->val, &orderid->order_id);
            }

            return CallResult<const OrderID>(orderid);
        }

        virtual CallResult<bool> cancel_order(const string& account_id, const string& code, int order_id) override
        {
            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("order_id",   order_id);
            pk.pack_map_item("code", code);

            auto rsp = m_conn->m_client->call("tapi.cancel_order", pk.sb.data, pk.sb.size);
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

        virtual CallResult<bool> cancel_order(const string& account_id, const string& code, const string& entrust_no) override
        {
            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("entrust_no", entrust_no);
            pk.pack_map_item("code", code);

            auto rsp = m_conn->m_client->call("tapi.cancel_order", pk.sb.data, pk.sb.size);
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

        virtual CallResult<string> query(const string& account_id, const string& command, const string& params) override
        {
            MsgPackPacker pk;
            pk.pack_map(3);
            pk.pack_map_item("account_id", account_id);
            pk.pack_map_item("command", command);
            pk.pack_map_item("params", params);

            auto rsp = m_conn->m_client->call("tapi.common_query", pk.sb.data, pk.sb.size);
            if (is_nil(rsp->result))
                return CallResult<string>(builld_errmsg(rsp->err_code, rsp->err_msg));

            //string text;
            if (is_map(rsp->result)) {
                string content;
                if (mp_map_get(rsp->result, "content", &content))
                    return CallResult<string>(make_shared<string>(content));
            }
            return CallResult<string>("-1,wrong data format");
        }

        virtual TradeApi_Callback* set_callback(TradeApi_Callback* callback) override
        {
            auto old = m_callback;
            m_callback = callback;
            return old;
        }

        virtual void on_notification(shared_ptr<MpRpcMessage> rpcmsg) override
        {
            if (!m_callback) return;

            if (rpcmsg->method == "tapi.order_status_ind") {
                auto order = make_shared<Order>();
                if (convert_orderstatus(rpcmsg->params, order.get()))
                    m_callback->on_order_status(order);
            }
            else if (rpcmsg->method == "tapi.order_trade_ind") {
                auto trade = make_shared<Trade>();
                if (convert_ordertrade(rpcmsg->params, trade.get()))
                    m_callback->on_order_trade(trade);
            }
            else if (rpcmsg->method == "tapi.account_status_ind") {
                auto act = make_shared<AccountInfo>();
                if (convert_account(rpcmsg->params, act.get()))
                    m_callback->on_account_status(act);
            }
        }
    };

} } }

#endif
