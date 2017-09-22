package xtz.tquant.api.java.impl;

import java.util.*;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.JavaType;
import com.fasterxml.jackson.databind.ObjectMapper;


import xtz.tquant.api.java.*;

@JsonIgnoreProperties(ignoreUnknown = true)
class HeartBeat {
    public long sub_hash;
};

@JsonIgnoreProperties(ignoreUnknown = true)
class SubscribeResult {
    public List<String> sub_codes;
    public long         sub_hash;
};

public class DataApiImpl implements DataApi {

    private JsonRpc.JsonRpcClient client;

    private JavaType quoteListClass = null;
    private JavaType barListClass = null;
    private JavaType stringListClass = null;
    private JavaType stringMapClass = null;

    private ObjectMapper mapper = new ObjectMapper();

    private DataApi.Callback callback = null;
    private HashSet<String> sub_codes = new HashSet<String>();
    private long sub_hash = 0;

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

    public void onConnected() {
    }

    public void onDisconnected() {

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

        if (codes!=null && codes.length > 0) {
            for ( String s : codes)
                this.sub_codes.add(s);
        }

        if (codes == null)
            codes = new String[0];

        Map<String, Object> params = new HashMap<String, Object>();

        StringBuffer sb = new StringBuffer();
        for (String s : codes)
            sb.append(s).append(",");

        params.put( "codes", sb.toString());

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tsq_sub", params, 6000);

        if (r.result != null) {
            SubscribeResult result = mapper.convertValue(r.result, SubscribeResult.class);
            if (result != null && codes.length > 0)
                this.sub_hash = result.sub_hash;

            return new CallResult(result.sub_codes, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<List<String>> unsubscribe(String[] codes) {

        if (codes!=null && codes.length > 0) {
            for(String s : codes)
                this.sub_codes.remove(s);
        }

        if (codes == null)
            codes = new String[0];

        Map<String, Object> params = new HashMap<String, Object>();

        StringBuffer sb = new StringBuffer();
        for (String s : codes)
            sb.append(s).append(",");

        params.put( "codes", sb.toString());

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tsq_unsub", params, 6000);

        if (r.result != null) {
            SubscribeResult result = mapper.convertValue(r.result, SubscribeResult.class);
            if (result != null)
                this.sub_hash = result.sub_hash;
            return new CallResult(result.sub_codes, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    @JsonIgnoreProperties(ignoreUnknown = true)
    static class BarInd {
        public String cycle;
        public Bar bar;
    }

    private void onHeartBeat(Object value) {

        if (this.sub_codes.isEmpty())
            return;

        HeartBeat h = mapper.convertValue(value, HeartBeat.class);
        if (h.sub_hash == this.sub_hash && this.sub_hash != 0)
            return;

        Map<String, Object> params = new HashMap<String, Object>();

        Thread thread = new Thread ( new Runnable() { public void run() {
            StringBuffer sb = new StringBuffer();
            for (String s : sub_codes)
                sb.append(s).append(",");

            params.put( "codes", sb.toString());

            //System.out.println("subscribe " + sb.toString());

            JsonRpc.JsonRpcCallResult r = client.call("dapi.tsq_sub", params, 1500);

            if (r.result != null) {
                SubscribeResult result = mapper.convertValue(r.result, SubscribeResult.class);
                if (result != null) {
                    sub_hash = result.sub_hash;
                    //System.out.println("subsscribe again: " + sub_hash);
                }
            }else {
                //System.out.println("subsscribe again error: " + getErrorText(r.error));
            }

        }});

        thread.setDaemon(true);
        thread.start();
    }
    public void onNotification(String event, Object value) {

        try {
            if (event.equals(".sys.heartbeat")) {
                onHeartBeat(value);

            } else {
                if (this.callback == null) return;

                if (event.equals("dapi.quote")) {
                    MarketQuote q = mapper.convertValue(value, MarketQuote.class);
                    if (q != null) this.callback.onMarketQuote(q);

                } else if (event.equals("dapi.bar")) {
                    BarInd ind = mapper.convertValue(value, BarInd.class);
                    if (ind != null)
                        this.callback.onBar(ind.cycle, ind.bar);
                }
            }
        }catch (Throwable t) {
            t.printStackTrace();
        }
    }
}
