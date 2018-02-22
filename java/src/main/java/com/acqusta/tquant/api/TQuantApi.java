package com.acqusta.tquant.api;

import com.acqusta.tquant.api.impl.TQuantApiImpl;

public class TQuantApi {

    private TQuantApiImpl impl;
    /**
     * 初始化 TQuantApi
     * @param addr 服务器地址，取值通常为 tcp://127.0.0.1:10082
     */
    public TQuantApi(String addr) throws Exception{
        impl = new TQuantApiImpl(addr);
    }

    /**
     * 取数据接口
     *
     * @return
     */
    public TradeApi getTradeApi() {
        return impl.getTradeApi();
    }

    /**
     *  取交易接口
     *
     * @return
     */
    public DataApi  getDataApi(String source) {
        return impl.getDataApi(source);
    }


}
