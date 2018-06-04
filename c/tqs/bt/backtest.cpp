#include <iostream>
#include "sim_context.h"
#include "sim_data.h"
#include "sim_trade.h"
#include "backtest.h"
#include "jsoncpp/inc/json/json.h"
#include "myutils/unicode.h"

namespace tquant { namespace stralet { namespace backtest {

static vector<int> get_calendar(DataApi* dapi)
{
    auto sh000001 = dapi->daily_bar("000001.SH", "", true);
    if (!sh000001.value) {
        cerr << "Can't get daily_bar 000001.SH\n";
        throw std::runtime_error("Can't get calendar");
    }
    vector<int> dates;
    for (auto & e : *sh000001.value)
        dates.push_back(e.date);

    return dates;
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
        Json::Value dapi_addr = conf.get("dapi_addr", empty);
        if (dapi_addr.isString()) cfg.dapi_addr = dapi_addr.asString();

        Json::Value data_level = conf.get("data_level", empty);
        if (data_level.isString())    cfg.data_level = data_level.asString();

        Json::Value begin_date = conf.get("begin_date", empty);
        if (begin_date.isNumeric())    cfg.begin_date = begin_date.asInt();

        Json::Value end_date = conf.get("end_date", empty);
        if (end_date.isNumeric())    cfg.end_date = end_date.asInt();

        Json::Value result_dir = conf.get("result_dir", empty);
        if (result_dir.isString())    cfg.result_dir = result_dir.asString();

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

    DataApi* dapi = create_data_api(cfg.dapi_addr.c_str());

    SimStraletContext* sc = new SimStraletContext();
    vector<SimAccount*> accounts;
    for (auto& e : cfg.accounts) {
        SimAccount* act = new SimAccount(sc, e.account_id, e.init_balance, e.init_holdings);
        accounts.push_back(act);
    }

    SimDataApi* sim_dapi = new SimDataApi(sc, dapi);
    SimTradeApi* sim_tapi = new SimTradeApi(sc, accounts);

    DataLevel dl;
    if      (cfg.data_level == "tk") dl = BT_TICK;
    else if (cfg.data_level == "1m") dl = BT_BAR1M;
    else if (cfg.data_level == "1d") dl = BT_BAR1D;
    else {
        cerr << "unknown data_level " << cfg.data_level;
        return;
    }

    sc->init(sim_dapi, dl, sim_tapi);

    auto calendar = get_calendar(dapi);
    //calendar.push_back(20180529);
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
        act->save_data(".");
        delete act;
    }
    delete sc;
    delete sim_dapi;
    delete sim_tapi;
    delete dapi;
}

} } }