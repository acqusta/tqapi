extern crate tqapi;
//extern crate time;
extern crate chrono;

//use std::time::{SystemTime, UNIX_EPOCH};
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

fn test() {
    let cb = Callback{};
    let mut api = DataApi::new("tcp://127.0.0.1:10001");
    api.set_callback(Some(&cb));

    print!("subscribe\n");
    let code = "RB.SHF";
    api.subscribe( &vec!["RB.SHF", "000001.SH"].join(",")).expect("subscribed error");
    print!("call get_ticks\n");

    match api.get_ticks(code, 0) {
        Ok(quotes) => for q in quotes { }//println!("{}", q);}
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

    for _ in 0..10 {
        sleep(Duration::new(1,0));
    }
}

fn main() {
    test();
}
