#include <assert.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "stralet.h"
#include "sim_context.h"
#include "sim_data.h"
#include "sim_trade.h"

using namespace tquant::api;
using namespace tquant::stralet;
using namespace tquant::stralet::backtest;


int32_t SimAccount::g_fill_id = 0;
int32_t SimAccount::g_order_id = 0;

static bool is_finished_status(const string& status)
{
    return status == OS_Filled || status == OS_Rejected || status == OS_Cancelled;
}

static void get_action_effect(const string& action, string* pos_side, int* inc_dir)
{
    if (action == EA_Buy) {
        *pos_side = SD_Long;
        *inc_dir = 1;
    }
    else if (action == EA_Sell ||
        action == EA_SellToday || action == EA_SellYesterday) {
        *pos_side = SD_Long;
        *inc_dir = -1;
    }
    else if (action == EA_Short) {
        *pos_side = SD_Short;
        *inc_dir = 1;
    }
    else {
        *pos_side = SD_Short;
        *inc_dir = -1;
    }
}


CallResult<const vector<AccountInfo>> SimTradeApi::query_account_status()
{
    auto infos = make_shared<vector<AccountInfo>>();

    for (auto & e : m_accounts) {
        AccountInfo info;
        info.account_id = e.second->m_tdata->account_id;
        info.broker = "SimBroker";
        info.account = info.account_id;
        info.status = "Connected";
        info.account_type = "stock_future";// e.second->account_type;

        infos->push_back(info);
    }

    return CallResult<const vector<AccountInfo>>(infos);
}

CallResult<const Balance> SimTradeApi::query_balance(const string& account_id)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->query_balance() :
        CallResult<const Balance>("-1,no such account");
}

CallResult<const vector<Order>> SimTradeApi::query_orders(const string& account_id)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->query_orders() :
        CallResult<const vector<Order>>("-1,no such account");
}

CallResult<const vector<Trade>> SimTradeApi::query_trades(const string& account_id)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->query_trades() :
        CallResult<const vector<Trade>>("-1,no such account");
}

CallResult<const vector<Position>> SimTradeApi::query_positions(const string& account_id)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->query_positions() :
        CallResult<const vector<Position>>("-1,no such account");
}

CallResult<const OrderID> SimTradeApi::place_order(const string& account_id, const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->place_order(code, price, size, action, price_type, order_id):
        CallResult<const OrderID>("-1,no such account");
}

CallResult<bool> SimTradeApi::cancel_order(const string& account_id, const string& code, int order_id)
{
    // XXX only works if entrust_no can be constructed by order_id!
    char entrust_no[100]; sprintf(entrust_no, "sim-%.6d", order_id);
    return cancel_order(account_id, code, entrust_no);
}

CallResult<bool> SimTradeApi::cancel_order(const string& account_id, const string& code, const string& entrust_no)
{
    auto act = get_account(account_id);
    return act ? act->cancel_order(code, entrust_no) :
        CallResult<bool>("-1,no such account");
}

CallResult<string> SimTradeApi::query(const string& account_id, const string& command, const string& params)
{
    //// TODO: load commands from configuration.
    //if (strcmp(command, "ctp_codetable") == 0) {
    //    string path = m_ctx->get_parameter("_etc_path", "");
    //    path += string("ctp_codetable.csv");
    //    ifstream file;
    //    file._Openmask()

    //}
    return CallResult<string>("-1,unknown command");
}

TradeApi_Callback* SimTradeApi::set_callback(TradeApi_Callback* callback)
{
    // TODO:
    return nullptr;
}

void SimTradeApi::move_to(int trading_day)
{
    for (auto& e : m_accounts) {
        e.second->move_to(trading_day);
    }
}

void SimTradeApi::try_match()
{
    for (auto & e : m_accounts) { e.second->try_match(); };
}

SimAccount::SimAccount(SimStraletContext* ctx, const string& account_id,
                       double init_balance,
                       const vector<Holding> & holdings)
{
    m_ctx = ctx;
    auto tdata = make_shared<TradeData>();
    tdata->account_id     = account_id;
    tdata->init_balance   = init_balance;
    tdata->enable_balance = init_balance;
    tdata->frozen_balance = 0.0;
    tdata->trading_day    = 0;

    for (auto& h : holdings) {
        auto pos = make_shared<Position>();
        pos->account_id  = account_id;
        pos->code        = h.code;
        pos->enable_size = pos->current_size = pos->init_size = h.size;
        pos->cost_price  = h.cost_price * h.size;
        pos->cost        = h.cost_price * h.size;
        tdata->positions[h.code + "-" + h.side] = pos;
    }

    m_tdata = tdata;
    m_his_tdata.push_back(m_tdata);
}

CallResult<const Balance> SimAccount::query_balance()
{
    auto bal = make_shared<Balance>();
    bal->account_id     = m_tdata->account_id;
    bal->fund_account   = m_tdata->account_id;
    bal->init_balance   = m_tdata->init_balance;
    bal->enable_balance = m_tdata->enable_balance;
    //bal->margin = m_margin;
    //bal->float_pnl = m_float_pnl;
    //bal->close_pnl = m_close_pnl;

    return CallResult<const Balance>(bal);
}

CallResult<const vector<Order>> SimAccount::query_orders()
{
    auto orders = make_shared<vector<Order>>();
    for (auto & e : m_tdata->orders)
        orders->push_back(*e.second->order);

    sort(orders->begin(), orders->end(), [](Order& a, Order&b) {
        return a.entrust_no < b.entrust_no;
    });

    return CallResult<const vector<Order>>(orders);
}

CallResult<const vector<Trade>> SimAccount::query_trades()
{
    auto trades = make_shared<vector<Trade>>();
    for (auto & e : m_tdata->trades)
        trades->push_back(*e.second);

    sort(trades->begin(), trades->end(), [](Trade& a, Trade&b) {
        return a.fill_no < b.fill_no;
    });

    return CallResult<const vector<Trade>>(trades);
}

CallResult<const vector<Position>> SimAccount::query_positions()
{
    auto poses = make_shared<vector<Position>>();
    for (auto & e : m_tdata->positions)
        poses->push_back(*e.second);

    sort(poses->begin(), poses->end(), [](Position& a, Position&b) {
        return a.code < b.code;
    });

    return CallResult<const vector<Position>>(poses);
}

CallResult<const OrderID> SimAccount::validate_order(const string& code, double price, int64_t size, const string& action, const string& price_type)
{
    DateTime dt = m_ctx->cur_time();

    auto q = m_ctx->data_api()->quote(code).value;
    //cout << "place_order: " << dt.date << "," << dt.time << ","
    //    << m_tdata->account_id << "," << code << "," << price << "," << size << "," << action << ","
    //    << "price: " << q->last << "," << q->ask1 << "," << q->bid1 << endl;

    const char *p = strrchr(code.c_str(), '.');
    if (!p)
        return CallResult<const OrderID>("-1,wrong code");

    if (price <= 0.000001)
        return CallResult<const OrderID>("-1,wrong price");
    if (size <= 0)
        return CallResult<const OrderID>("-1,wrong size");

    string mkt(p + 1);
    bool is_open_time = false;
    if (mkt == "SH" || mkt == "SZ" ||  mkt == "CFE") {
        is_open_time =
            (dt.time >= HMS(9, 30) && dt.time < HMS(11, 30) ||
                dt.time >= HMS(13, 0) && dt.time < HMS(15, 00));
    }
    else {
        is_open_time =
            (dt.time >= HMS(9, 0) && dt.time < HMS(10, 15) ||
                dt.time >= HMS(10, 30) && dt.time < HMS(11, 30) ||
                dt.time >= HMS(13, 30) && dt.time < HMS(15, 00));
    }

    if (!is_open_time)
        return CallResult<const OrderID>("-1,market is closed");

    string pos_side;
    int inc_dir = 0;

    get_action_effect(action, &pos_side, &inc_dir);
    auto pos = get_position(code, pos_side);
    if (inc_dir == -1) {
        if (pos->enable_size - pos->frozen_size < size) {
            stringstream ss;
            ss << "-1,no enough size for close(" << pos->enable_size << ","
                << pos->frozen_size << "," << size << ")";
            return CallResult<const OrderID>(ss.str());
        }
        // XXX move to place_order
        pos->frozen_size += size;
    }
    else {
        if (price*size > m_tdata->enable_balance - m_tdata->frozen_balance)
            return CallResult<const OrderID>("-1,no enough money");

        // XXX move to place_order
        m_tdata->frozen_balance += price*size;
    }

    int32_t my_order_id = ++g_order_id;
    Order ord;
    char entrust_no[100]; sprintf(entrust_no, "sim-%.6d", my_order_id);

    auto oid = make_shared<OrderID>();
    oid->entrust_no = entrust_no;
    oid->order_id = my_order_id;
    return CallResult<const OrderID>(oid);
}

CallResult<const OrderID> SimAccount::place_order(const string& code, double price, int64_t size, const string& action, const string& price_type, int order_id)
{
    auto r = validate_order(code, price, size, action, price_type);

    string status;
    string status_msg;
    string entrust_no;
    int32_t my_order_id;

    if (r.value) {
        entrust_no = r.value->entrust_no;
        my_order_id = r.value->order_id;
        status = OS_New;
    }
    else {
        my_order_id = ++g_order_id;
        char tmp[100]; sprintf(tmp, "sim-%.6d", my_order_id);
        entrust_no = tmp;
        status = OS_Rejected;
        status_msg = r.msg;
    }

    DateTime dt = m_ctx->cur_time();

    auto order = make_shared<Order>();
    order->account_id     = m_tdata->account_id;
    order->code           = code;
    order->name           = code;
    order->entrust_no     = entrust_no;
    order->entrust_action = action;
    order->entrust_price  = price;
    order->entrust_size   = size;
    order->entrust_date   = dt.date;
    order->entrust_time   = dt.time;
    order->fill_price     = 0.0;
    order->fill_size      = 0L;
    order->status         = status;
    order->status_msg     = "";
    order->order_id       = my_order_id;
    order->status_msg     = status_msg;

    auto od = make_shared<OrderData>();
    od->order = order;
    od->price_type = price_type;
    m_tdata->orders[entrust_no] = od;

    if (r.value)
        m_ord_status_ind_list.push_back(make_shared<Order>(*order));

    return r;
}

CallResult<bool> SimAccount::cancel_order(const string& code, const string& entrust_no)
{
    //cout << "cancel_order: " << m_tdata->account_id << "," << code << "," << entrust_no << endl;

    auto it = m_tdata->orders.find(entrust_no);
    if (it == m_tdata->orders.end())
        return CallResult<bool>("-1,no such order");
    auto od = it->second;
    if (od->order->code != code)
        return CallResult<bool>("-1,code doesn't match");

    if (!is_finished_status(od->order->status)) {
        string pos_side;
        int inc_dir;
        get_action_effect(od->order->entrust_action, &pos_side, &inc_dir);
        auto pos = get_position(code, pos_side);
        if (inc_dir == 1)
            m_tdata->frozen_balance -= od->order->entrust_price * od->order->entrust_size;
        else
            pos->frozen_size -= od->order->entrust_size;

        od->order->status = OS_Cancelled;
    }

    // Must make a copy!
    m_ord_status_ind_list.push_back(make_shared<Order>(*od->order));

    return CallResult<bool>(make_shared<bool>(true));
}


void SimAccount::try_match()
{
    for (auto & e : m_tdata->orders) {
        auto ord = e.second.get();
        if (!is_finished_status(ord->order->status)) {
            if      (ord->order->entrust_action == EA_Buy)             try_buy   (ord);
            else if (ord->order->entrust_action == EA_Short)           try_short (ord);
            else if (ord->order->entrust_action == EA_Cover)           try_cover (ord);
            else if (ord->order->entrust_action == EA_Sell)            try_sell  (ord);
            else if (ord->order->entrust_action == EA_Short)           try_short (ord);
            else if (ord->order->entrust_action == EA_SellToday)       try_sell  (ord);
            else if (ord->order->entrust_action == EA_SellYesterday)   try_sell  (ord);
            else if (ord->order->entrust_action == EA_CoverToday)      try_cover (ord);
            else if (ord->order->entrust_action == EA_CoverYesterday)  try_cover (ord);
        }
    }
}

bool is_future(const char* code)
{
    const char*p = strrchr(code, '.');
    if (!p) return false;
    p++;
    return strcmp(p, "SH") && strcmp(p, "SZ");
}

Position* SimAccount::get_position(const string& code, const string& side)
{
    shared_ptr<Position> pos = nullptr;
    auto id = code + "-" + side;
    auto it = m_tdata->positions.find(id);
    if (it == m_tdata->positions.end()) {
        pos = make_shared<Position>();
        pos->account_id = m_tdata->account_id;
        pos->code = code;
        pos->side = side;
        pos->name = pos->code;
        m_tdata->positions[id] = pos;
    }
    else {
        pos = it->second;
    }

    return pos.get();
}

bool SimAccount::reject_order(Order* order, const char* msg)
{
    //cout << "make_trade: " << m_tdata->account_id << "," << order->code << "," << order->entrust_size << ","
    //    << order->entrust_action << "," << fill_price << endl;

    string pos_side;
    int inc_dir = 0;

    int64_t fill_size = order->entrust_size;

    get_action_effect(order->entrust_action, &pos_side, &inc_dir);
    auto pos = get_position(order->code, pos_side);
    if (inc_dir == 1) {
        if (is_future(order->code.c_str()))
            pos->enable_size += fill_size;

        m_tdata->frozen_balance -= order->entrust_size * order->entrust_price;
    }
    else {
        pos->frozen_size -= order->entrust_size;
    }

    order->fill_price = 0;
    order->fill_size = 0;
    order->status = OS_Rejected;
    order->status_msg = msg;

    // Must make a copy!
    m_ord_status_ind_list.push_back(make_shared<Order>(*order));
}

void SimAccount::make_trade(Order* order, double fill_price)
{
    //cout << "make_trade: " << m_tdata->account_id << "," << order->code << "," << order->entrust_size << ","
    //    << order->entrust_action << "," << fill_price << endl;

    string pos_side;
    int inc_dir = 0;

    int64_t fill_size = order->entrust_size;

    get_action_effect(order->entrust_action, &pos_side, &inc_dir);
    auto pos = get_position(order->code, pos_side);
    if (inc_dir == 1) {
        pos->current_size += fill_size;
        if (is_future(order->code.c_str()))
            pos->enable_size += fill_size;
        pos->cost += fill_price * fill_size;
        pos->cost_price = pos->cost / pos->current_size;

        m_tdata->frozen_balance -= order->entrust_size * order->entrust_price;
        m_tdata->enable_balance -= fill_size * fill_price;
        //this->m_ctx->logger() << "enable_balance -= " << fill_size * fill_price << endl;
    }
    else {
        pos->frozen_size  -= order->entrust_size;
        pos->current_size -= fill_size;
        pos->enable_size  -= fill_size; // TODO: check if it is right
        assert(pos->enable_size >= 0);
        if (pos->side == SD_Long) {
            pos->close_pnl += fill_size * (fill_price - pos->cost_price);
            m_tdata->enable_balance += fill_size * fill_price;
        }
        else {
            pos->close_pnl += fill_size * (pos->cost_price - fill_price);
            m_tdata->enable_balance += fill_size * (pos->cost_price*2 - fill_price);
        }
        //this->m_ctx->logger() << "enable_balance += " << fill_size * fill_price << endl;
        if (pos->current_size == 0) {
            pos->cost = 0.0;
            pos->cost_price = 0.0;
        }
    }

    order->fill_price = fill_price;
    order->fill_size  = fill_size;
    order->status     = OS_Filled;

    // Must make a copy!
    m_ord_status_ind_list.push_back(make_shared<Order>(*order));

    DateTime dt = m_ctx->cur_time();

    int32_t fill_id = ++g_fill_id;
    char fill_no[100];
    sprintf(fill_no, "sim-t-%.6d", fill_id);

    auto trade = make_shared<Trade>();
    trade->account_id     = m_tdata->account_id;
    trade->code           = order->code;
    trade->entrust_action = order->entrust_action;
    trade->name           = order->name;
    trade->entrust_no     = order->entrust_no;
    trade->entrust_action = order->entrust_action;
    trade->fill_price     = order->fill_price;
    trade->fill_size      = order->entrust_size;
    trade->fill_date      = dt.date;
    trade->fill_time      = dt.time;
    trade->fill_no        = fill_no;
    trade->order_id       = order->order_id;

    m_tdata->trades[trade->fill_no] = trade;
    m_trade_ind_list.push_back(make_shared<Trade>(*trade));
}

bool SimAccount::check_quote_time(const MarketQuote* quote, const Order* order)
{
    DateTime entrust_time(order->entrust_date, order->entrust_time);
    if (DateTime(quote->date, quote->time).cmp(entrust_time) > 0)
        return true;

    const char* p = strchr(order->code.c_str(), '.');
    if (p == nullptr) return true;
    p++;

    if (strcmp(p, "SH") == 0 || strcmp(p, "SZ") == 0) {
        return m_ctx->cur_time().sub(entrust_time) >= seconds(5);
    }
    else {
        // 按照500毫秒对齐，然后保证当前时间和下单时间中有一个行情
        // 如果没有新行情，假设经过一个tick行情没有变化，可以撮合
        DateTime cur_time = m_ctx->cur_time();
        DateTime a(cur_time.date, (cur_time.time / 500)*500); 
        DateTime b(order->entrust_date, (order->entrust_time / 500) * 500);
        return a.sub(b) >= milliseconds(500);
    }
}

const bool double_equal(double v1, double v2)
{
    return fabs(v1 - v2) < 0.000001;
}

void SimAccount::try_buy(OrderData* od)
{
    if (m_ctx->data_level() == BT_TICK) {
        auto q = m_ctx->data_api()->quote(od->order->code.c_str());

        if (double_equal(q.value->ask1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach high limit");
            return;
        }

        if (q.value &&
            (q.value->ask1 <= od->order->entrust_price || od->price_type=="any") &&
            check_quote_time(q.value.get(), od->order.get()))
        {
            make_trade(od->order.get(), q.value->ask1);
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (bar && bar->low < od->order->entrust_price) {
            double fill_price = min(od->order->entrust_price, bar->high);
            make_trade(od->order.get(), fill_price);
        }
        else if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
    }
    else {
        assert(false);
    }
}

void SimAccount::try_sell(OrderData* od)
{
    if (m_ctx->data_level() == BT_TICK) {
        auto q = m_ctx->data_api()->quote(od->order->code.c_str());

        if (double_equal(q.value->bid1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach low limit");
            return;
        }

        if (q.value && 
            (q.value->bid1 >= od->order->entrust_price || od->price_type == "any") &&
            check_quote_time(q.value.get(), od->order.get()))
        {
            make_trade(od->order.get(), q.value->bid1);
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (bar && bar->high > od->order->entrust_price) {
            double fill_price = max(od->order->entrust_price, bar->low);
            make_trade(od->order.get(), fill_price);
        }
        else if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
    }
    else {
        assert(false);
    }
}

void SimAccount::try_short(OrderData* od)
{
    // fixme
    if (m_ctx->data_level() == BT_TICK) {
        auto q = m_ctx->data_api()->quote(od->order->code.c_str());

        if (double_equal(q.value->bid1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach low limit");
            return;
        }

        if (q.value && 
            ( q.value->bid1 >= od->order->entrust_price || od->price_type == "any") &&
            check_quote_time(q.value.get(), od->order.get()))
        {
            make_trade(od->order.get(), q.value->bid1);
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (bar && bar->high > od->order->entrust_price) {
            double fill_price = max(od->order->entrust_price, bar->low);
            make_trade(od->order.get(), fill_price);
        }
        else if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
    }
    else {
        assert(false);
    }
}

void SimAccount::try_cover(OrderData* od)
{
    if (m_ctx->data_level() == BT_TICK) {
        auto q = m_ctx->data_api()->quote(od->order->code.c_str());

        if (double_equal(q.value->ask1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach low limit");
            return;
        }

        if (q.value &&
            (q.value->ask1 < od->order->entrust_price || od->price_type == "any") &&
            check_quote_time(q.value.get(), od->order.get()))
        {
            make_trade(od->order.get(), q.value->ask1);
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (bar && bar->low < od->order->entrust_price) {
            double fill_price = min(od->order->entrust_price, bar->high);
            make_trade(od->order.get(), fill_price);
        }
        else if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
    }
    else {
        assert(false);
    }
}

void SimAccount::move_to(int trading_day)
{
    if (m_tdata->trading_day == trading_day) return;

    if (m_tdata->trading_day == 0) {
        m_tdata->trading_day = trading_day;
        return;
    }

    auto tdata = make_shared<TradeData>();
    tdata->account_id     = m_tdata->account_id;
    tdata->init_balance   = m_tdata->enable_balance + m_tdata->frozen_balance;
    tdata->enable_balance = tdata->init_balance;
    tdata->trading_day    = trading_day;
    tdata->frozen_balance = 0.0;

    for (auto e : m_tdata->positions) {
        if (e.second->current_size == 0) continue;

        auto pos = e.second;
        auto new_pos = make_shared<Position>();
        *new_pos = *pos;
        new_pos->init_size    = pos->current_size;
        new_pos->enable_size  = pos->current_size;
        new_pos->current_size = pos->current_size;
        new_pos->today_size   = 0;
        new_pos->frozen_size  = 0;
        new_pos->commission   = 0.0;
        tdata->positions[e.first] = new_pos;
    }

    m_his_tdata.push_back(tdata);
    m_tdata = tdata;

    m_ord_status_ind_list.clear();
    m_trade_ind_list.clear();
}

void SimAccount::save_data(const string& dir)
{
    {
        stringstream ss;
        ss << dir << "/" << m_tdata->account_id << "-balance.csv";
        ofstream out;
        out.open(ss.str());
        if (!out.is_open()) {
            cerr << "Can't open file " << ss.str();
            return;
        }
        out << "account_id,trading_day,init_balance,enable_balance,frozen_balance\n";
        for (auto& tdata : m_his_tdata) {
            out << setprecision(4) << fixed
                << tdata->account_id << ","
                << tdata->trading_day << ","
                << tdata->init_balance << ","
                << tdata->enable_balance << ","
                << tdata->frozen_balance << endl;
        }
        out.close();
    }
    {
        stringstream ss;
        ss << dir << "/" << m_tdata->account_id << "-positions.csv";
        ofstream out;
        out.open(ss.str());
        if (!out.is_open()) {
            cerr << "Can't open file " << ss.str();
            return;
        }
        out << "account_id,trading_day,code,name,side,init_size,current_size,enable_size,frozen_size,today_size,"
            << "cost,cost_price,close_pnl,float_pnl,margin,commission\n";
        for (auto& tdata : m_his_tdata) {
            vector<shared_ptr<Position>> positions;
            for (auto& e : tdata->positions) positions.push_back(e.second);
            sort(positions.begin(), positions.end(), [](shared_ptr<Position> a, shared_ptr<Position> b) {
                char buf1[100]; sprintf(buf1, "%s-%s", a->code.c_str(), a->side.c_str());
                char buf2[100]; sprintf(buf2, "%s-%s", b->code.c_str(), b->side.c_str());
                return strcmp(buf1, buf2) < 0;
            });
            
            for (auto& pos : positions)
                out << setprecision(4) << fixed
                    << tdata->account_id << ","
                    << tdata->trading_day << ","
                    << pos->code << "," << pos->name << "," << pos->side << ","
                    << pos->init_size << "," << pos->current_size << ","
                    << pos->enable_size << "," << pos->frozen_size << "," << pos->today_size << ","
                    << pos->cost << "," << pos->cost_price << "," << pos->close_pnl << "," << pos->float_pnl << ","
                    << pos->margin << "," << pos->commission << endl;
        }
        out.close();
    }
    {
        stringstream ss;
        ss << dir << "/" << m_tdata->account_id << "-orders.csv";
        ofstream out;
        out.open(ss.str());
        if (!out.is_open()) {
            cerr << "Can't open file " << ss.str();
            return;
        }
        out << "account_id,trading_day,code,name,entrust_date,entrust_time,entrust_size,entrust_price,"
            << "entrust_action,fill_size,fill_price,status,status_msg\n";
        for (auto& tdata : m_his_tdata) {
            vector<shared_ptr<Order>> orders;
            for (auto& e : tdata->orders) orders.push_back(e.second->order);
            sort(orders.begin(), orders.end(), [](shared_ptr<Order> a, shared_ptr<Order> b) {
                return strcmp(a->entrust_no.c_str(), b->entrust_no.c_str()) < 0;
            });

            for (auto& ord : orders)
                out << setprecision(4) << fixed
                    << tdata->account_id << ","
                    << tdata->trading_day << ","
                    << ord->code << "," << ord->name << ","
                    << ord->entrust_date << "," << ord->entrust_time << ","
                    << ord->entrust_size << "," << ord->entrust_price << "," << ord->entrust_action << ","
                    << ord->fill_size << "," << ord->fill_price << ","
                    << ord->status << ",\"" << ord->status_msg << "\"" << endl;
        }
        out.close();
    }

    {
        stringstream ss;
        ss << dir << "/" << m_tdata->account_id << "-trades.csv";
        ofstream out;
        out.open(ss.str());
        if (!out.is_open()) {
            cerr << "Can't open file " << ss.str();
            return;
        }
        out << "account_id,trading_day,code,name,entrust_no,entrust_action,"
            << "fill_no,fill_date,fill_time,fill_size,fill_price\n";
        for (auto& tdata : m_his_tdata) {
            vector<shared_ptr<Trade>> trades;
            for (auto& e : tdata->trades) trades.push_back(e.second);
            sort(trades.begin(), trades.end(), [](shared_ptr<Trade> a, shared_ptr<Trade> b) {
                return strcmp(a->fill_no.c_str(), b->fill_no.c_str()) < 0;
            });

            for (auto& trd : trades)
                out << setprecision(4) << fixed
                    << tdata->account_id << ","
                    << tdata->trading_day << ","
                    << trd->code << "," << trd->name << ","
                    << trd->entrust_no << "," << trd->entrust_action << ","
                    << trd->fill_no << "," << trd->fill_date << "," << trd->fill_time << ","
                    << trd->fill_size << "," << trd->fill_price << endl;
        }
        out.close();
    }
}
