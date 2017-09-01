package xtz.tquant.api.scala

import com.fasterxml.jackson.annotation.JsonIgnoreProperties

/**
 * 交易接口
 *
 * 功能：
 *   资金、持仓、订单、成交查询接口。查询接口都是基于当前交易日查询，不支持查询历史数据。
 *   下单接口
 *
 * 操作返回两个字段 （value, msg)。value 为空，表示该操作失败，失败原因通过msg返回。
 *
 * 通用字段说明
 *
 *  code 证券代码
 *      格式为  交易代码 + '.' + 交易所代码
 *          SH  上交所
 *          SZ  深交所
 *          CFE 中金所
 *          DCE 大连商品交易所
 *          CZC 郑州商品交易所
 *          SHF 上海期货交易所
 *      例子： 000001.SH, IF1702.CFE, cu1702.SHF
 *
 *  entrust_action 委托动作
 *      使用字符串表示订单的委托动作，取值如下：
 *          Buy             股票 买        期货 开 多
 *          Short               -             开  空
 *          Cover               -             平  空
 *          Sell                卖            平  多
 *          CoverToday          -             平今 空
 *          SellToday           -             平今 多
 *          CoverYesterday      -             平昨 空
 *          SellYesterday       -             平昨 多
 *
 *      股票只有 Buy, Sell，期货有动作种操作
 *
 *  size, entrust_size, fill_size 股票数量
 *      单位为 股，不是手
 *
 *  account_id
 *      用户自定义的帐号编号，不能重复。每个证券账户需要有一个唯一的帐号，用于查询、下单等操作。
 *
 *  time 时间格式
 *      使用整数表示，Hour+minute+second+millisecond
 *          123001234 = "12:30:01.234"

 *  date 日期格式
 *      使用整数表示，year + month + day
 *          20170210 = "2017-02-10"
 *
 */
object TradeApi {

    trait Callback {
        def onOrderStatus    (order   : Order)
        def onOrderTrade     (trade   : Trade)
        def onAccountStatus  (account : AccountInfo)
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class AccountInfo (
        account_id : String,      // 帐号编号
        broker     : String,      // 交易商名称，如招商证券
        account    : String,      // 交易帐号
        status     : String,      // 连接状态，取值 Disconnected, Connected, Connecting
        msg        : String       // 状态信息，如登录失败原因
    )

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class Balance (
        account_id      : String,  // 帐号编号
        fund_account    : String,  // 资金帐号
        init_balance    : Double,  // 初始化资金
        enable_balance  : Double,  // 可用资金
        margin          : Double,  // 保证金
        float_pnl       : Double,  // 浮动盈亏
        close_pnl       : Double   // 实现盈亏
    )

    trait OrderStatus {
        val New        = "New"
        val Accepted   = "Accepted"
        val Filled     = "Filled"
        val Rejected   = "Rejected"
        val Cancelled  = "Cancelled"
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class Order (
        account_id     : String  ,   // 帐号编号
        code           : String  ,   // 证券代码
        name           : String  ,   // 证券名称
        entrust_no     : String  ,   // 委托编号
        entrust_action : String  ,   // 委托动作
        entrust_price  : Double  ,   // 委托价格
        entrust_size   : Long    ,   // 委托数量，单位：股
        entrust_date   : Int     ,   // 委托日期
        entrust_time   : Int     ,   // 委托时间
        fill_price     : Double  ,   // 成交价格
        fill_size      : Long    ,   // 成交数量
        status         : String      // 订单状态：取值: OrderStatus
    )

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class Trade (
        account_id     : String  ,  // 帐号编号
        code           : String  ,  // 证券代码
        name           : String  ,  // 证券名称
        entrust_no     : String  ,  // 委托编号
        entrust_action : String  ,  // 委托动作
        fill_no        : String  ,  // 成交编号
        fill_size      : Long    ,  // 成交数量
        fill_price     : Double  ,  // 成交价格
        file_date      : Int     ,  // 成交日期
        fill_time      : Int        // 成交时间
    )

    trait Side {
        val Long = "Long"
        val Short = "Short"
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class Position (
        account_id     : String ,   // 帐号编号
        code           : String ,   // 证券代码
        name           : String ,   // 证券名称
        current_size   : Long   ,   // 当前数量
        enable_size    : Long   ,   // 可用（可交易）数量，
        side           : String ,   // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
        cost           : Double ,   // 成本
        cost_price     : Double ,   // 成本价格
        last_price     : Double ,   // 最新价格
        holding_pnl    : Double ,   // 持仓盈亏
        margin         : Double     // 保证金
    )

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class OrderID (
       entrust_no       : String,    // 订单委托号
       order_id         : Int        // 订单自定义ID
    )
}

trait TradeApi {

    import TradeApi._

    /**
     * 查询帐号连接状态。
     *
     * @return
     */
    def queryAccountStatus  () : (Seq[AccountInfo], String)

    /**
     * 查询某个帐号的资金使用情况
     *
     * @param account_id
     * @return
     */
    def queryBalance (account_id: String) : (Balance, String)

    /**
     * 查询某个帐号的当天的订单
     *
     * @param account_id
     * @return
     */
    def queryOrders( account_id : String) : (Seq[Order], String)

    /**
     * 查询某个帐号的当天的成交
     *
     * @param account_id
     * @return
     */
    def queryTrades(account_id: String) : (Seq[Trade], String)

    /**
     * 查询某个帐号的当天的持仓
     *
     * @param account_id
     * @return
     */
    def queryPosition( account_id : String) : (Seq[Position], String)

    /**
     * 下单
     *
     *  股票通道为同步下单模式，即必须下单成功必须返回委托号 entrust_no。
     *  CTP交易通道为异步下单模式，下单后立即返回自定义编号order_id。当交易所接受订单，生成委托号好，通过 Callback.onOrderStatus通知
     *  用户。用户可以通过order_id匹配。如果订单没有被接收，onOrderStatus回调函数中entrust_no为空，状态为Rejected。
     *  当参数order_id不为0，表示用户自己对订单编号，这时用户必须保证编号的唯一性。如果交易通道不支持order_id，该函数返回错误代码。
     *
     * @param account_id    帐号编号
     * @param code          证券代码
     * @param price         委托价格
     * @param size          委托数量
     * @param action        委托动作
     * @param order_id      自定义订单编号
     * @return 委托编号
     */
    def placeOrder(account_id: String, code: String, price : Double, size: Int,
                   action: String,
                   order_id : Int = 0) : (OrderID, String)

    /**
     * 撤单
     *
     * security 不能为空
     *
     * @param account_id    帐号编号
     * @param code          证券代码
     * @param entrust_no    委托编号
     * @return              是否成功
     */
    def cancelOrder(account_id: String, code: String,
                    entrust_no: String = "",
                    order_id:  Int = 0) : (Boolean, String)

    /**
      * 通用查询接口
      *
      * 用于查询交易通道特有的信息。如查询 CTP的代码表 command="ctp_codetable".
      * 返回字符串。
      *
      * @param account_id
      * @param command
      * @param params
      * @return
      */

    def query(account_id: String, command: String, params:String = "") : (String, String)
    /**
      * 设置 TradeApi.Callback
      *
      * @param callback
      */
    def setCallback(callback: Callback)
}
