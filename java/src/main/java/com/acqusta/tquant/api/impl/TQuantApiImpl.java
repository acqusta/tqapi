package com.acqusta.tquant.api.impl;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.api.TradeApi;

import java.util.HashMap;
import java.util.Map;

public class TQuantApiImpl {
    private TradeApiImpl tapi;
    private Map<String, DataApiImpl> dapi_map = new HashMap<String, DataApiImpl>();
    private TQuantApiJni tqapi;

    public TQuantApiImpl(String addr) throws Exception {
        tqapi = new TQuantApiJni(addr);
        tapi = new TradeApiImpl(tqapi);
    }

    public TradeApi getTradeApi() {
        return tapi;
    }

    public DataApi getDataApi(String source) {
        synchronized (dapi_map){
            source = source !=null?source.trim():"";
            DataApiImpl dapi = dapi_map.get(source);

            if (dapi != null)
                return dapi;

            try {
                dapi = new DataApiImpl(tqapi, source);
                dapi_map.put(source, dapi);
                return dapi;
            } catch (Exception e) {
                System.out.println(e);
                return null;
            }
        }
    }
}
