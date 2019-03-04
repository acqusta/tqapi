package com.acqusta.tquant.stralet;

import java.util.Dictionary;

import com.acqusta.tquant.stralet.impl.StraletJni;
import com.fasterxml.jackson.databind.ObjectMapper;

public class BackTest {

    public static class Config {
        public class Holding {
            public String code;
            public String side;
            public long   size;
            public double cost_price;
        }

        public class AccountConfig {
            public String    account_id;
            public double    init_balance;
            public Holding[] init_holdings;
        }

        public String dapi_addr;
        public String data_level;
        public int    begin_date;
        public int    end_date;
        public String result_dir;

        public AccountConfig[] accounts;
        public Dictionary<String, Object> properties;
    }

    public static void run(Config cfg, StraletCreator straletCreator) throws Exception {

        ObjectMapper mapper = new ObjectMapper();
        String txt = mapper.writeValueAsString(cfg);
        StraletJni.runBacktest(txt, straletCreator);
    }
}
