#include <iostream>
#include <sstream>
#include <string.h>
#include <list>
#include <assert.h>
#include "stralet.h"


// https://uqer.io/ R-Breaker策略
//
using namespace tquant::stralet;
using namespace tquant::api;


static inline int HMS(int h, int m, int s = 0, int ms = 0) { return h * 10000000 + m * 100000 + s * 1000; }

class DoubleMAStralet : public Stralet {
public:
	virtual void on_init			() override;
	virtual void on_fini			() override;
	//virtual void on_quote			(shared_ptr<const MarketQuote> quote) override;
	virtual void on_bar				(const string& cycle, shared_ptr<const Bar> bar) override;
	//virtual void on_order			(shared_ptr<const Order> order) override;
	//virtual void on_trade			(shared_ptr<const Trade> trade) override;
	//virtual void on_timer			(int64_t id, int64_t data) override;
	//virtual void on_event			(const string& name, int64_t data) { }
	//virtual void on_account_status	(shared_ptr<const AccountInfo> account) { }

    int cancel_unfinished_order();

    void place_order(const string& code, double price, int64_t size, const string action, const string& price_type="");
private:
    string m_account_id;
    string m_contract;
    size_t m_fast_ma_len = 0;
    size_t m_slow_ma_len = 0;
};

void DoubleMAStralet::on_init()
{
    ctx()->logger() << "on_init: " << ctx()->trading_day() << endl;
    m_fast_ma_len = 5;
    m_slow_ma_len = 10;
    m_account_id  = "sim";
    m_contract    = "IF.CFE";

    auto r = ctx()->data_api()->subscribe(vector<string>{m_contract});
    assert(r.value);

}

void DoubleMAStralet::on_fini() 
{
    auto r = ctx()->data_api()->bar(m_contract.c_str(), "1m", 0, true, 0);
    if (r.value)
        ctx()->logger(INFO) <<  "on_fini: " << ctx()->trading_day() << ", bar count " << r.value->size() << endl;
    else
        ctx()->logger(ERROR) << "on_fini: " << ctx()->trading_day() << "," << r.msg << endl;
}

static bool is_finished(const Order* order)
{
    return
        order->status == OS_Filled ||
        order->status == OS_Cancelled ||
        order->status == OS_Rejected;
}

int DoubleMAStralet::cancel_unfinished_order() 
{
    auto tapi = ctx()->trade_api();
    auto r = tapi->query_orders(m_account_id.c_str());
    if (!r.value) {
        ctx()->logger() << "error: query_orders: " << r.msg << endl;
        return -1;
    }
    int count = 0;
    for (auto&ord : *r.value) {
        if (ord.code == m_contract && !is_finished(&ord)) {
            m_ctx->logger() << "cancel order " << ord.code << "," << ord.entrust_price << "," << ord.entrust_action << endl;
            tapi->cancel_order(this->m_account_id.c_str(), ord.code.c_str(), ord.entrust_no.c_str());
            count++;
        }
    }
    return count;
}

void DoubleMAStralet::place_order(const string& code, double price, int64_t size, const string action, const string& price_type)
{
    ctx()->logger(INFO) << "place order: " << code << "," << price << "," << size << "," << action << endl;
    auto r = m_ctx->trade_api()->place_order(m_account_id.c_str(), code.c_str(),price, size, action.c_str(), price_type, 0);
    if (!r.value)
        ctx()->logger(ERROR) << "place_order error:" << r.msg << endl;;
}

void DoubleMAStralet::on_bar(const string& cycle, shared_ptr<const Bar> bar)
{
	if (cycle != "1m") return;

    if (bar->code != m_contract) return;

    auto tapi = ctx()->trade_api();
    auto dapi = ctx()->data_api();

    //// 只交易日盘, 至少有m_slowma_len个bar
    if (bar->time < HMS(9 + (m_slow_ma_len /60), m_slow_ma_len % 60, 0) || bar->time > HMS(15, 0))
        return;

    // 简单处理，如果有在途订单，直接取消
    if (cancel_unfinished_order() > 0) return;

    int64_t long_size = 0, short_size = 0;
    {
        auto r = tapi->query_positions(m_account_id.c_str());
        if (!r.value) return;
        for (auto &pos : *r.value) {
            if (pos.code == m_contract) {
                if (pos.side == SD_Long)
                    long_size += pos.current_size;
                else
                    short_size += pos.current_size;
            }
        }
    }

    shared_ptr<const MarketQuote> quote = dapi->quote(m_contract.c_str()).value;
    if (!quote) return;

    if (0 && bar->time >= HMS(14, 55)) {
        if (long_size != 0)
            place_order(m_contract, quote->bid1, long_size, EA_Sell, "any");
        if (short_size != 0)
            place_order(m_contract, quote->bid1, short_size, EA_Cover, "any");
        return;
    }

    auto r = dapi->bar(m_contract.c_str(), "1m", 0, true, 0);
    if (!r.value) return;
    auto bars = r.value;

    if (bars->size() < m_slow_ma_len) return;

    double slow_ma = 0.0;
    double fast_ma = 0.0;

    for (size_t i = bars->size() - m_slow_ma_len; i < bars->size(); i++) {
        //ctx()->logger(ERROR) << bars->at(i).close;
        slow_ma += bars->at(i).close;
    }
    slow_ma /= m_slow_ma_len;

    for (size_t i = bars->size() - m_fast_ma_len; i < bars->size(); i++)
        fast_ma += bars->at(i).close;

    fast_ma /= m_fast_ma_len;

//    ctx()->logger(ERROR) << "fast vs slow " << fast_ma << "," << slow_ma;

    // 交易逻辑：当快线向上穿越慢线且当前没有持仓，则买入1手；当快线向下穿越慢线且当前有持仓，则平仓
    if (fast_ma > slow_ma ) {
        if (short_size != 0)
            place_order(m_contract, quote->ask1, short_size, EA_Cover);

        if (long_size == 0)
            place_order(m_contract, quote->ask1, 1, EA_Buy);
    } else if (fast_ma < slow_ma) {
        if (long_size != 0)
            place_order(m_contract, quote->bid1, long_size, EA_Sell);

        if (short_size == 0)
            place_order(m_contract, quote->bid1, 1, EA_Short);
    }
}


Stralet* create_doublema()
{
    return new DoubleMAStralet();
}
