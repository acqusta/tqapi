#include <iostream>

#include "sim_context.h"
#include "sim_data.h"
#include "sim_trade.h"
#include "sim_utils.h"

using namespace tquant::stralet::backtest;

SimStraletContext::SimStraletContext()
    : m_dapi(nullptr)
    , m_tapi(nullptr)
    , m_data_level(BT_TICK)
    , m_trading_day(0)
    , m_mode("backtest")
{
}

void SimStraletContext::init(SimDataApi* dapi, DataLevel level, SimTradeApi* tapi)
{
    m_dapi = dapi;
    m_data_level = level;
    m_tapi = tapi;
}


void SimStraletContext::move_to(int trading_day)
{
    if (trading_day == m_trading_day) return;

    m_trading_day = trading_day;
    m_timers.clear();
}

int32_t SimStraletContext::trading_day()
{
    return m_trading_day;
}

DateTime SimStraletContext::cur_time()
{
    return m_now;
}

system_clock::time_point SimStraletContext::cur_time_as_tp()
{
    return m_now_tp;
}

void SimStraletContext::post_event(const char* evt, void* data)
{
    // TODO:
}

void SimStraletContext::set_timer(Stralet* stralet, int32_t id, int32_t delay, void* data)
{
    auto triger_time = m_now_tp + milliseconds(delay);
    auto timer = make_shared<TimerInfo>();
    timer->stralet = stralet;
    timer->id      = id;
    timer->delay   = delay;
    timer->data    = data;
    timer->is_dead = false;
    timer->trigger_time = triger_time;

    m_timers.push_back(timer);
}

void SimStraletContext::kill_timer(Stralet* stralet, int32_t id)
{
    auto it = find_if(m_timers.begin(), m_timers.end(), [stralet, id](shared_ptr<TimerInfo>& x) {
        return x->stralet == stralet && x->id == id;
    });

    if (it != m_timers.end()) {
        it->get()->is_dead = true;
        m_timers.erase(it);
    }
}

DataApi*  SimStraletContext::data_api(const char* source)
{
    if (source && *source != '\0')
        return nullptr;
    else
        return m_dapi;
}

TradeApi* SimStraletContext::trade_api()
{
    return m_tapi;
}

ostream& SimStraletContext::logger(LogLevel level)
{
    static const char* str_level[] = {
        "I",
        "W",
        "E",
        "F"
    };

    DateTime now = cur_time();

    char label[100];
    
    sprintf(label, "%08d %06d.%03d %s| ", now.date, now.time / 1000, now.time % 1000, str_level[level]);
    cout << label;
    return cout;
}

void SimStraletContext::register_algo(AlgoStralet* algo)
{
    m_algos.push_back(algo);
}

void SimStraletContext::unregister_algo(AlgoStralet* algo)
{
    auto it = find(m_algos.begin(), m_algos.end(), algo);
    if (it != m_algos.end())
        m_algos.erase(it);
}

string SimStraletContext::get_parameter(const char* name, const char* def_value)
{
    return "";
}

const string& SimStraletContext::mode()
{
    return m_mode;
}


void SimStraletContext::calc_next_timer_time(DateTime* dt)
{
    if (m_timers.empty()) {
        dt->date = 99999999;
        dt->time = 0;
        return;
    }

    auto tp = (*m_timers.begin())->trigger_time;
    for (auto& e : m_timers)
        if (e->trigger_time < tp) tp = e->trigger_time;

    *dt = tp_to_dt(tp);
}

void SimStraletContext::execute_timer(Stralet* stralet)
{
    vector<shared_ptr<TimerInfo>> timers;
    //for (auto& e : m_timers)
    for (auto it = m_timers.begin(); it != m_timers.end(); ) {
        if ( (*it)->trigger_time <= m_now_tp) {
            timers.push_back(*it);
            it = m_timers.erase(it);
        }
        else {
            it++;
        }
    }

    for (auto& t : timers) {
        if (!t->is_dead)
            stralet->on_timer(t->id, t->data);
    }
}

void SimStraletContext::set_sim_time(const DateTime& dt)
{
    m_now = dt;
    m_now_tp = dt_to_tp(m_now.date, m_now.time);
}

SimAccount* SimStraletContext::get_account(const string& account_id)
{
    auto it = m_tapi->m_accounts.find(account_id);
    return it != m_tapi->m_accounts.end() ? it->second : nullptr;
}

void SimStraletContext::run_one_day(Stralet* stralet)
{
    DateTime end_dt(m_trading_day, HMS(15, 0, 0));

    // FIXME: ²»Ö§³ÖÒ¹ÅÌ¡£
    set_sim_time(DateTime(m_trading_day, HMS(8, 50)));

    stralet->on_init(this);

    while (m_now.cmp(end_dt) < 0) {
        DateTime dt1, dt2;
        m_dapi->calc_nex_time(&dt1);
        calc_next_timer_time(&dt2);

        set_sim_time(dt1.cmp(dt2) < 0 ? dt1 : dt2);

        m_tapi->try_match();

        for (auto& e : m_tapi->m_accounts) {
            auto& act = e.second;
            {
                auto ind_list = act->m_ord_status_ind_list;
                act->m_ord_status_ind_list.clear();
                for (auto& ind : ind_list) {
                    for (auto& algo : m_algos) algo->on_order_status(ind);
                    stralet->on_order_status(ind);
                }
            }
            {
                auto ind_list = act->m_trade_ind_list;
                act->m_trade_ind_list.clear();
                for (auto& ind : ind_list) {
                    for (auto& algo : m_algos) algo->on_order_trade(ind);
                    stralet->on_order_trade(ind);
                }
            }
        }

        vector<shared_ptr<MarketQuote>> quotes;
        for (auto& code : m_dapi->m_codes) {
            auto q = m_dapi->next_quote(code);
            if (q)
                quotes.push_back(q);
        }
        for (auto & q : quotes) {
            for (auto& algo : m_algos) algo->on_quote(q);
            stralet->on_quote(q);
        }

        vector<shared_ptr<Bar>> bars;
        for (auto& code : m_dapi->m_codes) {
            auto bar = m_dapi->next_bar(code);
            if (bar)
                bars.push_back(bar);
        }

        for (auto& bar : bars) {
            const char* cycle = "1m";
            for (auto& algo : m_algos) algo->on_bar(cycle, bar);
            stralet->on_bar(cycle, bar);
        }

        execute_timer(stralet);
    }

    stralet->on_fini();
}
