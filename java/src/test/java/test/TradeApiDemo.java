package test;

import java.util.List;

import xtz.tquant.api.java.TQuantApi;
import xtz.tquant.api.java.TradeApi;
import xtz.tquant.api.java.TradeApi.*;

public class TradeApiDemo {

    TQuantApi api = new TQuantApi("tcp://127.0.0.1:10001");

    TradeApi tapi = api.getTradeApi();

    String account_id = "glsc";

    void test() {
        testQueryAccountStatus();
        testQueryPosition();
        testQueryOrder();
        testQueryTrade();
        testPlaceOrder();
        testCancelOrder();
    }

    void testQueryAccountStatus() {
        TradeApi.CallResult<List<AccountInfo>> result = tapi.queryAccountStatus();

        if ( result.value !=null) {
            for (TradeApi.AccountInfo act : result.value) {
                System.out.println(act.account_id + "," + act.broker + "," + act.account + "," + act.status + "," + act.msg);
            }
        } else {
            System.out.println("queryAccount failed: " + result.msg);
        }
    }

    void testQueryPosition() {
        TradeApi.CallResult<List<Position>> result = tapi.queryPosition(account_id);

        if ( result.value !=null) {
            for (Position pos : result.value) {
                System.out.println("Position " + pos.account_id + "," + pos.code + "," + pos.name + "," + pos.side + "," + pos.current_size + "," + pos.cost_price);
            }
        } else {
            System.out.println("queryPosition failed: " + result.msg);
        }
    }

    void testQueryOrder() {
        TradeApi.CallResult<List<Order>> result = tapi.queryOrders(account_id);

        if ( result.value !=null) {
            for (Order ord : result.value) {
                System.out.println("Order " + ord.account_id + "," + ord.code + "," + ord.name + ","
                        + ord.entrust_action + "," + ord.entrust_price + "," + ord.entrust_size + ","
                        + ord.fill_price + "," + ord.fill_size + "," + ord.status);
            }
        } else {
            System.out.println("queryPosition failed: " + result.msg);
        }
    }

    void testQueryTrade() {
        TradeApi.CallResult<List<Trade>> result = tapi.queryTrades(account_id);

        if ( result.value !=null) {
            for (Trade trade : result.value) {
                System.out.println("Trade " + trade.account_id + "," + trade.code + "," + trade.name + ","
                        + trade.entrust_no + "," + trade.fill_no + "," + trade.entrust_action + "," +
                        + trade.fill_price + "," + trade.fill_size);
            }
        } else {
            System.out.println("queryPosition failed: " + result.msg);
        }
    }
    void testPlaceOrder() {
        CallResult<String> result = tapi.placeOrder(account_id, "000001.SH", 1.0, 100, "Buy" );
        System.out.println("entrust_no: " + (result.value!=null ? result.value : "<null>"));
        System.out.println("msg: " + (result.msg));
    }

    void testCancelOrder() {
        CallResult<String> placeResult = tapi.placeOrder(account_id, "399001.SZ", 1.0, 100, "Buy" );
        System.out.println("entrust_no: " + (placeResult.value!=null ? placeResult.value : "<null>"));
        System.out.println("msg: " + (placeResult.msg));
        CallResult<Boolean> result = tapi.cancelOrder(account_id, "399001.SZ", placeResult.value);
        System.out.println("result: " + (result.value!=null ? result.value : "<null>"));
        System.out.println("msg: " + (result.msg));
    }

    public static void main(String[] args) {

        new TradeApiDemo().test();
    }
}
