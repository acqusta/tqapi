package xtz.tquant.api.java;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.JavaType;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class TQuantApi {

    private String addr;
    private JsonRpc.JsonRpcClient client;
    private TradeApiImpl trade_api;
    private DataApiImpl  data_api;

    /**
     * 初始化 TQuantApi
     * @param addr 服务器地址，取值通常为 tcp://127.0.0.1:10082
     */
    public TQuantApi(String addr) {

        this.addr = addr;
        this.client = new JsonRpc.JsonRpcClient();
        this.client.connect(addr);
        this.trade_api = new TradeApiImpl(client);
        this.data_api  = new DataApiImpl(client);

        client.setCallback(
                new JsonRpc.JsonRpcClient.Callback() {
                    @Override
                    public void onConnected() {
                    }

                    @Override
                    public void onDisconnected() {
                    }

                    @Override
                    public void onNotification(String event, Object value) {
                        if (event.startsWith("tapi.")) {
                            trade_api.onNotification(event, value);
                        } else if (event.startsWith("dapi.")) {
                            data_api.onNotification(event, value);
                        }
                    }
                });
    }

    /**
     * 取数据接口
     *
     * @return
     */
    public TradeApi getTradeApi() {
        return trade_api;
    }

    /**
     *  取交易接口
     *
     * @return
     */
    public DataApi  getDataApi() {
        return data_api;
    }


}

class TradeApiImpl implements TradeApi {

    private JsonRpc.JsonRpcClient client;

    private JavaType accountInfoListClass = null;
    private JavaType orderListClass = null;
    private JavaType positionListClass = null;
    private JavaType tradeListClass = null;
    private ObjectMapper mapper = new ObjectMapper();

    TradeApiImpl(JsonRpc.JsonRpcClient client) {
        this.client = client;

        accountInfoListClass = getCollectionType(List.class, AccountInfo.class);
        orderListClass       = getCollectionType(List.class, Order.class);
        positionListClass    = getCollectionType(List.class, Position.class);
        tradeListClass       = getCollectionType(List.class, Trade.class);

        mapper.setSerializationInclusion(JsonInclude.Include.NON_NULL);
        mapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
    }

    private JavaType getCollectionType(Class<?> collectionClass, Class<?>... elementClasses) {
            return mapper.getTypeFactory().constructParametricType(collectionClass, elementClasses);
    }

    private String getErrorText(JsonRpc.JsonRpcError error) {
        if (error != null)
            return error.error + "," + error.message;
        else
            return "";
    }

    @Override
    public CallResult<List<AccountInfo>> queryAccountStatus() {

        Map<String, Object> params = new HashMap<String, Object>();

        JsonRpc.JsonRpcCallResult r = client.call("tapi.account_status", params, 10000);

        if (r.result != null) {
            List<AccountInfo> accounts = mapper.convertValue(r.result, accountInfoListClass);
            return new CallResult(accounts, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<Balance> queryBalance(String account_id) {

        Map<String, Object> params = new HashMap<String, Object>();

        JsonRpc.JsonRpcCallResult r = client.call("tapi.query_balance", params, 10000);

        if (r.result != null) {
            Balance bal = mapper.convertValue(r.result, Balance.class );
            return new CallResult(bal, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<List<Order>> queryOrders(String account_id) {

        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "account_id", account_id);

        JsonRpc.JsonRpcCallResult r = client.call("tapi.query_orders", params, 10000);

        if (r.result != null) {
            List<Order> orders = mapper.convertValue(r.result, orderListClass);
            return new CallResult(orders, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<List<Trade>> queryTrades(String account_id) {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "account_id", account_id);

        JsonRpc.JsonRpcCallResult r = client.call("tapi.query_trades", params, 10000);

        if (r.result != null) {
            List<Trade> trades = mapper.convertValue(r.result, tradeListClass);
            return new CallResult(trades, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<List<Position>> queryPosition(String account_id) {

        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "account_id", account_id);

        JsonRpc.JsonRpcCallResult r = client.call("tapi.query_positions", params, 10000);

        if (r.result != null) {
            List<Position> positions = mapper.convertValue(r.result, positionListClass);
            return new CallResult(positions, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<String> placeOrder(String account_id, String code, double price, long size, String action) {

        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "account_id", account_id);
        params.put( "code", code);
        params.put( "price", price);
        params.put( "size", size);
        params.put( "action", action);

        JsonRpc.JsonRpcCallResult r = client.call("tapi.place_order", params, 10000);

        if (r.result != null) {
            return new CallResult( (String)r.result, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<Boolean> cancelOrder(String account_id, String code, String entrust_no) {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "account_id", account_id);
        params.put( "entrust_no", entrust_no);
        params.put( "code",   code);

        JsonRpc.JsonRpcCallResult r = client.call("tapi.cancel_order", params, 10000);

        if (r.result != null) {
            return new CallResult( (Boolean)r.result, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    void onNotification(String event, Object value) {

    }
}


class DataApiImpl implements DataApi {

    private JsonRpc.JsonRpcClient client;

    private JavaType quoteListClass = null;
    private JavaType barListClass = null;
    private JavaType stringListClass = null;

    private ObjectMapper mapper = new ObjectMapper();

    private DataApi.Callback callback = null;

    DataApiImpl(JsonRpc.JsonRpcClient client) {
        this.client = client;

        quoteListClass      = getCollectionType(List.class, MarketQuote.class);
        barListClass        = getCollectionType(List.class, Bar.class);
        stringListClass     = getCollectionType(List.class, String.class);

        mapper.setSerializationInclusion(JsonInclude.Include.NON_NULL);
        mapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
    }
    private JavaType getCollectionType(Class<?> collectionClass, Class<?>... elementClasses) {
        return mapper.getTypeFactory().constructParametricType(collectionClass, elementClasses);
    }

    private String getErrorText(JsonRpc.JsonRpcError error) {
        if (error != null)
            return error.error + "," + error.message;
        else
            return "";
    }

    @Override
    public CallResult<List<MarketQuote>> tick(String code, int tradingday) {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "code", code);
        if (tradingday > 0 )
            params.put( "tradingday", tradingday);

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tst", params, 6000);

        if (r.result != null) {
            List<MarketQuote> ticks = mapper.convertValue(r.result, quoteListClass);
            return new CallResult(ticks, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
    }

    @Override
    public CallResult<List<Bar>> bar(String code, String cycle, int tradingday) {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put( "code", code);
        if (cycle != null && !cycle.isEmpty())
            params.put("cycle", cycle);
        if (tradingday > 0 )
            params.put( "tradingday", tradingday);

        JsonRpc.JsonRpcCallResult r = client.call("dapi.tsi", params, 6000);

        if (r.result != null) {
            List<Bar> bars = mapper.convertValue(r.result, quoteListClass);
            return new CallResult(bars, getErrorText(r.error));
        }else {
            return new CallResult<>(null, getErrorText(r.error));
        }
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

    void onNotification(String event, Object value) {

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
