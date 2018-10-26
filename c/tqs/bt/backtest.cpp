#include <iostream>
#include "sim_context.h"
#include "sim_data.h"
#include "sim_trade.h"
#include "backtest.h"
#include "jsoncpp/inc/json/json.h"
#include "myutils/unicode.h"
#include "myutils/misc.h"

namespace tquant { namespace stralet { namespace backtest {

static vector<int> get_calendar(DataApi* dapi)
{
    dapi->subscribe(vector<string>{"000001.SH"});
    auto r = dapi->daily_bar("000001.SH", "", true);
    if (!r.value) {
        cerr << "Can't get daily_bar 000001.SH:" << r.msg;
        throw std::runtime_error("Can't get calendar");
    }
    vector<int> dates;
    for (size_t i=0; i < r.value->size(); i++)
        dates.push_back(r.value->at(i).date);

    return dates;
}

static 
bool get_string(Json::Value& value, const char* key, string* str)
{
    Json::Value empty;
    Json::Value tmp = value.get(key, empty);
    if (tmp.isString()) {
        *str = tmp.asString();
        return true;
    }
    else {
        return false;
    }
}

static
bool get_double(Json::Value& value, const char* key, double* v)
{
    Json::Value empty;
    Json::Value tmp = value.get(key, empty);
    if (tmp.isNumeric()) {
        *v = tmp.asDouble();
        return true;
    }
    else {
        return false;
    }
}

static
bool get_int64(Json::Value& value, const char* key, int64_t* v)
{
    Json::Value empty;
    Json::Value tmp = value.get(key, empty);
    if (tmp.isNumeric()) {
        *v = tmp.asInt64();
        return true;
    }
    else {
        return false;
    }
}

static
bool get_int(Json::Value& value, const char* key, int32_t* v)
{
    Json::Value empty;
    Json::Value tmp = value.get(key, empty);
    if (tmp.isNumeric()) {
        *v = tmp.asInt();
        return true;
    }
    else {
        return false;
    }
}

void run(const char* cfg_str, function<Stralet*()> creator)
{
    string utf8 = gbk_to_utf8(cfg_str);

    Json::Value conf;
    Json::Reader reader;
    if (!reader.parse(utf8, conf)) {
        cerr << "parse conf failure: " << reader.getFormattedErrorMessages();
        return;
    }

    BackTestConfig cfg;
    try {
        Json::Value empty;

        get_string(conf, "dapi_addr",  &cfg.dapi_addr);
        get_string(conf, "data_level", &cfg.data_level);
        get_int   (conf, "begin_date", &cfg.begin_date);
        get_int   (conf, "end_date",   &cfg.end_date);
        get_string(conf, "result_dir", &cfg.result_dir);

        Json::Value properties = conf.get("properties", empty);
        if (!properties.isNull()) cfg.properties = properties.toStyledString();

        Json::Value accounts = conf.get("accounts", empty);
        if (!accounts.isNull()) {
            for (auto iter = accounts.begin(); iter != accounts.end(); iter++) {

                string account_id;
                double init_balance;

                get_string(*iter, "account_id", &account_id);
                get_double(*iter, "init_balance", &init_balance);

                Json::Value init_holdings = iter->get("init_holdings", empty);

                AccountConfig act(account_id, init_balance);
                for (auto iter2 = init_holdings.begin(); iter2 != init_holdings.end(); iter2++)
                {
                    Holding h;
                    get_string (*iter2, "code",       &h.code);
                    get_double (*iter2, "cost_price", &h.cost_price);
                    get_string (*iter2, "side",       &h.side);
                    get_int64  (*iter2, "size",       &h.size);

                    act.init_holdings.push_back(h);
                }

                cfg.accounts.push_back(act);
            }
        }
    }
    catch (exception& e) {
        cerr << "parse conf failure: " << e.what();
        return;
    }

    backtest::run(cfg, creator);
}

void run(const BackTestConfig & a_cfg, function<Stralet*()> creator)
{
    BackTestConfig cfg = a_cfg;
    if (cfg.dapi_addr.empty())  cfg.dapi_addr = "ipc://tqc_10001";
    if (cfg.accounts.empty())   cfg.accounts.push_back(AccountConfig("sim", 1e8, vector<Holding>()));
    if (cfg.data_level.empty()) cfg.data_level = "tk";
    if (cfg.result_dir.empty()) cfg.result_dir = "result";
    
    cout << "backtest: " << cfg.begin_date << "-" << cfg.end_date << "," << cfg.data_level << endl
         << "          outdir " << cfg.result_dir << endl;

    myutils::make_abs_dir(cfg.result_dir);

    DataApi* dapi = create_data_api(cfg.dapi_addr.c_str());

    SimStraletContext* sc = new SimStraletContext();
    vector<SimAccount*> accounts;
    for (auto& e : cfg.accounts) {
        SimAccount* act = new SimAccount(sc, e.account_id, e.init_balance, e.init_holdings);
        accounts.push_back(act);
    }

    SimDataApi* sim_dapi = new SimDataApi(sc, dapi);
    SimTradeApi* sim_tapi = new SimTradeApi(sc, accounts);

    DataLevel data_level;
    if      (cfg.data_level == "tk") data_level = BT_TICK;
    else if (cfg.data_level == "1m") data_level = BT_BAR1M;
    else if (cfg.data_level == "1d") data_level = BT_BAR1D;
    else {
        cerr << "unknown data_level " << cfg.data_level;
        return;
    }

    Json::Value properties;
    if (cfg.properties.size()) {
        Json::Reader reader;
        if (!reader.parse(cfg.properties, properties)) {
            cerr << "parse conf failure: " << reader.getFormattedErrorMessages();
            return;
        }
    }

    auto calendar = get_calendar(dapi);

    sc->init(sim_dapi, sim_tapi, data_level, properties, calendar, cfg.result_dir);

    for (auto & date : calendar) {
        if (date < cfg.begin_date) continue;
        if (date > cfg.end_date) break;
     
        sc->move_to(date);
        sim_dapi->move_to(date);
        sim_tapi->move_to(date);

        Stralet* stralet = creator();
        sc->run_one_day(stralet);
        delete stralet;
    }

    for (auto& act : accounts) {
        act->save_data(cfg.result_dir);
        delete act;
    }
    delete sc;
    delete sim_dapi;
    delete sim_tapi;
    delete dapi;
}

} } }
