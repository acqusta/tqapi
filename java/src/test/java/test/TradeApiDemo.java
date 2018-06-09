package test;

import com.acqusta.tquant.api.TQuantApi;
import com.acqusta.tquant.api.TradeApi;
import com.acqusta.tquant.api.TradeApi.*;

public class TradeApiDemo{

    TradeApi tapi = TQuantApi.createTradeApi("ipc://tqc_10001");

    String account_id = "glsc";

    public TradeApiDemo() throws Exception {
    }

    void test() {
        testQueryAccountStatus();
        testQueryPosition();
        testQueryOrder();
        testQueryTrade();
        testPlaceOrder();
        testCancelOrder();
    }

    void testQueryAccountStatus() {
        TradeApi.CallResult<AccountInfo[]> result = tapi.queryAccountStatus();

        if ( result.value !=null) {
            for (TradeApi.AccountInfo act : result.value) {
                System.out.println(act.account_id + "," + act.broker + "," + act.account + "," + act.status + "," + act.msg);
                TradeApi.CallResult<Balance> r = tapi.queryBalance(act.account_id);
                if (r.value!=null)
                    System.out.println("Bal: " + r.value.fund_account + "," + r.value.init_balance + "," + r.value.enable_balance);
            }
        } else {
            System.out.println("queryAccount failed: " + result.msg);
        }
    }

    void testQueryPosition() {
        TradeApi.CallResult<Position[]> result = tapi.queryPositions(account_id);

        if ( result.value !=null) {
            for (Position pos : result.value) {
                System.out.println("Position " + pos.account_id + "," + pos.code + "," + pos.name + "," + pos.side + "," + pos.current_size + "," + pos.cost_price);
            }
        } else {
            System.out.println("queryPosition failed: " + result.msg);
        }
    }

    void testQueryOrder() {
        TradeApi.CallResult<Order[]> result = tapi.queryOrders(account_id);

        if ( result.value !=null) {
            for (Order ord : result.value) {
                System.out.println("Order " + ord.account_id + "," + ord.code + "," + ord.name + ","
                        + ord.entrust_action + "," + ord.entrust_price + "," + ord.entrust_size + ","
                        + ord.fill_price + "," + ord.fill_size
                        + "," + ord.status);
            }
        } else {
            System.out.println("queryPosition failed: " + result.msg);
        }
    }

    void testQueryTrade() {
        TradeApi.CallResult<Trade[]> result = tapi.queryTrades(account_id);

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
        CallResult<OrderID> result = tapi.placeOrder(account_id, "000001.SH", 1.0, 100, EntrustAction.Buy, 0 );
        if (result.value != null) {
            System.out.println("entrust_no: " + (result.value.entrust_no!=null ? result.value.entrust_no : "<null>"));
            System.out.println("order_id: " + result.value.order_id);
        } else {
            System.out.println("msg: " + (result.msg));
        }
    }

    void testCancelOrder() {
        CallResult<OrderID> placeResult = tapi.placeOrder(account_id, "399001.SZ", 1.0, 100, EntrustAction.Buy, 0 );
        if (placeResult.value != null) {
            System.out.println("entrust_no: " + (placeResult.value!=null ? placeResult.value : "<null>"));
        } else {
            System.out.println("msg: " + (placeResult.msg));
        }
        CallResult<Boolean> result = tapi.cancelOrder(account_id, "399001.SZ", placeResult.value.entrust_no);
        if (result.value != null)
            System.out.println("result: " + (result.value!=null ? result.value : "<null>"));
        else
            System.out.println("msg: " + (result.msg));
    }

    public static void main(String[] args) throws Exception {

        new TradeApiDemo().test();
    }
}
