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
        print!("on_quote {}", quote);
    }
    fn on_bar  (&mut self, cycle : &str, bar : Bar) {
        print!("on_bar {}, {}",cycle, bar);
    }
}

fn test() {
    let cb = Callback{};
    let mut api = DataApi::new("tcp://127.0.0.1:10001");
    api.set_callback(Some(&cb));

    print!("subscribe\n");
    let code = "RB.SHF";
    api.subscribe( &vec!["RB.SHF", "000001.SH"].join(".")).expect("subscribed error");
    print!("call get_ticks\n");

    let count = 1;
    let begin_time = chrono::Local::now();

    for _ in 0..count {
        let quotes = api.get_bars(code, "1m", 0, true).ok().expect("get_ticks failed");
        for q in quotes {
            println!("{}", q);
        }
    }

    let end_time = chrono::Local::now();
    println!("{} ", (end_time - begin_time) / count);
    println!("call get_ticks done\n");

    while true {
        sleep(Duration::new(1,0));
    }
}

fn main() {
    test();
}
