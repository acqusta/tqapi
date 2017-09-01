package xtz.tquant.api.java.impl;

import java.util.*;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.JavaType;
import com.fasterxml.jackson.databind.ObjectMapper;


import xtz.tquant.api.java.*;


public class DataApiImpl implements DataApi {

    private JsonRpc.JsonRpcClient client;

    private JavaType quoteListClass = null;
    private JavaType barListClass = null;
    private JavaType stringListClass = null;
    private JavaType stringMapClass = null;

    private ObjectMapper mapper = new ObjectMapper();

    private DataApi.Callback callback = null;

    public DataApiImpl(JsonRpc.JsonRpcClient client) {
        this.client = client;

        quoteListClass      = getCollectionType(List.class, MarketQuote.class);
        barListClass        = getCollectionType(List.class, Bar.class);
        stringListClass     = getCollectionType(List.class, String.class);
        stringMapClass      = getCollectionType(Map.class, String.class, Object.class);

        mapper.setSerializationInclusion(JsonInclude.Include.NON_NULL);
        mapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
    }
    private JavaType getCollectionType(Class<?> collectionClass, Class<?>... elementClasses) {
        return mapper.getTypeFactory().constructParametricType(collectionClass, elementClasses);
    }

    private String getErrorText(JsonRpc.JsonRpcError error) {
        if (error != null)
            return error.code + "," + error.message;
        else
            return "";
    }

    @Override
    public CallResult<List<MarketQuote>> tick(String code, int trading_day) {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "code", code);

        if (trading_day > 0 )
            params.put( "trading_day", trading_day);

        //params.put("_format", "rowset");

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tst", params, 6000);

        if (r.result == null)
            return new CallResult<>(null, getErrorText(r.error));

        Map<String, Object> data = mapper.convertValue(r.result, stringMapClass);
        List<String> codes = (List<String>)data.getOrDefault("code", null);
        ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        for (int i = 0; i < codes.size(); i++)
            list.add(new HashMap<String, Object>());

        for ( String key : data.keySet()) {
            List<Object> value = (List<Object>)data.get(key);
            int i = 0;
            for (Object obj : value) {
                list.get(i).put(key, obj);
                i++;
            }
        }
        List<MarketQuote> ticks = mapper.convertValue(list, quoteListClass);
//        List<MarketQuote> ticks = mapper.convertValue(r.result, quoteListClass);
        return new CallResult(ticks, getErrorText(r.error));
    }

    @Override
    public CallResult<List<Bar>> bar(String code, String cycle, int trading_day, String price_adj) {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "code", code);

        if (cycle != null && !cycle.isEmpty())
            params.put("cycle", cycle);

        if (trading_day > 0 )
            params.put( "trading_day", trading_day);

        if (price_adj != null & !price_adj.isEmpty())
            params.put("price_adj", price_adj);

        //params.put("_format", "rowset");

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tsi", params, 6000);

        if (r.result == null)
            return new CallResult<>(null, getErrorText(r.error));

        Map<String, Object> data = mapper.convertValue(r.result, stringMapClass);
        List<String> codes = (List<String>)data.getOrDefault("code", null);
        ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        for (int i = 0; i < codes.size(); i++)
            list.add(new HashMap<String, Object>());

        for ( String key : data.keySet()) {
            List<Object> value = (List<Object>)data.get(key);
            int i = 0;
            for (Object obj : value) {
                list.get(i).put(key, obj);
                i++;
            }
        }
        List<Bar> bars = mapper.convertValue(list, barListClass);

        //List<Bar> bars = mapper.convertValue(r.result, barListClass);

        return new CallResult(bars, getErrorText(r.error));
    }

    @Override
    public CallResult<MarketQuote> quote(String code) {

        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "code", code);

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tsq_quote", params, 6000);

        if (r.result != null) {
            MarketQuote q = mapper.convertValue(r.result, MarketQuote.class);
            return new CallResult(q, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<List<String>> subscribe(String[] codes) {

        if (codes == null)
            codes = new String[0];

        Map<String, Object> params = new HashMap<String, Object>();

        StringBuffer sb = new StringBuffer();
        for (String s : codes)
            sb.append(s).append(",");

        params.put( "codes", sb.toString());

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tsq_sub", params, 6000);

        if (r.result != null) {
            List<String> subscribed = mapper.convertValue(r.result, stringListClass);
            return new CallResult(subscribed, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<List<String>> unsubscribe(String[] codes) {

        if (codes == null)
            codes = new String[0];

        Map<String, Object> params = new HashMap<String, Object>();

        StringBuffer sb = new StringBuffer();
        for (String s : codes)
            sb.append(s).append(",");

        params.put( "codes", sb.toString());

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tsq_unsub", params, 6000);

        if (r.result != null) {
            List<String> subscribed = mapper.convertValue(r.result, stringListClass);
            return new CallResult(subscribed, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public void onNotification(String event, Object value) {

        if (this.callback == null) return;

        try {
            if (event.equals("dapi.quote")) {
                MarketQuote q = mapper.convertValue(value, MarketQuote.class);
                if (q != null) this.callback.onMarketQuote(q);
            }
        }catch (Throwable t) {
            t.printStackTrace();
        }
    }
}
