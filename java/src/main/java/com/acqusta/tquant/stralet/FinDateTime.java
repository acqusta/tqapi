package com.acqusta.tquant.stralet;

import java.time.LocalDateTime;

public class FinDateTime {
    public FinDateTime(int date, int time) {
        this.date = date;
        this.time = time;
    }

    public LocalDateTime toLocalDateTime() {
        int y = date / 10000;
        int m = (date / 100) % 100;
        int d = (date % 100);
        int ns = (time % 1000) * 1000000;
        int tmp = time / 1000;
        int S = tmp % 100;
        int M = (tmp / 100) % 100;
        int H = (tmp / 10000);

        return LocalDateTime.of(y,m,d, H,M,S,ns);
    }

    public int date;
    public int time;
}
