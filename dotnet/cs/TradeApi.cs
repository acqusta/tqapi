using System;
using System.Runtime.InteropServices;

namespace TQuant
{
    namespace Api
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
        public class AccountInfo
        {
            public String account_id;       // 帐号编号
            public String broker;           // 交易商名称，如招商证券
            public String account;          // 交易帐号
            public String status;           // 连接状态，取值 Disconnected, Connected, Connecting
            public String msg;              // 状态信息，如登录失败原因
            public String account_type;     // 帐号类型，如 stock, ctp
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
		public class Balance
        {
            public String account_id;       // 帐号编号
            public String fund_account;     // 资金帐号
            public Double init_balance;     // 初始化资金
            public Double enable_balance;   // 可用资金
            public Double margin;           // 保证金
            public Double float_pnl;        // 浮动盈亏
            public Double close_pnl;        // 实现盈亏
        }

        public class OrderStatus
        {
            public const String New        = "New"      ;
            public const String Accepted   = "Accepted" ;
            public const String Filled     = "Filled"   ;
            public const String Rejected   = "Rejected" ;
            public const String Cancelled  = "Cancelled";
        }

        public class EntrustAction
        {
            public const String Buy            = "Buy"            ;
            public const String Short          = "Short"          ;
            public const String Cover          = "Cover"          ;
            public const String Sell           = "Sell"           ;
            public const String CoverToday     = "CoverToday"     ;
            public const String CoverYesterday = "CoverYesterday" ;
            public const String SellToday      = "SellToday"      ;
            public const String SellYesterday = "SellYesterday"   ;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
		public class Order
        {
            public String account_id;       // 帐号编号
            public String code;             // 证券代码
            public String name;             // 证券名称
            public String entrust_no;       // 委托编号
            public String entrust_action;   // 委托动作
            public Double entrust_price;    // 委托价格
            public Int64  entrust_size;     // 委托数量，单位：股
            public Int32  entrust_date;     // 委托日期
            public Int32  entrust_time;     // 委托时间
            public Double fill_price;       // 成交价格
            public Int64  fill_size;        // 成交数量
            public String status;           // 订单状态：取值: OrderStatus
            public String status_msg;       // 状态消息
            public Int32  order_id;         // 自定义订单编号
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
		public class Trade
        {
            public String account_id;       // 帐号编号
            public String code;             // 证券代码
            public String name;             // 证券名称
            public String entrust_no;       // 委托编号
            public String entrust_action;   // 委托动作
            public String fill_no;          // 成交编号
            public Int64  fill_size;        // 成交数量
            public Double fill_price;       // 成交价格
            public Int32  fill_date;        // 成交日期
            public Int32  fill_time;        // 成交时间
        }

        public struct Side
        {
            public const String Long = "Long";
            public const String Short = "Short";
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
		public class Position
        {
            public String account_id;       // 帐号编号
            public String code;             // 证券代码
            public String name;             // 证券名称
            public Int64  current_size;     // 当前持仓
            public Int64  enable_size;      // 可用（可交易）持仓
            public Int64  init_size;        // 初始持仓
            public Int64  today_size;       // 今日持仓
            public Int64  frozen_size;      // 冻结持仓
            public String side;             // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
            public Double cost;             // 成本
            public Double cost_price;       // 成本价格
            public Double last_price;       // 最新价格
            public Double float_pnl;        // 持仓盈亏
            public Double close_pnl;        // 平仓盈亏
            public Double margin;           // 保证金
            public Double commission;       // 手续费
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
		public class OrderID
        {
            public String entrust_no;
            public Int32  order_id;
        }

        public interface TradeApiCallback
        {
            void OnOrderStatus  (Order order);
            void OnOrderTrade   (Trade trade);
            void OnAccountStatus(AccountInfo account);
        };

        public interface TradeApi
        {
            /**
            * 查询帐号连接状态。
            *
            * @return
            */
            CallResult<AccountInfo[]> QueryAccountStatus();

            /**
            * 查询某个帐号的资金使用情况
            *
            * @param account_id
            * @return
            */
            CallResult<Balance> QueryBalance(string account_id);

            /**
            * 查询某个帐号的当天的订单
            *
            * @param account_id
            * @return
            */
            CallResult<Order[]> QueryOrders(string account_id);

            /**
            * 查询某个帐号的当天的成交
            *
            * @param account_id
            * @return
            */
            CallResult<Trade[]> QueryTrades(string account_id);

            /**
            * 查询某个帐号的当天的持仓
            *
            * @param account_id
            * @return
            */
            CallResult<Position[]> QueryPositions(string account_id);

            /**
            * 下单
            *
            * 股票通道为同步下单模式，即必须下单成功必须返回委托号 entrust_no。
            *
            * CTP交易通道为异步下单模式，下单后立即返回自定义编号order_id。当交易所接受订单，生成委托号好，通过 Callback.on_order_status通知
            * 用户。用户可以通过order_id匹配。如果订单没有被接收，on_order_status回调函数中entrust_no为空，状态为Rejected。
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
            CallResult<OrderID> PlaceOrder(string account_id, string code, double price, long size, string action, int order_id = 0);

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
            CallResult<Boolean> CanceOrder(string account_id, string code, int order_id);

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
            CallResult<Boolean> CanceOrder(string account_id, string code, string entrust_no, int order_id = 0);

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
            CallResult<String> Query(string account_id, string command, string data);

            /**
            * 设置 TradeApi.Callback
            *
            * @param callback
            */
            void SetCallback(TradeApiCallback callback);
        };
    }
}
