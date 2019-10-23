package com.acqusta.tquant.stralet;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.api.TradeApi;
import com.acqusta.tquant.stralet.impl.StraletContxtImpl;

public class Stralet {
    private StraletContext ctx;

    public Stralet() {
    }

    private void setContext(long handle) throws Exception {
        this.ctx = new StraletContxtImpl(handle);
    }

    public StraletContext getContext() {
        return this.ctx;
    }

    public void onInit() {

    }

    public void onFini() {
    }

    public void onQuote(DataApi.MarketQuote quote) {

    }

    public void onBar(String cycle, DataApi.Bar bar) {
    }

    public void onOrder(TradeApi.Order order) {

    }

    public void onTrade(TradeApi.Trade trade) {

    }

    public void onTimer(long id, long data) {

    }

    public void onEvent(String name, long data) {

    }

    public void onAccountStatus(TradeApi.AccountInfo account) {

    }
}
