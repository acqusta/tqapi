extern crate libc;
use std::mem;

use std::fmt;
use std::ffi::CStr;
use std::ffi::CString;
use std::os::raw::c_char;
use super::tqapi_ffi::*;

#[derive(Debug)]
pub struct MarketQuote{
    pub code : String,
    pub date : i32,
    pub time : i32,
    pub recv_time : i64,
    pub trade_date: i32,
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
    pub date       : i32,
    pub time       : i32,
    pub trade_date : i32,
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
    pub trade_date : i32,
    pub time       : i32,
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


pub struct DataApi <'a> {
    cb     : Option<&'a DataApiCallback>,
    dapi   : *mut CDataApi,
}

impl <'a> Drop for DataApi<'a> {
    fn drop(&mut self) {
        unsafe {
            let old_cb = tqapi_dapi_set_callback(self.dapi, core::ptr::null_mut());
            if !old_cb.is_null() {
                Box::from_raw((*old_cb).obj);
                Box::from_raw(old_cb);
            }
            tqapi_free_data_api(self.dapi);
        }
    }
}

impl <'a> DataApi<'a> {
    pub fn new(addr: &str) -> DataApi {
        unsafe {
            let dapi = tqapi_create_data_api(addr.as_ptr() as *const c_char);

            DataApi{ cb : None, dapi : dapi}
        }
    }

    extern "C" fn on_quote(c_quote: *mut CMarketQuote, obj : *mut FFITraitObject) {

        assert!(!c_quote.is_null() && !obj.is_null());
        let mut cb: Box<DataApiCallback> = unsafe {
            mem::transmute( (*obj).copy())
        };
        let quote = unsafe { (*c_quote).to_rs()};
        cb.as_mut().on_quote(quote);
    }

    extern "C" fn on_bar(c_cycle : *mut c_char, c_bar: *mut CBar, obj : *mut FFITraitObject) {
        let mut cb: Box<DataApiCallback> = unsafe {
            mem::transmute( (*obj).copy())
        };

        let cycle = unsafe {CStr::from_ptr(c_cycle).to_str().unwrap()};
        let bar   = unsafe {(*c_bar).to_rs()};

        cb.as_mut().on_bar(cycle, bar);
    }

    pub fn set_callback(&mut self, cb : Option<&'a DataApiCallback>) {
        self.cb = cb;
        match self.cb {
            None =>
                unsafe {
                    tqapi_dapi_set_callback(self.dapi, core::ptr::null_mut());
                }
            Some(callback) =>
                unsafe {
                    println!("set_callback");
                    let trait_obj : FFITraitObject = mem::transmute(callback);

                    let raw_cb = Box::into_raw(
                        Box::new(CDataApiCallback {
                            obj      : Box::into_raw(Box::new(trait_obj)) as *mut FFITraitObject,
                            on_bar   : DataApi::on_bar,
                            on_quote : DataApi::on_quote,
                        }));

                    let old_cb = tqapi_dapi_set_callback(self.dapi, raw_cb);
                    if !old_cb.is_null() {
                        Box::from_raw( (*old_cb).obj);
                        Box::from_raw(old_cb);
                    }
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

    pub fn get_ticks(&mut self, code : &str, trade_date : i32) -> Result<Vec<MarketQuote>, String> {
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

    pub fn get_bars(&mut self, code : &str, cycle : &str, trade_date : i32, align : bool) -> Result<Vec<Bar>, String> {
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
