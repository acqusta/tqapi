package com.acqusta.tquant.api.impl;

import com.acqusta.tquant.api.DataApi;

import javax.xml.transform.Source;

public class DataApiImpl implements DataApi {

    private DataApiJni dapi;

    public DataApiImpl(TQuantApiJni tqapi, String source) throws Exception {
        this.dapi = new DataApiJni(tqapi, source);
    }
    
    @Override
    public CallResult<MarketQuote[]> getTick(String code, int trading_day) {
        try {
            return new CallResult(DataApiJni.getTick(dapi.handle, code, trading_day), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<DailyBar[]> getDailyBar (String code, String price_adj, Boolean align) {
        try {
            return new CallResult(DataApiJni.getDailyBar(dapi.handle, code, price_adj, align), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Bar[]> getBar(String code, String cycle, int trading_day, Boolean align) {
        try {
            return new CallResult(DataApiJni.getBar(dapi.handle, code,  cycle, trading_day, align), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<MarketQuote> getQuote(String code) {
        try {
            return new CallResult(DataApiJni.getQuote(dapi.handle, code), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<String[]> subscribe(String[] codes) {
        try {
            return new CallResult(DataApiJni.subscribe(dapi.handle, codes), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public CallResult<String[]> unsubscribe(String[] codes) {
        try {
            return new CallResult(DataApiJni.unsubscribe(dapi.handle, codes), "");
        }catch (Exception e) {
            return new CallResult(null, e.getMessage());
        }
    }

    @Override
    public void setCallback(Callback callback) {
        DataApiJni.setCallback(dapi.handle, callback);
    }
}
