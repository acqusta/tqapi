package com.acqusta.tquant.api;

/**
 *  数据接口
 *
 *  功能：
 *      查实时行情，当天和历史的tick, 分钟线
 *      订阅和推送行情
 */
public interface DataApi {

    interface Callback {
        void onMarketQuote(MarketQuote quote);

        void onBar(String cycle, Bar bar);
    }

    class MarketQuote {
        public String code;          // 证券代码
        public int date;             // 行情日期
        public int time;             // 行情时间
        public int trading_day;      // 交易日
        public double open;          // 开盘价
        public double high;          // 最高价
        public double low;           // 最低价
        public double close;         // 收盘价
        public double last;          // 最新价
        public double high_limit;    // 涨停价
        public double low_limit;     // 跌停价
        public double pre_close;     // 昨收价
        public long   volume;        // 成交量
        public double turnover;      // 成交金额
        public double ask1;          // 卖一价
        public double ask2;
        public double ask3;
        public double ask4;
        public double ask5;
        public double ask6;
        public double ask7;
        public double ask8;
        public double ask9;
        public double ask10;
        public double bid1;          // 买一价
        public double bid2;
        public double bid3;
        public double bid4;
        public double bid5;
        public double bid6;
        public double bid7;
        public double bid8;
        public double bid9;
        public double bid10;
        public long   ask_vol1;     // 卖一量
        public long   ask_vol2;
        public long   ask_vol3;
        public long   ask_vol4;
        public long   ask_vol5;
        public long   ask_vol6;
        public long   ask_vol7;
        public long   ask_vol8;
        public long   ask_vol9;
        public long   ask_vol10;
        public long   bid_vol1;     // 买一量
        public long   bid_vol2;
        public long   bid_vol3;
        public long   bid_vol4;
        public long   bid_vol5;
        public long   bid_vol6;
        public long   bid_vol7;
        public long   bid_vol8;
        public long   bid_vol9;
        public long   bid_vol10;
        public double settle;       // 结算价
        public double pre_settle;   // 昨结算价
        public long   oi;           // OpenInterest       未平仓量
        public long   pre_oi;       // Pre-OpenInterest   昨未平仓量
    }

    class Bar {
        public String code;              // 证券代码
        public int    date;              // 行情日期
        public int    time;              // 行情时间
        public int    trading_day;       // 交易日
        public double open;              // bar的开盘价
        public double high;              // bar的最高价
        public double low;               // bar的最低价
        public double close;             // bar的收盘价
        public long   volume;            // bar的成交量
        public double turnover;          // bar的成交金额
        public long   oi;                // bar结束时总持仓量
    }

    class DailyBar {
        public String code;              // 证券代码
        public int    date;              // 行情日期
        public double open;              // bar的开盘价
        public double high;              // bar的最高价
        public double low;               // bar的最低价
        public double close;             // bar的收盘价
        public long   volume;            // bar的成交量
        public double turnover;          // bar的成交金额
        public long   oi;                // bar结束时总持仓量
        public double settle;
        public double pre_close;
        public double pre_settle;
    }

    class CallResult<ValueType> {
        public ValueType value = null;
        public String    msg = "";

        public CallResult(ValueType result, String msg) {
            this.value = result;
            this.msg = msg;
        }
    }

    /**
     * 取某交易日的某个代码的 ticks
     *
     * tradingday 为0，表示当前交易日
     *
     * @param code
     * @param trading_day
     * @param source
     * @return
     */
    CallResult<MarketQuote[]> getTick(String code, int trading_day, String source);

    /**
     * 取某个代码的Bar
     *
     * 目前只支持分钟线。
     *  当 cycle == "1m"时，返回trading_day的分钟线，trading_day=0表示当前交易日。
     *
     * @param code          证券代码
     * @param cycle         "1m"
     * @param trading_day   交易日
     * @param align         是否对齐
     * @param source
     * @return
     */
    CallResult<Bar[]> getBar (String code, String cycle, int trading_day, Boolean align, String source);

    /**
     * 取某个代码的DailyBar
     *
     * @param code          证券代码
     * @param price_adj     价格复权，取值
     *                        back -- 后复权
     *                        forward -- 前复权
     * @param align         是否对齐
     * @param source
     * @return
     */
    CallResult<DailyBar[]> getDailyBar (String code, String price_adj, Boolean align, String source);

    /**
     * 取当前的行情快照
     *
     * @param code
     * @param source
     * @return
     */
    CallResult<MarketQuote> getQuote (String code, String source);

    /**
     * 订阅行情
     *
     * codes为新增的订阅列表，返回所有已经订阅的代码,包括新增的列表。如果codes为空，可以返回已订阅列表。
     *
     * @param codes
     * @param source
     * @return 所有已经订阅的代码
     */
    CallResult<String[]> subscribe(String[] codes, String source);

    /**
     * 取消订阅
     *
     * codes为需要取消的列表，返回所有还在订阅的代码。
     * 如果需要取消所有订阅，先通过 subscribe 得到所有列表，然后使用unscribe取消
     *
     * @param codes
     * @param source
     * @return
     */
    CallResult<String[]> unsubscribe(String[] codes, String source);

    /**
     * 设置推送行情的回调函数
     *
     * 当订阅的代码列表中有新的行情，会通过该callback通知用户。
     *
     * @param callback
     */
    void setCallback(Callback callback);
}
