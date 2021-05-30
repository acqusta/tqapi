#include <iostream>
#include <chrono>
#include <sstream>

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
    , m_should_exit(false)
    , m_mode("backtest")
{
}

void SimStraletContext::init(
    SimDataApi* dapi, SimTradeApi* tapi,
    DataLevel level,
    Json::Value& properties,
    const vector<int>& calendar,
    const string& log_dir
    )
{
    m_dapi = dapi;
    m_data_level = level;
    m_tapi = tapi;
    m_properties = properties;
    m_properties_str = m_properties.toStyledString();
    m_log_dir = log_dir;

    for (auto date : calendar) m_calendar.insert(date);
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

string to_str(system_clock::time_point& tp)
{
    auto dt = DateTime::from_timepoint(tp);
    stringstream ss;
    ss << dt.date << "-"<<dt.time;
    return ss.str();   
}

void SimStraletContext::set_timer(int64_t id, int64_t delay, void* data)
{
    if (delay <= 0) delay = 1;
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

LogStream SimStraletContext::logger(LogSeverity severity)
{
    auto buf = make_shared<LogStreamBuf>(m_log_dir, severity, m_now.date, m_now.time);
    return LogStream(buf);
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

void SimStraletContext::stop()
{
    m_should_exit = true;
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

    for (auto & m_timer : m_timers) {
        auto& timer = m_timer.second;

        if ( timer->trigger_time <= m_now_tp) {        
            timers.push_back(timer);
            timer->trigger_time = m_now_tp + milliseconds(timer->delay);
        }
    }

    for (auto& t : timers) {
        if (!t->is_dead) {
            m_stralet->on_timer(t->id, t->data);
            if (m_should_exit) break;
        }
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

bool SimStraletContext::is_trading_day(int date)
{
    return m_calendar.find(date) != m_calendar.end();
}

/*
 * 设置每个交易日的开始时间
 * 如果期货有夜盘，从上一个交易日的20：00开始，否则从当日的 8:50开始。
 */
void SimStraletContext::init_sim_time()
{
    int action_day = -1;
    for (int i = 1; i <= 3; i++) {
        struct tm tm;
        memset(&tm, 0, sizeof(tm));
        tm.tm_mday = m_trading_day % 100;
        tm.tm_mon = (m_trading_day / 100) % 100 - 1;
        tm.tm_year = (m_trading_day / 10000) - 1900;
        time_t t = mktime(&tm) - 3600 * 24 * i;
        tm = *localtime(&t);

        // sunday or saturday
        if (tm.tm_wday == 0 || tm.tm_wday == 6)
            continue;
        action_day = (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
        break;
    }

    if (action_day == -1 || !is_trading_day(action_day)) {
        action_day = m_trading_day;
        set_sim_time(DateTime(action_day, HMS(8, 50)));
    }
    else {
        set_sim_time(DateTime(action_day, HMS(20, 0)));
    }
}

void SimStraletContext::execute_trade()
{
    m_tapi->try_match();
    m_tapi->update_last_prices();

    for (auto& e : m_tapi->m_accounts) {
        auto& act = e.second;
        {
            auto ind_list = act->m_ord_status_ind_list;
            act->m_ord_status_ind_list.clear();
            for (auto& ind : ind_list) {
                m_stralet->on_order(ind);
                if (this->m_should_exit) return;
            }
        }
        {
            auto ind_list = act->m_trade_ind_list;
            act->m_trade_ind_list.clear();
            for (auto& ind : ind_list) {
                m_stralet->on_trade(ind);
                if (this->m_should_exit) return;
            }
        }
    }
}

void SimStraletContext::execute_market_data(vector<shared_ptr<MarketQuote>>& quotes, vector<shared_ptr<Bar>>& bars)
{
    for (auto & q : quotes) {
        m_stralet->on_quote(q);
        if (this->m_should_exit) return;
    }

    string cycle = "1m";
    for (auto& bar : bars) {
        m_stralet->on_bar(cycle, bar);
        if (this->m_should_exit) return;
    }
}

void SimStraletContext::execute_event()
{
    if (m_events.empty()) return;
    auto events = m_events;
    m_events.clear();
    for (auto& evt : events) {
        m_stralet->on_event(evt->name, evt->data);
        if (this->m_should_exit) return;
    }
}

void SimStraletContext::run_one_day(Stralet* stralet)
{
    init_sim_time();

    m_stralet = stralet;

    this->m_should_exit = false;
    m_stralet->set_context(this);
    m_stralet->on_init();

    const DateTime end_dt(m_trading_day, HMS(16, 01, 0));

    DateTime last_time;

    while (m_now.cmp(end_dt) < 0 && !m_should_exit) {
        DateTime dt_quote, dt_timer;

        m_dapi->calc_nex_time(&dt_quote);
        calc_next_timer_time(&dt_timer);

        DateTime now = dt_timer.date == 99999999 ? dt_quote:
            ( dt_quote.cmp(dt_timer) < 0 ? dt_quote : dt_timer);

        if (now.cmp(last_time) == 0) {
            logger(WARNING) << "break because of now == last_time " << now.time << "," << last_time.time;
            break;
        }
        if (now.cmp(last_time) < 0) {
            logger(FATAL) << "wrong now time: " << now.time << " should > " << last_time.time;
            break;
        }

        last_time = now;
        set_sim_time(now);

        // If time is not forward, it means no date

        // Set latest quotes before try_match
        vector<shared_ptr<MarketQuote>> quotes;
        vector<shared_ptr<Bar>> bars;
        for (auto& code : m_dapi->m_codes) {
            auto q = m_dapi->next_quote(code);
            if (q)
                quotes.push_back(q);
            auto bar = m_dapi->next_bar(code);
            if (bar)
                bars.push_back(bar);
        }

        // Move all tick data to right position
        for (auto& code : m_dapi->m_pinned_codes) {
            if (m_dapi->m_codes.find(code) != m_dapi->m_codes.end()) continue;
            m_dapi->next_quote(code);
            m_dapi->next_bar(code);
        }

        // 注意事件顺序: try_match -> order -> trade -> event -> quote -> bar

        if (!m_should_exit) execute_trade();
        if (!m_should_exit) execute_event();
        if (!m_should_exit) execute_market_data(quotes, bars);
        if (!m_should_exit) execute_timer();

        // No more event, break
        if (quotes.empty() && bars.empty() && m_timers.empty() && m_events.empty()) {
            //logger(WARNING) << "break because of no data, times and events any more";
            break;
        }
    }

    set_sim_time(end_dt);
    m_dapi->set_data_to_curtime();
    m_dapi->set_end_of_day();
    m_tapi->update_last_prices();
    m_tapi->settle();

    stralet->on_fini();
}
