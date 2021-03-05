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
			//"000001.SH", "399001.SH", "000001.SZ", "600000.SH", "RB1905.SHF", "T1906.CFE", "IF1904.CFE",
            "000001.SH" };
			//"000001.SH", "600000.SH", "000001.SZ", "399001.SZ" };
        ctx()->data_api()->subscribe(codes);
    }

    virtual void on_fini() override {
        ctx()->logger() << "on_fini: " << ctx()->trading_day() << endl;
    }

    virtual void on_quote(shared_ptr<const MarketQuote> quote) override {
        // auto q = quote.get();
        // ctx()->logger() << "on_quote: " << q->code << "," << q->date << "," << q->time << "," << q->last;
    }

    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override {
        ctx()->logger() << "on_bar: " << bar->code << "," << bar->date << "," << bar->time << "," << bar->close;
        if (bar->code == string("000001.SH") && bar->time == 93100000) {
            ctx()->set_timer(1, 30000, 0);
            ctx()->set_timer(2, 30000, 0);
            ctx()->set_timer(3, 30000, 0);
            ctx()->set_timer(4, 30000, 0);
            ctx()->set_timer(5, 0, 0);
            ctx()->set_timer(6, 0, 0);
            ctx()->set_timer(7, 0, 0);
        }
    }
    
    virtual void on_order(shared_ptr<const Order> order) override {
        ctx()->logger() << "on_order: " << order->code << "," << order->entrust_action << "," << order->status;

    }
    virtual void on_trade(shared_ptr<const Trade> trade) override {
        ctx()->logger() << "on_trade: " << trade->code << "," << trade->entrust_action << "," << trade->fill_price;
    }
    virtual void on_timer(int64_t id, void* data) override {
        ctx()->logger() << "on_timer: " << id;
        ctx()->kill_timer(id);

        if (id>=5) {
            ctx()->set_timer(id, 30000, 0);
        }
        
    }
};

int test1()
{

    auto begin_time = system_clock::now();

    const char* txt = "{ \
        \"dapi_addr\": \"tcp://192.168.50.132:10002\",\
        \"data_level\" : \"tk\",\
        \"begin_date\" : 20191030,\
        \"end_date\" : 20191030,\
        \"result_dir\" : null,\
        \"accounts\" : [{\
        \"account_id\": \"sim\",\
            \"init_balance\" : 1.0E10,\
            \"init_holdings\" : [{\
            \"code\": \"600000.SH\",\
                \"side\" : \"Long\",\
                \"size\" : 100000,\
                \"cost_price\" : 10.0\
        }, {\
            \"code\": \"000001.SZ\",\
            \"side\" : \"Long\",\
            \"size\" : 100000,\
            \"cost_price\" : 10.0\
        }]\
    }],\
        \"properties\": null\
}";
    backtest::run(txt, []() { return new MyStralet(); });

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";
    
    getchar();
    return 0;
}

Stralet* create_rbreaker();

int test2() 
{
    backtest::BackTestConfig cfg;
    cfg.dapi_addr = "tcp://192.168.50.132:10002";
    cfg.begin_date = 20200710;
    cfg.end_date = 20200810;
    cfg.data_level = "tk";
    cfg.accounts.push_back(backtest::AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    backtest::run(cfg, []() { return new MyStralet(); });

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";
   return 0;
}

Stralet *create_doublema();

int test3()
{
    backtest::BackTestConfig cfg;
    cfg.dapi_addr  = "tcp://192.168.50.132:10003";
    cfg.begin_date = 20201030;
    cfg.end_date   = 20201030;
    cfg.data_level = "tk";
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
    test2();
	//test_realtime();
    return 0;
}
