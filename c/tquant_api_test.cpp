#include <assert.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <chrono>
#include <sstream>
#include "tquant_api.h"

using namespace std;
using namespace std::chrono;
using namespace tquant::api;

class MyCallback : public DataApi_Callback {
public:
    virtual void on_market_quote(shared_ptr<MarketQuote> quote) override 
    {
        auto q = quote.get();
        // cout << "onQuote: " << q->code << "," << q->date << "," << q->time << ","
        //     << q->open << "," << q->high << "," << q->low << "," << q->close << ","
        //     << q->volume << "," << q->turnover << "," << q->oi << endl;

        {
            static int64_t tick_count;
            static int64_t csum_time;

            auto t = quote;
            if (t->recv_time > 0) {
                auto now = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
                tick_count++;
                csum_time += now - t->recv_time;
                if (tick_count % 10 == 0) {
                    std::cout << "tqc delay: " << (csum_time / tick_count) << " microseconds" << endl;
                    tick_count = 0;
                    csum_time = 0;
                }
            }
            else {
                std::cout << "wrong recv_time: " << t->recv_time << "," << t->code << endl;
            }
        }

    }

    virtual void on_bar(const char* cycle, shared_ptr<Bar> bar) override
    {
        Bar& b = *bar;
        cout << "on_bar: " << cycle<< "," << b.code << "," << b.date << "," << b.time << ","
            << b.open << "," << b.high << "," << b.low << "," << b.close << ","
            << b.volume << "," << b.turnover << "," << b.oi << endl;
    }
};

MyCallback callback;

void test_dapi(TQuantApi* api)
{
    const char* code = "rb.SHF";// .CFE";
    vector<string> codes;
    codes.push_back(code);
    codes.push_back("rb1805.SHF");

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
        size_t total_count = 0;
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

    while (true) {
        this_thread::sleep_for(seconds(1));
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

void perf_test(DataApi* dapi)
{
    auto r = dapi->quote("000001.SH");
    //assert(r.value);

    {
        int count = 3000;
        auto begin_time = system_clock::now();
        for (int i = 0; i < count; i++) {
            auto r = dapi->quote("000001.SH");
            if (!r.value) {
                std::cout << "quote error: " << r.msg << endl;
            }
        }

        int ms = duration_cast<milliseconds>(system_clock::now() - begin_time).count();
        cout << (ms*1.0/count) << endl;
    }
    {
        int count = 3000;
        auto begin_time = system_clock::now();
        for (int i = 0; i < count; i++)
            dapi->quote("");

        int ms = duration_cast<milliseconds>(system_clock::now() - begin_time).count();
        cout << (ms*1.0 / count) << endl;
    }
}

void perf_test2(DataApi* dapi)
{
    const char* code = "rb.SHF";
    vector<string> codes = { code };
    while (true) {
        dapi->subscribe(codes);
        auto r = dapi->quote(code);
        if (r.value) break;
        cout << "quote failed: " << r.msg << endl;
        this_thread::sleep_for(seconds(1));
    }

    auto begin_time = system_clock::now();
    int total_count = 0;
    auto bars = dapi->daily_bar(code, "", true);
    int date_count = 0;
    assert(bars.value);
    for (auto& bar : *bars.value) {
        if (bar.date < 20170401) continue;

        //auto ticks = dapi->bar(code, "1m", bar.date, false);

        if (bar.date > 20170601) break;
        auto ticks = dapi->tick(code, bar.date);

        //cout << bar.date << endl;

        if (ticks.value)
            total_count += ticks.value->size();
        else
            cout << "tick error: " << code << "," << bar.date << ": " << ticks.msg << endl;
        date_count++;
    }

    size_t used_time = duration_cast<microseconds>(system_clock::now() - begin_time).count();
    cout << "used time    : " << used_time << endl
        << "total records: " << total_count << endl
        << "total date   : " << date_count << endl
        << "time per day : " << (used_time / date_count) / 1000.0 << endl;

}

void test_dapi_local(DataApi* dapi)
{
    dapi->set_callback(&callback);

    vector<string> codes = { "000001.SH", "600000.SH", "000001.SZ", "399001.SZ", "rb1805.SHF", "IF1801.CFE", "M1805.DCE", "i1803.SHF" };
    //codes
    auto r = dapi->subscribe(codes, "local");
    if (r.value) {
        stringstream ss;
        for (auto & s : *r.value) ss << s << ",";
        cout << "subscribe result: " << ss.str() << endl;
    }
    else {
        cout << "subscribe error: " << r.msg << endl;
    }

    while (true) {
        this_thread::sleep_for(seconds(1));
        //auto r = dapi->quote("rb1805.SHF", "local");
        //if (r.value) {
        //    auto q = r.value;
        //    cout << "quote: " << q->code << "," << q->date << "," << q->time << "," << q->last << "," << q->volume << endl;
        //}
    }
}

int main()
{
    //const char* addr = "tcp://127.0.0.1:10001";
    const char* addr = "ipc://tqc_10001";

    std::cout << addr << endl;
    TQuantApi* api = TQuantApi::create(addr);

    //perf_test(api->data_api());
    //perf_test2(api->data_api());

    test_dapi_local(api->data_api());
    test_dapi(api);
    //test_tapi(api->trade_api());
    getchar();

}
;
