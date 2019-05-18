extern crate libc;
//use std::mem;

use std::fmt;
use std::ptr;
use std::ffi::CStr;
use std::ffi::CString;
use std::os::raw::c_char;
use super::tqapi_ffi::*;

#[derive(Debug,Clone)]
pub struct MarketQuote{
    pub code : String,
    pub date : u32,
    pub time : u32,
    pub recv_time : u64,
    pub trade_date: u32,
    pub open  : f64,
    pub high  : f64,
    pub low   : f64,
    pub close : f64,
    pub last  : f64,
    pub high_limit : f64,
    pub low_limit  : f64,
    pub pre_close  : f64,
    pub volume     : i64,
    pub turnover   : f64,
    pub ask1 : f64,
    pub ask2 : f64,
    pub ask3 : f64,
    pub ask4 : f64,
    pub ask5 : f64,
    pub bid1 : f64,
    pub bid2 : f64,
    pub bid3 : f64,
    pub bid4 : f64,
    pub bid5 : f64,
    pub ask_vol1 : i64,
    pub ask_vol2 : i64,
    pub ask_vol3 : i64,
    pub ask_vol4 : i64,
    pub ask_vol5 : i64,
    pub bid_vol1 : i64,
    pub bid_vol2 : i64,
    pub bid_vol3 : i64,
    pub bid_vol4 : i64,
    pub bid_vol5 : i64,
    pub settle   : f64,
    pub pre_settle : f64,
    pub oi         : i64,
    pub pre_oi     : i64
}

impl fmt::Display for MarketQuote {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "MarketQuote({},{},{},{})", self.time,self.code,self.last, self.volume)
    }
}

pub struct Bar {
    pub code       : String,
    pub date       : u32,
    pub time       : u32,
    pub trade_date : u32,
    pub open       : f64,
    pub high       : f64,
    pub low        : f64,
    pub close      : f64,
    pub volume     : i64,
    pub turnover   : f64,
    pub oi         : i64
}

impl Bar {
    pub fn new() -> Bar{
        Bar { code : String::from(""), date : 0, time : 0, trade_date : 0,
            open : 0.0, high : 0.0, low : 0.0, close : 0.0,
            volume : 0, turnover : 0.0, oi : 0}
    }
}

impl fmt::Display for Bar {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Bar({},{},{},{},{},{})", self.time,self.code,self.open,self.high,self.low,self.close)
    }
}

pub struct DailyBar {
    pub code       : String,
    pub trade_date : u32,
    pub time       : u32,
    pub open       : f64,
    pub high       : f64,
    pub low        : f64,
    pub close      : f64,
    pub volume     : i64,
    pub turnover   : f64,
    pub oi         : i64,
    pub settle     : f64,
    pub pre_close  : f64,
    pub pre_settle : f64,
    pub af         : f64
}

pub trait DataApiCallback {
    fn on_quote(&mut self, quote : MarketQuote);
    fn on_bar  (&mut self, cycle : &str, bar : Bar);
}

struct DefaultDataApiCallback {}

impl DataApiCallback for DefaultDataApiCallback {
    fn on_quote(&mut self, _quote : MarketQuote)
    {}
    fn on_bar  (&mut self, _cycle : &str, _bar : Bar)
    {}
}

struct DataApiHook{
    pub cb  : *mut DataApiCallback,
}

pub struct DataApi {
    //cb       : *mut DataApiCallback,
    dapi_cb_null  : *mut DataApiCallback,
    dapi     : *mut CDataApi,
    hook     : *mut DataApiHook,
    is_owner : bool
}

impl Drop for DataApi {
    fn drop(&mut self) {
        unsafe {
            if (*self.hook).cb != self.dapi_cb_null {
                tqapi_dapi_set_callback(self.dapi, ptr::null_mut());
                Box::from_raw( (*self.hook).cb);
                (*self.hook).cb = self.dapi_cb_null;
            }
            Box::from_raw(self.hook);
            Box::from_raw(self.dapi_cb_null);

            if self.is_owner {
                tqapi_free_data_api(self.dapi);
            }
        }
    }
}

impl DataApi {
    pub fn new(addr: &str) -> DataApi {
        unsafe {
            let dapi = tqapi_create_data_api(addr.as_ptr() as *const c_char);
            let null_cb = Box::into_raw(Box::new(DefaultDataApiCallback{}));
            let hook = Box::into_raw(Box::new(DataApiHook{ cb : null_cb}));

            DataApi{ dapi_cb_null : null_cb, hook: hook, dapi : dapi, is_owner : true}
        }
    }

    pub fn from(dapi : *mut CDataApi) -> DataApi {
            let null_cb = Box::into_raw(Box::new(DefaultDataApiCallback{}));
            let hook = Box::into_raw(Box::new(DataApiHook{ cb : null_cb}));

            DataApi{ dapi_cb_null : null_cb, hook: hook, dapi : dapi, is_owner : false}
    }

    extern "C" fn on_quote(obj : *mut libc::c_void, c_quote: *mut CMarketQuote) {

        unsafe {
            assert!(!c_quote.is_null() && !obj.is_null());
            let quote = (*c_quote).to_rs();
            let cb = (*(obj as *mut DataApiHook)).cb as *mut DataApiCallback;
            (*cb).on_quote(quote);
        }
    }

    extern "C" fn on_bar(obj: *mut libc::c_void, c_cycle : *mut c_char, c_bar: *mut CBar) {

        unsafe {
            let cycle = CStr::from_ptr(c_cycle).to_str().unwrap();
            let bar   = (*c_bar).to_rs();

            let cb = (*(obj as *mut DataApiHook)).cb;
            (*cb).on_bar(cycle, bar);
        }
    }

    pub fn set_callback(&mut self, cb : Option<Box<DataApiCallback>>) {
        if !self.is_owner { return; }

        unsafe {
            if (*self.hook).cb != self.dapi_cb_null {
                let old_cb = tqapi_dapi_set_callback(self.dapi, core::ptr::null_mut());
                Box::from_raw(old_cb as *mut CDataApiCallback);
                Box::from_raw( (*self.hook).cb);
                (*self.hook).cb = self.dapi_cb_null;
            }

            if let Some(callback) = cb {
                (*self.hook).cb = Box::into_raw(callback);
                let raw_cb = Box::into_raw(
                    Box::new(CDataApiCallback {
                        obj      : self.hook as *mut libc::c_void,
                        on_bar   : DataApi::on_bar,
                        on_quote : DataApi::on_quote,
                    }));

                tqapi_dapi_set_callback(self.dapi, raw_cb);
            }
        }
    }

    pub fn subscribe(&mut self, codes: & str) -> Result<Vec<String>, String> {
        let c_codes = CString::new(codes).unwrap();
        let mut result : Result<Vec<String>, String>;
        unsafe {
            let r = tqapi_dapi_subscribe(self.dapi, c_codes.as_ptr() as *const c_char);
            if !(*r).codes.is_null() {

                let mut v : Vec<String> = Vec::new();
                for code in CStr::from_ptr( (*r).codes).to_string_lossy().split(",") {
                    v.push(String::from(code));
                }

                result = Ok(v);
            } else {
                result = Err(CStr::from_ptr( (*r).msg).to_string_lossy().to_string());
            }

            tqapi_dapi_free_subscribe_result(self.dapi, r);
        }
        return result;
    }

    pub fn unsubscribe(&mut self, codes: & str) -> Result<Vec<String>, String> {
        let c_codes = CString::new(codes).unwrap();
        let mut result : Result<Vec<String>, String>;
        unsafe {
            let r = tqapi_dapi_unsubscribe(self.dapi, c_codes.as_ptr() as *const c_char);
            if !(*r).codes.is_null() {

                let mut v : Vec<String> = Vec::new();
                for code in CStr::from_ptr( (*r).codes).to_string_lossy().split(",") {
                    v.push(String::from(code));
                }

                result = Ok(v);
            } else {
                result = Err(CStr::from_ptr( (*r).msg).to_string_lossy().to_string());
            }

            tqapi_dapi_free_unsubscribe_result(self.dapi, r);
        }
        return result;
    }

    pub fn get_ticks(&mut self, code : &str, trade_date : u32) -> Result<Vec<MarketQuote>, String> {
        let c_code = CString::new(code).unwrap();
        let mut result : Result<Vec<MarketQuote>, String>;

        unsafe {
            let r = tqapi_dapi_get_ticks(self.dapi, c_code.as_ptr() as *const c_char, trade_date);
            if !(*r).ticks.is_null() {
                let mut quotes : Vec<MarketQuote> = Vec::new();
                quotes.reserve_exact( (*r).ticks_length as usize);
                for i in 0..((*r).ticks_length) {
                    let q : *mut CMarketQuote= (*r).ticks.offset(i as isize);
                    quotes.push( (*q).to_rs())
                }
                result = Ok(quotes)
            } else {
                result = Err(CStr::from_ptr( (*r).msg).to_string_lossy().to_string());
            }

            tqapi_dapi_free_get_ticks_result(self.dapi, r);
        }
        return result;
    }

    pub fn get_bars(&mut self, code : &str, cycle : &str, trade_date : u32, align : bool) -> Result<Vec<Bar>, String> {
        let c_code = CString::new(code).unwrap();
        let c_cycle = CString::new(cycle).unwrap();

        let mut result : Result<Vec<Bar>, String>;
        unsafe {
            let r = tqapi_dapi_get_bars(self.dapi,
                                        c_code.as_ptr() as *const c_char,
                                        c_cycle.as_ptr() as *const c_char,
                                        trade_date,
                                        if align { 1 } else { 0 } );
            if !(*r).ticks.is_null() {
                let mut bars : Vec<Bar> = Vec::new();
                bars.reserve_exact( (*r).ticks_length as usize);
                for i in 0..((*r).ticks_length) {
                    let q : *mut CBar = (*r).ticks.offset(i as isize);
                    bars.push( (*q).to_rs())
                }
                result = Ok(bars)
            } else {
                result = Err(CStr::from_ptr( (*r).msg).to_string_lossy().to_string());
            }

            tqapi_dapi_free_get_bars_result(self.dapi, r);
        }
        return result;
    }

    pub fn get_dailybars(&mut self, code : &str, price_type : &str, align : bool) -> Result<Vec<DailyBar>, String> {
        let c_code       = CString::new(code).unwrap();
        let c_price_type = CString::new(price_type).unwrap();

        let mut result : Result<Vec<DailyBar>, String>;

        unsafe {
            let r = tqapi_dapi_get_dailybars(self.dapi,
                                             c_code.as_ptr() as *const c_char,
                                             c_price_type.as_ptr() as *const c_char,
                                             if align { 1} else {0});
            if !(*r).ticks.is_null() {
                let mut bars : Vec<DailyBar> = Vec::new();
                bars.reserve_exact( (*r).ticks_length as usize);
                for i in 0..((*r).ticks_length) {
                    let q : *mut CDailyBar = (*r).ticks.offset(i as isize);
                    bars.push( (*q).to_rs())
                }
                result = Ok(bars)
            } else {
                result = Err(CStr::from_ptr( (*r).msg).to_string_lossy().to_string());
            }

            tqapi_dapi_free_get_dailybars_result(self.dapi, r);
        }
        return result;
    }

    pub fn get_quote(&mut self, code : &str) -> Result<MarketQuote, String> {
        let c_code = CString::new(code).unwrap();
        let mut result : Result<MarketQuote, String>;

        unsafe {
            let r = tqapi_dapi_get_quote(self.dapi, c_code.as_ptr() as *const c_char);
            if !(*r).quote.is_null() {
                result = Ok( (*(*r).quote).to_rs());
            } else {
                result = Err(CStr::from_ptr( (*r).msg).to_string_lossy().to_string());
            }

            tqapi_dapi_free_get_quote_result(self.dapi, r);
        }
        return result;
    }

}
