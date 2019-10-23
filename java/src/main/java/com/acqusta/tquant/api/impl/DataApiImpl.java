package com.acqusta.tquant.api.impl;

import com.acqusta.tquant.api.DataApi;

public class DataApiImpl implements DataApi {

    private long handle;

    public DataApiImpl(String addr) throws Exception {
        this.handle = DataApiJni.create(addr);
    }

    public DataApiImpl(long handle) {
        this.handle = handle;
    }

    @Override
    public void finalize()
    {
        DataApiJni.destroy(this.handle);
    }

    @Override
    public CallResult<MarketQuote[]> getTick(String code, int trading_day) {
        try {
            return new CallResult(DataApiJni.getTick(this.handle, code, trading_day), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<DailyBar[]> getDailyBar (String code, String price_adj, Boolean align) {
        try {
            return new CallResult(DataApiJni.getDailyBar(this.handle, code, price_adj, align), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Bar[]> getBar(String code, String cycle, int trading_day, Boolean align) {
        try {
            return new CallResult(DataApiJni.getBar(this.handle, code,  cycle, trading_day, align), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<MarketQuote> getQuote(String code) {
        try {
            return new CallResult(DataApiJni.getQuote(this.handle, code), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<String[]> subscribe(String[] codes) {
        try {
            return new CallResult(DataApiJni.subscribe(this.handle, codes), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<String[]> unsubscribe(String[] codes) {
        try {
            return new CallResult(DataApiJni.unsubscribe(this.handle, codes), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public void setCallback(Callback callback) {
        DataApiJni.setCallback(this.handle, callback);
    }
}
