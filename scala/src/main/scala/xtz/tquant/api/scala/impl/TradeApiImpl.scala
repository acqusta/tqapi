package xtz.tquant.api.scala.impl

import xtz.tquant.api.java.JsonRpc
import xtz.tquant.api.scala.TradeApi

import scala.collection.mutable

class TradeApiImpl (client : JsonRpc.JsonRpcClient) extends TradeApi {

    import TradeApi._

    var callback : Callback = _

    def onConnected() = {

    }

    def onDisconnected() = {

    }

    override
    def queryAccountStatus  () : (Seq[AccountInfo], String) = {

        val params = Map[String, Any]()

        val r = client.call("tapi.account_status", params, 10000)
        JsonHelper.extractResult[Seq[AccountInfo]](r)
    }


    override
    def queryBalance(account_id: String) : (Balance, String) = {

        val params = Map( "account_id" -> account_id)

        val r = client.call("tapi.query_balance", params, 10000)

        JsonHelper.extractResult[Balance](r)
    }

    override
    def queryOrders(account_id: String) : (Seq[Order], String) = {

        val params = Map( "account_id" -> account_id)

        val r = client.call("tapi.query_orders", params, 10000)

        JsonHelper.extractResult[Seq[Order]](r)
    }

    override
    def queryTrades(account_id : String) : (Seq[Trade], String) = {

        val params = Map( "account_id" -> account_id)

        val r = client.call("tapi.query_trades", params, 10000)

        JsonHelper.extractResult[Seq[Trade]](r)
    }

    override
    def queryPosition(account_id : String) : (Seq[Position], String) = {

        val params = Map( "account_id" -> account_id)

        val r = client.call("tapi.query_positions", params, 10000)

        JsonHelper.extractResult[Seq[Position]](r)
    }

    override
    def placeOrder(account_id: String, code: String, price : Double, size: Long,
                   action: String,
                   order_id: Int) : (OrderID, String) = {

        var params = Map(
            "account_id" -> account_id,
            "code"       -> code,
            "price"      -> price,
            "size"       -> size,
            "action"     -> action)

        if (order_id != 0)
            params = params ++ Map("order_id" -> order_id)

        val r = client.call("tapi.place_order", params, 10000)

        JsonHelper.extractResult[OrderID](r)
    }

    override
    def cancelOrder(account_id: String, code: String, entrust_no: String, order_id: Int) : (Boolean, String) = {

        val params = mutable.HashMap[String, Any] (
            "account_id" -> account_id,
            "code"       -> code
        )
        if (entrust_no != null && entrust_no.nonEmpty)
            params += "entrust_no" -> entrust_no

        if (order_id != 0)
            params += "order_id" -> order_id


        val r = client.call("tapi.cancel_order", params.toMap, 10000)

        JsonHelper.extractResult[Boolean](r, false)
    }

    override
    def query(account_id: String, command: String, params: String): (String, String) = {

        val rpc_params = mutable.HashMap[String, Any] (
            "account_id" -> account_id,
            "command"    -> command
        )
        if (params != null && params.nonEmpty)
            rpc_params += "params" -> params

        val r = client.call("tapi.common_query", rpc_params.toMap, 10000)

        JsonHelper.extractResult[String](r, "")
    }

    override
    def setCallback(callback: _root_.xtz.tquant.api.scala.TradeApi.Callback): Unit = {
        this.callback = callback
    }

    def onNotification( event : String, value : Any) : Unit = {

        val cb = callback
        if (cb == null) return

        try {
            event match {
                case "tapi.order_status_ind"    => cb.onOrderStatus  (JsonHelper.convert[Order](value))
                case "tapi.order_trade_ind"     => cb.onOrderTrade   (JsonHelper.convert[Trade](value))
                case "tapi.account_status_ind"  => cb.onAccountStatus(JsonHelper.convert[AccountInfo](value))
                case _ =>
            }
        } catch {
            case t : Throwable => println(t.getMessage)
        }
    }
}
