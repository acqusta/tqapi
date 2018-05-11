#include <iostream>
#include "sim_context.h"
#include "sim_data.h"
#include "sim_trade.h"
#include "backtest.h"

vector<int> get_calendar(DataApi* dapi)
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

void bt_run(const BackTestConfig & a_cfg, function<Stralet*()> creator)
{
    BackTestConfig cfg = a_cfg;
    if (cfg.dapi_addr.empty())  cfg.dapi_addr = "ipc://tqc_10001";// "tcp://127.0.0.1:10001";
    if (cfg.accounts.empty())   cfg.accounts.push_back(AccountConfig("sim", 1e8, vector<Holding>()));
    if (cfg.data_level.empty()) cfg.data_level = "tk";
    if (cfg.result_dir.empty()) cfg.result_dir = "result";
    
    cout << "backtest: " << cfg.begin_date << "-" << cfg.end_date << "," << cfg.data_level << endl
         << "          outdir " << cfg.result_dir << endl;

    TQuantApi* tqapi = TQuantApi::create(cfg.dapi_addr);
    DataApi* dapi = tqapi->data_api();

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
    delete tqapi;
}
