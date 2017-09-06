package xtz.tquant.api.scala

import com.fasterxml.jackson.annotation.JsonIgnoreProperties


/**
 *  数据查询接口
 *
 *  功能：
 *      查实时行情，当天的tick, 分钟线
 *      订阅和推送行情
 */
object DataApi {

    trait Callback {
        def onMarketQuote(quote : MarketQuote)
        def onBar(cycle: String, bar : Bar)
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class MarketQuote (
        code        : String  ,    // 证券代码
        date        : Int     ,    // 行情日期
        time        : Int     ,    // 行情时间
        trading_day : Int     ,    // 交易日
        open        : Double  ,    // 开盘价
        high        : Double  ,    // 最高价
        low         : Double  ,    // 最低价
        close       : Double  ,    // 收盘价
        last        : Double  ,    // 最新价
        high_limit  : Double  ,    // 涨停价
        low_limit   : Double  ,    // 跌停价
        pre_close   : Double  ,    // 昨收价
        volume      : Long    ,    // 成交量
        turnover    : Double  ,    // 成交金额
        ask1        : Double  ,    // 卖一价
        ask2        : Double  ,
        ask3        : Double  ,
        ask4        : Double  ,
        ask5        : Double  ,
        ask6        : Double  ,
        ask7        : Double  ,
        ask8        : Double  ,
        ask9        : Double  ,
        ask10       : Double  ,
        bid1        : Double  ,    // 买一价
        bid2        : Double  ,
        bid3        : Double  ,
        bid4        : Double  ,
        bid5        : Double  ,
        bid6        : Double  ,
        bid7        : Double  ,
        bid8        : Double  ,
        bid9        : Double  ,
        bid10       : Double  ,
        ask_vol1    : Long    ,   // 卖一量
        ask_vol2    : Long    ,
        ask_vol3    : Long    ,
        ask_vol4    : Long    ,
        ask_vol5    : Long    ,
        ask_vol6    : Long    ,
        ask_vol7    : Long    ,
        ask_vol8    : Long    ,
        ask_vol9    : Long    ,
        ask_vol10   : Long    ,
        bid_vol1    : Long    ,   // 买一量
        bid_vol2    : Long    ,
        bid_vol3    : Long    ,
        bid_vol4    : Long    ,
        bid_vol5    : Long    ,
        bid_vol6    : Long    ,
        bid_vol7    : Long    ,
        bid_vol8    : Long    ,
        bid_vol9    : Long    ,
        bid_vol10   : Long    ,
        settle      : Double  ,   // 结算价
        pre_settle  : Double  ,   // 昨结算价
        oi          : Long    ,   // OpenInterest       未平仓量
        pre_oi      : Long       // Pre-OpenInterest   昨未平仓量
    )

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class Bar (
        code            : String ,  // 证券代码
        date            : Int    ,  // 行情日期
        time            : Int    ,  // 行情时间
        trading_day     : Int    ,  // 交易日
        open            : Double ,  // bar的开盘价
        high            : Double ,  // bar的最高价
        low             : Double ,  // bar的最低价
        close           : Double ,  // bar的收盘价
        volume          : Long   ,  // bar的成交量
        turnover        : Double ,  // bar的成交金额
        oi              : Long      // bar结束时的总// 持仓量
    )
}

trait DataApi {

    import DataApi._

    /**
     * 取某交易日的某个代码的 ticks
     *
     * tradingday 为0，表示当前交易日
     *
     * @param code
     * @param trading_day
     * @return
     */
    def tick(code: String, trading_day : Int = 0) : (Seq[MarketQuote], String)

    /**
     * 取Bar
     *
     * 目前只支持分钟线和日线。
     *  当 cycle == "1m"时，返回trading_day的分钟线，trading_day=0表示当前交易日。
     *  当 cycle == "1d"时，返回所有的日线，trading_day值无意义。
     *
     * @param code          证券代码
     * @param cycle         "1m" 或 "1d"
     * @param trading_day   交易日，对分钟线有意义
     * @param price_adj     价格复权，取值
     *                        back -- 后复权
     *                        forward -- 前复权
     *
      * @return
     */
    def bar (code : String, cycle : String, trading_day: Int = 0, price_adj : String = "") : (Seq[Bar], String)

        /**
     * 取当前的行情快照
     *
     * @param code
     * @return
     */
    def quote (code: String) : (MarketQuote, String)

    /**
     * 订阅行情
     *
     * codes为新增的订阅列表，返回所有已经订阅的代码,包括新增的列表。如果codes为空，可以返回已订阅列表。
     *
     * @param codes
     * @return 所有已经订阅的代码
     */
    def subscribe(codes: Seq[String]) : (Seq[String], String)

    /**
     * 取消订阅
     *
     * codes为需要取消的列表，返回所有还在订阅的代码。
     * 如果需要取消所有订阅，先通过 subscribe 得到所有列表，然后使用unscribe取消

     * @param codes
     * @return
     */
    def unsubscribe(codes: Seq[String]) : (Seq[String], String)

    /**
     * 设置推送行情的回调函数
     *
     * 当订阅的代码列表中有新的行情，会通过该callback通知用户。
     *
     * @param callback
     */
    def setCallback(callback : Callback)
}
