package test

//import java.util.List
//
import xtz.tquant.api.scala.TQuantApi

object TradeApiDemo extends App{

    val api = new TQuantApi("tcp://127.0.0.1:10001")

    val tapi = api.tradeApi

    val account_id = "glsc"

    def testQueryAccountStatus() {
        val (accounts, msg) = tapi.queryAccountStatus()

        if ( accounts !=null) {
            for ( act <- accounts) {
                println(act.account_id + "," + act.broker + "," + act.account + "," + act.status + "," + act.msg)
            }
        } else {
            System.out.println("queryAccount failed: " + msg)
        }
    }

    def testQueryPosition() : Unit = {
        val (positions, msg) = tapi.queryPosition(account_id)

        if ( positions !=null) {
            for (pos <- positions) {
                println("Position " + pos.account_id + "," + pos.code + "," + pos.name + "," + pos.side + "," + pos.current_size + "," + pos.cost_price)
            }
        } else {
            println("queryPosition failed: " + msg)
        }
    }

    def testQueryOrder() : Unit = {
        val (orders, msg) = tapi.queryOrders(account_id)

        if ( orders != null) {
            for ( ord <- orders) {
                println("Order " + ord.account_id + "," + ord.code + "," + ord.name + ","
                        + ord.entrust_action + "," + ord.entrust_price + "," + ord.entrust_size + ","
                        + ord.fill_price + "," + ord.fill_size + "," + ord.status)
            }
        } else {
            println("queryOrders failed: " + msg)
        }
    }

    def testQueryTrade() {
        val (trades, msg) = tapi.queryTrades(account_id)

        if ( trades != null) {
            for (trade <- trades) {
                println("Trade " + trade.account_id + "," + trade.code + "," + trade.name + ","
                        + trade.entrust_no + "," + trade.fill_no + "," + trade.entrust_action + "," +
                        + trade.fill_price + "," + trade.fill_size)
            }
        } else {
            println("queryTrades failed: " + msg)
        }
    }

    def testPlaceOrder() {
        val (entrust_no, msg) = tapi.placeOrder(account_id, "000001.SH", 1.0, 100, "Buy" )
        println("entrust_no: " + ( if (entrust_no!=null) entrust_no else "<null>") )
        println("msg: " + msg)
    }

    def testCancelOrder() {
        val order_id = {
            val (order_id, msg)  = tapi.placeOrder(account_id, "399001.SZ", 1.0, 100, "Buy" )
            println("order_id: " + ( if (order_id!=null) order_id else "<null>") )
            println("msg: " + msg)

            order_id
        }

        if (order_id != null) {
            val (cancel_result, msg) = tapi.cancelOrder(account_id, "399001.SZ",
                entrust_no = order_id.entrust_no,
                order_id = order_id.order_id)

            println("result: " + cancel_result)
            println("msg: " + msg)
        }
    }

    def test() {
        testQueryAccountStatus()
        testQueryPosition()
        testQueryOrder()
        testQueryTrade()
//        testPlaceOrder()
//        testCancelOrder()
    }

    test()

}
