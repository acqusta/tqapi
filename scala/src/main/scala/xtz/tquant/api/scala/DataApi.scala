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
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    class MarketQuote {
        var code        : String  = _    // 证券代码
        var date        : Int     = _    // 行情日期
        var time        : Int     = _    // 行情时间
        var trading_day : Int     = _    // 交易日
        var open        : Double  = _    // 开盘价
        var high        : Double  = _    // 最高价
        var low         : Double  = _    // 最低价
        var close       : Double  = _    // 收盘价
        var last        : Double  = _    // 最新价
        var high_limit  : Double  = _    // 涨停价
        var low_limit   : Double  = _    // 跌停价
        var pre_close   : Double  = _    // 昨收价
        var volume      : Long    = _    // 成交量
        var turnover    : Double  = _    // 成交金额
        var ask1        : Double  = _    // 卖一价
        var ask2        : Double  = _
        var ask3        : Double  = _
        var ask4        : Double  = _
        var ask5        : Double  = _
        var ask6        : Double  = _
        var ask7        : Double  = _
        var ask8        : Double  = _
        var ask9        : Double  = _
        var ask10       : Double  = _
        var bid1        : Double  = _    // 买一价
        var bid2        : Double  = _
        var bid3        : Double  = _
        var bid4        : Double  = _
        var bid5        : Double  = _
        var bid6        : Double  = _
        var bid7        : Double  = _
        var bid8        : Double  = _
        var bid9        : Double  = _
        var bid10       : Double  = _
        var ask_vol1    : Long    = _   // 卖一量
        var ask_vol2    : Long    = _
        var ask_vol3    : Long    = _
        var ask_vol4    : Long    = _
        var ask_vol5    : Long    = _
        var ask_vol6    : Long    = _
        var ask_vol7    : Long    = _
        var ask_vol8    : Long    = _
        var ask_vol9    : Long    = _
        var ask_vol10   : Long    = _
        var bid_vol1    : Long    = _   // 买一量
        var bid_vol2    : Long    = _
        var bid_vol3    : Long    = _
        var bid_vol4    : Long    = _
        var bid_vol5    : Long    = _
        var bid_vol6    : Long    = _
        var bid_vol7    : Long    = _
        var bid_vol8    : Long    = _
        var bid_vol9    : Long    = _
        var bid_vol10   : Long    = _
        var settle      : Double  = _   // 结算价
        var pre_settle  : Double  = _   // 昨结算价
        var oi          : Long    = _   // OpenInterest       未平仓量
        var pre_oi      : Long    = _   // Pre-OpenInterest   昨未平仓量
    }

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
        turnover        : Double    // bar的成交金额
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
     * @return
     */
    def bar (code : String, cycle : String, trading_day: Int) : (Seq[Bar], String)

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
