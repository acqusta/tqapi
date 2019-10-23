#include <iostream>
#include "stralet.h"
#include "backtest.h"
#include "realtime.h"

using namespace tquant::stralet;

class MyStralet : public Stralet {
public:
    virtual void on_init() override {
        ctx()->logger() << "on_init: " << ctx()->trading_day() << endl;
        vector<string> codes = {
			"000001.SH", "399001.SH", "000001.SZ", "600000.SH", "RB1905.SHF", "T1906.CFE", "IF1904.CFE" };
			//"000001.SH", "600000.SH", "000001.SZ", "399001.SZ" };
        ctx()->data_api()->subscribe(codes);
    }

    virtual void on_fini() override {
        ctx()->logger() << "on_fini: " << ctx()->trading_day() << endl;
    }

    virtual void on_quote(shared_ptr<const MarketQuote> quote) override {
        auto q = quote.get();
        ctx()->logger() << "on_quote: " << q->code << "," << q->date << "," << q->time << "," << q->last;
    }

    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override {
        ctx()->logger() << "on_bar: " << bar->code << "," << bar->date << "," << bar->time << "," << bar->close;
    }
    
    virtual void on_order(shared_ptr<const Order> order) override {
        ctx()->logger() << "on_order: " << order->code << "," << order->entrust_action << "," << order->status;

    }
    virtual void on_trade(shared_ptr<const Trade> trade) override {
        ctx()->logger() << "on_trade: " << trade->code << "," << trade->entrust_action << "," << trade->fill_price;
    }
};

int test1()
{	getchar();

    backtest::BackTestConfig cfg;
    //cfg.dapi_addr = "tcp://127.0.0.1:10001";
    cfg.begin_date = 20170101;
    cfg.end_date = 20171231;
    cfg.data_level = "tk";
    cfg.accounts.push_back(backtest::AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    backtest::run(cfg, []() { return new MyStralet(); });

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";
    
    getchar();
    return 0;
}

Stralet* create_rbreaker();

int test2() 
{
    backtest::BackTestConfig cfg;
    cfg.dapi_addr = "tcp://127.0.0.1:10001";
    cfg.begin_date = 20170101;
    cfg.end_date = 20180321;
    cfg.data_level = "tk";
    cfg.accounts.push_back(backtest::AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    backtest::run(cfg, create_rbreaker);

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";

    getchar();
    return 0;
}

Stralet *create_doublema();

int test3()
{
    backtest::BackTestConfig cfg;
    cfg.dapi_addr  = "tcp://192.168.2.231:10002";
    cfg.begin_date = 20190101;
    cfg.end_date   = 20191031;
    cfg.data_level = "1m";
    cfg.accounts.push_back(backtest::AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    backtest::run(cfg, create_doublema);

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";

    return 0;
}

int test_realtime()
{
	realtime::RealTimeConfig cfg;
	cfg.data_api_addr = "tcp://127.0.0.1:10001";

	realtime::run(cfg, []() { return new MyStralet(); });

	return 0;
}

int main()
{
    test3();
	//test_realtime();
    return 0;
}
