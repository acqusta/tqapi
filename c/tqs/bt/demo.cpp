#include <iostream>
#include "stralet.h"
#include "backtest.h"


class MyStralet : public Stralet {
public:
    virtual void on_init(StraletContext* ctx) override {
        Stralet::on_init(ctx);
        //cout << "on_init: " << ctx->trading_day() << endl;
        vector<string> codes = { "000001.SH", "600000.SH", "000001.SZ", "399001.SZ" };
        ctx->data_api()->subscribe(codes);
    }

    virtual void on_fini() override {
        //cout << "on_fini: " << ctx()->trading_day() << endl;
    }

    virtual void on_quote(shared_ptr<MarketQuote> q) {
        //cout << "on_quote: " << q->code << "," << q->date << "," << q->time << "," << q->last << endl;
    }

    virtual void on_bar(const char* cycle, shared_ptr<const Bar> bar) override {
        //cout << "on_bar: " << bar->code << "," << bar->date << "," << bar->time << "," << bar->close << endl;
    }

    virtual void on_order_status(shared_ptr<const Order> order) override {
        cout << "on_order_status: " << order->code << "," << order->entrust_action << "," << order->status << endl;
    }
    virtual void on_order_trade(shared_ptr<const Trade> trade) override {
        cout << "on_order_trade: " << trade->code << "," << trade->entrust_action << "," << trade->fill_price << endl;
    }
};

int test1()
{
    BackTestConfig cfg;
    //cfg.dapi_addr = "tcp://127.0.0.1:10001";
    cfg.begin_date = 20170101;
    cfg.end_date = 20171231;
    cfg.data_level = "tk";
    cfg.accounts.push_back(AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    bt_run(cfg, []() { return new MyStralet(); });

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";
    
    getchar();
    return 0;
}

Stralet* create_rbreaker();

int test2() 
{
    BackTestConfig cfg;
    cfg.dapi_addr = "tcp://127.0.0.1:10001";
    cfg.begin_date = 20170101;
    cfg.end_date = 20180321;
    cfg.data_level = "1m";
    cfg.accounts.push_back(AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    bt_run(cfg, create_rbreaker);

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";

    getchar();
    return 0;
}

Stralet *create_doublema();

int test3()
{
    BackTestConfig cfg;
    cfg.dapi_addr = "tcp://127.0.0.1:10001";
    cfg.begin_date = 20170101;
    cfg.end_date = 20180321;
    cfg.data_level = "1m";
    cfg.accounts.push_back(AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    bt_run(cfg, create_doublema);

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";

    getchar();
    return 0;
}

Stralet *create_ifhft();

int test_ifhft()
{
    BackTestConfig cfg;
    //cfg.dapi_addr = "tcp://127.0.0.1:10001";
    cfg.dapi_addr = "ipc://tqc_10001?timeout=30";
    cfg.begin_date = 20180101;
    cfg.end_date = 20180330;
    cfg.data_level = "tk";
    cfg.accounts.push_back(AccountConfig("sim", 1e8));

    auto begin_time = system_clock::now();

    bt_run(cfg, create_ifhft);

    auto end_time = system_clock::now();
    cout << "used time: " << duration_cast<milliseconds>(end_time - begin_time).count() << "ms\n";

    getchar();
    return 0;
}

int main()
{
    test_ifhft();

    return 0;
}
