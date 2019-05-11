extern crate libc;
extern crate serde_derive;
extern crate serde_json;

use std::ptr;
use std::ffi::CString;
use std::os::raw::c_char;
use super::tqapi_ffi::*;

use super::dapi::*;
use super::tapi::*;

pub enum LogSeverity {
    INFO,
    WARNING,
    ERROR,
    FATAL
}

pub struct FinDateTime {
    pub date : i32,
    pub time : i32
}

pub enum RunMode {
    BACKTEST,
    REALTIME
}

pub trait StraletContext{
    fn get_trade_date (&self) -> i32 ;
    fn get_cur_time   (&self) -> FinDateTime;
    fn post_event     (&self, evt: &str, data: usize );
    fn set_timer      (&self, id: i64, delay : i64, data: usize);
    fn kill_timer     (&self, id : i64);
    fn get_data_api   (&self) -> & mut DataApi;
    fn get_trade_api  (&self) -> & mut TradeApi;
    fn log            (&self, severity: LogSeverity, txt: &str);
    fn log_info       (&self, txt: &str);
    fn log_error      (&self, txt: &str);
    fn log_fatal      (&self, txt: &str);
    fn get_properties (&self ) -> String;
    fn get_mode       (&self ) -> RunMode;
    fn stop           (&self);
}

#[allow(unused_variables)]
pub trait Stralet {
    fn on_init           (&mut self, ctx: & mut StraletContext)                             { }
    fn on_fini           (&mut self, ctx: & mut StraletContext)                             { }
    fn on_quote          (&mut self, ctx: & mut StraletContext, quote : MarketQuote)        { }
    fn on_bar            (&mut self, ctx: & mut StraletContext, cycle : &str, bar : Bar)    { }
    fn on_order          (&mut self, ctx: & mut StraletContext, order : Order)              { }
    fn on_trade          (&mut self, ctx: & mut StraletContext, trade : Trade)              { }
    fn on_timer          (&mut self, ctx: & mut StraletContext, id    : i64,  data : usize) { }
    fn on_event          (&mut self, ctx: & mut StraletContext, name  : &str, data : usize) { }
    fn on_account_status (&mut self, ctx: & mut StraletContext, account : AccountInfo)      { }
}


pub struct StraletContextImpl {
    ctx  : * mut CStraletContext,
    dapi : * mut DataApi,
    tapi : * mut TradeApi,
}

impl Drop for StraletContextImpl {
    fn drop(&mut self) {
        unsafe {
            Box::from_raw(self.dapi);
            Box::from_raw(self.tapi);
        }
    }
}

impl StraletContextImpl {
    fn new(ctx : *mut CStraletContext) -> StraletContextImpl {
        unsafe {
            StraletContextImpl {
                ctx : ctx,
                dapi : Box::into_raw(Box::new(DataApi::from( tqapi_sc_data_api(ctx)))),
                tapi : Box::into_raw(Box::new(TradeApi::from( tqapi_sc_trade_api(ctx)))),
            }
        }
    }
}
impl StraletContext for StraletContextImpl {

    fn get_trade_date(&self) -> i32 {
        unsafe {
            return tqapi_sc_trading_day(self.ctx);
        }
    }

    fn get_cur_time(&self) -> FinDateTime {
        unsafe {
            let t = tqapi_sc_cur_time(self.ctx);
            FinDateTime{ date : t.date, time : t.time }
        }
    }

    fn post_event(&self, evt: &str, data: usize ) {
        unsafe {
            tqapi_sc_post_event(self.ctx, CString::new(evt).unwrap().as_ptr(), data);
        }
    }

    fn set_timer (&self, id: i64, delay : i64, data: usize) {
        unsafe {
            tqapi_sc_set_timer(self.ctx, id, delay, data);
        }
    }

    fn kill_timer (&self, id : i64) {
        unsafe {
            tqapi_sc_kill_timer(self.ctx, id);
        }
    }

    fn get_data_api(&self) -> &mut DataApi {
        unsafe {
            &mut (*self.dapi)
        }
    }

    fn get_trade_api(&self) -> &mut TradeApi {
        unsafe {
            &mut (*self.tapi)
        }
    }

    fn log(&self, severity: LogSeverity, txt: &str) {
        unsafe {
            let i = match severity {
                LogSeverity::INFO    => 0,
                LogSeverity::WARNING => 1,
                LogSeverity::ERROR   => 2,
                LogSeverity::FATAL   => 3
            };

            tqapi_sc_log(self.ctx, i, CString::new(txt).unwrap().as_ptr());
        }
    }

    fn log_info(&self, txt: &str) {
        self.log(LogSeverity::INFO, txt);
    }

    fn log_error(&self, txt: &str) {
        self.log(LogSeverity::ERROR, txt);
    }

    fn log_fatal(&self, txt: &str) {
        self.log(LogSeverity::FATAL, txt);
    }

    fn get_properties (&self ) -> String {
        unsafe {
            let r = tqapi_sc_get_properties(self.ctx);
            let s = c_str_to_string(r);
            return s;
        }
    }

    fn get_mode (&self ) -> RunMode {
        unsafe {
            let r = tqapi_sc_get_mode(self.ctx);
            let s = c_str_to_string(r);
            // FREE
            //tqapi_sc_
            match s.as_str() {
                "backtest" => RunMode::BACKTEST,
                "relatime" => RunMode::REALTIME,
                _ => { assert!(false, "Unknown mode"); RunMode::BACKTEST}
            }
        }
    }

    fn stop(&self) {

    }
}


#[derive(Debug)]
struct StraletWrap {
    stralet : *mut Stralet,
    sc      : *mut StraletContextImpl,
}

impl Drop for StraletWrap {
    fn drop(&mut self) {
        unsafe {
            Box::from_raw(self.sc);
            Box::from_raw(self.stralet);
        }
    }
}

impl StraletWrap {
    fn new (stralet : Box<Stralet>) -> StraletWrap {
        StraletWrap { stralet : Box::into_raw(stralet), sc : ptr::null_mut() }
    }

    extern "C" fn on_init (obj : *mut libc::c_void, ctx: *mut CStraletContext) {
        unsafe {
            let sc = Box::new(StraletContextImpl::new(ctx));
            let sw = obj as *mut StraletWrap;
            (*sw).sc =  Box::into_raw(sc);
            (*(*sw).stralet).on_init(& mut *((*sw).sc));
        }
    }

    extern "C" fn on_fini (obj : *mut libc::c_void, _ctx: *mut CStraletContext) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            (*(*sw).stralet).on_fini(& mut *((*sw).sc));
        }
    }

    extern "C" fn on_quote (obj : *mut libc::c_void, _ctx: *mut CStraletContext, quote : *mut CMarketQuote) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            (*(*sw).stralet).on_quote(& mut *((*sw).sc), (*quote).to_rs());
        }
    }

    extern "C" fn on_bar (obj : *mut libc::c_void, _ctx: *mut CStraletContext, cycle : *const c_char, bar : *mut CBar) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            let s_cycle = c_str_to_string(cycle);
            (*(*sw).stralet).on_bar(&mut *(*sw).sc, s_cycle.as_str(), (*bar).to_rs());
        }
    }

    extern "C" fn on_order (obj : *mut libc::c_void, _ctx: *mut CStraletContext, order : *mut COrder) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            (*(*sw).stralet).on_order(&mut *(*sw).sc, (*order).to_rs());
        }
    }

    extern "C" fn on_trade (obj : *mut libc::c_void, _ctx: *mut CStraletContext, trade : *mut CTrade) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            (*(*sw).stralet).on_trade(&mut *(*sw).sc, (*trade).to_rs());
        }
    }

    extern "C" fn on_timer (obj : *mut libc::c_void, _ctx: *mut CStraletContext, id    : i64,  data : usize) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            (*(*sw).stralet).on_timer(&mut *(*sw).sc, id, data);
        }
    }

    extern "C" fn on_event (obj : *mut libc::c_void, _ctx: *mut CStraletContext, name  : *const c_char, data : usize) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            (*(*sw).stralet).on_event(&mut *(*sw).sc, c_str_to_string(name).as_str(), data);
        }
    }

    extern "C" fn on_account_status (obj : *mut libc::c_void, _ctx: *mut CStraletContext, account : *mut CAccountInfo) {
        unsafe {
            let sw = obj as *mut StraletWrap;
            (*(*sw).stralet).on_account_status(&mut *(*sw).sc, (*account).to_rs());
        }
    }
}

pub struct StraletFactory {
    create_stralet : fn() -> Box<Stralet>
}

impl StraletFactory {
    pub extern "C" fn create  (user_data : *mut libc::c_void) -> *mut CStralet {
        unsafe {
            let factory = user_data as *mut StraletFactory;
            let stralet = ((*factory).create_stralet)();
            let sw = Box::into_raw(Box::new(StraletWrap::new (stralet))) as *mut libc::c_void;

            Box::into_raw(
                Box::new(
                    CStralet{
                        obj      : sw,
                        on_init  : StraletWrap::on_init,
                        on_fini  : StraletWrap::on_fini,
                        on_quote : StraletWrap::on_quote,
                        on_bar   : StraletWrap::on_bar  ,
                        on_order : StraletWrap::on_order,
                        on_trade : StraletWrap::on_trade,
                        on_timer : StraletWrap::on_timer,
                        on_event : StraletWrap::on_event,
                        on_account_status : StraletWrap::on_account_status,
            }))
        }
    }
    pub extern "C" fn destory (_user_data : *mut libc::c_void, c_stralet : *mut CStralet) {
        unsafe {
            let sw = (*c_stralet).obj as *mut StraletWrap;
            Box::from_raw(sw);
        }
    }
}

pub struct BackTest {
}

#[derive(Serialize, Deserialize, Debug, Default)]
pub struct BackTestConfig<'a> {
    pub dapi_addr  : Option<&'a str>,
    pub data_level : Option<&'a str>,
    pub begin_date : i32,
    pub end_date   : i32,
    pub result_dir : Option<&'a str>,
    pub properties : Option<&'a str>
}

impl <'a> BackTest {

    pub fn run(cfg : &BackTestConfig, create_stralet : fn () -> Box<Stralet>) {

        unsafe {
            let cfg_str = serde_json::to_string(&cfg).expect("Wrong cfg");
            let c_cfg = CString::new(cfg_str).unwrap();
            let factory = StraletFactory{ create_stralet : create_stralet};

            let c_factory = Box::into_raw(
                Box::new(CStraletFactory {
                    obj     : Box::into_raw(Box::new(factory)) as *mut libc::c_char,
                    create  : StraletFactory::create,
                    destroy : StraletFactory::destory,
                }));

            tqapi_bt_run(c_cfg.as_ptr(), c_factory);
            Box::from_raw((*c_factory).obj);  // rust factory
            Box::from_raw(c_factory);
        }
    }
}
