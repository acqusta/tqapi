extern crate libc;

use std::ffi::CStr;
use std::os::raw::c_char;
use super::dapi::MarketQuote;
use super::dapi::Bar;
use super::dapi::DailyBar;
use super::tapi::{Balance, AccountInfo, Order, Trade, Position, EntrustAction, OrderStatus, Side};


pub fn c_str_to_string(s : *const c_char) -> String {
    unsafe {
        if !s.is_null() {
            CStr::from_ptr(s).to_string_lossy().into_owned()
        } else {
            String::from("")
        }
    }
}


#[repr(C, packed)]
pub struct CMarketQuote{
    code : *const c_char,
    date : u32,
    time : u32,
    recv_time : u64,
    trading_day: u32,
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
                date : self.date, time : self.time, recv_time : self.recv_time, trading_day: self.trading_day,
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

#[repr(C, packed)]
pub struct CBar {
    code  : *const c_char,
    date  : u32,
    time  : u32,
    trading_day: u32,
    open  : f64,
    high  : f64,
    low   : f64,
    close : f64,
    volume : i64,
    turnover : f64,
    oi       : i64
}

impl CBar {
    pub fn to_rs(&self) -> Bar{
        unsafe {
            Bar {
                code: CStr::from_ptr(self.code).to_string_lossy().into_owned(),
                date : self.date, time : self.time, trading_day: self.trading_day,
                open : self.open, high : self.high, low : self.low, close : self.close,
                volume : self.volume, turnover : self.turnover, oi : self.oi
            }
        }
    }
}

#[repr(C, packed)]
pub struct CDailyBar {
    code       : *const c_char,
    trading_day: u32,
    time       : u32,
    open       : f64,
    high       : f64,
    low        : f64,
    close      : f64,
    volume     : i64,
    turnover   : f64,
    oi         : i64,
    settle     : f64,
    pre_close  : f64,
    pre_settle : f64,
    af         : f64
}

impl CDailyBar {
    pub fn to_rs(&self) -> DailyBar{
        unsafe {
            DailyBar {
                code: CStr::from_ptr(self.code).to_string_lossy().into_owned(),
                trading_day: self.trading_day, time : self.time,
                open : self.open, high : self.high, low : self.low, close : self.close,
                volume : self.volume, turnover : self.turnover, oi : self.oi,
                settle : self.settle, pre_close : self.pre_close, pre_settle : self.pre_settle,
                af : self.af
            }
        }
    }
}

#[repr(C, packed)]
pub struct GetTicksResult {
    _data            : *mut libc::c_void,
    pub ticks        : *mut CMarketQuote,
    pub ticks_length : i32,
    pub element_size : i32,
    pub msg          : *const c_char,
}

#[repr(C, packed)]
pub struct GetBarsResult {
    _data            : *mut libc::c_void,
    pub ticks        : *mut CBar,
    pub ticks_length : i32,
    pub element_size : i32,
    pub msg          : *const c_char,
}

#[repr(C, packed)]
pub struct GetDailyBarResult {
    _data            : *mut libc::c_void,
    pub ticks        : *mut CDailyBar,
    pub ticks_length : i32,
    pub element_size : i32,
    pub msg          : *const c_char,
}

#[repr(C, packed)]
pub struct GetQuoteResult {
    _data            : *mut libc::c_void,
    pub quote        : *mut CMarketQuote,
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

#[repr(C, packed)]
pub struct CDataApiCallback {
    pub obj        : *mut libc::c_void,
    pub on_quote   : extern "C" fn(obj : *mut libc::c_void, quote : *mut CMarketQuote ),
    pub on_bar     : extern "C" fn(obj : *mut libc::c_void, cycle : *mut c_char, bar : *mut CBar )
}

pub enum CDataApi {  }

#[link(name = "tqapi")]//, kind = "static")]
extern "C" {
    pub fn tqapi_create_data_api(addr : *const c_char) -> *mut CDataApi;
    pub fn tqapi_free_data_api  (dapi : *mut CDataApi);

    pub fn tqapi_dapi_get_ticks                (dapi : *mut CDataApi, code : *const c_char, trading_day : u32, number: i32) -> *mut GetTicksResult;
    pub fn tqapi_dapi_free_get_ticks_result    (dapi : *mut CDataApi, result : *mut GetTicksResult);
    pub fn tqapi_dapi_get_bars                 (dapi : *mut CDataApi, code : *const c_char, cycle : *const c_char, trading_day : u32, align : i32, number: i32) -> *mut GetBarsResult;
    pub fn tqapi_dapi_free_get_bars_result     (dapi : *mut CDataApi, result : *mut GetBarsResult);

    pub fn tqapi_dapi_get_dailybars            (dapi : *mut CDataApi, code : *const c_char, price_type : *const c_char, align : i32, number: i32) -> *mut GetDailyBarResult;
    pub fn tqapi_dapi_free_get_dailybars_result(dapi : *mut CDataApi, result : *mut GetDailyBarResult);

    pub fn tqapi_dapi_get_quote                (dapi : *mut CDataApi, code : *const c_char) -> *mut GetQuoteResult;
    pub fn tqapi_dapi_free_get_quote_result    (dapi : *mut CDataApi, result : *mut GetQuoteResult);

    pub fn tqapi_dapi_subscribe                (dapi: *mut CDataApi, codes : *const c_char) -> *mut SubscribeResult;
    pub fn tqapi_dapi_free_subscribe_result    (dapi: *mut CDataApi, result : *mut SubscribeResult);

    pub fn tqapi_dapi_unsubscribe              (dapi: *mut CDataApi, codes : *const c_char) -> *mut UnSubscribeResult;
    pub fn tqapi_dapi_free_unsubscribe_result  (dapi: *mut CDataApi, result : *mut UnSubscribeResult);

    pub fn tqapi_dapi_set_callback             (dapi: *mut CDataApi, callback : *mut CDataApiCallback) -> * mut CDataApiCallback;
}

// TradeApi

#[repr(C, packed)]
pub struct CAccountInfo {
   pub account_id   : *const c_char,    // 帐号编号
   pub broker       : *const c_char,    // 交易商名称，如招商证券
   pub account      : *const c_char,    // 交易帐号
   pub status       : *const c_char,    // 连接状态，取值 Disconnected, Connected, Connecting
   pub msg          : *const c_char,    // 状态信息，如登录失败原因
   pub account_type : *const c_char,    // 帐号类型，如 stock, ctp
}

impl CAccountInfo {
    pub fn to_rs(&self) -> AccountInfo {
        AccountInfo{
            account_id   : c_str_to_string(self.account_id),
            broker       : c_str_to_string(self.broker),
            account      : c_str_to_string(self.account),
            status       : c_str_to_string(self.status),
            msg          : c_str_to_string(self.msg),
            account_type : c_str_to_string(self.account_type)
        }
    }
}

#[repr(C, packed)]
pub struct CBalance {
   pub account_id     : *const c_char,
   pub fund_account   : *const c_char,
   pub init_balance   : f64,
   pub enable_balance : f64,
   pub margin         : f64,
   pub float_pnl      : f64,
   pub close_pnl      : f64,
}

impl CBalance {
    pub fn to_rs(&self) -> Balance {
        Balance{
            account_id     : c_str_to_string(self.account_id),
            fund_account   : c_str_to_string(self.fund_account),
            init_balance   : self.init_balance,
            enable_balance : self.enable_balance,
            margin         : self.margin,
            float_pnl      : self.float_pnl,
            close_pnl      : self.close_pnl,
        }
    }
}

#[repr(C, packed)]
pub struct COrder {
    pub account_id       : *const c_char,         // 帐号编号
    pub code             : *const c_char,         // 证券代码
    pub name             : *const c_char,         // 证券名称
    pub entrust_no       : *const c_char,         // 委托编号
    pub entrust_action   : *const c_char,         // 委托动作
    pub entrust_price    : f64,                   // 委托价格
    pub entrust_size     : i64,                   // 委托数量，单位：股
    pub entrust_date     : u32,                   // 委托日期
    pub entrust_time     : u32,                   // 委托时间
    pub fill_price       : f64,                   // 成交价格
    pub fill_size        : i64,                   // 成交数量
    pub status           : *const c_char,         // 订单状态：取值: OrderStatus
    pub status_msg       : *const c_char,                // 状态消息
    pub order_id         : i32                    // 自定义订单编号
}

impl COrder {
    pub fn to_rs(&self) -> Order {
        unsafe {
            Order {
                account_id       : c_str_to_string( self.account_id )      ,
                code             : c_str_to_string( self.code       )      ,
                name             : c_str_to_string( self.name       )      ,
                entrust_no       : c_str_to_string( self.entrust_no )      ,
                entrust_action   : EntrustAction::from(CStr::from_ptr(self.entrust_action).to_str().unwrap()),
                entrust_price    : self.entrust_price    ,
                entrust_size     : self.entrust_size     ,
                entrust_date     : self.entrust_date     ,
                entrust_time     : self.entrust_time     ,
                fill_price       : self.fill_price       ,
                fill_size        : self.fill_size        ,
                status           : OrderStatus::from(CStr::from_ptr(self.status).to_str().unwrap()),
                status_msg       : c_str_to_string( self.status_msg )      ,
                order_id         : self.order_id         ,
            }
        }
    }
}
#[repr(C, packed)]
pub struct CTrade {
    pub account_id       : *const c_char,     // 帐号编号
    pub code             : *const c_char,     // 证券代码
    pub name             : *const c_char,     // 证券名称
    pub entrust_no       : *const c_char,     // 委托编号
    pub entrust_action   : *const c_char, // 委托动作
    pub fill_no          : *const c_char,        // 成交编号
    pub fill_size        : i64,           // 成交数量
    pub fill_price       : f64,           // 成交价格
    pub fill_date        : u32,           // 成交日期
    pub fill_time        : u32,           // 成交时间
    pub order_id         : i32,           // 自定义订单编号
}

impl CTrade {
    pub fn to_rs(&self) -> Trade {
        unsafe {
            Trade {
                account_id       : c_str_to_string(self.account_id)         ,
                code             : c_str_to_string(self.code      )         ,
                name             : c_str_to_string(self.name      )         ,
                entrust_no       : c_str_to_string(self.entrust_no)         ,
                entrust_action   : EntrustAction::from(CStr::from_ptr(self.entrust_action).to_str().unwrap()) ,
                fill_no          : c_str_to_string(self.fill_no)            ,
                fill_size        : self.fill_size        ,
                fill_price       : self.fill_price       ,
                fill_date        : self.fill_date        ,
                fill_time        : self.fill_time        ,
                order_id         : self.order_id         ,
            }
        }
    }
}
#[repr(C, packed)]
pub struct CPosition {
    pub account_id    : *const c_char,   // 帐号编号
    pub code          : *const c_char,   // 证券代码
    pub name          : *const c_char,   // 证券名称
    pub current_size  : i64,      // 当前持仓
    pub enable_size   : i64,      // 可用（可交易）持仓
    pub init_size     : i64,      // 初始持仓
    pub today_size    : i64,      // 今日持仓
    pub frozen_size   : i64,      // 冻结持仓
    pub side          : *const c_char,     // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
    pub cost          : f64,      // 成本
    pub cost_price    : f64,      // 成本价格
    pub last_price    : f64,      // 最新价格
    pub float_pnl     : f64,      // 持仓盈亏
    pub close_pnl     : f64,      // 平仓盈亏
    pub margin        : f64,      // 保证金
    pub commission    : f64,      // 手续费
}

impl CPosition {
    pub fn to_rs(&self) -> Position {
        unsafe {
            Position {
                account_id    : c_str_to_string(self.account_id)    ,
                code          : c_str_to_string(self.code      )    ,
                name          : c_str_to_string(self.name      )    ,
                current_size  : self.current_size  ,
                enable_size   : self.enable_size   ,
                init_size     : self.init_size     ,
                today_size    : self.today_size    ,
                frozen_size   : self.frozen_size   ,
                side          : Side::from(CStr::from_ptr(self.side).to_str().unwrap()),
                cost          : self.cost          ,
                cost_price    : self.cost_price    ,
                last_price    : self.last_price    ,
                float_pnl     : self.float_pnl     ,
                close_pnl     : self.close_pnl     ,
                margin        : self.margin        ,
                commission    : self.commission    ,
            }
        }
    }
}

#[repr(C, packed)]
pub struct COrderId {
    pub entrust_no   : *const c_char,    // 订单委托号
    pub order_id     : i32               // 自定义编号
}

#[repr(C, packed)]
pub struct CTradeApiCallback {
    pub obj                : *mut libc::c_void,
    pub on_trade           : extern "C" fn(obj : *mut libc::c_void, trade : *mut CTrade       ),
    pub on_order           : extern "C" fn(obj : *mut libc::c_void, order : *mut COrder       ),
    pub on_account_status  : extern "C" fn(obj : *mut libc::c_void, status: *mut CAccountInfo ),
}

#[repr(C, packed)]
pub struct CNewOrder {
    pub action     : *const c_char,
    pub code       : *const c_char,
    pub size       : i64,
    pub price      : f64,
    pub price_type : *const c_char,
    pub order_id   : i32
}

#[repr(C, packed)]
pub struct CPlaceOrderResult {
    pub _data      : *mut libc::c_void,
    pub order_id : *mut COrderId,
    pub msg      : *const c_char,
}

#[repr(C, packed)]
pub struct CQueryPositionsResult {
    pub _data      : *mut libc::c_void,
    pub array      : *mut CPosition,
    pub array_size : i32,
    pub element_size : i32,
    pub msg        : *const c_char,
}

#[repr(C, packed)]
pub struct CQueryTradesResult {
    pub _data      : *mut libc::c_void,
    pub array      : *mut CTrade,
    pub array_size : i32,
    pub element_size : i32,
    pub msg        : *const c_char,
}

#[repr(C, packed)]
pub struct CQueryOrdersResult {
    pub _data      : *mut libc::c_void,
    pub array      : *mut COrder,
    pub array_size : i32,
    pub element_size : i32,
    pub msg        : *const c_char,
}

#[repr(C, packed)]
pub struct CQueryBalanceResult {
    pub _data      : *mut libc::c_void,
    pub balance    : *mut CBalance,
    pub msg        : *const c_char,
}

#[repr(C, packed)]
pub struct CQueryAccountsResult {
    pub _data      : *mut libc::c_void,
    pub array      : *mut CAccountInfo,
    pub array_size : i32,
    pub element_size : i32,
    pub msg        : *const c_char,
}

#[repr(C, packed)]
pub struct CCancelOrderResult {
    pub _data      : *mut libc::c_void,
    pub success   : i32, // bool
    pub msg       : *const c_char,
}

#[repr(C, packed)]
pub struct CQueryResult {
    pub _data      : *mut libc::c_void,
    pub text      : *const c_char,
    pub msg       : *const c_char,
}

pub enum CTradeApi {  }



//#[link(name = "tqapi")]//, kind = "static")]
extern "C" {
    pub fn tqapi_create_trade_api(addr : *const c_char) -> *mut CTradeApi;
    pub fn tqapi_free_trade_api  (dapi : *mut CTradeApi);

    pub fn tqapi_tapi_place_order                  (tapi : *mut CTradeApi, account_id : *const c_char, order : *mut CNewOrder) -> *mut CPlaceOrderResult;
    pub fn tqapi_tapi_cancel_order                 (tapi : *mut CTradeApi, account_id : *const c_char, code : *const c_char, oid : *mut COrderId) -> *mut CCancelOrderResult;
    pub fn tqapi_tapi_query_balance                (tapi : *mut CTradeApi, account_id : *const c_char) ->*mut CQueryBalanceResult;
    pub fn tqapi_tapi_query_positions              (tapi : *mut CTradeApi, account_id : *const c_char, codes: *const c_char) -> *mut CQueryPositionsResult;
    pub fn tqapi_tapi_query_orders                 (tapi : *mut CTradeApi, account_id : *const c_char, codes: *const c_char) -> *mut CQueryOrdersResult;
    pub fn tqapi_tapi_query_trades                 (tapi : *mut CTradeApi, account_id : *const c_char, codes: *const c_char) -> *mut CQueryTradesResult;
    pub fn tqapi_tapi_query                        (tapi : *mut CTradeApi, account_id : *const c_char, command: *const c_char, params: *const c_char) -> *mut CQueryResult;
    pub fn tqapi_tapi_query_accounts               (tapi : *mut CTradeApi) -> *mut CQueryAccountsResult;
    pub fn tqapi_tapi_free_place_order_result      (tapi : *mut CTradeApi, result : *mut CPlaceOrderResult);
    pub fn tqapi_tapi_free_cancel_order_result     (tapi : *mut CTradeApi, result : *mut CCancelOrderResult);
    pub fn tqapi_tapi_free_query_accounts_result   (tapi : *mut CTradeApi, result : *mut CQueryAccountsResult);
    pub fn tqapi_tapi_free_query_balance_result    (tapi : *mut CTradeApi, result : *mut CQueryBalanceResult);
    pub fn tqapi_tapi_free_query_positions_result  (tapi : *mut CTradeApi, result : *mut CQueryPositionsResult);
    pub fn tqapi_tapi_free_query_orders_result     (tapi : *mut CTradeApi, result : *mut CQueryOrdersResult);
    pub fn tqapi_tapi_free_query_trades_result     (tapi : *mut CTradeApi, result : *mut CQueryTradesResult);
    pub fn tqapi_tapi_free_query_result            (tapi : *mut CTradeApi, result : *mut CQueryResult);
    pub fn tqapi_tapi_set_callback                 (tapi : *mut CTradeApi, callback : *mut CTradeApiCallback) -> * mut CTradeApiCallback;
}


#[repr(C, packed)]
pub struct CDateTime {
    pub date : u32,
    pub time : u32
}

#[repr(C, packed)]
pub struct CStralet {
    pub obj               : *mut libc::c_void,
    pub on_init           : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext),
    pub on_fini           : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext),
    pub on_quote          : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext, quote : *mut CMarketQuote),
    pub on_bar            : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext, cycle : *const c_char, bar : *mut CBar),
    pub on_order          : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext, order : *mut COrder),
    pub on_trade          : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext, trade : *mut CTrade),
    pub on_timer          : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext, id    : i64,  data : usize),
    pub on_event          : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext, name  : *const c_char, data : usize),
    pub on_account_status : extern "C" fn(obj : *mut libc::c_void, ctx: *mut CStraletContext, account : *mut CAccountInfo),
}

#[repr(C, packed)]
pub struct CStraletFactory {
    pub obj     : *mut libc::c_char,
    pub create  : extern "C" fn (obj : *mut libc::c_void) -> *mut CStralet,
    pub destroy : extern "C" fn (obj : *mut libc::c_void, stralet : *mut CStralet),// -> libc::c_void
}

pub enum CStraletContext {}
extern "C" {
    pub fn tqapi_sc_trading_day     (ctx : * mut CStraletContext ) -> i32;
    pub fn tqapi_sc_cur_time        (ctx : * mut CStraletContext ) -> CDateTime;
    pub fn tqapi_sc_post_event      (ctx : * mut CStraletContext, evt: *const c_char, data : usize);
    pub fn tqapi_sc_set_timer       (ctx : * mut CStraletContext, id : i64, delay : i64, data: usize);
    pub fn tqapi_sc_kill_timer      (ctx : * mut CStraletContext, id : i64);
    pub fn tqapi_sc_data_api        (ctx : * mut CStraletContext  ) -> *mut CDataApi;
    pub fn tqapi_sc_trade_api       (ctx : * mut CStraletContext  ) -> *mut CTradeApi;
    pub fn tqapi_sc_log             (ctx : * mut CStraletContext, severity : i32, txt : *const c_char);
    pub fn tqapi_sc_get_properties  (ctx : * mut CStraletContext ) -> * const c_char;
    pub fn tqapi_sc_get_mode        (ctx : * mut CStraletContext ) -> * const c_char;
    pub fn tqapi_bt_run             (cfg : * const c_char, factory : *mut CStraletFactory);
    pub fn tqapi_rt_run             (cfg : * const c_char, factory : *mut CStraletFactory);
}
