package xtz.tquant.api.java.impl;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.JavaType;
import com.fasterxml.jackson.databind.ObjectMapper;

import xtz.tquant.api.java.*;

public class TradeApiImpl implements TradeApi {

    private JsonRpc.JsonRpcClient client;

    private JavaType accountInfoListClass = null;
    private JavaType orderListClass = null;
    private JavaType positionListClass = null;
    private JavaType tradeListClass = null;
    private ObjectMapper mapper = new ObjectMapper();

    public TradeApiImpl(JsonRpc.JsonRpcClient client) {
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
            return error.code + "," + error.message;
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

    public void onNotification(String event, Object value) {

    }
}

