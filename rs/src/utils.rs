
pub fn fin_time(h:i32, m:i32, s:i32) -> i32 {
    (h * 10000 + m * 100 + s) * 1000
}

#[inline]
pub fn fin_time_to_ms(t : i32) -> i32 {
    let ms = t % 1000;
    let hms = t / 1000;
    let h = hms / 10000;
    let m = (hms/100) % 100;
    let s = hms % 100;

    (h*3600 + m*60 + s) * 1000 + ms
}

pub fn fin_time_diff( t1 : i32, t2 : i32) -> i32 {
    fin_time_to_ms(t1)  - fin_time_to_ms(t2)
}