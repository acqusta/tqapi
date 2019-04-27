// extern crate libc;
// //extern crate tqapi;

// mod tqapi_ffi;

mod tqapi_ffi;
mod dapi;

pub mod api {
    pub use super::dapi::*;
}

// pub mod tquant { pub mod api {

//     use std::fmt;
//     use std::ffi::CStr;
//     use std::os::raw::c_char;
//     // use std::os::raw::c_int;
//     // use std::os::raw::c_double;
//     use tqapi_ffi::*;

//     pub struct MarketQuote{
//         code : String,
//         date : i32,
//         time : i32,
//         recv_time : i64,
//         trade_date: i32,
//         open  : f64,
//         high  : f64,
//         low   : f64,
//         close : f64,
//         last  : f64,
//         high_limit : f64,
//         low_limit  : f64,
//         pre_close  : f64,
//         volume     : i64,
//         turnover   : f64,
//         ask1 : f64,
//         ask2 : f64,
//         ask3 : f64,
//         ask4 : f64,
//         ask5 : f64,
//         bid1 : f64,
//         bid2 : f64,
//         bid3 : f64,
//         bid4 : f64,
//         bid5 : f64,
//         ask_vol1 : i64,
//         ask_vol2 : i64,
//         ask_vol3 : i64,
//         ask_vol4 : i64,
//         ask_vol5 : i64,
//         bid_vol1 : i64,
//         bid_vol2 : i64,
//         bid_vol3 : i64,
//         bid_vol4 : i64,
//         bid_vol5 : i64,
//         settle   : f64,
//         pre_settle : f64,
//         oi         : i64,
//         pre_oi     : i64
//     }

//     pub struct Bar {
//         code  : String,
//         date  : i32,
//         time  : i32,
//         trade_date : i32,
//         open  : f64,
//         high  : f64,
//         low   : f64,
//         close : f64,
//         volume : i64,
//         turnover : f64,
//         oi       : i64
//     }

//     impl Bar {
//         pub fn new() -> Bar{
//             Bar { code : String::from(""), date : 0, time : 0, trade_date : 0,
//              open : 0.0, high : 0.0, low : 0.0, close : 0.0,
//              volume : 0, turnover : 0.0, oi : 0}
//         }
//     }

//     impl fmt::Display for Bar {
//         fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//             write!(f, "Bar({},{},{},{},{},{})", self.time,self.code,self.open,self.high,self.low,self.close)
//         }
//     }

//     pub struct DailyBar {
//         code : String,
//         date : i32,
//         time : i32,
//         trade_date : i32,
//         open     : f64,
//         high     : f64,
//         low      : f64,
//         close    : f64,
//         volume   : i64,
//         turnover : f64,
//         oi       : i64,
//         settle   : f64,
//         pre_close : f64,
//         pre_settle : f64,
//         af  : f64
//     }

//     pub trait DataApiCallback {
//         fn on_quote(&mut self, quote : MarketQuote);
//         fn on_bar  (&mut self, cycle : String, bar : Bar);
//     }


//     pub struct DataApi {
//         cb : Option<Box<DataApiCallback>>,
//         dapi : *mut CDataApi,
//     }

//     impl DataApi {
//         pub fn new(addr: &str) -> DataApi {

//             DataApi{ cb : None }
//         }

//         pub fn get_ticks(&mut self, code : &str, trade_date : i32) -> Result<Vec<MarketQuote>, String> {
//             Err("Wrong".to_string())
//         }

//         pub fn get_bars(&mut self, code : &str, trade_date : i32) -> Result<Vec<Bar>, String> {
//             let bars = Vec::<Bar>::new();
//             return Ok(bars);
//         }

//         pub fn get_dailybars(&mut self, code : &str, trade_date : i32) -> Result<Vec<DailyBar>, String> {
//             Err("Wrong".to_string())
//         }

//         pub fn get_quote(&mut self, code : &str, trade_date : i32) -> Result<Vec<MarketQuote>, String> {
//             Err("Wrong".to_string())
//         }

//         pub fn subscribe(&mut self, codes : &Vec<String>) -> Result<Vec<String>, String> {
//             Err("Wrong".to_string())
//         }

//         pub fn unsubscribe(&mut self, codes : &Vec<String>) -> Result<Vec<String>, String> {
//             Err("Wrong".to_string())
//         }

//         pub fn set_callback<T: DataApiCallback + 'static >(&mut self, cb : T) {
//             self.cb = Some(Box::new(cb));
//         }
//     }


//     // TradeApi

//     pub struct AccountInfo {
//         account_id   : String,    // 帐号编号
//         broker       : String,    // 交易商名称，如招商证券
//         account      : String,    // 交易帐号
//         status       : String,    // 连接状态，取值 Disconnected, Connected, Connecting
//         msg          : String,    // 状态信息，如登录失败原因
//         account_type : String,    // 帐号类型，如 stock, ctp
//     }

//     pub struct Balance {
//         account_id : String,
//         fund_account : String,
//         init_balance : String,
//         enable_balance : String,
//         margin : f64,
//         float_pnl : f64,
//         close_pnl : f64,
//     }

//     pub enum OrderStatus {
//         New,
//         Accepted,
//         Filled,
//         Rejected,
//         Cancelled,
//         Other(String)
//     }

//     pub enum EntrustAction {
//         Buy,
//         Short,
//         Cover,
//         Sell,
//         CoverToday,
//         CoverYesterday,
//         SellToday,
//         SellYesterday,
//         Other(String)
//     }

//     pub struct Order {
//         account_id       : String,         // 帐号编号
//         code             : String,         // 证券代码
//         name             : String,         // 证券名称
//         entrust_no       : String,         // 委托编号
//         entrust_action   : EntrustAction,  // 委托动作
//         entrust_price    : f64,            // 委托价格
//         entrust_size     : i64,            // 委托数量，单位：股
//         entrust_date     : i32,            // 委托日期
//         entrust_time     : i32,            // 委托时间
//         fill_price       : f64,            // 成交价格
//         fill_size        : i64,            // 成交数量
//         status           : OrderStatus,    // 订单状态：取值: OrderStatus
//         status_msg       : String,         // 状态消息
//         order_id         : i32             // 自定义订单编号
//     }

//     pub struct Trade {
//         account_id       : String,     // 帐号编号
//         code             : String,     // 证券代码
//         name             : String,     // 证券名称
//         entrust_no       : String,     // 委托编号
//         entrust_action   : EntrustAction, // 委托动作
//         fill_no          : String,        // 成交编号
//         fill_size        : i64,           // 成交数量
//         fill_price       : f64,           // 成交价格
//         fill_date        : i32,           // 成交日期
//         fill_time        : i32,           // 成交时间
//         order_id         : i32,           // 自定义订单编号
//     }

//     pub enum Side {
//         Long,
//         Short,
//         Other(String)
//     }

//     pub struct Position {
//         account_id    : String,   // 帐号编号
//         code          : String,   // 证券代码
//         name          : String,   // 证券名称
//         current_size  : i64,      // 当前持仓
//         enable_size   : i64,      // 可用（可交易）持仓
//         init_size     : i64,      // 初始持仓
//         today_size    : i64,      // 今日持仓
//         frozen_size   : i64,      // 冻结持仓
//         side          : Side,     // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
//         cost          : f64,      // 成本
//         cost_price    : f64,      // 成本价格
//         last_price    : f64,      // 最新价格
//         float_pnl     : f64,      // 持仓盈亏
//         close_pnl     : f64,      // 平仓盈亏
//         margin        : f64,      // 保证金
//         commission    : f64,      // 手续费
//     }

//     pub struct OrderID {
//         entrust_no   : String,    // 订单委托号
//         order_id     : i32        // 自定义编号
//     }


//     pub trait TradeApiCallback {
//         fn on_order_status   (order   : Order);
//         fn on_order_trade    (trade   : Trade);
//         fn on_account_status (account : AccountInfo);
//     }

//     pub struct TradeApi {

//     }

//     impl TradeApi {
//         fn query_account_status() -> Result<Vec<AccountInfo>, String> {
//             Err("Wrong".to_string())
//         }

//         fn query_balance(account_id : &str) -> Result<Balance, String> {
//             Err("Wrong".to_string())
//         }

//         fn query_orders(account_id : &str, codes : Option<&Vec<String>> ) -> Result<Vec<Order>, String> {
//             Err("Wrong".to_string())
//         }

//         fn query_trades (account_id : &str, codes : Option<&Vec<String>> ) -> Result<Vec<Trade>, String> {
//             Err("Wrong".to_string())
//         }

//         fn query_positions (account_id : &str, codes : Option<&Vec<String>>) -> Result<Vec<Position>, String> {
//             Err("Wrong".to_string())
//         }

//         fn place_order(account_id : &str, code : &str, price : f64, size : i64, action : &EntrustAction,
//                 price_type: &str, order_id : i32) -> Result<OrderID, String> {
//             Err("Wrong".to_string())
//         }

//         fn cancel_order(account_id: &str, code : &str, order: &OrderID) -> Result<bool, String> {
//             Err("Wrong".to_string())
//         }

//         // fn cancel_order(account_id: &str, code : &str, entrust_no : &str) -> Result<bool, String> {

//         // }

//         fn query(account_id : &str, command : &str, params: &str) -> Result<String, String> {
//             Err("Wrong".to_string())
//         }

//         fn set_callback(cb : impl TradeApiCallback) {
//         }

//         // static pub fn new(addr : &str) -> TradeApi {
//         // }
//     }

//     // _TQAPI_EXPORT DataApi*  create_data_api (const string& addr);

//     // _TQAPI_EXPORT TradeApi* create_trade_api(const string& addr);

//     // _TQAPI_EXPORT void set_params(const string& key, const string& value);

//     // typedef DataApi*  (*T_create_data_api)(const char* str_params);
//     // typedef TradeApi* (*T_create_trade_api)(const char* str_params);


//     // _TQAPI_EXPORT void register_trade_api_factory(T_create_trade_api factory);

// } }

// #[cfg(test)]
// mod tests {
//     #[test]
//     fn it_works() {
//         assert_eq!(2 + 2, 4);
//     }
// }
