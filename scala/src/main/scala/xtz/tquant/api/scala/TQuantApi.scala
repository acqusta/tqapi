package xtz.tquant.api.scala

import java.lang.reflect.{ParameterizedType, Type}

import com.fasterxml.jackson.annotation.JsonInclude.Include
import com.fasterxml.jackson.databind.{DeserializationFeature, JavaType, JsonNode, ObjectMapper}
import com.fasterxml.jackson.core.`type`.TypeReference
import com.fasterxml.jackson.module.scala.DefaultScalaModule
import xtz.tquant.api.scala.DataApi
import xtz.tquant.api.scala.TradeApi

import xtz.tquant.api.java.JsonRpc

/**
  * 初始化 TQuantApi
  * @param addr 服务器地址，取值通常为 tcp://127.0.0.1:10082
  */

class TQuantApi (addr: String) {

    private var _client    : JsonRpc.JsonRpcClient = _
    private var _trade_api : TradeApiImpl = _
    private var _data_api  : DataApiImpl  = _

    init()

    private def init() {
        _client = new JsonRpc.JsonRpcClient()
        _client.connect(addr)
        _trade_api = new TradeApiImpl(_client)
        _data_api  = new DataApiImpl(_client)

        _client.setCallback(
            new JsonRpc.JsonRpcClient.Callback() {
                override
                def onConnected() {
                }

                override
                def onDisconnected() {
                }

                override
                def onNotification(event: String , value : Any) {
                    if (event.startsWith("tapi.")) {
                        _trade_api.onNotification(event, value)
                    } else if (event.startsWith("dapi.")) {
                        _data_api.onNotification(event, value)
                    }
                }
            })
    }

    /**
     * 取数据接口
     *
     * @return
     */
    def trade_api = _trade_api

    /**
     *  取交易接口
     *
     * @return
     */
    def data_api = _data_api

}

object JsonHelper {

    val mapper = new ObjectMapper()
    mapper.setSerializationInclusion(Include.NON_NULL)
    mapper.registerModule(DefaultScalaModule)
    mapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false)


    def serialize(value: Any): String = mapper.writeValueAsString(value)

    def deserialize[T: Manifest](value: String) : T = mapper.readValue(value, typeReference[T])

    def toJson(value: String): JsonNode = mapper.readTree(value)

    private [this] def typeReference[T: Manifest] = new TypeReference[T] {
        override def getType = typeFromManifest(manifest[T])
    }

    private[this] def typeFromManifest(m: Manifest[_]): Type = {
        if (m.typeArguments.isEmpty) {
            m.erasure
        } else{
            new ParameterizedType {
                def getRawType = m.erasure
                def getActualTypeArguments = m.typeArguments.map(typeFromManifest).toArray
                def getOwnerType = null
            }
        }
    }


    def convert[T: Manifest] (obj: Any): T = {
        mapper.convertValue(obj, typeReference[T])
    }


    def erroToString(error: JsonRpc.JsonRpcError) = {
        if (error != null) {
            if (error.message != null )
                error.error.toString() + "," + error.message
            else
                error.error.toString() + ","
        } else {
            ","
        }
    }

    def extractResult[T: Manifest](cr: JsonRpc.JsonRpcCallResult,  errValue: T = null): (T, String) = {
        try {
            val value =
                if (cr.result != null )
                    JsonHelper.convert[T](cr.result)
                else
                    errValue
            val message =
                if ( cr.error != null )
                    erroToString(cr.error)
                else
                    null

            (value, message)
        } catch {
            case e: Throwable =>
                e.printStackTrace()
                (errValue, e.getMessage)
        }

    }
}

class TradeApiImpl (client : JsonRpc.JsonRpcClient) extends TradeApi {

    import TradeApi._

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
    def placeOrder(account_id: String, code: String, price : Double, size: Int,
                   action: String) : (String, String) = {

        val params = Map(
            "account_id" -> account_id,
            "code"       -> code,
            "price"      -> price,
            "size"       -> size,
            "action"     -> action)

        val r = client.call("tapi.place_order", params, 10000)

        JsonHelper.extractResult[String](r)
    }

    override
    def cancelOrder(account_id: String, code: String, entrust_no: String) : (Boolean, String) = {
        val params = Map(
            "account_id" -> account_id,
            "code"       -> code,
            "entrust_no" -> entrust_no )

        val r = client.call("tapi.cancel_order", params, 10000)

        JsonHelper.extractResult[Boolean](r, false)
    }

    def onNotification( event : String, value : Any) = {

    }
}


class DataApiImpl(client: JsonRpc.JsonRpcClient) extends DataApi {

    import DataApi._

    var callback : Callback = _

    override
    def tick(code: String, trading_day : Int) : (Seq[MarketQuote], String) = {
        var params = Map[String, Any]( "code" -> code )
        if (trading_day > 0 )
            params += "tradingday" -> trading_day

        val r = client.call("dapi.tst", params, 6000)

        JsonHelper.extractResult[Seq[MarketQuote]](r)
    }

    override
    def bar (code : String, cycle : String, trading_day: Int) : (Seq[Bar], String) = {
        var params = Map[String, Any]( "code" -> code )
        if (cycle != null && cycle.nonEmpty )
            params += "cycle" -> cycle
        if (trading_day > 0 )
            params += "tradingday" -> trading_day

        val r = client.call("dapi.tsi", params, 6000)

        JsonHelper.extractResult[Seq[Bar]](r)
    }

    override
    def quote (code: String) : (MarketQuote, String) = {

        var params = Map( "code" -> code )

        val r = client.call("dapi.tsq_quote", params, 6000)

        JsonHelper.extractResult[MarketQuote](r)
    }

    override
    def subscribe(codes: Seq[String]) : (Seq[String], String) = {

        var params = Map[String, Any]()

        if (codes != null)
            params += "codes" -> codes.mkString(",")
        else
            params += "codes" -> ""

        val r = client.call("dapi.tsq_sub", params, 6000)
        JsonHelper.extractResult[Seq[String]](r)
    }

    override
    def unsubscribe(codes: Seq[String]) : (Seq[String], String) = {

        var params = Map[String, Any]()

        if (codes != null)
            params += "codes" -> codes.mkString(",")
        else
            params += "codes" -> ""

        val r = client.call("dapi.tsq_unsub", params, 6000)
        JsonHelper.extractResult[Seq[String]](r)
    }

    override
    def setCallback(callback : Callback) {
        this.callback = callback
    }

    def onNotification(event: String, value : Any) : Unit = {

        if (this.callback == null) return

        try {
            if (event.equals("dapi.quote")) {
                val q = JsonHelper.convert[MarketQuote](value)
                if (q != null)
                    this.callback.onMarketQuote(q)
            }
        } catch {
            case t: Throwable => t.printStackTrace()
        }
    }
}
