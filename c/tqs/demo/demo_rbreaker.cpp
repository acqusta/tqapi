#include <iostream>
#include <sstream>
#include <string.h>
#include "stralet.h"


// https://uqer.io/ R-Breaker策略
//
// R-Breaker 是一种短线日内交易策略，它结合了趋势和反转两种交易方式。该策略也长期被Future Thruth 杂志评为最赚钱的策略之一，尤其在标普
// 500 股指期货上效果最佳。该策略的主要特点如下：
//
// 第一、根据前一个交易日的收盘价、最高价和最低价数据通过一定方式计算出六个价位，从大到小依次为突破买入价、观察卖出价、反转卖出价、反转买
// 入价、观察买入价和突破卖出价，以此来形成当前交易日盘中交易的触发条件。通过对计算方式的调整，可以调节六个价格间的距离，进一步改变触发条
// 件。
//
// 第二、根据盘中价格走势，实时判断触发条件，具体条件如下：
// 1) 当日内最高价超过观察卖出价后，盘中价格出现回落，且进一步跌破反转卖出价构成的支撑线时，采取反转策略，即在该点位（反手、开仓）做空；
// 2) 当日内最低价低于观察买入价后，盘中价格出现反弹，且进一步超过反转买入价构成的阻力线时，采取反转策略，即在该点位（反手、开仓）做多；
// 3) 在空仓的情况下，如果盘中价格超过突破买入价，则采取趋势策略，即在该点位开仓做多；
// 4) 在空仓的情况下，如果盘中价格跌破突破卖出价，则采取趋势策略，即在该点位开仓做空。
//
// 第三、设定止损以及止盈条件；
//
// 第四、设定过滤条件；
//
// 第五、在每日收盘前，对所持合约进行平仓。
//
// 具体来看，这六个价位形成的阻力和支撑位计算过程如下：
//
// 观察卖出价 = High + 0.35 * (Close – Low)
// 观察买入价 = Low – 0.35 * (High – Close)
// 反转卖出价 = 1.07 / 2 * (High + Low) – 0.07 * Low
// 反转买入价 = 1.07 / 2 * (High + Low) – 0.07 * High
// 突破买入价 = 观察卖出价 + 0.25 * (观察卖出价 – 观察买入价)
// 突破卖出价 = 观察买入价 – 0.25 * (观察卖出价 – 观察买入价)
// 其中，High、Close、Low 分别为昨日最高价、昨日收盘价和昨日最低价。这六个价位从大到小一次是，
// 突破买入价、观察爱出价、反转卖出价、反转买入价、观察买入价和突破卖出价。

using namespace tquant::stralet;
using namespace tquant::api;

struct PriceRange {
    double sell_setup;  // 观察价
    double buy_setup;   
    double sell_enter;  // 反转价
    double buy_enter;
    double sell_break;  // 突破价
    double buy_break;
};

static inline int HMS(int h, int m, int s = 0, int ms = 0) { return h * 10000000 + m * 100000 + s * 1000; }

class RBreakerStralet : public Stralet {

    PriceRange price_range;
    int count_1 = 0;
    int count_2 = 0;
    string account_id = "sim";
    string contract = "CU.SHF";

public:
    virtual void on_event(shared_ptr<StraletEvent> evt) override;
    void on_init();
    void on_bar(const char* cycle, shared_ptr<const Bar> bar);
    void on_fini();

    int cancel_unfinished_order();

    void place_order(const string& code, double price, int64_t size, const string action);
};

void RBreakerStralet::on_event(shared_ptr<StraletEvent> evt)
{
    switch (evt->evt_id) {
    case STRALET_EVENT_ID::ON_INIT:
        on_init();
        break;
    case STRALET_EVENT_ID::ON_FINI:
        on_fini();
        break;
    case STRALET_EVENT_ID::ON_BAR: {
        auto on_bar = evt->as<OnBar>();
        this->on_bar(on_bar->cycle.c_str(), on_bar->bar);
        break;
    }
    }
}

void RBreakerStralet::on_init()
{
    ctx()->logger() << "on_init: " << ctx()->trading_day() << endl;
    // TODO: 从配置中得到要交易的商品期货，然后从主力合约映射表中得到今日交易的合约
    ctx()->data_api()->subscribe(vector<string>{contract});
    //ctx()->data_api()->subscribe(vector<string>{"000001.SH"});

    // TODO: 从上个交易日价格计算出今天的价格区间
}

void RBreakerStralet::on_fini() 
{
    auto r = ctx()->data_api()->bar(this->contract.c_str(), "1m", 0, true);
    if (r.value)
        ctx()->logger(INFO) <<  "on_fini: " << ctx()->trading_day() << ", bar count " << r.value->size() << endl;
    else
        ctx()->logger(FATAL) << "on_fini: " << ctx()->trading_day() << "," << r.msg << endl;
}

static PriceRange calc_price_range(double high, double low, double close ) 
{
    double high_beta = 0.35;
    double low_beta = 0.25;
    double enter_beta = 0.07;

    double sell_setup = high + high_beta * (close - low); // 观察卖出价
    double buy_setup  = low - high_beta * (high - close); // 观察买入价
    double sell_enter = (1 + enter_beta) / 2 * (high + low) - enter_beta * low;  // 反转卖出价
    double buy_enter  = (1 + enter_beta) / 2 * (high + low) - enter_beta * high; // 反转买入价
    double sell_break = buy_setup - low_beta * (sell_setup - buy_setup); // 突破卖出价
    double buy_break  = sell_setup + low_beta * (sell_setup - buy_setup); //突破买入价

    PriceRange range;
    range.sell_setup = sell_setup  ;
    range.buy_setup  = buy_setup   ;
    range.sell_enter = sell_enter  ;
    range.buy_enter  = buy_enter   ;
    range.sell_break = sell_break  ;
    range.buy_break  = buy_break   ;
    return range;
}

static bool is_finished(const Order* order)
{
    return
        order->status == OS_Filled ||
        order->status == OS_Cancelled ||
        order->status == OS_Rejected;
}

int RBreakerStralet::cancel_unfinished_order() 
{
    auto tapi = ctx()->trade_api();
    auto r = tapi->query_orders(account_id.c_str());
    if (!r.value) {
        ctx()->logger() << "error: query_orders: " << r.msg << endl;
        return -1;
    }
    int count = 0;
    for (auto&ord : *r.value) {
        if (ord.code == contract && !is_finished(&ord)) {
            m_ctx->logger() << "cancel order " << ord.code << "," << ord.entrust_price << "," << ord.entrust_action << endl;
            tapi->cancel_order(this->account_id.c_str(), ord.code.c_str(), ord.entrust_no.c_str());
            count++;
        }
    }
    return count;
}

void RBreakerStralet::place_order(const string& code, double price, int64_t size, const string action)
{
    DateTime dt = m_ctx->cur_time();

    ctx()->logger(INFO) << dt.date <<"," << dt.time <<", place order: " << code << "," << price << "," << size << "," << action << endl;
    auto r = m_ctx->trade_api()->place_order(account_id.c_str(), code.c_str(),price, size, action.c_str(), "", 0);
    if (!r.value)
        ctx()->logger(ERROR) << "place_order error:" << r.msg << endl;;
}

void RBreakerStralet::on_bar(const char* cycle, shared_ptr<const Bar> bar)
{
    if (strcmp(cycle, "1m")) return;
    if (bar->code != this->contract) return;

    auto tapi = ctx()->trade_api();
    auto dapi = ctx()->data_api();

    if (bar->time == HMS(9, 31)) {
        // 不交易夜盘，因此从夜盘行情中取 high, low, close计算
        double high = 0.0;
        double low = 100000000.0;
        double close = 0.0;
        auto r = ctx()->data_api()->bar(contract.c_str(), "1m", 0, true);
        //for (auto & b : *r.value) {
        for (int i =0; i < r.value->size(); i++) {
            auto b = &r.value->at(i);
            if (b->high > high) high = b->high;
            if (b->low < low)   low  = b->low;
            close = b->close;
        }
        price_range = calc_price_range(high, low, close);
        return;
    }

    // 只交易日盘, 至少两个bar
    if (bar->time < HMS(9, 32, 0) || bar->time > HMS(15, 0))
        return;

    // 简单处理，如果有在途订单，直接取消
    if (cancel_unfinished_order() > 0) return;

    auto r = dapi->bar(contract.c_str(), "1m", 0, true);
    if (!r.value && r.value->size() < 2) {
        ctx()->logger() << "error: dapi.bar: " << r.msg << endl;
        return;
    }

    auto bars = r.value;
    auto bar_2 = &bars->at(bars->size() - 2);
    auto bar_1 = &bars->at(bars->size() - 1);

    int64_t long_size = 0, short_size = 0;
    {
        auto r = tapi->query_positions(account_id.c_str());
        if (!r.value) return;
        for (auto &pos : *r.value) {
            if (pos.code == this->contract) {
                if (pos.side == SD_Long)
                    long_size += pos.current_size;
                else
                    short_size += pos.current_size;
            }
        }
    }

    shared_ptr<const MarketQuote> quote;
    {
        auto r = dapi->quote(contract.c_str());
        if (!r.value) return;
        quote = r.value;
    }

    if (bar->time >= HMS(14, 55)) {
        if (long_size != 0)
            place_order(contract, quote->bid1, long_size, EA_Sell);
        if (short_size != 0)
            place_order(contract, quote->bid1, short_size, EA_Cover);
        return;
    }
    // 向上突破
    if (bar_2->close <= price_range.buy_break && bar_1->close > price_range.buy_break) {
        if (long_size == 0)
            place_order(contract, quote->ask1, 1, EA_Buy);

        if (short_size != 0)
            place_order(contract, quote->ask1, short_size, EA_Cover);
    }

    // 向下突破
    if (bar_2->close >= price_range.sell_break && bar_1->close < price_range.sell_break) {
        if (short_size == 0)
            place_order(contract, quote->bid1, 1, EA_Short);

        if (long_size != 0)
            place_order(contract, quote->bid1, long_size, EA_Sell);
    }

    // 多单反转
    if (bar_1->high > price_range.sell_setup && bar_1->close > price_range.sell_enter)
        count_1 = 1;

    if (count_1 == 1 && bar_1->close < price_range.sell_enter) {
        if (long_size > 0) {
            place_order(contract, quote->bid1, long_size, EA_Sell);
            place_order(contract, quote->bid1, 1, EA_Short);
        }
    }
    // 空单反转
    if (bar_1->low < price_range.buy_setup) 
        count_2 = 1;

    if (count_2 == 1 && bar_1->close > price_range.buy_enter) {
        if (short_size > 0) {
            place_order(contract, quote->ask1, short_size, EA_Cover);
            place_order(contract, quote->ask1, 1, EA_Buy);
        }
    }
}


Stralet* create_rbreaker()
{
    return new RBreakerStralet();
}
