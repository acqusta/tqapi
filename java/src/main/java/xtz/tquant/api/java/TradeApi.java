package xtz.tquant.api.java;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

import java.util.List;

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
 *
 *  date 日期格式
 *      使用整数表示，year + month + day
 *          20170210 = "2017-02-10"
 *
 */
public interface TradeApi {

    interface Callback {
        void onOrderStatus   (Order order);
        void onOrderTrade    (Trade trade);
        void onAccountStatus (AccountInfo account);
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    class AccountInfo {
        public String account_id;       // 帐号编号
        public String broker;           // 交易商名称，如招商证券
        public String account;          // 交易帐号
        public String status;           // 连接状态，取值 Disconnected, Connected, Connecting
        public String msg;              // 状态信息，如登录失败原因
        public String account_type;     // 帐号类型，如 stock, ctp
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    class Balance {
        public String account_id;       // 帐号编号
        public String fund_account;     // 资金帐号
        public double init_balance;     // 初始化资金
        public double enable_balance;   // 可用资金
        public double margin;           // 保证金
        public double float_pnl;        // 浮动盈亏
        public double close_pnl;        // 实现盈亏
    }

    class  OrderStatus {
        public static String New        = "New";
        public static String Accepted   = "Accepted";
        public static String Filled     = "Filled";
        public static String Rejected   = "Rejected";
        public static String Cancelled  = "Cancelled";
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    class Order {
        public String account_id;       // 帐号编号
        public String code;             // 证券代码
        public String name;             // 证券名称
        public String entrust_no;       // 委托编号
        public String entrust_action;   // 委托动作
        public double entrust_price;    // 委托价格
        public long   entrust_size;     // 委托数量，单位：股
        public int    entrust_date;     // 委托日期
        public int    entrust_time;     // 委托时间
        public double fill_price;       // 成交价格
        public long   fill_size;        // 成交数量
        public String status;           // 订单状态：取值: OrderStatus
        public int    order_id;         // 自定义订单编号
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    class Trade {
        public String account_id;       // 帐号编号
        public String code;             // 证券代码
        public String name;             // 证券名称
        public String entrust_no;       // 委托编号
        public String entrust_action;   // 委托动作
        public String fill_no;          // 成交编号
        public long   fill_size;        // 成交数量
        public double fill_price;       // 成交价格
        public int    fill_date;        // 成交日期
        public int    fill_time;        // 成交时间
    }

    class Side {
        public static String Long = "Long";
        public static String Short = "Short";
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    class Position {
        public String account_id;       // 帐号编号
        public String code;             // 证券代码
        public String name;             // 证券名称
        public long   current_size;     // 当前持仓
        public long   enable_size;      // 可用（可交易）持仓
        public long   init_size;        // 初始持仓
        public long   today_size;       // 今日持仓
        public long   frozen_size;      // 冻结持仓
        public String side;             // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
        public double cost;             // 成本
        public double cost_price;       // 成本价格
        public double last_price;       // 最新价格
        public double float_pnl;        // 持仓盈亏
        public double close_pnl;        // 平仓盈亏
        public double margin;           // 保证金
        public double commission;       // 手续费
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    class OrderID {
        public String entrust_no;       // 订单委托号
        public String order_id;         // 自定义编号
    }


    /**
     * 返回类型的基类
     *
     * @param <ValueType>
     */
    class CallResult<ValueType> {
        public ValueType value = null;  // 非null, 表示操作成功，成功返回的值，null 表示失败
        public String    msg = "";      // 结果消息，如果 value == null，为错误原因

        public CallResult(ValueType result, String msg) {
            this.value = result;
            this.msg = msg;
        }
    }


    /**
     * 查询帐号连接状态。
     *
     * @return
     */
    CallResult<List<AccountInfo>> queryAccountStatus  ();

    /**
     * 查询某个帐号的资金使用情况
     *
     * @param account_id
     * @return
     */
    CallResult<Balance> queryBalance (String account_id);

    /**
     * 查询某个帐号的当天的订单
     *
     * @param account_id
     * @return
     */
    CallResult<List<Order>> queryOrders(String account_id);

    /**
     * 查询某个帐号的当天的成交
     *
     * @param account_id
     * @return
     */
    CallResult<List<Trade>> queryTrades(String account_id);

    /**
     * 查询某个帐号的当天的持仓
     *
     * @param account_id
     * @return
     */
    CallResult<List<Position>> queryPosition(String account_id);

    /**
     * 下单
     *
     * 股票通道为同步下单模式，即必须下单成功必须返回委托号 entrust_no。
     *
     * CTP交易通道为异步下单模式，下单后立即返回自定义编号order_id。当交易所接受订单，生成委托号好，通过 Callback.onOrderStatus通知
     * 用户。用户可以通过order_id匹配。如果订单没有被接收，onOrderStatus回调函数中entrust_no为空，状态为Rejected。
     * 当参数order_id不为0，表示用户自己对订单编号，这时用户必须保证编号的唯一性。如果交易通道不支持order_id，该函数返回错误代码。
     *
     * @param account_id    帐号编号
     * @param code          证券代码
     * @param price         委托价格
     * @param size          委托数量
     * @param action        委托动作
     * @param order_id      自定义订单编号，不为0表示有值
     * @return OrderID      订单ID
     */
    CallResult<OrderID> placeOrder(String account_id, String code, double price, long size, String action, int order_id);

    /**
     * 根据订单号撤单
     *
     * security 不能为空
     *
     * @param account_id    帐号编号
     * @param code          证券代码
     * @param order_id      订单号
     * @return 是否成功
     */
    CallResult<Boolean> cancelOrder(String account_id, String code, int order_id);

    /**
     * 根据委托号撤单
     *
     * security 不能为空
     *
     * @param account_id    帐号编号
     * @param code          证券代码
     * @param entrust_no    委托编号
     * @return 是否成功
     */
    CallResult<Boolean> cancelOrder(String account_id, String code, String entrust_no);


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
    CallResult<String> query(String account_id, String command, String params);
    /**
     * 设置 TradeApi.Callback
     *
     * @param callback
     */
    void setCallback(Callback callback);
}
