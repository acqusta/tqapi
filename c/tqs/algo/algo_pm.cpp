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
#include "algo_pm.h"


namespace tquant { namespace stra {

    using namespace tquant::api;

    static inline int HMS(int h, int m, int s = 0, int ms = 0) { return h * 10000000 + m * 100000 + s * 1000; }

    bool is_future(const string& code)
    {
        const char*p = strrchr(code.c_str(), '.');
        if (!p) return false;
        p++;
        return strcmp(p, "SH") && strcmp(p, "SZ");
    }

    static bool is_finished(const Order* order)
    {
        return
            order->status == OS_Filled ||
            order->status == OS_Cancelled ||
            order->status == OS_Rejected;
    }

    bool is_finished_status(const string& status)
    {
        return status == OS_Filled || status == OS_Rejected || status == OS_Cancelled;
    }

    void get_action_effect(const string& action, string* pos_side, int* inc_dir)
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

    class PorfolioManagerAlgoImpl : public AlgoStralet {
        struct Task;
        struct OrderInfo {
            OrderID order_id;
            Order   order;
            unordered_map<string, Trade> trades;
            Task*   task;

            OrderInfo(Task* task, const string& account_id,
                const string& code, int date, int time,
                double price, int64_t size, const string& action,
                OrderID oid);
        };

        struct Task {
            int64_t task_id;
            string  code;
            double  entrust_price;
            int64_t inc_size;
            double  fill_price;
            int64_t fill_size;
            bool is_finished;

            vector <shared_ptr<OrderInfo>> orders;
        };

        struct PositionPair {
            Position long_pos;
            Position short_pos;
        };
    public:
        static const int TIMER_SYNC_POSITIONS = 1;

        // interface AlgoStralet
        void on_init(PorfolioManagerAlgo* obj, StraletContext* sc) {
            m_obj = obj;
            m_ctx = sc;
        }

        void on_fini            () { }
        void on_timer           (int32_t id, void* data);
        void on_order_status    (shared_ptr<const Order> order);
        void on_order_trade     (shared_ptr<const Trade> trade);
        void on_account_status  (shared_ptr<const AccountInfo> account);

        void init(const string& account_id, const vector<string>& codes);

        CallResult<const vector<NetPosition>> query_net_position();
        CallResult<const vector<Position>>    query_position();

        void set_target(const vector<NetPosition>& target);
        void stop_target();
        bool is_stopped();


        // return task_id, not entrust_id
        CallResult<int64_t>  place_order (const string& code, double price, int64_t inc_size);
        CallResult<bool>     cancel_order (const string& code, int64_t task_id);

        void sync_positions();

        shared_ptr<PositionPair> get_position(const string& code);
    private:
        PorfolioManagerAlgo* m_obj;
        StraletContext* m_ctx;

        string m_account_id;
        unordered_set<string> m_universe;
        unordered_map<string, shared_ptr<PositionPair>> m_positions;
        vector<Task>     m_tasks;
        unordered_map<string, shared_ptr<OrderInfo>> m_orders;

        int64_t m_next_task_id = 0;
    };

    void PorfolioManagerAlgoImpl::init(const string& account_id, const vector<string>& codes)
    {
        m_account_id = account_id;

        for (auto& code : codes)
            m_universe.insert(code);

        m_ctx->set_timer(m_obj, TIMER_SYNC_POSITIONS, 10, nullptr);
    }

    void PorfolioManagerAlgoImpl::on_timer(int32_t id, void* data)
    {
        switch (id) {
        case TIMER_SYNC_POSITIONS:
            sync_positions();
            break;            
        }
    }

    void PorfolioManagerAlgoImpl::sync_positions()
    {  
        shared_ptr<const vector<Position>> positions;
        auto r = m_ctx->trade_api()->query_positions(m_account_id);
        if (!r.value) {
            m_ctx->logger(ERROR) << "Failed to query position: " << m_account_id << "," << r.msg << endl;
            m_ctx->set_timer(this->m_obj, TIMER_SYNC_POSITIONS, 1000, nullptr);
            return;
        }
        else {
            positions = r.value;
        }

        m_positions.clear();
        for (auto& code : m_universe) {
            auto pair = make_shared<PositionPair>();

            pair->long_pos.account_id = m_account_id;
            pair->long_pos.name       = m_account_id;
            pair->long_pos.side = SD_Long;

            pair->short_pos.account_id = m_account_id;
            pair->short_pos.name       = m_account_id;
            pair->short_pos.side = SD_Short;

            m_positions[code] = pair;
        }

        m_ctx->set_timer(this->m_obj, TIMER_SYNC_POSITIONS, 6000, nullptr);
        // FIXME: support force syncing
        if (!is_stopped()) return;
        
        for (auto& pos : *positions) {
            auto it = m_positions.find(pos.code);
            auto pair = it->second;
            if (pos.side == SD_Long)
                pair->long_pos = pos;
            else
                pair->short_pos = pos;
        }
    }

    CallResult<const vector<NetPosition>> PorfolioManagerAlgoImpl::query_net_position()
    {
        auto net_positions = make_shared<vector<NetPosition>>();
        for (auto& e : m_positions) {
            auto pair = e.second;

            NetPosition pos;
            pos.account_id   = m_account_id;
            pos.code         = pair->long_pos.code;
            pos.name         = pair->long_pos.name;
            pos.init_size    = pair->long_pos.init_size - pair->short_pos.init_size;
            pos.current_size = pair->long_pos.current_size - pair->short_pos.current_size;
            pos.cost         = pos.current_size > 0 ? pair->long_pos.cost : pair->short_pos.cost;
            pos.cost_price   = pos.current_size > 0 ? pair->long_pos.cost_price : pair->short_pos.cost_price;
            net_positions->push_back(pos);
        }

        sort(net_positions->begin(), net_positions->end(), [](NetPosition& a, NetPosition& b) {
            return a.code < a.code;
        });

        return net_positions;
    }

    CallResult<const vector<Position>> PorfolioManagerAlgoImpl::query_position()
    {
        auto positions = make_shared<vector<Position>>();
        for (auto& e : m_positions) {
            auto pair = e.second;

            if (pair->long_pos.current_size > 0 || pair->long_pos.close_pnl != 0.000001)
                positions->push_back(pair->long_pos);

            if (pair->short_pos.current_size > 0 || pair->short_pos.close_pnl != 0.000001)
                positions->push_back(pair->short_pos);
        }

        sort(positions->begin(), positions->end(), [](Position& a, Position& b) {
            return a.code == a.code ? a.side < b.side : a.code < b.code;
        });

        return positions;
    }

    void PorfolioManagerAlgoImpl::on_order_status(shared_ptr<const Order> order)
    {
        // TODO: orders and trades?
    }

    void PorfolioManagerAlgoImpl::on_order_trade(shared_ptr<const Trade> trade)
    {
        if (trade->fill_size == 0) return;

        // FIXME: When orders and positions are not synched, this trade will be ignored!
        auto it = m_orders.find(trade->entrust_no);
        if (it == m_orders.end()) return;

        auto order_info = it->second;
        auto it2 = order_info->trades.find(trade->fill_no);
        // Ignore trades being processed before.
        if (it2 != order_info->trades.end()) return;

        order_info->trades[trade->fill_no] = *trade;
        
        auto pos_pair = get_position(trade->code);
        if (!pos_pair) {
            m_ctx->logger(ERROR) << "Can't find position of " << trade->code << endl;
            return;
        }

        string side; int inc_dir = 0;
        get_action_effect(order_info->order.entrust_action, &side, &inc_dir);
        auto pos = side == SD_Long ? &pos_pair->long_pos : &pos_pair->short_pos;
        
        auto order = &order_info->order;
        if (inc_dir == 1) {
            pos->current_size += trade->fill_size;
            if (is_future(order->code))
                pos->enable_size += trade->fill_size;
            pos->cost += trade->fill_price * trade->fill_size;
            pos->cost_price = pos->cost / pos->current_size;
        }
        else {
            pos->frozen_size  -= order->entrust_size;
            pos->current_size -= trade->fill_size;
            pos->enable_size  -= trade->fill_size; // TODO: check if it is right
            assert(pos->enable_size >= 0);
            pos->close_pnl += trade->fill_size * (trade->fill_price - pos->cost_price);

            if (pos->current_size == 0) {
                pos->cost = 0.0;
                pos->cost_price = 0.0;
            }
        }

        // Trades maybe come after finished order_status.
        if (trade->fill_size + order->fill_size <= order->entrust_size) {
            double turnover = order->fill_price * order->fill_size + trade->fill_price * trade->fill_size;
            order->fill_size += trade->fill_size;
            order->fill_price = turnover / order->fill_size;
            if (order->fill_size == order->entrust_size)
                order->status = OS_Filled;
        }
    }

    void PorfolioManagerAlgoImpl::on_account_status(shared_ptr<const AccountInfo> account)
    {
        // TODO:
    }

    void PorfolioManagerAlgoImpl::set_target(const vector<NetPosition>& target)
    {
        // TODO:
    }

    void PorfolioManagerAlgoImpl::stop_target()
    {
        // TODO:
    }

    bool PorfolioManagerAlgoImpl::is_stopped()
    {
        return false;
    }


    PorfolioManagerAlgoImpl::OrderInfo::OrderInfo(Task* task, const string& account_id,
                                                  const string& code, int date, int time,
                                                  double price, int64_t size, const string& action,
                                                  OrderID oid)
    {
        this->task = task;
        this->order_id = oid;
        this->order.account_id     = account_id;
        this->order.code           = code;
        this->order.entrust_action = action;
        this->order.entrust_date   = 0; // FIXME
        this->order.entrust_time   = 0;
        this->order.entrust_price  = price;
        this->order.entrust_size   = size;
        this->order.name           = code;
        this->order.status         = OS_New;
        this->order.fill_size      = 0;
        this->order.fill_price     = 0.0;
    }

    // return task_id, not entrust_id
    CallResult<int64_t>  PorfolioManagerAlgoImpl::place_order(const string& code, double price, int64_t inc_size)
    {
        if (price <=0.000001 || inc_size ==0)
            return CallResult<int64_t>("-1,wrong price or size");

        if (m_positions.size() == 0)
            return CallResult<int64_t>("-1,position is not synced");

        auto pos_pair = get_position(code);
        if (!pos_pair)
            return CallResult<int64_t>("-1,code is not in universe");

        auto task = make_shared<Task>();
        task->code = code;
        task->entrust_price = price;
        task->inc_size      = inc_size;
        task->fill_size     = 0;
        task->fill_price    = 0;
        task->task_id       = ++m_next_task_id;

        DateTime dt;
        m_ctx->cur_time(&dt);

        bool has_error = false;
        string err_msg;
        auto tapi = m_ctx->trade_api();
        if (inc_size>0) {
            // Cover
            if (pos_pair->short_pos.current_size - pos_pair->short_pos.frozen_size > 0) {
                auto pos = &pos_pair->short_pos;
                int64_t entrust_size = pos->current_size - pos->frozen_size;
                auto r = tapi->place_order(m_account_id, code, price, entrust_size, EA_Cover, 0);
                if (r.value) {
                    auto info = make_shared<OrderInfo>(task.get(), m_account_id, code, dt.date, dt.time, price, entrust_size, EA_Cover, *r.value);
                    task->orders.push_back(info);
                    inc_size -= entrust_size;
                    pos->frozen_size += entrust_size;
                    // FIXME:
                    this->m_orders[info->order.entrust_no] = info;
                }
                else {
                    has_error = true;
                    err_msg = r.msg;
                }
            }

            // Buy
            if (inc_size > 0 && !has_error) {
                auto r = tapi->place_order(m_account_id, code, price, inc_size, EA_Cover, 0);
                if (r.value) {
                    auto info = make_shared<OrderInfo>(task.get(), m_account_id, code, dt.date, dt.time, price, inc_size, EA_Buy, *r.value);
                    task->orders.push_back(info);
                    // FIXME:
                    this->m_orders[info->order.entrust_no] = info;
                }
                else {
                    has_error = true;
                    err_msg = r.msg;
                }
            }
        }
        else {
            // Cover
            if (pos_pair->long_pos.current_size - pos_pair->long_pos.frozen_size > 0) {
                auto pos = &pos_pair->long_pos;
                int64_t entrust_size = pos->current_size - pos->frozen_size;
                auto r = tapi->place_order(m_account_id, code, price, entrust_size, EA_Sell, 0);
                if (r.value) {
                    auto info = make_shared<OrderInfo>(task.get(), m_account_id, code, dt.date, dt.time, price, entrust_size, EA_Sell, *r.value);
                    task->orders.push_back(info);
                    inc_size -= entrust_size;
                    pos->frozen_size += entrust_size;
                    // FIXME:
                    this->m_orders[info->order.entrust_no] = info;
                }
                else {
                    has_error = true;
                    err_msg = r.msg;
                }
            }

            // Buy
            if (inc_size > 0 && !has_error) {
                auto r = tapi->place_order(m_account_id, code, price, inc_size, EA_Short, 0);
                if (r.value) {
                    auto info = make_shared<OrderInfo>(task.get(), m_account_id, code, dt.date, dt.time, price, inc_size, EA_Short, *r.value);
                    task->orders.push_back(info);
                    // FIXME:
                    this->m_orders[info->order.entrust_no] = info;
                }
                else {
                    has_error = true;
                    err_msg = r.msg;
                }
            }
        }

        if (task->orders.size() > 0)
            return CallResult<int64_t>(err_msg);
        else
            return CallResult<int64_t>(make_shared<int64_t>(task->task_id));
    }

    CallResult<bool>    PorfolioManagerAlgoImpl::cancel_order(const string& code, int64_t task_id)
    {
        return CallResult<bool>("-1,to be implemented");
    }


    shared_ptr<PorfolioManagerAlgoImpl::PositionPair> PorfolioManagerAlgoImpl::get_position(const string& code)
    {
        auto it = m_positions.find(code);
        return it != m_positions.end() ? it->second : nullptr;
    }

    PorfolioManagerAlgo::PorfolioManagerAlgo()
    {
        m_impl = new PorfolioManagerAlgoImpl();
    }

    PorfolioManagerAlgo::~PorfolioManagerAlgo()
    {
        delete m_impl;
    }


    void PorfolioManagerAlgo::on_init(StraletContext* sc)
    {
        AlgoStralet::on_init(sc);
        m_impl->on_init(this, sc);
    }

    void PorfolioManagerAlgo::on_timer(int32_t id, void* data)
    {
        m_impl->on_timer(id, data);
    }

    void PorfolioManagerAlgo::on_order_status(shared_ptr<const Order> order)
    {
        m_impl->on_order_status(order);
    }

    void PorfolioManagerAlgo::on_order_trade(shared_ptr<const Trade> trade)
    {
        m_impl->on_order_trade(trade);
    }

    void PorfolioManagerAlgo::on_account_status(shared_ptr<const AccountInfo> account)
    {
        m_impl->on_account_status(account);
    }

    void PorfolioManagerAlgo::init(const string& account_id, const vector<string>& codes)
    {
        m_impl->init(account_id, codes);
    }


    CallResult<const vector<NetPosition>> PorfolioManagerAlgo::query_net_position()
    {
        return m_impl->query_net_position();
    }

    CallResult<const vector<Position>> PorfolioManagerAlgo::query_position()
    {
        return m_impl->query_position();
    }

    void PorfolioManagerAlgo::set_target(const vector<NetPosition>& target)
    {
        return m_impl->set_target(target);
    }

    void PorfolioManagerAlgo::stop_target()
    {
        m_impl->stop_target();
    }

    bool PorfolioManagerAlgo::is_stopped()
    {
        return m_impl->is_stopped();
    }


    // return task_id, not entrust_id
    CallResult<int64_t>  PorfolioManagerAlgo::place_order(const string& code, double price, int64_t inc_size)
    {
        return m_impl->place_order(code, price, inc_size);
    }

    CallResult<bool>     PorfolioManagerAlgo::cancel_order(const string& code, int64_t task_id)
    {
        return m_impl->cancel_order(code, task_id);
    }

} }