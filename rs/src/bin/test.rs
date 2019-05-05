extern crate tqapi;
extern crate chrono;

use tqapi::api::*;
use std::thread::sleep;
use std::time::Duration;


struct Callback {
}

impl DataApiCallback for Callback {
    fn on_quote(&mut self, quote : MarketQuote) {
        println!("on_quote {}", quote);
    }
    fn on_bar  (&mut self, cycle : &str, bar : Bar) {
        println!("on_bar {}, {}",cycle, bar);
    }
}

impl TradeApiCallback for Callback {
    fn on_order (&mut self, order : Order) {
        println!("on_order {}", order);
    }

    fn on_trade (&mut self, trade : Trade) {
        println!("on_trade {}", trade);
    }

    fn on_account_status (&mut self, account : AccountInfo) {
        println!("on_account_status {}", account);
    }
}

fn test_dapi() {
    let cb = Callback{};
    let mut api = DataApi::new("tcp://127.0.0.1:10001");
    api.set_callback(Some(Box::new(cb)));

    print!("subscribe\n");
    let code = "RB.SHF";
    api.subscribe( &vec!["RB.SHF", "000001.SH"].join(",")).expect("subscribed error");
    print!("call get_ticks\n");

    match api.get_ticks(code, 0) {
        Ok(quotes) => for q in quotes { println!("{}", q);}
        Err(msg) => println!("error: {}", msg)
    }
    // let count = 1;
    // let begin_time = chrono::Local::now();

    // for _ in 0..count {
    //     match api.get_ticks(code, 0) {
    //         Ok(quotes) => for q in quotes { println!("{}", q);}
    //         Err(msg) => println!("error: {}", msg)
    //     }
    // }

    // let end_time = chrono::Local::now();
    // println!("{} ", (end_time - begin_time) / count);
    // println!("call get_ticks done\n");

    for _ in 0..3 {
        sleep(Duration::new(1,0));
    }
}

fn test_tapi() {
    let cb = Callback{};
    let mut api = TradeApi::new("tcp://127.0.0.1:10001");
    api.set_callback(Some(Box::new(cb)));

    println!("query_accounts");
    match api.query_accounts() {
        Ok(accounts) => for a in accounts { println!("{}", a);}
        Err(msg)     => println!("error: {}", msg)
    }

    for _ in 0..3 {
        sleep(Duration::new(1,0));
    }
}


pub struct TestStralet {
}

impl Stralet for TestStralet {
    fn on_init (&mut self, ctx: &mut StraletContext) {
        ctx.log_info( format!{"on_init {}", ctx.get_trade_date()}.as_str() );
        ctx.get_data_api().subscribe("000001.SH").expect("subscribe error");
    }

    fn on_fini (&mut self, ctx: &mut StraletContext) {
        ctx.log_info( format!{"on_fini {}", ctx.get_trade_date()}.as_str() );
    }

    fn on_quote (&mut self, _ctx: &mut StraletContext, _quote : MarketQuote) {
        //ctx.log_info( format!{"on_quote: {}", quote}.as_str() );
    }

    fn on_bar (&mut self, _ctx: &mut StraletContext, _cycle : &str, _bar : Bar) {

    }

    fn on_order (&mut self, _ctx: &mut StraletContext, _order : Order) {

    }

    fn on_trade (&mut self, _ctx: &mut StraletContext, _trade : Trade) {

    }

    fn on_timer (&mut self, _ctx: &mut StraletContext, _id : i64,  _data : usize) {

    }

    fn on_event (&mut self, _ctx: &mut StraletContext, _name  : &str, _data : usize) {

    }

    fn on_account_status (&mut self, _ctx: &mut StraletContext, _account : AccountInfo) {

    }
}

fn create_stralet() -> Box<Stralet>{
    Box::new(TestStralet{})
}

pub fn test_stralet() {
    let cfg = BackTestConfig {
        dapi_addr  : Some("tcp://127.0.0.1:10001"),
        data_level : Some("tk"),
        begin_date : 20190101,
        end_date   : 20190501,
        // result_dir : None,
        // properties : None
        ..Default::default()
    };

    BackTest::run(&cfg, create_stralet );
}


fn main() {
    //test_dapi();
    //test_tapi();
    test_stralet();
}
