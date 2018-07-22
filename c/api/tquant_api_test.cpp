#include <assert.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include "tquant_api.h"
#include "myutils/timeutils.h"
#include "myutils/csvparser.h"
#include "myutils/unicode.h"

using namespace std;
using namespace std::chrono;
using namespace tquant::api;

class MyCallback : public DataApi_Callback {
public:
    virtual void on_market_quote(shared_ptr<const MarketQuote> quote) override 
    {
        auto q = quote.get();
        
        if (1) {
            cout << "onQuote: " << q->code << "," << q->date << "," << q->time << ","
                << q->open << "," << q->high << "," << q->low << "," << q->close << ","
                << q->volume << "," << q->turnover << "," << q->oi << endl;
        } else {
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

    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override
    {
        const Bar& b = *bar;
        cout << "on_bar: " << cycle<< "," << b.code << "," << b.date << "," << b.time << ","
            << b.open << "," << b.high << "," << b.low << "," << b.close << ","
            << b.volume << "," << b.turnover << "," << b.oi << endl;
    }
};

MyCallback callback;

void test_dapi(DataApi* dapi)
{
    const char* code = "RB.SHF";// .CFE";
    vector<string> codes;
    codes.push_back(code);
    codes.push_back("RB1805.SHF");

    dapi->set_callback(&callback);

    {
        for (int i = 0; i < 100; i++) {
            auto r = dapi->subscribe(codes);
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
        auto  r = dapi->bar(code, "1m", 0, true);
        if (r.value) {
            for (size_t i = 0; i < r.value->size(); i++) {
                auto& b = r.value->at(i);
                cout << code << "," << b.date << "," << b.time << ","
                    << b.open << "," << b.high << "," << b.low << "," << b.close << ","
                    << b.volume << "," << b.turnover << b.oi << endl;
            }
        }
        else {
            cout << "bar error: " << r.msg;
        }
    }

    if (1) {
        auto  r = dapi->daily_bar(code, "forward", true);
        if (r.value) {
            for (size_t i = 0; i < r.value->size(); i++) {
                auto& b = r.value->at(i);
                cout << code << "," << b.date << ","
                    << b.open << "," << b.high << "," << b.low << "," << b.close << ","
                    << b.volume << "," << b.turnover << "," << b.oi << endl;
            }
        }
        else {
            cout << "bar error: " << r.msg << endl;
        }
    }

    if (1) {
        auto r = dapi->tick(code, 0);
        if (r.value) {
            //for (auto& t : *r.value)
            for (size_t i = 0; i < r.value->size(); i++) {
                auto& t = r.value->at(i);
                cout << code << "," << t.date << "," << t.time << "," << t.last << "," << t.volume << endl;
            }
        }
        else {
            cout << "tick error: " << code << "," << r.msg << endl;
        }
    }

    if (0) {
        auto  r = dapi->quote(code);
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
            auto ticks = dapi->tick(code, dates[i]);
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
                cout << act.account_id << "|" << utf8_to_local(act.broker) << "|" << act.account << "|"
                    << act.account_type << "|" << act.status << "|" << act.msg << endl;
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
                cout << "Order: " << ord.account_id << "|" << ord.code << "|" << utf8_to_local(ord.name) << "|"
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
        auto r = tapi->place_order("glsc", "000001.SH", 1.0, 1, "Buy", "", 1);
        if (r.value) {
            cout << "place_order result: " << r.value->entrust_no << "," << r.value->order_id << endl;
        }
        else {
            cout << "place_order error: " << utf8_to_local(r.msg) << endl;
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

        auto ms = duration_cast<milliseconds>(system_clock::now() - begin_time).count();
        cout << (ms*1.0/count) << endl;
    }
    {
        int count = 3000;
        auto begin_time = system_clock::now();
        for (int i = 0; i < count; i++)
            dapi->quote("");

        auto ms = duration_cast<milliseconds>(system_clock::now() - begin_time).count();
        cout << (ms*1.0 / count) << endl;
    }
}

struct CodeMapping {
    string  code;
    int32_t date;
    string  target_code;
};

unordered_map<string, vector<CodeMapping>> g_code_maping_map;

void load_codemap()
{
    csv::Parser parser("d:/tquant/tqc/tmp/code_mapping.csv");
    for (uint32_t i = 0; i < parser.rowCount(); i++) {
        CodeMapping mapping;
        auto& row = parser.getRow(i);
        mapping.code = row[0];
        mapping.date = atoi(row[1].c_str());
        mapping.target_code = row[2];
        g_code_maping_map[mapping.code].push_back(mapping);
    }
}

string get_code(string code, int32_t date)
{
    auto it = g_code_maping_map.find(code);
    if (it == g_code_maping_map.end())
        return code;

    auto& mappings = it->second;

    for(auto it2 = mappings.rbegin(); it2 != mappings.rend(); it2++) {
        if (it2->date <= date)
            return it2->target_code;
    }

    return mappings.back().target_code;
}

void perf_test2(DataApi* dapi)
{
    load_codemap();

    //const char* code = "IF.CFE";
    const char* code = "RB.SHF";
    vector<string> codes = { code };
    dapi->subscribe(codes);
    
    //while (true) {
    
    //    auto r = dapi->quote(code);
    //    if (r.value) break;
    //    cout << "quote failed: " << r.msg << endl;
    //    this_thread::sleep_for(seconds(1));
    //}

    auto begin_time = system_clock::now();
    size_t total_count = 0;
    //auto bars = dapi->daily_bar(code, "", true);
    //assert(bars.value);
    int date_count = 0;
    //for (auto& bar : *bars.value) {
    for (int date = 20170101; date < 20180101; date = fin_nextday(date)) {

        string real_code = get_code(code, date);
        //auto ticks = dapi->bar(real_code, "1m", bar.date, false);
        auto ticks = dapi->tick(real_code, date);

        if (ticks.value) {
            date_count++;
            total_count += ticks.value->size();
        }
        else
            std::cout << "tick error: " << real_code << "," << date << ": " << ticks.msg << endl;
        
    }

    auto used_time = duration_cast<microseconds>(system_clock::now() - begin_time).count();
    std::cout
        << "used time     : " << used_time / 1000 << " milliseconds" << endl
        << "total records : " << total_count << endl
        << "total date    : " << date_count << endl
        << "ticks per day : " << (total_count / date_count) << endl
        << "time per day  : " << (used_time / date_count) << " microseconds" << endl;


}

void test_dapi2(DataApi* dapi)
{
    dapi->set_callback(&callback);

    vector<string> codes = { "000001.SH", "600000.SH", "000001.SZ", "399001.SZ", "RB1805.SHF", "IF1801.CFE", "M1805.DCE", "I1803.SHF", "000999.SH" };
    auto r = dapi->subscribe(codes);
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
        auto r = dapi->quote("RB1805.SHF");
        if (r.value) {
            auto q = r.value;
            cout << "quote: " << q->code << "," << q->date << "," << q->time << "," << q->last << "," << q->volume << endl;
        }
        else {
            cout << "error quote: " << r.msg << endl;
        }
    }
}

void perf_test3(DataApi* dapi)
{
    const char* code = "600000.SH";
    vector<string> codes = { code };
    dapi->subscribe(codes);

    auto begin_time = system_clock::now();

    size_t total_count = 0;
    int loop = 1000;
    for (int i = 0; i < loop; i++) {
        //auto ticks = dapi->tick(code, 20);
        auto ticks = dapi->bar(code, "1m", 0, true);
        if (ticks.value) {
            total_count += ticks.value->size();
            for (int n = 0; n < ticks.value->size(); n++) {
                auto b = &ticks.value->at(n);
                //cout << "bar: " << b->code << "," << b->date << "," << b->time << "," << b->open <<"," << b->high <<"," << b->low << "," << b->close << endl;
            }

        }
        //else
        //    cout << "tick error: " << code << "," << bar.date << ": " << ticks.msg << endl;
    }

    size_t used_time = duration_cast<microseconds>(system_clock::now() - begin_time).count();
    cout << "used time    : " << used_time << endl
        << "total records: " << total_count << endl
//        << "total date   : " << date_count << endl
         << "time per code : " << (used_time / 1000.0) / loop << " millis" <<endl;
}

int main(int argc, const char** argv)
{
    {

        const char* addr[3] = {
            "ipc://tqc_10001",
            "tcp://127.0.0.1:10001"
        };

#ifdef _WIN32        
        addr[2] = "mdapi://file://d:/tquant/tqc?hisdata_only=true";
        set_params("plugin_path", "D:\\tquant\\md\\bin;D:\\tquant\\trade\\bin;D:\\tquant\\trade\\bin_x64;");

#else
        addr[2] = "mdapi://file:///opt/tquant/md";
        set_params("plugin_path", "/opt/tquant/md/lib");
#endif


        int i = 0;
        if (argc>1) i = atoi(argv[1]);
        std::cout << addr[i] << endl;
        
        DataApi* dapi = create_data_api(addr[i]);
        //perf_test(api->data_api());
        //perf_test2(api->data_api());
        perf_test3(dapi);
        
        //test_dapi(dapi);
        //test_dapi2(dapi);
        delete dapi;

    }
    if (0) {
        const char* addr = "tradeapi://tcp://127.0.0.1:10202";

        set_params("plugin_path", "D:\\tquant\\md\\bin;D:\\tquant\\trade\\bin;D:\\tquant\\trade\\bin_x64");

        std::cout << addr << endl;

        TradeApi* tapi = create_trade_api(addr);
        test_tapi(tapi);
        delete tapi;
    }
    
    getchar();

}
;
