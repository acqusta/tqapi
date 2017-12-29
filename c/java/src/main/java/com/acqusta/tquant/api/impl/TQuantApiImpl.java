package com.acqusta.tquant.api.impl;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.api.TradeApi;

public class TQuantApiImpl {
    private TradeApiImpl trade_api;
    private DataApiImpl  data_api;

    public TQuantApiImpl(String addr) throws Exception {
        TQuantApiJni tqapi = new TQuantApiJni(addr);
        trade_api = new TradeApiImpl(tqapi);
        data_api = new DataApiImpl(tqapi);
    }

    public TradeApi getTradeApi() {
        return trade_api;
    }

    public DataApi getDataApi() {
        return data_api;
    }
}
