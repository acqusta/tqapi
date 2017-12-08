#include <thread>
#include <chrono>
#include <iostream>
#include <chrono>
#include "tquant_api.h"

using namespace std;
using namespace std::chrono;
using namespace tquant::api;

class MyCallback : public DataApi_Callback {
public:
    virtual void on_market_quote(shared_ptr<MarketQuote> quote) override 
    {
        auto q = quote.get();
        cout << "onQuote: " << q->code << "," << q->date << "," << q->time << ","
            << q->open << "," << q->high << "," << q->low << "," << q->close << ","
            << q->volume << "," << q->turnover << "," << q->oi << endl;

    }

    virtual void on_bar(const char* cycle, shared_ptr<Bar> bar) override
    {
        Bar& b = *bar;
        cout << "on_bar: " << cycle<< "," << b.code << "," << b.date << "," << b.time << ","
            << b.open << "," << b.high << "," << b.low << "," << b.close << ","
            << b.volume << "," << b.turnover << "," << b.oi << endl;
    }
};

void test_dapi(TQuantApi* api)
{
    const char* code = "rb.SHF";// .CFE";
    vector<string> codes;
    codes.push_back(code);
    codes.push_back("rb1801.SHF");

    MyCallback callback;

    api->data_api()->set_callback(&callback);

    {
        for (int i = 0; i < 100; i++) {
            auto r = api->data_api()->subscribe(codes);
            if (r.value) {
                for (string code : *r.value) cout << "sub code: " << code << endl;
                break;
            }
            else {
                cout << "sub error: " << r.msg << endl;
                continue;
            }
        }
    }

    if (0) {
        auto  r = api->data_api()->bar(code, "1m", 0, true);
        if (r.value) {
            for (auto& b : *r.value)
                cout << code << "," << b.date << "," << b.time << ","
                << b.open << "," << b.high << "," << b.low << "," << b.close << ","
                << b.volume << "," << b.turnover << b.oi << endl;
        }
        else {
            cout << "bar error: " << r.msg;
        }
    }

    if (1) {
        auto  r = api->data_api()->daily_bar(code, "forward", true);
        if (r.value) {
            for (auto& b : *r.value)
                cout << code << "," << b.date << ","
                << b.open << "," << b.high << "," << b.low << "," << b.close << ","
                << b.volume << "," << b.turnover << "," << b.oi << endl;
        }
        else {
            cout << "bar error: " << r.msg << endl;
        }
    }

    if (1) {
        auto r = api->data_api()->tick(code, 0);
        if (r.value) {
            for (auto& t : *r.value)
                cout << code << "," << t.date << "," << t.time << "," << t.last << "," << t.volume << endl;
        }
        else {
            cout << "tick error: " << code << "," << r.msg << endl;
        }
    }

    if (0) {
        auto  r = api->data_api()->quote(code);
        if (r.value) {
            auto q = r.value;
            cout << code << "," << q->date << "," << q->time << ","
                << q->open << "," << q->high << "," << q->low << "," << q->close << ","
                << q->volume << "," << q->turnover << q->oi << endl;
        }
        else {
            cout << "bar error: " << r.msg;
        }
    }

    if (1) {
        auto begin_time = system_clock::now();
        int total_count = 0;
        int dates[] = {
            20171106, 20171107, 20171108, 20171109, 20171110,
            20171113, 20171114, 20171115, 20171116, 20171117,
            20171120, 20171121, 20171122, 20171123, 20171124 };
        for (int i = 0; i < 15; i++) {
            auto ticks = api->data_api()->tick(code, dates[i]);
            if (ticks.value) {
                total_count += ticks.value->size();
                //for (auto& t : *ticks.value)
                //    cout << code << "," << t.date << "," << t.time << "," << t.last << "," << t.volume << endl;
            }
            else {
                cout << "tick error: " << code << "," << dates[i] << ": " << ticks.msg << endl;
            }
        }

        cout << "used time    : " << duration_cast<milliseconds>(system_clock::now() - begin_time).count() << endl
            << "total records: " << total_count << endl
            << "total date   : " << 15 << endl;

    }
}


void test_tapi(TradeApi* tapi)
{
    {
        auto r = tapi->query_account_status();
        if (r.value) {
            for (auto & act : *r.value) {
                cout << act.account_id << "|" << act.account << "|" << act.account_type << "|"
                    << act.broker << "|" << act.status << "|" << act.msg << endl;
            }
        }
    }

    this_thread::sleep_for(seconds(2));

    {
        auto r = tapi->query_positions("glsc");
        if (r.value) {
            for (auto& pos : *r.value) {
                cout << "Pos: "
                    << pos.account_id << "|" << pos.code << "|"
                    << pos.init_size << "|" << pos.current_size << "|" << pos.enable_size << "|"
                    << pos.today_size << "|" << pos.frozen_size << "|"
                    << pos.side << "|" << pos.cost << "|" << pos.cost_price
                    << pos.last_price << "|" << pos.float_pnl << "|" << pos.close_pnl << "|"
                    << pos.margin << "|" << pos.commission << endl;
            }
        }
        else {
            cout << "query_postion error: " << r.msg << endl;
        }
    }

    this_thread::sleep_for(seconds(2));

    {
        auto r = tapi->query_trades("glsc");
        if (r.value) {
            for (auto& trd : *r.value) {
                cout << "Trade: " << trd.account_id << "|" << trd.code << "|" << trd.name << "|"
                    << trd.entrust_no << "|" << trd.entrust_action << "|" << trd.fill_no << "|"
                    << trd.fill_size << "|" << trd.fill_price << "|"
                    << trd.fill_date << "|" << trd.fill_time << endl;
            }
        }
        else {
            cout << "query_trades error: " << r.msg << endl;
        }
    }

    this_thread::sleep_for(seconds(2));

    {
        auto r = tapi->query_orders("glsc");
        if (r.value) {
            for (auto& ord : *r.value) {
                cout << "Order: " << ord.account_id << "|" << ord.code << "|" << ord.name << "|"
                    << ord.entrust_no << "|" << ord.entrust_action << "|" << ord.entrust_price << "|"
                    << ord.entrust_size << "|" << ord.fill_price << "|"
                    << ord.entrust_date << "|" << ord.entrust_time << "|"
                    << ord.entrust_date << "|" << ord.entrust_time << "|"
                    << ord.fill_price << "|" << ord.fill_size << "|"
                    << ord.status << "|" << ord.status_msg << "|"
                    << ord.order_id << endl;
            }
        }
        else {
            cout << "query_orders error: " << r.msg << endl;
        }
    }

    {
        auto r = tapi->place_order("glsc", "000001.SH", 1.0, 1, "Buy", 1);
        if (r.value) {
            cout << "place_order result: " << r.value->entrust_no << "," << r.value->order_id << endl;
        }
        else {
            cout << "place_order error: " << r.msg << endl;
        }
    }

}

int main()
{
    TQuantApi* api = TQuantApi::create("tcp://127.0.0.1:10001");

    test_dapi(api);
    //test_tapi(api->trade_api());
    getchar();

}
