#include <iostream>
#include <chrono>
#include "tquant_api.h"

using namespace std;
using namespace std::chrono;
using namespace tquant::api;

class MyCallback : public DataApi_Callback {
public:
    virtual void onMarketQuote(shared_ptr<MarketQuote> quote) override 
    {
        auto q = quote.get();
        cout << "onQuote: " << q->code << "," << q->date << "," << q->time << ","
            << q->open << "," << q->high << "," << q->low << "," << q->close << ","
            << q->volume << "," << q->turnover << "," << q->oi << endl;

    }

    virtual void onBar(const char* cycle, shared_ptr<Bar> bar) override
    {
        Bar& b = *bar;
        cout << "onBar: " << cycle<< "," << b.code << "," << b.date << "," << b.time << ","
            << b.open << "," << b.high << "," << b.low << "," << b.close << ","
            << b.volume << "," << b.turnover << "," << b.oi << endl;
    }
};

int main()
{
    TQuantApi* api = TQuantApi::create("tcp://127.0.0.1:10001");

    const char* code = "rb.SHF";// .CFE";
    vector<string> codes;
    codes.push_back(code);
    codes.push_back("rb1801.SHF");

    MyCallback callback;

    api->data_api()->setCallback(&callback);

    {
        auto r = api->data_api()->subscribe(codes);
        if (r.value) {
            for (string code : *r.value) cout << "sub code: " << code << endl;
        }
        else {
            cout << "sub error: " << r.msg;
            return 0;
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

    if (0) {
        auto  r = api->data_api()->daily_bar(code, "forward", true);
        if (r.value) {
            for (auto& b : *r.value)
                cout << code << "," << b.date << ","
                << b.open << "," << b.high << "," << b.low << "," << b.close << ","
                << b.volume << "," << b.turnover << "," << b.oi << endl;
        }
        else {
            cout << "bar error: " << r.msg;
        }
    }

    if (0) {
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
    getchar();
    return 0;
}