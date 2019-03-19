#include <assert.h>
#include <string.h>
#include <math.h>
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
#include "myutils/stringutils.h"
#include "myutils/misc.h"

using namespace tquant::api;
using namespace tquant::stralet;
using namespace tquant::stralet::backtest;


int32_t SimAccount::g_fill_id = 0;
int32_t SimAccount::g_order_id = 0;

static bool is_finished_status(const string& status)
{
    return status == OS_Filled || status == OS_Rejected || status == OS_Cancelled;
}

static void get_action_effect(const string& action, string* pos_side, int* size_inc_dir)
{
    if (action == EA_Buy) {
        *pos_side = SD_Long;
        *size_inc_dir = 1;
    }
    else if (action == EA_Sell ||
        action == EA_SellToday || action == EA_SellYesterday) {
        *pos_side = SD_Long;
        *size_inc_dir = -1;
    }
    else if (action == EA_Short) {
        *pos_side = SD_Short;
        *size_inc_dir = 1;
    }
    else {
        *pos_side = SD_Short;
        *size_inc_dir = -1;
    }
}


struct CodeInfo {
    string code;
    string name;
    string mkt;
    string product_id;
    string product_class; // "Futurs", "Options" ???
    double volume_multiple;
    double price_tick;
    double margin_ratio;
};

shared_ptr<CodeInfo> get_code_info(const string& code);

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

CallResult<const vector<Order>> SimTradeApi::query_orders(const string& account_id, const unordered_set<string>* codes)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->query_orders(codes) :
        CallResult<const vector<Order>>("-1,no such account");
}

CallResult<const vector<Order>> SimTradeApi::query_orders(const string& account_id, const string& codes)
{
    unordered_set<string> code_set;
    vector<string> ss;    
    split(codes, ",", &ss);
    for (auto&s : ss) if (s.size()) code_set.insert(s);

    auto act = m_ctx->get_account(account_id);
    return act ? act->query_orders(&code_set) :
        CallResult<const vector<Order>>("-1,no such account");
}

CallResult<const vector<Trade>> SimTradeApi::query_trades(const string& account_id, const unordered_set<string>* codes)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->query_trades(codes) :
        CallResult<const vector<Trade>>("-1,no such account");
}

CallResult<const vector<Trade>> SimTradeApi::query_trades(const string& account_id, const string& codes)
{
    unordered_set<string> code_set;
    vector<string> ss;
    split(codes, ",", &ss);
    for (auto&s : ss) if (s.size()) code_set.insert(s);

    auto act = m_ctx->get_account(account_id);
    return act ? act->query_trades(&code_set) :
        CallResult<const vector<Trade>>("-1,no such account");
}

CallResult<const vector<Position>> SimTradeApi::query_positions(const string& account_id, const unordered_set<string>* codes)
{
    auto act = m_ctx->get_account(account_id);
    return act ? act->query_positions(codes) :
        CallResult<const vector<Position>>("-1,no such account");
}

CallResult<const vector<Position>> SimTradeApi::query_positions(const string& account_id, const string& codes)
{
    unordered_set<string> code_set;
    vector<string> ss;
    split(codes, ",", &ss);
    for (auto&s : ss) if (s.size()) code_set.insert(s);

    auto act = m_ctx->get_account(account_id);
    return act ? act->query_positions(&code_set) :
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

void SimTradeApi::update_last_prices()
{
    for (auto& e : m_accounts) { e.second->update_last_prices(); }
}

SimAccount::SimAccount(SimStraletContext* ctx, const string& account_id,
                       double init_balance,
                       const vector<Holding> & holdings)
{
    m_ctx = ctx;
    auto tdata = make_shared<TradeData>();
    tdata->account_id     = account_id;
    tdata->init_balance   = init_balance;
    tdata->avail_balance = init_balance;
    tdata->frozen_balance = 0.0;
    tdata->trading_day    = 0;

    for (auto& h : holdings) {
        auto pos = make_shared<Position>();
        pos->account_id  = account_id;
        pos->code        = h.code;
        pos->enable_size = pos->current_size = pos->init_size = h.size;
        pos->cost_price  = h.cost_price;
        pos->cost        = h.cost_price * h.size;
        pos->side        = h.side;
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
    bal->enable_balance = m_tdata->avail_balance - m_tdata->frozen_balance;
    //bal->margin = m_margin;
    //bal->float_pnl = m_float_pnl;
    //bal->close_pnl = m_close_pnl;

    return CallResult<const Balance>(bal);
}

CallResult<const vector<Order>> SimAccount::query_orders(const unordered_set<string>* codes)
{
	vector<const Order*> orders(m_tdata->orders.size());
    size_t count = 0;
    for (auto & e : m_tdata->orders) {
        if (!codes || codes->empty() || codes->find(e.second->order->code) != codes->end())
            orders[count++] = e.second->order.get();
    }
    orders.resize(count);

    sort(orders.begin(), orders.end(), [](const Order* a, const Order* b) {
        return a->entrust_no < b->entrust_no;
    });

	auto ret_orders = make_shared<vector<Order>>(count);
    for (size_t i = 0; i < orders.size(); i++)
        (*ret_orders)[i] = *orders[i];

	return CallResult<const vector<Order>>(ret_orders);
}

CallResult<const vector<Trade>> SimAccount::query_trades(const unordered_set<string>* codes)
{
	vector<const Trade*> trades(m_tdata->trades.size());

    size_t count = 0;
    for (auto & e : m_tdata->trades) {
        if (!codes || codes->empty() || codes->find(e.second->code) != codes->end()) {
            trades[count++] = e.second.get();
        }
    }
    trades.resize(count);

    sort(trades.begin(), trades.end(), [](const Trade* a, const Trade* b) {
        return a->fill_no < b->fill_no;
    });

	auto ret_trades = make_shared<vector<Trade>>(count);
    for (size_t i = 0; i < count; i++)
        (*ret_trades)[i] = *trades[i];

	return CallResult<const vector<Trade>>(ret_trades);
}

CallResult<const vector<Position>> SimAccount::query_positions(const unordered_set<string>* codes)
{
	vector<const Position*> positions(m_tdata->positions.size());
    size_t count = 0;
    for (auto & e : m_tdata->positions) {
        if (!codes || codes->empty() || codes->find(e.second->code) != codes->end())
            positions[count++] = e.second.get();
    }
    positions.resize(count);

    sort(positions.begin(), positions.end(), [](const Position *a, const Position* b) {
        return a->code < b->code;
    });

	auto ret_value = make_shared<vector<Position>>(count);
    for (size_t i = 0; i < count; i++)
        (*ret_value)[i] = *positions[i];

	return CallResult<const vector<Position>>(ret_value);
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
    if (mkt == "SH" || mkt == "SZ") {
        is_open_time =
            ((dt.time >= HMS(9, 30) && dt.time < HMS(11, 30)) ||
             (dt.time >= HMS(13, 0) && dt.time < HMS(15, 00)));
    }
    else if (mkt == "CFE") {
        if (code[0] == 'T') {
            is_open_time =
                ((dt.time >= HMS(9, 15) && dt.time < HMS(11, 30)) ||
                (dt.time >= HMS(13, 0) && dt.time < HMS(15, 15)));
        }
        else {
            is_open_time =
                ((dt.time >= HMS(9, 30) && dt.time < HMS(11, 30)) ||
                (dt.time >= HMS(13, 0) && dt.time < HMS(15, 0)));
        }
    }
    else {
        is_open_time =
            ((dt.time >= HMS( 9,  0) && dt.time < HMS(10, 15)) ||
             (dt.time >= HMS(10, 30) && dt.time < HMS(11, 30)) ||
             (dt.time >= HMS(13, 30) && dt.time < HMS(15, 00)) ||
             (dt.date < m_ctx->trading_day() && dt.time > HMS(21,0)) ||
             (dt.time < HMS(2,0)));
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
            ss << "-1,no enough size for close(enable " << pos->enable_size << ", frozen "
                << pos->frozen_size << ", close " << size << ")";
            return CallResult<const OrderID>(ss.str());
        }
        // XXX move to place_order
        pos->frozen_size += size;
    }
    else {
        if (price*size > m_tdata->avail_balance - m_tdata->frozen_balance)
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

    auto code_info = get_code_info(code);
    if (!code_info && r.value) {
        r.value = nullptr;
        r.msg = string("Can't find code info for ") + code;
    }

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
    od->order           = order;
    od->price_type      = price_type;
    od->last_volume     = 0;
    od->last_turnover   = 0.0;
    od->volume_in_queue = 1e8;

    if (code_info) {
        od->volume_multiple = code_info->volume_multiple;
        od->price_tick      = code_info->price_tick;
    }
    else {
        od->volume_multiple = 1.0;
        od->price_tick      = 1.0;
    }

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

static bool is_futures_or_index(const char* code)
{
    const char*p = strrchr(code, '.');
    if (!p) return false;
    p++;

    if (strcmp(p, "SH") == 0)
        return strncmp(code, "000", 3) == 0;
    else if (strcmp(p, "SZ") == 0)
        return strncmp(code, "399", 3) == 0;
    else
        return true;
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
    if (inc_dir == 1)
        m_tdata->frozen_balance -= order->entrust_size * order->entrust_price;
    else
        pos->frozen_size -= order->entrust_size;

    order->fill_price = 0;
    order->fill_size = 0;
    order->status = OS_Rejected;
    order->status_msg = msg;

    // Must make a copy!
    m_ord_status_ind_list.push_back(make_shared<Order>(*order));
    return true;
}

static void get_commission_rate(const char* code, double* open_rate, double* close_rate)
{
    const char* p = strchr(code, '.');
    if (strcmp(p, ".SH") == 0 || strcmp(p, ".SZ") == 0) {

        switch (code[0]) {
        case '1':
        case '5':
            *open_rate = 0.00025;
            *close_rate = 0.00025;
            break;
        default:
            *open_rate = 0.00025;
            *close_rate = 0.00125;
        }
    }
    //else if (strcmp(p, ".CFE") == 0) {
    //    if (code[0] == 'I') {
    //        *open_rate = 0.000025;
    //        *close_rate = 0.000025;
    //    }
    //}
    else {
        *open_rate  = 0.0000;
        *close_rate = 0.0000;
    }
    return;
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

    // Stock 
    //    cost = turnover + commission
    //    cost_price = cost / current_size
    // Futures
    //    cost == margin = turnover
    //    cost_price = cost /current_size
    // 
    //  avail += -turnover - commission + profit
    // Commission is always removed from avail_balance.

    if (inc_dir == 1) {
        double open_rate = 0.0, close_rate = 0.0;
        get_commission_rate(order->code.c_str(), &open_rate, &close_rate);

        double turnover = fill_price * fill_size;
        double commisson = turnover *open_rate;

        pos->current_size += fill_size;

        if (is_futures_or_index(order->code.c_str())) {
            pos->enable_size += fill_size;
            pos->cost        += turnover;
            pos->cost_price   = pos->cost / pos->current_size;
        }
        else {
            pos->cost       += turnover + commisson;
            pos->cost_price  = pos->cost / pos->current_size;
        }

        pos->commission   += commisson;

        m_tdata->frozen_balance -= order->entrust_size * order->entrust_price;
        m_tdata->avail_balance  -= turnover + commisson;
        m_tdata->commission     += commisson;
    }
    else {
        pos->frozen_size  -= order->entrust_size;
        pos->current_size -= fill_size;
        pos->enable_size  -= fill_size; // TODO: check if it is right
        assert(pos->enable_size >= 0);

        double open_rate = 0.0;
        double close_rate = 0.0;
        get_commission_rate(order->code.c_str(), &open_rate, &close_rate);

        double turnover = 0;
        double commission = 0;
        if (is_futures_or_index(order->code.c_str())) {
            // turnover is money payed to account!
            if (pos->side == SD_Short) {
                turnover = fill_size * (2 * pos->cost_price - fill_price);
                pos->close_pnl += fill_size * (pos->cost_price - fill_price) - commission;
            }
            else {
                turnover = fill_size * fill_price;
                pos->close_pnl += fill_size * (fill_price - pos->cost_price) - commission;
            }
            commission = fill_size* fill_price *close_rate;
            
        } else {
            turnover = fill_size* fill_price;
            commission = turnover *close_rate;
            pos->close_pnl += fill_size * (fill_price - pos->cost_price) - commission;
        }
        pos->commission += commission;
        
        if (pos->current_size) {
            pos->cost = pos->cost_price * pos->current_size;
        } else {
            pos->cost = 0.0;
            pos->cost_price = 0.0;
        }

        m_tdata->avail_balance += turnover - commission;
        m_tdata->commission += commission;
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

bool is_buy_action(const string& action)
{
    return
        action == EA_Buy ||
        action == EA_Cover ||
        action == EA_CoverToday ||
        action == EA_CoverYesterday;
}

void SimAccount::estimate_vol_in_queue(OrderData* od, const MarketQuote* q)
{
    bool is_buy = is_buy_action(od->order->entrust_action);

    if (od->last_volume == 0) {
        if (is_buy) {
            if (od->order->entrust_price >= q->ask1) {
                od->volume_in_queue = 0;
                return;
            }
            else if (od->order->entrust_price < q->bid1) {
                return;
            }
        }
        else {
            if (od->order->entrust_price <= q->bid1) {
                od->volume_in_queue = 0;
                return;
            }
            else if (od->order->entrust_price > q->ask1) {
                return;
            }
        }

        od->last_volume   = q->volume;
        od->last_turnover = q->turnover;
        if (is_buy)
            od->volume_in_queue = /*q->bid_vol1 > 6 ? q->bid_vol1 * 2 / 3 :*/ q->bid_vol1;
        else
            od->volume_in_queue = /*q->ask_vol1 > 6 ? q->ask_vol1 * 2 / 3 :*/ q->ask_vol1;
        od->volume_in_queue += od->order->entrust_size;
        assert(od->volume_in_queue);
        return;
    }

    size_t filled_volume = q->volume - od->last_volume;
    double turnvoer      = q->turnover - od->last_turnover;
    if (!filled_volume) return;

    od->last_volume   = q->volume;
    od->last_turnover = q->turnover;

    double volume_multiple = od->volume_multiple;
    double price_tick      = od->price_tick;

    double avg_px = turnvoer / filled_volume / volume_multiple;    
    
    // 估计逻辑: 以 Buy为例
    //   当 entrust_price >= ask，buy队列中全部成交
    //   当 entrust_price < bid, 不能估算成交数量，因此buy_queue保持不变
    //   当 entrust_price == bid
    //      如果 avg_px > ask, 不能估算成交数量，因此buy_queue保持不变
    //      如果 avg_px < bid, buy队列全部成交
    //      如果 avg_px 在 (bid, bid+price_tick)之间，按比例算出 以小于等于avg_px的成交的数量，从 buy_queue中减去
    //
    if (is_buy) {
        if (od->order->entrust_price >= q->ask1) {
            od->volume_in_queue = 0;
        }
        else if (od->order->entrust_price < q->bid1) {
            // pass
        }
        else {
            if (avg_px < q->bid1) {
                od->volume_in_queue = 0;
            }
            else if (avg_px <= q->bid1 + price_tick) {
                double down_px = q->bid1;
                double up_px   = down_px + price_tick;
                double down_r = (up_px - avg_px) / price_tick;
                filled_volume *= down_r;
                if (down_r > 0) {
                    if (filled_volume == 0) filled_volume = 1;
                    if (filled_volume < od->volume_in_queue)
                        od->volume_in_queue -= filled_volume;
                    else
                        od->volume_in_queue = 0;
                }
                if (od->volume_in_queue > q->bid_vol1)
                    od->volume_in_queue = q->bid_vol1;
            }
        }
    }
    else {
        if (od->order->entrust_price <= q->bid1) {
            od->volume_in_queue = 0;
        }
        else if (od->order->entrust_price > q->ask1) {
            // pass
        }
        else {
            if (avg_px > q->ask1) {
                od->volume_in_queue = 0;
            }
            else if (avg_px >= q->ask1 - price_tick) {
                double up_px   = q->ask1;
                double down_px = up_px - price_tick;
                double up_r    = (avg_px - down_px) / price_tick;
                filled_volume *= up_r;
                if (up_r > 0) {
                    if (filled_volume == 0) filled_volume = 1;
                    if (filled_volume < od->volume_in_queue)
                        od->volume_in_queue -= filled_volume;
                    else
                        od->volume_in_queue = 0;
                }
                if (od->volume_in_queue > q->ask_vol1)
                    od->volume_in_queue = q->ask_vol1;
            }
        }
    }
}

void SimAccount::try_buy(OrderData* od)
{
    if (m_ctx->data_level() == BT_TICK) {
        auto q = m_ctx->data_api()->quote(od->order->code.c_str()).value;
        if (!q) return;

        if (double_equal(q->ask1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach high limit");
            return;
        }

        estimate_vol_in_queue(od, q.get());

        if (check_quote_time(q.get(), od->order.get())) {
            if (od->price_type == "any" && q->ask_vol1 > 0) {
                make_trade(od->order.get(), q->ask1);
            }
            else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
                const char* p = od->price_type.c_str() + 8;
                if (*p && *p == ':') {
                    p++;
                    uint32_t rate = atof(p) * 1000;
                    uint32_t t = ((uint32_t)myutils::random()) % 1000;
                    if (t > rate) {
                        reject_order(od->order.get(), "reject any_test:xxx");
                        return;
                    }
                }
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (q->last <= od->order->entrust_price && od->volume_in_queue == 0) {
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (od->price_type == "fak" || od->price_type == "fok") {
                reject_order(od->order.get(), od->price_type.c_str());
            }
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (!bar || !bar->high || !bar->low || !bar->volume) return;

        if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
        else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
            const char* p = od->price_type.c_str() + 8;
            if (*p && *p == ':') {
                p++;
                uint32_t rate = atof(p) * 1000;
                uint32_t t = ((uint32_t)myutils::random()) % 1000;
                if (t > rate) {
                    reject_order(od->order.get(), "reject any_test:xxx");
                    return;
                }
            }
            make_trade(od->order.get(), od->order->entrust_price);
        }
        else if (bar && bar->low < od->order->entrust_price) {
            double fill_price = min(od->order->entrust_price, bar->high);
            make_trade(od->order.get(), fill_price);
        }
        else if (od->price_type == "fak" || od->price_type == "fok") {
            reject_order(od->order.get(), od->price_type.c_str());
        }
    }
    else {
        assert(false);
    }
}

void SimAccount::try_sell(OrderData* od)
{
    if (m_ctx->data_level() == BT_TICK) {
        auto q = m_ctx->data_api()->quote(od->order->code.c_str()).value;
        if (!q) return;

        if (double_equal(q->bid1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach low limit");
            return;
        }

        estimate_vol_in_queue(od, q.get());

        if (check_quote_time(q.get(), od->order.get())) {
            if (od->price_type == "any" && q->bid_vol1 > 0) {
                make_trade(od->order.get(), q->bid1);
            }
            else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
                const char* p = od->price_type.c_str() + 8;
                if (*p && *p == ':') {
                    p++;
                    uint32_t rate = atof(p) * 1000;
                    uint32_t t = ((uint32_t)myutils::random()) % 1000;
                    if (t > rate) {
                        reject_order(od->order.get(), "reject any_test:xxx");
                        return;
                    }
                }
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (q->last >= od->order->entrust_price && od->volume_in_queue == 0) {
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (od->price_type == "fak" || od->price_type == "fok") {
                reject_order(od->order.get(), od->price_type.c_str());
            }
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (!bar || !bar->high || !bar->low || !bar->volume) return;

        if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
        else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
            const char* p = od->price_type.c_str() + 8;
            if (*p && *p == ':') {
                p++;
                uint32_t rate = atof(p) * 1000;
                uint32_t t = ((uint32_t)myutils::random()) % 1000;
                if (t > rate) {
                    reject_order(od->order.get(), "reject any_test:xxx");
                    return;
                }
            }
            make_trade(od->order.get(), od->order->entrust_price);
        }
        else if (bar && bar->high > od->order->entrust_price) {
            double fill_price = max(od->order->entrust_price, bar->low);
            make_trade(od->order.get(), fill_price);
        }
        else if (od->price_type == "fak" || od->price_type == "fok") {
            reject_order(od->order.get(), od->price_type.c_str());
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
        auto q = m_ctx->data_api()->quote(od->order->code.c_str()).value;
        if (!q) return;

        if (double_equal(q->bid1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach low limit");
            return;
        }

        estimate_vol_in_queue(od, q.get());

        if (check_quote_time(q.get(), od->order.get())) {
            if (od->price_type == "any" && q->bid_vol1 > 0) {
                make_trade(od->order.get(), q->bid1);
            }
            else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
                const char* p = od->price_type.c_str() + 8;
                if (*p && *p == ':') {
                    p++;
                    uint32_t rate = atof(p) * 1000;
                    uint32_t t = ((uint32_t)myutils::random()) % 1000;
                    if (t > rate) {
                        reject_order(od->order.get(), "reject any_test:xxx");
                        return;
                    }
                }
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (q->last >= od->order->entrust_price && od->volume_in_queue == 0) {
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (od->price_type == "fak" || od->price_type == "fok") {
                reject_order(od->order.get(), od->price_type.c_str());
            }
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (!bar || !bar->high || !bar->low || !bar->volume) return;

        if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
        else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
            const char* p = od->price_type.c_str() + 8;
            if (*p && *p == ':') {
                p++;
                uint32_t rate = atof(p) * 1000;
                uint32_t t = ((uint32_t)myutils::random()) % 1000;
                if (t > rate) {
                    reject_order(od->order.get(), "reject any_test:xxx");
                    return;
                }
            }
            make_trade(od->order.get(), od->order->entrust_price);
        }
        else if (bar && bar->high > od->order->entrust_price) {
            double fill_price = max(od->order->entrust_price, bar->low);
            make_trade(od->order.get(), fill_price);
        }
        else if (od->price_type == "fak" || od->price_type == "fok") {
            reject_order(od->order.get(), od->price_type.c_str());
        }
    }
    else {
        assert(false);
    }
}

void SimAccount::try_cover(OrderData* od)
{
    if (m_ctx->data_level() == BT_TICK) {
        auto q = m_ctx->data_api()->quote(od->order->code.c_str()).value;
        if (!q) return;

        if (double_equal(q->ask1, 0.0) && od->price_type == "any") {
            reject_order(od->order.get(), "reach low limit");
            return;
        }

        estimate_vol_in_queue(od, q.get());

        if (check_quote_time(q.get(), od->order.get())) {
            if (od->price_type == "any" && q->ask_vol1 > 0) {
                make_trade(od->order.get(), q->ask1);
            }
            else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
                const char* p = od->price_type.c_str() + 8;
                if (*p && *p == ':') {
                    p++;
                    uint32_t rate = atof(p) * 1000;
                    uint32_t t = ((uint32_t)myutils::random()) % 1000;
                    if (t > rate) {
                        reject_order(od->order.get(), "reject any_test:xxx");
                        return;
                    }
                }
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (q->last <= od->order->entrust_price && od->volume_in_queue == 0) {
                make_trade(od->order.get(), od->order->entrust_price);
            }
            else if (od->price_type == "fak" || od->price_type == "fok") {
                reject_order(od->order.get(), od->price_type.c_str());
            }
        }
    }
    else if (m_ctx->data_level() == BT_BAR1M) {
        auto bar = m_ctx->sim_dapi()->last_bar(od->order->code.c_str());
        if (!bar || !bar->high || !bar->low || !bar->volume) return;

        if (bar && od->price_type == "any") {
            double fill_price = (bar->high + bar->low) / 2;
            make_trade(od->order.get(), fill_price);
        }
        else if (strncmp(od->price_type.c_str(), "any_test", 8) == 0) {
            const char* p = od->price_type.c_str() + 8;
            if (*p && *p == ':') {
                p++;
                uint32_t rate = atof(p) * 1000;
                uint32_t t = ((uint32_t)myutils::random()) % 1000;
                if (t > rate) {
                    reject_order(od->order.get(), "reject any_test:xxx");
                    return;
                }
            }
            make_trade(od->order.get(), od->order->entrust_price);
        }
        else if (bar && bar->low < od->order->entrust_price) {
            double fill_price = min(od->order->entrust_price, bar->high);
            make_trade(od->order.get(), fill_price);
        }
        else if (od->price_type == "fak" || od->price_type == "fok") {
            reject_order(od->order.get(), od->price_type.c_str());
        }
    }
    else {
        assert(false);
    }
}

void SimAccount::update_last_prices()
{
    auto dapi = m_ctx->data_api();
    for (auto& e : m_tdata->positions) {
        auto q = dapi->quote(e.second->code).value;
        if (q)
            e.second->last_price = q->last;
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
    tdata->init_balance   = m_tdata->avail_balance;
    tdata->avail_balance = tdata->init_balance;
    tdata->trading_day    = trading_day;
    tdata->frozen_balance = 0.0;

    for (auto e : m_tdata->positions) {
        if (e.second->current_size == 0) continue;

        auto pos = e.second;
        auto new_pos = make_shared<Position>();
        new_pos->account_id   = pos->account_id;
        new_pos->code         = pos->code;
        new_pos->name         = pos->name;
        new_pos->init_size    = pos->current_size;
        new_pos->enable_size  = pos->current_size;
        new_pos->current_size = pos->current_size;
        new_pos->side         = pos->side;
        new_pos->cost         = pos->cost;
        new_pos->cost_price   = pos->cost_price;
        new_pos->margin       = pos->margin;

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
        out << "account_id,trading_day,init_balance,avail_balance,frozen_balance\n";
        for (auto& tdata : m_his_tdata) {
            out << setprecision(4) << fixed
                << tdata->account_id << ","
                << tdata->trading_day << ","
                << tdata->init_balance << ","
                << tdata->avail_balance << ","
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
            << "cost,cost_price,close_pnl,float_pnl,margin,commission,last_price\n";
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
                    << pos->margin << "," << pos->commission  << "," << pos->last_price
                    << endl;
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
        out << "account_id,trading_day,code,name,entrust_no,order_id,"
            << "entrust_date,entrust_time,entrust_size,entrust_price,"
            << "entrust_action,fill_size,fill_price,status,status_msg,\n";
        for (auto& tdata : m_his_tdata) {
            vector<shared_ptr<Order>> orders;
            for (auto& e : tdata->orders) orders.push_back(e.second->order);
            sort(orders.begin(), orders.end(), [](shared_ptr<Order> a, shared_ptr<Order> b) {
                return a->entrust_no < b->entrust_no;
            });

            for (auto& ord : orders)
                out << setprecision(4) << fixed
                    << tdata->account_id << ","
                    << tdata->trading_day << ","
                    << ord->code << "," << ord->name << ","
                    << ord->entrust_no << "," << ord->order_id << ","
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


shared_ptr<CodeInfo> get_code_info(const string& code)
{
   static vector<CodeInfo> g_contracts {
        { "A.DCE",  "A.DCE", "DCE", "A.DCE", "Futures", 10, 1.0, 0.1 },
        { "AG.SHF", "AG.SHF", "SHF", "AG.SHF", "Futures", 15, 1.0, 0.1 },
        { "AL.SHF", "AL.SHF", "SHF", "AL.SHF", "Futures", 5, 5.0, 0.1 },
        { "AP.CZC", "AP.CZC", "CZC", "AP.CZC", "Futures", 10, 1.0, 0.1 },
        { "AU.SHF", "AU.SHF", "SHF", "AU.SHF", "Futures", 1000, 0.05, 0.1 },
        { "B.DCE", "B.DCE", "DCE", "B.DCE", "Futures", 10, 1.0, 0.1 },
        { "BB.DCE", "BB.DCE", "DCE", "BB.DCE", "Futures", 500, 0.05, 0.1 },
        { "BU.SHF", "BU.SHF", "SHF", "BU.SHF", "Futures", 10, 2.0, 0.1 },
        { "C.DCE", "C.DCE", "DCE", "C.DCE", "Futures", 10, 1.0, 0.1 },
        { "CF.CZC", "CF.CZC", "CZC", "CF.CZC", "Futures", 5, 5.0, 0.1 },
        { "CS.DCE", "CS.DCE", "DCE", "CS.DCE", "Futures", 10, 1.0, 0.1 },
        { "CU.SHF", "CU.SHF", "SHF", "CU.SHF", "Futures", 5, 10.0, 0.1 },
        { "CY.CZC", "CY.CZC", "CZC", "CY.CZC", "Futures", 5, 5.0, 0.1 },
        { "FB.DCE", "FB.DCE", "DCE", "FB.DCE", "Futures", 500, 0.05, 0.1 },
        { "FG.CZC", "FG.CZC", "CZC", "FG.CZC", "Futures", 20, 1.0, 0.1 },
        { "FU.SHF", "FU.SHF", "SHF", "FU.SHF", "Futures", 10, 1.0, 0.1 },
        { "HC.SHF", "HC.SHF", "SHF", "HC.SHF", "Futures", 10, 1.0, 0.1 },
        { "I.DCE", "I.DCE", "DCE", "I.DCE", "Futures", 100, 0.5, 0.1 },
        { "IC.CFE", "IC.CFE", "CFE", "IC.CFE", "Futures", 200, 0.2, 0.1 },
        { "IF.CFE", "IF.CFE", "CFE", "IF.CFE", "Futures", 300, 0.2, 0.1 },
        { "IH.CFE", "IH.CFE", "CFE", "IH.CFE", "Futures", 300, 0.2, 0.1 },
        { "J.DCE", "J.DCE", "DCE", "J.DCE", "Futures", 100, 0.5, 0.1 },
        { "JD.DCE", "JD.DCE", "DCE", "JD.DCE", "Futures", 10, 1.0, 0.1 },
        { "JM.DCE", "JM.DCE", "DCE", "JM.DCE", "Futures", 60, 0.5, 0.1 },
        { "JR.CZC", "JR.CZC", "CZC", "JR.CZC", "Futures", 20, 1.0, 0.1 },
        { "L.DCE", "L.DCE", "DCE", "L.DCE", "Futures", 5, 5.0, 0.1 },
        { "LR.CZC", "LR.CZC", "CZC", "LR.CZC", "Futures", 20, 1.0, 0.1 },
        { "M.DCE", "M.DCE", "DCE", "M.DCE", "Futures", 10, 1.0, 0.1 },
        { "MA.CZC", "MA.CZC", "CZC", "MA.CZC", "Futures", 10, 1.0, 0.1 },
        { "NI.SHF", "NI.SHF", "SHF", "NI.SHF", "Futures", 1, 10.0, 0.1 },
        { "OI.CZC", "OI.CZC", "CZC", "OI.CZC", "Futures", 10, 1.0, 0.1 },
        { "P.DCE", "P.DCE", "DCE", "P.DCE", "Futures", 10, 2.0, 0.1 },
        { "PB.SHF", "PB.SHF", "SHF", "PB.SHF", "Futures", 5, 5.0, 0.1 },
        { "PM.CZC", "PM.CZC", "CZC", "PM.CZC", "Futures", 50, 1.0, 0.1 },
        { "PP.DCE", "PP.DCE", "DCE", "PP.DCE", "Futures", 5, 1.0, 0.1 },
        { "RB.SHF", "RB.SHF", "SHF", "RB.SHF", "Futures", 10, 1.0, 0.1 },
        { "RI.CZC", "RI.CZC", "CZC", "RI.CZC", "Futures", 20, 1.0, 0.1 },
        { "RM.CZC", "RM.CZC", "CZC", "RM.CZC", "Futures", 10, 1.0, 0.1 },
        { "RS.CZC", "RS.CZC", "CZC", "RS.CZC", "Futures", 10, 1.0, 0.1 },
        { "RU.SHF", "RU.SHF", "SHF", "RU.SHF", "Futures", 10, 5.0, 0.1 },
        { "SC.INE", "SC.INE", "INE", "SC.INE", "Futures", 1000, 0.1, 0.1 },
        { "SF.CZC", "SF.CZC", "CZC", "SF.CZC", "Futures", 5, 2.0, 0.1 },
        { "SM.CZC", "SM.CZC", "CZC", "SM.CZC", "Futures", 5, 2.0, 0.1 },
        { "SN.SHF", "SN.SHF", "SHF", "SN.SHF", "Futures", 1, 10.0, 0.1 },
        { "SR.CZC", "SR.CZC", "CZC", "SR.CZC", "Futures", 10, 1.0, 0.1 },
        { "T.CFE", "T.CFE", "CFE", "T.CFE", "Futures", 10000, 0.005, 0.1 },
        { "TA.CZC", "TA.CZC", "CZC", "TA.CZC", "Futures", 5, 2.0, 0.1 },
        { "TF.CFE", "TF.CFE", "CFE", "TF.CFE", "Futures", 10000, 0.005, 0.1 },
        { "TS.CFE", "TS.CFE", "CFE", "TS.CFE", "Futures", 20000, 0.005, 0.1 },
        { "V.DCE", "V.DCE", "DCE", "V.DCE", "Futures", 5, 5.0, 0.1 },
        { "WH.CZC", "WH.CZC", "CZC", "WH.CZC", "Futures", 20, 1.0, 0.1 },
        { "Y.DCE", "Y.DCE", "DCE", "Y.DCE", "Futures", 10, 2.0, 0.1 },
        { "ZC.CZC", "ZC.CZC", "CZC", "ZC.CZC", "Futures", 100, 0.2, 0.1 },
        { "ZN.SHF", "ZN.SHF", "SHF", "ZN.SHF", "Futures", 5, 5.0, 0.1 }
    };

    static unordered_map<string, shared_ptr<CodeInfo>> g_code_map;
    if (g_code_map.empty()) {
       for (auto& c : g_contracts)
           g_code_map[c.code] = make_shared<CodeInfo>(c);
    }

    const char* p = strchr(code.c_str(), '.');
    assert(p);
    if (strcmp(p, ".SH") == 0 || strcmp(p, ".SZ")==0) {
        auto info = make_shared<CodeInfo>();
        info->code = code;
        info->name = code;
        info->margin_ratio = 1;
        info->mkt = p + 1;
        info->price_tick = 0.001;
        info->volume_multiple = 1.0;
        return info;
    }
    else {
        const char* p1 = code.c_str();
        while (isalpha(*p1)) p1++;
        // map contract to product code
        if (*p1 != '.') {
            string new_code = code.substr(0, p1 - code.c_str()) + string(p);
            auto it = g_code_map.find(new_code);
            return it != g_code_map.end() ? it->second : nullptr;
        }
        else {
            auto it = g_code_map.find(code);
            return it != g_code_map.end() ? it->second : nullptr;
        }
    }
}
