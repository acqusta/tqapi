package com.acqusta.tquant.api.impl;

import com.acqusta.tquant.api.TradeApi;

public class TradeApiImpl implements TradeApi {

    private Callback callback = null;

    private TQuantApiJni tqapi;

    private long handle;
    public TradeApiImpl(TQuantApiJni tqapi) {
        this.tqapi = tqapi;
        this.handle = TQuantApiJni.getTradeApi(this.tqapi.handle);
    }


    @Override
    public CallResult<AccountInfo[]> queryAccountStatus() {
        try {
            return new CallResult(TradeApiJni.queryAccountStatus(handle), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Balance> queryBalance(String account_id) {
        try {
            return new CallResult(TradeApiJni.queryBalance(handle, account_id), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Order[]> queryOrders(String account_id) {
        try {
            return new CallResult(TradeApiJni.queryOrders(handle, account_id), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Trade[]> queryTrades(String account_id) {
        try {
            return new CallResult(TradeApiJni.queryTrades(handle, account_id), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Position[]> queryPositions(String account_id) {
        try {
            return new CallResult(TradeApiJni.queryPositions(handle, account_id), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<OrderID> placeOrder(String account_id, String code, double price, long size, String action, int order_id) {
        try {
            return new CallResult(TradeApiJni.placeOrder(handle, account_id, code, price, size, action, order_id), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Boolean> cancelOrder(String account_id, String code, String entrust_no) {
        try {
            return new CallResult(TradeApiJni.cancelOrder(handle, account_id, code, entrust_no), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<Boolean> cancelOrder(String account_id, String code, int order_id) {
        try {
            return new CallResult(TradeApiJni.cancelOrder(handle, account_id, code, order_id), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }
    }

    @Override
    public CallResult<String> query(String account_id, String command, String params) {
        try {
            return new CallResult(TradeApiJni.query(handle, account_id, command, params), "");
        }catch (Exception e) {
            return new CallResult<>(null, e.getMessage());
        }

    }

    @Override
    public void setCallback(Callback callback)
    {
        TradeApiJni.setCallback(handle, callback);
    }
}

