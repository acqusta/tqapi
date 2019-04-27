extern crate libc;


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

pub trait DataApiCallback {
    fn on_quote(&mut self, quote : MarketQuote);
    fn on_bar  (&mut self, cycle : String, bar : Bar);
}


pub struct DataApi {
    cb : Option<Box<DataApiCallback>>,
    dapi : *mut CDataApi,
}

impl Drop for DataApi {
    fn drop(&mut self) {
        unsafe {
            tqapi_free_data_api(self.dapi);
        }
    }
}
impl DataApi {
    pub fn new(addr: &str) -> DataApi {
        unsafe {
            let dapi = tqapi_create_data_api(addr.as_ptr() as *const c_char);
            DataApi{ cb : None, dapi : dapi }
        }
    }

    pub fn subscribe(&mut self, codes: & Vec<String>) -> Result<Vec<String>, String> {
        let c_codes = CString::new(codes.join(".")).unwrap();
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

    pub fn get_ticks(&mut self, code : &str, trade_date : i32) -> Result<Vec<MarketQuote>, String> {
        let c_code = CString::new(code).unwrap();
        let mut result : Result<Vec<MarketQuote>, String>;

        unsafe {
            let r = tqapi_dapi_get_ticks(self.dapi, c_code.as_ptr() as *const c_char, trade_date);
            if !(*r).ticks.is_null() {
                let mut quotes : Vec<MarketQuote> = Vec::new();
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

    pub fn get_bars(&mut self, code : &str, trade_date : i32) -> Result<Vec<Bar>, String> {
        let bars = Vec::<Bar>::new();
        return Ok(bars);
    }

    pub fn get_dailybars(&mut self, code : &str, trade_date : i32) -> Result<Vec<DailyBar>, String> {
        Err("Wrong".to_string())
    }

    pub fn get_quote(&mut self, code : &str, trade_date : i32) -> Result<Vec<MarketQuote>, String> {
        Err("Wrong".to_string())
    }

    pub fn set_callback<T: DataApiCallback + 'static >(&mut self, cb : T) {
        self.cb = Some(Box::new(cb));
    }
}
