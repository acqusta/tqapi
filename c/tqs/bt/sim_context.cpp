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

void SimStraletContext::init(SimDataApi* dapi, DataLevel level, SimTradeApi* tapi, Json::Value& properties)
{
    m_dapi = dapi;
    m_data_level = level;
    m_tapi = tapi;
    m_properties = properties;
    m_properties_str = m_properties.toStyledString();
}


void SimStraletContext::move_to(int trading_day)
{
    if (trading_day == m_trading_day) return;

    m_trading_day = trading_day;
    m_timers.clear();
    m_events.clear();
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

void SimStraletContext::post_event(const char* name, void* data)
{
    auto evt = make_shared<EventData>();
    evt->name = name;
    evt->data = data;
    m_events.push_back(evt);
}

void SimStraletContext::set_timer(int64_t id, int64_t delay, void* data)
{
    auto triger_time = m_now_tp + milliseconds(delay);
    auto timer = make_shared<TimerInfo>();
    timer->id      = id;
    timer->delay   = delay;
    timer->data    = data;
    timer->is_dead = false;
    timer->trigger_time = triger_time;

    m_timers[id] = timer;
}

void SimStraletContext::kill_timer(int64_t id)
{
    auto it = m_timers.find(id);
    if (it != m_timers.end()) {
        it->second->is_dead = true;
        m_timers.erase(it);
    }
}

DataApi*  SimStraletContext::data_api()
{
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

string SimStraletContext::get_property(const char* name, const char* def_value)
{
    Json::Value empty;
    Json::Value v = m_properties.get(name, empty);
    switch (v.type()) {
    case Json::ValueType::nullValue: return def_value;
    case Json::ValueType::intValue:
    case Json::ValueType::uintValue:
    case Json::ValueType::realValue:
    case Json::ValueType::stringValue:
    case Json::ValueType::booleanValue: return v.asString();
    default:
        return v.toStyledString();
    }
}

const string& SimStraletContext::get_properties()
{
    return m_properties_str;
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

    auto tp = m_timers.begin()->second->trigger_time;
    for (auto& e : m_timers) {
        if (e.second->trigger_time < tp) tp = e.second->trigger_time;
    }
    *dt = DateTime::from_timepoint(tp);
}

void SimStraletContext::execute_timer()
{
    vector<shared_ptr<TimerInfo>> timers;

    for (auto it = m_timers.begin(); it != m_timers.end(); it++) {
        auto& timer = it->second;
        if ( timer->trigger_time <= m_now_tp) {            
            timers.push_back(timer);
            timer->trigger_time = m_now_tp + milliseconds(timer->delay);
        }
    }

    for (auto& t : timers) {
        if (!t->is_dead)
            m_stralet->on_event(make_shared<OnTimer>(t->id, t->data));
    }
}

void SimStraletContext::set_sim_time(const DateTime& dt)
{
    m_now = dt;
    m_now_tp = DateTime(m_now.date, m_now.time).to_timepoint();
}

SimAccount* SimStraletContext::get_account(const string& account_id)
{
    auto it = m_tapi->m_accounts.find(account_id);
    return it != m_tapi->m_accounts.end() ? it->second : nullptr;
}

void SimStraletContext::run_one_day(Stralet* stralet)
{
    m_stralet = stralet;

    DateTime end_dt(m_trading_day, HMS(15, 0, 0));

    // FIXME: 不支持夜盘。
    set_sim_time(DateTime(m_trading_day, HMS(8, 50)));

    stralet->set_context(this);
    stralet->on_event(make_shared<OnInit>());

    while (m_now.cmp(end_dt) < 0) {
        DateTime dt1, dt2;
        m_dapi->calc_nex_time(&dt1);
        calc_next_timer_time(&dt2);

        set_sim_time(dt1.cmp(dt2) < 0 ? dt1 : dt2);

        // Set latest quotes before try_match
        vector<shared_ptr<MarketQuote>> quotes;
        for (auto& code : m_dapi->m_codes) {
            auto q = m_dapi->next_quote(code);
            if (q)
                quotes.push_back(q);
        }

        vector<shared_ptr<Bar>> bars;
        for (auto& code : m_dapi->m_codes) {
            auto bar = m_dapi->next_bar(code);
            if (bar)
                bars.push_back(bar);
        }

        // 注意事件顺序: try_match -> order -> trade -> event -> quote -> bar

        m_tapi->try_match();

        for (auto& e : m_tapi->m_accounts) {
            auto& act = e.second;
            {
                auto ind_list = act->m_ord_status_ind_list;
                act->m_ord_status_ind_list.clear();
                for (auto& ind : ind_list) {                    
                    stralet->on_event(make_shared<OnOrder>(ind));
                }
            }
            {
                auto ind_list = act->m_trade_ind_list;
                act->m_trade_ind_list.clear();
                for (auto& ind : ind_list) {
                    stralet->on_event(make_shared<OnTrade>(ind));
                }
            }
        }

        if (m_events.size()) {
            auto events = m_events;
            m_events.clear();
            for (auto evt : events)
                stralet->on_event(make_shared<OnEvent>(evt->name, evt->data));
        }

        for (auto & q : quotes)
            stralet->on_event(make_shared<OnQuote>(q));

        for (auto& bar : bars) {
            const char* cycle = "1m";
            stralet->on_event(make_shared<OnBar>(cycle, bar));
        }

        execute_timer();
    }

    stralet->on_event(make_shared<OnFini>());
}
