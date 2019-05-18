
pub fn fin_time(h:u32, m:u32, s:u32) -> u32 {
    (h * 10000 + m * 100 + s) * 1000
}

#[inline]
pub fn fin_time_to_ms(t : u32) -> u32 {
    let ms = t % 1000;
    let hms = t / 1000;
    let h = hms / 10000;
    let m = (hms/100) % 100;
    let s = hms % 100;

    (h*3600 + m*60 + s) * 1000 + ms
}

pub fn fin_time_diff( t1 : u32, t2 : u32) -> i32 {
    let ms1 = fin_time_to_ms(t1);
    let ms2 = fin_time_to_ms(t2);
    if ms1 > ms2 {
        return (ms1 - ms2) as i32;
    } else {
        return -1 * ((ms2 - ms1) as i32);
    }
}