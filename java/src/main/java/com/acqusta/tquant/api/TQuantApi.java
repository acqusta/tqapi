package com.acqusta.tquant.api;

import com.acqusta.tquant.api.impl.DataApiImpl;
import com.acqusta.tquant.api.impl.TradeApiImpl;
import com.acqusta.tquant.api.impl.TQuantApiImpl;

public class TQuantApi {

    public static void setParams(String key, String value)
    {
        TQuantApiImpl.setParams(key, value);
    }

    /**
     * 创建 TradeApi
     * @param addr 服务器地址，取值通常为 tcp://127.0.0.1:10082
     */
    public static TradeApi createTradeApi(String addr) throws Exception{
        return new TradeApiImpl(addr);
    }

    /**
     * 创建 DataApi
     *
     * @return
     */
    public static DataApi  createDataApi(String addr) throws Exception{
        return new DataApiImpl(addr);
    }
}
