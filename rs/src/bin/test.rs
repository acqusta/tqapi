extern crate tqapi;

use tqapi::api::*;

struct Callback {

}

impl DataApiCallback for Callback {
    fn on_quote(&mut self, quote : MarketQuote) {
        print!("on_quote {}", quote);
    }
    fn on_bar  (&mut self, cycle : String, bar : Bar) {
        print!("on_bar {}, {}",cycle, bar);
    }
}

fn test() {
    let mut api = DataApi::new("tcp://127.0.0.1:10001");
    let cb = Callback{};

    api.set_callback(cb);

    print!("subscribe\n");
    api.subscribe( &vec![String::from("000001.SH")]);
    print!("call get_ticks\n");
    let quotes = api.get_ticks("000001.SH", 0).ok().expect("get_ticks failed");
    for q in quotes {
        println!("{}", q);
    }
    print!("call get_ticks done\n");
}

fn main() {
    test();
}
