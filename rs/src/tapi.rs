extern crate libc;

use std::fmt;
use std::ffi::CString;
use std::os::raw::c_char;
use super::tqapi_ffi::*;


// TradeApi

pub struct AccountInfo {
    pub account_id   : String,    // 帐号编号
    pub broker       : String,    // 交易商名称，如招商证券
    pub account      : String,    // 交易帐号
    pub status       : String,    // 连接状态，取值 Disconnected, Connected, Connecting
    pub msg          : String,    // 状态信息，如登录失败原因
    pub account_type : String,    // 帐号类型，如 stock, ctp
}

impl fmt::Display for AccountInfo {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "AccountInfo({},{},{},{},{})", self.account_id, self.broker, self.account, self.status, self.msg)
    }
}

pub struct Balance {
    pub account_id     : String,
    pub fund_account   : String,
    pub init_balance   : f64,
    pub enable_balance : f64,
    pub margin         : f64,
    pub float_pnl      : f64,
    pub close_pnl      : f64,
}

pub enum OrderStatus {
    New,
    Accepted,
    Filled,
    Rejected,
    Cancelled,
    Other(String)
}

impl OrderStatus {
    pub fn from ( s : &str) -> OrderStatus {
        match s {
            "New"       => OrderStatus::New,
            "Accepted"  => OrderStatus::Accepted,
            "Filled"    => OrderStatus::Filled,
            "Rejected"  => OrderStatus::Rejected,
            "Cancelled" => OrderStatus::Cancelled,
            _           => OrderStatus::Other(String::from(s))
        }
    }

    pub fn to_str(&self) -> &str {
        match self {
            OrderStatus::New        => "New",
            OrderStatus::Accepted   => "Accepted",
            OrderStatus::Filled     => "Filled",
            OrderStatus::Rejected   => "Rejected",
            OrderStatus::Cancelled  => "Cancelled",
            OrderStatus::Other(s)   => s
        }
    }
}

pub enum EntrustAction {
    Buy,
    Short,
    Cover,
    Sell,
    CoverToday,
    CoverYesterday,
    SellToday,
    SellYesterday,
    Other(String)
}

impl EntrustAction {
    pub fn from( s : &str) -> EntrustAction {
        match s {
            "Buy"           => EntrustAction::Buy,
            "Short"         => EntrustAction::Short,
            "Cover"         => EntrustAction::Cover,
            "Sell"          => EntrustAction::Sell,
            "CoverToday"    => EntrustAction::CoverToday,
            "CoverYesterday"=> EntrustAction::CoverYesterday,
            "SellToday"     => EntrustAction::SellToday,
            "SellYesterday" => EntrustAction::SellYesterday,
            _               => EntrustAction::Other(String::from(s))
        }
    }

    pub fn to_str(&self) -> &str {
        match self {
            EntrustAction::Buy            => "Buy"           ,
            EntrustAction::Short          => "Short"         ,
            EntrustAction::Cover          => "Cover"         ,
            EntrustAction::Sell           => "Sell"          ,
            EntrustAction::CoverToday     => "CoverToday"    ,
            EntrustAction::CoverYesterday => "CoverYesterday",
            EntrustAction::SellToday      => "SellToday"     ,
            EntrustAction::SellYesterday  => "SellYesterday" ,
            EntrustAction::Other(s)       =>  s,
        }
    }
}

pub struct Order {
    pub account_id       : String,         // 帐号编号
    pub code             : String,         // 证券代码
    pub name             : String,         // 证券名称
    pub entrust_no       : String,         // 委托编号
    pub entrust_action   : EntrustAction,  // 委托动作
    pub entrust_price    : f64,            // 委托价格
    pub entrust_size     : i64,            // 委托数量，单位：股
    pub entrust_date     : i32,            // 委托日期
    pub entrust_time     : i32,            // 委托时间
    pub fill_price       : f64,            // 成交价格
    pub fill_size        : i64,            // 成交数量
    pub status           : OrderStatus,    // 订单状态：取值: OrderStatus
    pub status_msg       : String,         // 状态消息
    pub order_id         : i32             // 自定义订单编号
}

impl fmt::Display for Order {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Order({},{},{},{},{},{},{},{},{},{})", self.account_id, self.code,
            self.entrust_action.to_str(), self.entrust_price, self.entrust_size,
            self.entrust_date, self.entrust_time, self.entrust_no, self.status.to_str(), self.order_id)
    }
}

pub struct Trade {
    pub account_id       : String,     // 帐号编号
    pub code             : String,     // 证券代码
    pub name             : String,     // 证券名称
    pub entrust_no       : String,     // 委托编号
    pub entrust_action   : EntrustAction, // 委托动作
    pub fill_no          : String,        // 成交编号
    pub fill_size        : i64,           // 成交数量
    pub fill_price       : f64,           // 成交价格
    pub fill_date        : i32,           // 成交日期
    pub fill_time        : i32,           // 成交时间
    pub order_id         : i32,           // 自定义订单编号
}

impl fmt::Display for Trade {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Trade({},{},{},{},{},{},{},{})", self.account_id, self.code,
            self.entrust_action.to_str(), self.fill_price, self.fill_size, self.fill_time,
            self.fill_no, self.entrust_no)
    }
}

pub enum Side {
    Long,
    Short,
    Other(String)
}

impl Side {
    pub fn from( s : &str) -> Side {
        match s {
            "Long"   => Side::Long,
            "Short"  => Side::Short,
            _        => Side::Other(String::from(s))
        }
    }
}

pub struct Position {
    pub account_id    : String,   // 帐号编号
    pub code          : String,   // 证券代码
    pub name          : String,   // 证券名称
    pub current_size  : i64,      // 当前持仓
    pub enable_size   : i64,      // 可用（可交易）持仓
    pub init_size     : i64,      // 初始持仓
    pub today_size    : i64,      // 今日持仓
    pub frozen_size   : i64,      // 冻结持仓
    pub side          : Side,     // 持仓方向，股票的持仓方向为 Long, 期货分 Long, Short
    pub cost          : f64,      // 成本
    pub cost_price    : f64,      // 成本价格
    pub last_price    : f64,      // 最新价格
    pub float_pnl     : f64,      // 持仓盈亏
    pub close_pnl     : f64,      // 平仓盈亏
    pub margin        : f64,      // 保证金
    pub commission    : f64,      // 手续费
}

pub struct OrderID {
    pub entrust_no   : String,    // 订单委托号
    pub order_id     : i32        // 自定义编号
}

pub struct NewOrder<'a> {
    pub action     : EntrustAction,
    pub code       : &'a str,
    pub size       : i64,
    pub price      : f64,
    pub price_type : &'a str,
    pub order_id   : i32
}


pub trait TradeApiCallback {
    fn on_order          (&mut self, order   : Order);
    fn on_trade          (&mut self, trade   : Trade);
    fn on_account_status (&mut self, account : AccountInfo);
}

struct NullTradeApiCallback;

impl TradeApiCallback for NullTradeApiCallback {
    fn on_order          (&mut self, _order   : Order)       {}
    fn on_trade          (&mut self, _trade   : Trade)       {}
    fn on_account_status (&mut self, _account : AccountInfo) {}
}

struct TradeApiHook {
    cb : *mut TradeApiCallback,
}

pub struct TradeApi{
    null_cb  : *mut TradeApiCallback,
    hook     : *mut TradeApiHook,
    tapi     : *mut CTradeApi,
    is_owner : bool
}

impl Drop for TradeApi {
    fn drop(&mut self) {
        unsafe {
            let old_cb = tqapi_tapi_set_callback(self.tapi, core::ptr::null_mut());
            if !old_cb.is_null() {
                Box::from_raw(old_cb);
            }

            Box::from_raw(self.hook);
            Box::from_raw(self.null_cb);

            if self.is_owner {
                tqapi_free_trade_api(self.tapi);
            }
        }
    }
}

impl TradeApi {
    pub fn new(addr: &str) -> TradeApi {
        unsafe {
            let tapi = tqapi_create_trade_api(addr.as_ptr() as *const c_char);
            let null_cb = Box::into_raw(Box::new(NullTradeApiCallback{}));
            let hook = Box::into_raw(Box::new(TradeApiHook{ cb : null_cb}));

            TradeApi{ null_cb : null_cb, hook : hook, tapi : tapi, is_owner: true}
        }
    }

    pub fn from(tapi : *mut CTradeApi) -> TradeApi {
            let null_cb = Box::into_raw(Box::new(NullTradeApiCallback{}));
            let hook = Box::into_raw(Box::new(TradeApiHook{ cb : null_cb}));

            TradeApi{ null_cb : null_cb, hook : hook, tapi : tapi, is_owner: false}
    }

    pub fn query_accounts(&mut self) -> Result<Vec<AccountInfo>, String> {
        let mut result;
        unsafe {
            let r = tqapi_tapi_query_accounts(self.tapi);
            if !(*r).array.is_null() {
                let mut v = Vec::new();
                for i in 0..(*r).array_size {
                    v.push( (*(*r).array.offset(i as isize)).to_rs());
                }
                result = Ok(v);
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_query_accounts_result(self.tapi, r);
        }
        return result;
    }
    pub fn query_balance(&mut self, account_id : &str) -> Result<Balance, String> {
        let c_account = CString::new(account_id).unwrap();
        let mut result;
        unsafe {
            let r = tqapi_tapi_query_balance(self.tapi, c_account.as_ptr());
            if !(*r).balance.is_null() {
                result = Ok( (*(*r).balance).to_rs());
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_query_balance_result(self.tapi, r);
        }
        return result;
    }

    pub fn query_orders(&mut self, account_id : &str, codes : &str) -> Result<Vec<Order>, String> {
        let c_account = CString::new(account_id).unwrap();
        let c_codes   = CString::new(codes).unwrap();
        let mut result;
        unsafe {
            let r = tqapi_tapi_query_orders(self.tapi, c_account.as_ptr(), c_codes.as_ptr());
            if !(*r).array.is_null() {
                let mut v = Vec::new();
                for i in 0..(*r).array_size {
                    v.push( (*(*r).array.offset(i as isize)).to_rs());
                }
                result = Ok(v);
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_query_orders_result(self.tapi, r);
        }
        return result;
    }

    pub fn query_trades(&mut self, account_id : &str, codes : &str) -> Result<Vec<Trade>, String> {
        let c_account = CString::new(account_id).unwrap();
        let c_codes   = CString::new(codes).unwrap();
        let mut result;
        unsafe {
            let r = tqapi_tapi_query_trades(self.tapi, c_account.as_ptr(), c_codes.as_ptr());
            if !(*r).array.is_null() {
                let mut v = Vec::new();
                for i in 0..(*r).array_size {
                    v.push( (*(*r).array.offset(i as isize)).to_rs());
                }
                result = Ok(v);
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_query_trades_result(self.tapi, r);
        }
        return result;
    }

    pub fn query_positions(&mut self, account_id : &str, codes : &str) -> Result<Vec<Position>, String> {
        let c_account = CString::new(account_id).unwrap();
        let c_codes   = CString::new(codes).unwrap();
        let mut result;
        unsafe {
            let r = tqapi_tapi_query_positions(self.tapi, c_account.as_ptr(), c_codes.as_ptr());
            if !(*r).array.is_null() {
                let mut v = Vec::new();
                for i in 0..(*r).array_size {
                    v.push( (*(*r).array.offset(i as isize)).to_rs());
                }
                result = Ok(v);
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_query_positions_result(self.tapi, r);
        }
        return result;
    }

    pub fn place_order(&mut self, account_id : &str, order: &NewOrder) -> Result<OrderID, String> {
        let c_account = CString::new(account_id).unwrap();

        let c_order = CNewOrder {
                action     : CString::new(order.action.to_str()).unwrap().as_ptr(),
                code       : CString::new(order.code).unwrap().as_ptr(),
                size       : order.size,
                price      : order.price,
                price_type : CString::new(order.price_type).unwrap().as_ptr(),
                order_id   : order.order_id
            };

        let mut result;
        unsafe {
            let raw_c_order = Box::into_raw( Box::new(c_order));
            let r = tqapi_tapi_place_order(self.tapi, c_account.as_ptr(), raw_c_order);
            if !(*r).order_id.is_null() {
                result = Ok(OrderID{
                            entrust_no : c_str_to_string((*(*r).order_id).entrust_no),
                            order_id   : (*(*r).order_id).order_id});
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_place_order_result(self.tapi, r);
        }
        return result;
    }

    pub fn cancel_order(&mut self, account_id: &str, code : &str, oid: &OrderID) -> Result<bool, String> {
        let c_account = CString::new(account_id).unwrap();
        let c_code    = CString::new(code).unwrap();
        let c_entrust_no = CString::new(oid.entrust_no.as_str()).unwrap();

        let c_oid = COrderId {
            entrust_no : c_entrust_no.as_ptr(),
            order_id   : oid.order_id
            };

        let mut result;
        unsafe {
            let raw_c_oid = Box::into_raw( Box::new(c_oid));
            let r = tqapi_tapi_cancel_order(self.tapi, c_account.as_ptr(), c_code.as_ptr(), raw_c_oid);
            if (*r).success == 1 {
                result = Ok(true)
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_cancel_order_result(self.tapi, r);
        }
        return result;
    }

    pub fn query(&mut self, account_id : &str, command : &str, params: &str) -> Result<String, String> {
        let c_account = CString::new(account_id).unwrap();
        let c_command = CString::new(command).unwrap();
        let c_params  = CString::new(params).unwrap();

        let mut result;
        unsafe {
            let r = tqapi_tapi_query(self.tapi, c_account.as_ptr(), c_command.as_ptr(), c_params.as_ptr());
            if !(*r).text.is_null() {
                result = Ok(c_str_to_string((*r).text));
            } else {
                result = Err(c_str_to_string( (*r).msg) );
            }

            tqapi_tapi_free_query_result(self.tapi, r);
        }
        return result;
    }

    extern "C" fn on_order(obj : *mut libc::c_void, c_order: *mut COrder) {
        unsafe {
            assert!(!c_order.is_null() && !obj.is_null());
            let order = (*c_order).to_rs();
            let cb = (*(obj as *mut TradeApiHook)).cb as *mut TradeApiCallback;
            (*cb).on_order(order);
        }
    }

    extern "C" fn on_trade(obj : *mut libc::c_void, c_trade: *mut CTrade) {
        unsafe {
            assert!(!c_trade.is_null() && !obj.is_null());
            let trade = (*c_trade).to_rs();
            let cb = (*(obj as *mut TradeApiHook)).cb as *mut TradeApiCallback;
            (*cb).on_trade (trade);
        }
    }

    extern "C" fn on_account_status(obj : *mut libc::c_void, account: *mut CAccountInfo) {
        unsafe {
            assert!(!account.is_null() && !obj.is_null());

            let account = (*account).to_rs();
            let cb = (*(obj as *mut TradeApiHook)).cb as *mut TradeApiCallback;
            (*cb).on_account_status(account);
        }
    }

    pub fn set_callback(&mut self, cb : Option<Box<TradeApiCallback>>) {
        if !self.is_owner { return;}

        unsafe {
            let old_cb = tqapi_tapi_set_callback(self.tapi, core::ptr::null_mut());
            if !old_cb.is_null() {
                Box::from_raw((*old_cb).obj);
                Box::from_raw(old_cb);
            }
            if let Some(callback) = cb {
                (*self.hook).cb = Box::into_raw(callback);
                let raw_cb = Box::into_raw(
                    Box::new(CTradeApiCallback {
                        obj               : self.hook as *mut libc::c_void,
                        on_account_status : TradeApi::on_account_status,
                        on_order          : TradeApi::on_order,
                        on_trade          : TradeApi::on_trade,
                    }));

                let old_cb = tqapi_tapi_set_callback(self.tapi, raw_cb);
                if !old_cb.is_null() {
                    Box::from_raw( (*old_cb).obj);
                    Box::from_raw(old_cb);
                }
            }
        }
    }

}
