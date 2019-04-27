extern crate libc;

use std::fmt;
use std::ffi::CStr;
use std::os::raw::c_char;
use super::dapi::MarketQuote;


#[repr(C, packed)]
pub struct CMarketQuote{
    code : *const c_char,
    date : i32,
    time : i32,
    recv_time : i64,
    trade_date: i32,
    open  : f64,
    high  : f64,
    low   : f64,
    close : f64,
    last  : f64,
    high_limit : f64,
    low_limit  : f64,
    pre_close  : f64,
    volume     : i64,
    turnover   : f64,
    ask1 : f64,
    ask2 : f64,
    ask3 : f64,
    ask4 : f64,
    ask5 : f64,
    bid1 : f64,
    bid2 : f64,
    bid3 : f64,
    bid4 : f64,
    bid5 : f64,
    ask_vol1 : i64,
    ask_vol2 : i64,
    ask_vol3 : i64,
    ask_vol4 : i64,
    ask_vol5 : i64,
    bid_vol1 : i64,
    bid_vol2 : i64,
    bid_vol3 : i64,
    bid_vol4 : i64,
    bid_vol5 : i64,
    settle   : f64,
    pre_settle : f64,
    oi         : i64,
    pre_oi     : i64
}

impl CMarketQuote {
    pub fn to_rs(&self) -> MarketQuote {
        unsafe {
            MarketQuote {
                code : CStr::from_ptr(self.code).to_string_lossy().into_owned(),
                date : self.date, time : self.time, recv_time : self.recv_time, trade_date: self.trade_date,
                open : self.open, high : self.high, low : self.low,close : self.close,
                last : self.last, high_limit : self.high_limit, low_limit : self.low_limit,
                pre_close : self.pre_close, volume : self.volume, turnover : self.turnover,
                ask1 : self.ask1, ask2 : self.ask2, ask3 : self.ask3, ask4 : self.ask4, ask5 : self.ask5,
                ask_vol1 : self.ask_vol1, ask_vol2 : self.ask_vol2, ask_vol3 : self.ask_vol3, ask_vol4 : self.ask_vol4,  ask_vol5 : self.ask_vol5,
                bid1 : self.bid1, bid2 : self.bid2, bid3 : self.bid3, bid4 : self.bid4, bid5 : self.bid5,
                bid_vol1 : self.bid_vol1, bid_vol2 : self.bid_vol2, bid_vol3 : self.bid_vol3, bid_vol4 : self.bid_vol4,  bid_vol5 : self.bid_vol5,
                settle : self.settle, pre_settle : self.pre_settle, oi : self.oi, pre_oi : self.pre_oi
            }
        }
    }
}

pub struct CBar {
    code  : String,
    date  : i32,
    time  : i32,
    trade_date : i32,
    open  : f64,
    high  : f64,
    low   : f64,
    close : f64,
    volume : i64,
    turnover : f64,
    oi       : i64
}
// impl Bar {
//     pub fn new() -> Bar{
//         Bar { code : String::from(""), date : 0, time : 0, trade_date : 0,
//          open : 0.0, high : 0.0, low : 0.0, close : 0.0,
//          volume : 0, turnover : 0.0, oi : 0}
//     }
// }

pub struct CDailyBar {
    code : String,
    date : i32,
    time : i32,
    trade_date : i32,
    open     : f64,
    high     : f64,
    low      : f64,
    close    : f64,
    volume   : i64,
    turnover : f64,
    oi       : i64,
    settle   : f64,
    pre_close : f64,
    pre_settle : f64,
    af  : f64
}

#[derive(Debug)]
#[repr(C, packed)]
pub struct GetTicksResult {
    _data            : *mut libc::c_void,
    pub ticks        : *mut CMarketQuote,
    pub ticks_length : i32,
    pub element_size : i32,
    pub msg          : *const c_char,
}

#[repr(C, packed)]
pub struct SubscribeResult {
    _data            : *mut libc::c_void,
    pub codes        : *const c_char,
    pub msg          : *const c_char,
}

#[repr(C, packed)]
pub struct UnSubscribeResult {
    _data            : *mut libc::c_void,
    pub codes        : *const c_char,
    pub msg          : *const c_char,
}

pub enum CDataApi {  }

#[link(name = "tqapi")]//, kind = "static")]
extern "C" {
    pub fn tqapi_create_data_api(addr : *const c_char) -> *mut CDataApi;
    pub fn tqapi_free_data_api(dapi : *mut CDataApi);

    pub fn tqapi_dapi_get_ticks(dapi : *mut CDataApi, code : *const c_char, trade_date : i32) -> *mut GetTicksResult;
    pub fn tqapi_dapi_free_get_ticks_result(dapi : *mut CDataApi, result : *mut GetTicksResult);

    pub fn tqapi_dapi_subscribe (dapi: *mut CDataApi, codes : *const c_char) -> *mut SubscribeResult;
    pub fn tqapi_dapi_free_subscribe_result (dapi: *mut CDataApi, result : *mut SubscribeResult);

    pub fn tqapi_dapi_unsubscribe (dapi: *mut CDataApi, codes : *const c_char) -> *mut UnSubscribeResult;
    pub fn tqapi_dapi_free_unsubscribe_result (dapi: *mut CDataApi, result : *mut UnSubscribeResult);

}

// TradeApi

struct CAccountInfo {
    account_id   : String,    // 帐号编号
    broker       : String,    // 交易商名称，如招商证券
    account      : String,    // 交易帐号
    status       : String,    // 连接状态，取值 Disconnected, Connected, Connecting
    msg          : String,    // 状态信息，如登录失败原因
    account_type : String,    // 帐号类型，如 stock, ctp
}

struct CBalance {
    account_id : String,
    fund_account : String,
    init_balance : String,
    enable_balance : String,
    margin : f64,
    float_pnl : f64,
    close_pnl : f64,
}


// struct COrder {
//     account_id       : String,         // 帐号编号
//     code             : String,         // 证券代码
//     name             : String,         // 证券名称
//     entrust_no       : String,         // 委托编号
//     entrust_action   : EntrustAction,  // 委托动作
//     entrust_price    : f64,            // 委托价格
//     entrust_size     : i64,            // 委托数量，单位：股
//     entrust_date     : i32,            // 委托日期
//     entrust_time     : i32,            // 委托时间
//     fill_price       : f64,            // 成交价格
//     fill_size        : i64,            // 成交数量
//     status           : OrderStatus,    // 订单状态：取值: OrderStatus
//     status_msg       : String,         // 状态消息
//     order_id         : i32             // 自定义订单编号
// }

// struct CTrade {
//     account_id       : String,     // 帐号编号
//     code             : String,     // 证券代码
//     name             : String,     // 证券名称
//     entrust_no       : String,     // 委托编号
//     entrust_action   : EntrustAction, // 委托动作
//     fill_no          : String,        // 成交编号
//     fill_size        : i64,           // 成交数量
//     fill_price       : f64,           // 成交价格
//     fill_date        : i32,           // 成交日期
//     fill_time        : i32,           // 成交时间
//     order_id         : i32,           // 自定义订单编号
// }

// struct CPosition {
//     account_id    : String,   // 帐号编号
//     code          : String,   // 证券代码
//     name          : String,   // 证券名称
//     current_size  : i64,      // 当前持仓
//     enable_size   : i64,      // 可用（可交易）持仓
//     init_size     : i64,      // 初始持仓
//     today_size    : i64,      // 今日持仓
//     frozen_size   : i64,      // 冻结持仓
//     side          : Side,     // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
//     cost          : f64,      // 成本
//     cost_price    : f64,      // 成本价格
//     last_price    : f64,      // 最新价格
//     float_pnl     : f64,      // 持仓盈亏
//     close_pnl     : f64,      // 平仓盈亏
//     margin        : f64,      // 保证金
//     commission    : f64,      // 手续费
// }

// struct COrderID {
//     entrust_no   : String,    // 订单委托号
//     order_id     : i32        // 自定义编号
// }
//}
