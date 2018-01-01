package com.acqusta.tquant.api.impl;

import com.acqusta.tquant.api.DataApi;

import javax.xml.transform.Source;

public class DataApiImpl implements DataApi {

    private TQuantApiJni tqapi;
    private long handle;

    public DataApiImpl(TQuantApiJni tqapi) {
        this.tqapi = tqapi;
        this.handle = TQuantApiJni.getDataApi(this.tqapi.handle);
    }
    
    @Override
    public CallResult<MarketQuote[]> getTick(String code, int trading_day, String source) {
        try {
            return new CallResult(DataApiJni.getTick(handle, code, trading_day, source), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<DailyBar[]> getDailyBar (String code, String price_adj, Boolean align, String source) {
        try {
            return new CallResult(DataApiJni.getDailyBar(handle, code, price_adj, align, source), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Bar[]> getBar(String code, String cycle, int trading_day, Boolean align, String source) {
        try {
            return new CallResult(DataApiJni.getBar(handle, code,  cycle, trading_day, align, source), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<MarketQuote> getQuote(String code, String source) {
        try {
            return new CallResult(DataApiJni.getQuote(handle, code, source), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<String[]> subscribe(String[] codes, String source) {
        try {
            return new CallResult(DataApiJni.subscribe(handle, codes, source), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<String[]> unsubscribe(String[] codes, String source) {
        try {
            return new CallResult(DataApiJni.unsubscribe(handle, codes, source), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public void setCallback(Callback callback) {
        DataApiJni.setCallback(handle, callback);
    }
}
