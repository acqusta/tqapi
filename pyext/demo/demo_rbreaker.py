# -*- coding: utf-8 -*-

# https://uqer.io/ R-Breaker策略
#
# R-Breaker 是一种短线日内交易策略，它结合了趋势和反转两种交易方式。该策略也长期被Future Thruth 杂志评为最赚钱的策略之一，尤其在标普
# 500 股指期货上效果最佳。该策略的主要特点如下：
#
# 第一、根据前一个交易日的收盘价、最高价和最低价数据通过一定方式计算出六个价位，从大到小依次为突破买入价、观察卖出价、反转卖出价、反转买
# 入价、观察买入价和突破卖出价，以此来形成当前交易日盘中交易的触发条件。通过对计算方式的调整，可以调节六个价格间的距离，进一步改变触发条
# 件。
#
# 第二、根据盘中价格走势，实时判断触发条件，具体条件如下：
# 1) 当日内最高价超过观察卖出价后，盘中价格出现回落，且进一步跌破反转卖出价构成的支撑线时，采取反转策略，即在该点位（反手、开仓）做空；
# 2) 当日内最低价低于观察买入价后，盘中价格出现反弹，且进一步超过反转买入价构成的阻力线时，采取反转策略，即在该点位（反手、开仓）做多；
# 3) 在空仓的情况下，如果盘中价格超过突破买入价，则采取趋势策略，即在该点位开仓做多；
# 4) 在空仓的情况下，如果盘中价格跌破突破卖出价，则采取趋势策略，即在该点位开仓做空。
#
# 第三、设定止损以及止盈条件；
#
# 第四、设定过滤条件；
#
# 第五、在每日收盘前，对所持合约进行平仓。
#
# 具体来看，这六个价位形成的阻力和支撑位计算过程如下：
#
# 观察卖出价 = High + 0.35 * (Close – Low)
# 观察买入价 = Low – 0.35 * (High – Close)
# 反转卖出价 = 1.07 / 2 * (High + Low) – 0.07 * Low
# 反转买入价 = 1.07 / 2 * (High + Low) – 0.07 * High
# 突破买入价 = 观察卖出价 + 0.25 * (观察卖出价 – 观察买入价)
# 突破卖出价 = 观察买入价 – 0.25 * (观察卖出价 – 观察买入价)
# 其中，High、Close、Low 分别为昨日最高价、昨日收盘价和昨日最低价。这六个价位从大到小一次是，
# 突破买入价、观察爱出价、反转卖出价、反转买入价、观察买入价和突破卖出价。

import tquant as tq

class PriceRange:
    def __init__(self):
        self.sell_setup = 0.0  # 观察价
        self.buy_setup  = 0.0
        self.sell_enter = 0.0 # 反转价
        self.buy_enter  = 0.0
        self.sell_break = 0.0 # 突破价
        self.buy_break  = 0.0

def HMS(h,m,s=0, ms = 0):
    return h * 10000000 + m * 100000 + s * 1000

def is_finished(order):
    return order['status'] in [ "Filled", "Cancelled", "Rejected"]


class RBreakerStralet(tq.Stralet):

    def on_init(self):
        self.logger.info("on_init", self.ctx.trading_day, self.ctx.mode)

        self.price_range = PriceRange()
        self.count_1 = 0
        self.count_2 = 0
        self.account_id = "sim"
        self.contract = "RB.SHF"

        self.data_api.subscribe(self.contract)

    def on_fini(self):
        self.logger.info("info", "--fini--")

    def on_quote(self, q):
        # print q['code'], q['time'], q['last']
        pass

    def on_order(self, order):
        # print order
        pass

    def on_trade(sef, trade):
        # print trade
        pass


    def calc_range(self, high, low, close):
        high_beta = 0.35
        low_beta = 0.25
        enter_beta = 0.07

        sell_setup = high + high_beta * (close - low)  # 观察卖出价
        buy_setup  = low - high_beta * (high - close)  # 观察买入价
        sell_enter = (1 + enter_beta) / 2 * (high + low) - enter_beta * low  # 反转卖出价
        buy_enter  = (1 + enter_beta) / 2 * (high + low) - enter_beta * high  # 反转买入价
        sell_break = buy_setup - low_beta * (sell_setup - buy_setup)  # 突破卖出价
        buy_break = sell_setup + low_beta * (sell_setup - buy_setup)  # 突破买入价

        self.price_range.sell_setup = sell_setup
        self.price_range.buy_setup  = buy_setup
        self.price_range.sell_enter = sell_enter
        self.price_range.buy_enter  = buy_enter
        self.price_range.sell_break = sell_break
        self.price_range.buy_break  = buy_break

    def place_order(self, code, price, size, action):
        self.logger.info("place order", code, price, size, action)

        orderid, msg = self.trade_api.place_order(self.account_id, code=code, price=price, size=size, action=action)
        if orderid is None:
            self.logger.error("place_order error:", msg)


    def cancel_unfinished_order(self):
        tapi = self.trade_api
        orders, msg = tapi.query_orders(self.account_id, to_dataframe=False)
        assert orders is not None, "query_orders error: " + msg

        count = 0
        for order in orders:
            if is_finished(order) and order["code"] == self.contract: continue
            self.logger.info("cancel_order", order["code"], order["entrust_price"], order["entrust_action"])
            tapi.cancel_order(self.account_id, order["code"], order["entrust_no"])
            count += 1
        return count


    def on_bar(self, cycle, bar):
        if cycle != "1m": return
        if bar["code"] != self.contract: return

        tapi = self.trade_api
        dapi = self.data_api

        if bar['time'] == HMS(9, 31):
            # 不交易夜盘，从夜盘行情中取 high, low, close计算
            all_bars, _ = dapi.bar(self.contract, "1m", 0, df_value=True)
            high = all_bars['high'].max()
            low  = all_bars['low'].min()
            close = all_bars['close'].iloc[-1]
            self.calc_range(high, low, close)
            return

        # 只交易日盘, 至少两个bar
        if bar['time'] < HMS(9, 32, 0) or bar["time"] > HMS(15, 0):
            return

        # 简单处理，如果有在途订单，直接取消
        if self.cancel_unfinished_order() > 0: return

        self.check_action()

    def check_action(self):
        tapi = self.trade_api
        dapi = self.data_api

        long_size = 0
        short_size = 0
        positions, msg = tapi.query_positions(self.account_id, to_dataframe=False)
        for pos in positions:
            if pos['code'] == self.contract:
                if pos['side'] == "Long":
                    long_size += pos['current_size']
                else:
                    short_size += pos['current_size']

        quote, _ = dapi.quote(self.contract)

        # 14:55 平仓结束交易

        if self.ctx.cur_fin_time['time'] > HMS(14,55):
            if long_size:
                self.place_order(self.contract, quote['bid1'], long_size, "SellToday")
            if short_size:
                self.place_order(self.contract, quote['bid1'], short_size, "CoverToday")

        bars, msg = dapi.bar(self.contract, "1m", 0, df_value=False)
        assert bars is not None, "dapi.bar error " + msg

        bar_1 = bars[-1]
        bar_2 = bars[-2]

        # 向上突破
        if bar_2['close'] <= self.price_range.buy_break and bar_1['close'] > self.price_range.buy_break:
            if not long_size:
                self.place_order(self.contract, quote['ask1'], 1, "Buy")

            if short_size:
                self.place_order(self.contract, quote['ask1'], short_size, "CoverToday")

        # 向下突破
        if bar_2['close'] >= self.price_range.sell_break and bar_1['close']< self.price_range.sell_break :
            if not short_size:
                self.place_order(self.contract, quote['bid1'], 1, "Short")

            if long_size:
                self.place_order(self.contract, quote['bid1'], long_size, "SellToday")

        # 多单反转
        if bar_1['high'] > self.price_range.sell_setup and bar_1['close'] > self.price_range.sell_enter:
            self.count_1 = 1

        if self.count_1 == 1 and bar_1['close'] < self.price_range.sell_enter :
            if long_size :
                self.place_order(self.contract, quote['bid1'], long_size, "SellToday")
                self.place_order(self.contract, quote['bid1'], 1, "Short")

        # 空单反转
        if bar_1["low"] < self.price_range.buy_setup:
            self.count_2 = 1

        if self.count_2 == 1 and bar_1['close'] > self.price_range.buy_enter:
            if short_size:
                self.place_order(self.contract, quote['ask1'], short_size, "CoverToday")
                self.place_order(self.contract, quote['ask1'], 1, "Buy")

def run_backtest():
    cfg = {
        'begin_date': 20180101,
        'end_date'  : 20181231,
        'data_level': '1m'
    }

    tq.bt_run(cfg, RBreakerStralet)


def run_realtime():
    cfg = {
        'dapi_addr': 'ipc://tqc_10001',
        'tapi_addr': 'ipc://10201'
    }

    tq.rt_run(cfg, RBreakerStralet)


if __name__ == "__main__":
    import sys

    if 'realtime' in sys.argv:
        run_realtime()
    else:
        run_backtest()

#run_backtest()


