package xtz.tquant.api.scala

import java.lang.reflect.{ParameterizedType, Type}

import com.fasterxml.jackson.annotation.JsonInclude.Include
import com.fasterxml.jackson.databind.{DeserializationFeature, JsonNode, ObjectMapper}
import com.fasterxml.jackson.core.`type`.TypeReference
import com.fasterxml.jackson.module.scala.DefaultScalaModule
import xtz.tquant.api.java.JsonRpc

import scala.collection.mutable

/**
  * 初始化TQuantApi
  * @param addr 服务器地址，取值通常为 tcp://127.0.0.1:10001
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
    def trade_api : TradeApi = _trade_api

    /**
     *  取交易接口
     *
     * @return
     */
    def data_api : DataApi = _data_api

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
        override def getType : Type = typeFromManifest(manifest[T])
    }

    private[this] def typeFromManifest(m: Manifest[_]): Type = {
        if (m.typeArguments.isEmpty) {
            m.runtimeClass
        } else{
            new ParameterizedType {
                override def getRawType : Type = m.runtimeClass
                override def getActualTypeArguments : Array[Type] = m.typeArguments.map(typeFromManifest).toArray
                override def getOwnerType : Type = null
            }
        }
    }


    def convert[T: Manifest] (obj: Any): T = {
        mapper.convertValue(obj, typeReference[T])
    }


    def erroToString(error: JsonRpc.JsonRpcError) : String = {
        if (error != null) {
            if (error.message != null )
                error.code.toString + "," + error.message
            else
                error.code.toString + ","
        } else {
            ","
        }
    }

    def extractResult[T: Manifest](cr: JsonRpc.JsonRpcCallResult,  errValue: T = null): (T, String) = {
        try {
            val value =
                if (cr.result != null ) {
                    JsonHelper.convert[T](cr.result)
                }
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

    def extractResultMapList(cr: JsonRpc.JsonRpcCallResult): (Map[String, List[_]], String) = {
        try {
            val value =
                if (cr.result != null )
                    cr.result.asInstanceOf[Map[String, List[_]]]
                else
                    null

            val message =
                if ( cr.error != null )
                    erroToString(cr.error)
                else
                    null

            (value, message)


        } catch {
            case e: Throwable =>
                e.printStackTrace()
                (null, e.getMessage)
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

    def onNotification( event : String, value : Any) : Unit = {

    }
}


class DataApiImpl(client: JsonRpc.JsonRpcClient) extends DataApi {

    import DataApi._

    var callback : Callback = _

    def _convert[T: Manifest](data: Map[String, List[_]], errValue : T = null): T = {
        try {
            val size = data.getOrElse("code", null).size
            val bars = new Array[mutable.HashMap[String, Any]](size)
            for (i <- 0 until  size) bars(i) = new mutable.HashMap[String, Any]()
            for ( key <- data.keys) {
                val values = data(key)
                var i = 0
                values.foreach( x=> { bars(i) += key -> x; i+=1} )
            }

            return errValue //JsonHelper.convert[T](bars)
        }catch {
            case t: Throwable =>
                t.printStackTrace()
                return errValue
        }
    }

//    def _convertTicks(data: Map[String, List[_]]) : Seq[MarketQuote] = {
//        try {
//            val size = data.getOrElse("code", null).size
//            val ticks = new Array[MarketQuote](size)
//            for (i <- 0 until  size) ticks(i) = new MarketQuote()
//
//            for ( (k,v) <- data) {
//                k match {
//                    case "code"         => val iter = v.iterator; ticks.foreach( _.code         = iter.next().asInstanceOf[String])
//                    case  "date"        => val iter = v.iterator; ticks.foreach( _.date         = iter.next().asInstanceOf[Int   ])
//                    case  "time"        => val iter = v.iterator; ticks.foreach( _.time         = iter.next().asInstanceOf[Int   ])
//                    case  "trading_day" => val iter = v.iterator; ticks.foreach( _.trading_day  = iter.next().asInstanceOf[Int   ])
//                    case  "open"        => val iter = v.iterator; ticks.foreach( _.open         = iter.next().asInstanceOf[Double])
//                    case  "high"        => val iter = v.iterator; ticks.foreach( _.high         = iter.next().asInstanceOf[Double])
//                    case  "low"         => val iter = v.iterator; ticks.foreach( _.low          = iter.next().asInstanceOf[Double])
//                    case  "close"       => val iter = v.iterator; ticks.foreach( _.close        = iter.next().asInstanceOf[Double])
//                    case  "last"        => val iter = v.iterator; ticks.foreach( _.last         = iter.next().asInstanceOf[Double])
//                    case  "high_limit"  => val iter = v.iterator; ticks.foreach( _.high_limit   = iter.next().asInstanceOf[Double])
//                    case  "low_limit"   => val iter = v.iterator; ticks.foreach( _.low_limit    = iter.next().asInstanceOf[Double])
//                    case  "pre_close"   => val iter = v.iterator; ticks.foreach( _.pre_close    = iter.next().asInstanceOf[Double])
//                    case  "volume"      => val iter = v.iterator; ticks.foreach( _.volume       = iter.next() match { case v:Int => v ; case v:Long => v} )
//                    case  "turnover"    => val iter = v.iterator; ticks.foreach( _.turnover     = iter.next().asInstanceOf[Double])
//                    case  "ask1"        => val iter = v.iterator; ticks.foreach( _.ask1         = iter.next().asInstanceOf[Double])
//                    case  "ask2"        => val iter = v.iterator; ticks.foreach( _.ask2         = iter.next().asInstanceOf[Double])
//                    case  "ask3"        => val iter = v.iterator; ticks.foreach( _.ask3         = iter.next().asInstanceOf[Double])
//                    case  "ask4"        => val iter = v.iterator; ticks.foreach( _.ask4         = iter.next().asInstanceOf[Double])
//                    case  "ask5"        => val iter = v.iterator; ticks.foreach( _.ask5         = iter.next().asInstanceOf[Double])
//                    case  "ask6"        => val iter = v.iterator; ticks.foreach( _.ask6         = iter.next().asInstanceOf[Double])
//                    case  "ask7"        => val iter = v.iterator; ticks.foreach( _.ask7         = iter.next().asInstanceOf[Double])
//                    case  "ask8"        => val iter = v.iterator; ticks.foreach( _.ask8         = iter.next().asInstanceOf[Double])
//                    case  "ask9"        => val iter = v.iterator; ticks.foreach( _.ask9         = iter.next().asInstanceOf[Double])
//                    case  "ask10"       => val iter = v.iterator; ticks.foreach( _.ask10        = iter.next().asInstanceOf[Double])
//                    case  "bid1"        => val iter = v.iterator; ticks.foreach( _.bid1         = iter.next().asInstanceOf[Double])
//                    case  "bid2"        => val iter = v.iterator; ticks.foreach( _.bid2         = iter.next().asInstanceOf[Double])
//                    case  "bid3"        => val iter = v.iterator; ticks.foreach( _.bid3         = iter.next().asInstanceOf[Double])
//                    case  "bid4"        => val iter = v.iterator; ticks.foreach( _.bid4         = iter.next().asInstanceOf[Double])
//                    case  "bid5"        => val iter = v.iterator; ticks.foreach( _.bid5         = iter.next().asInstanceOf[Double])
//                    case  "bid6"        => val iter = v.iterator; ticks.foreach( _.bid6         = iter.next().asInstanceOf[Double])
//                    case  "bid7"        => val iter = v.iterator; ticks.foreach( _.bid7         = iter.next().asInstanceOf[Double])
//                    case  "bid8"        => val iter = v.iterator; ticks.foreach( _.bid8         = iter.next().asInstanceOf[Double])
//                    case  "bid9"        => val iter = v.iterator; ticks.foreach( _.bid9         = iter.next().asInstanceOf[Double])
//                    case  "bid10"       => val iter = v.iterator; ticks.foreach( _.bid10        = iter.next().asInstanceOf[Double])
//                    case  "ask_vol1"    => val iter = v.iterator; ticks.foreach( _.ask_vol1     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol2"    => val iter = v.iterator; ticks.foreach( _.ask_vol2     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol3"    => val iter = v.iterator; ticks.foreach( _.ask_vol3     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol4"    => val iter = v.iterator; ticks.foreach( _.ask_vol4     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol5"    => val iter = v.iterator; ticks.foreach( _.ask_vol5     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol6"    => val iter = v.iterator; ticks.foreach( _.ask_vol6     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol7"    => val iter = v.iterator; ticks.foreach( _.ask_vol7     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol8"    => val iter = v.iterator; ticks.foreach( _.ask_vol8     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol9"    => val iter = v.iterator; ticks.foreach( _.ask_vol9     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "ask_vol10"   => val iter = v.iterator; ticks.foreach( _.ask_vol10    = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol1"    => val iter = v.iterator; ticks.foreach( _.bid_vol1     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol2"    => val iter = v.iterator; ticks.foreach( _.bid_vol2     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol3"    => val iter = v.iterator; ticks.foreach( _.bid_vol3     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol4"    => val iter = v.iterator; ticks.foreach( _.bid_vol4     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol5"    => val iter = v.iterator; ticks.foreach( _.bid_vol5     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol6"    => val iter = v.iterator; ticks.foreach( _.bid_vol6     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol7"    => val iter = v.iterator; ticks.foreach( _.bid_vol7     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol8"    => val iter = v.iterator; ticks.foreach( _.bid_vol8     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol9"    => val iter = v.iterator; ticks.foreach( _.bid_vol9     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "bid_vol10"   => val iter = v.iterator; ticks.foreach( _.bid_vol10    = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "settle"      => val iter = v.iterator; ticks.foreach( _.settle       = iter.next().asInstanceOf[Double])
//                    case  "pre_settle"  => val iter = v.iterator; ticks.foreach( _.pre_settle   = iter.next().asInstanceOf[Double])
//                    case  "oi"          => val iter = v.iterator; ticks.foreach( _.oi           = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case  "pre_oi"      => val iter = v.iterator; ticks.foreach( _.pre_oi       = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
//                    case _ =>
//                }
//            }
//
//            return  ticks
//        } catch {
//            case t: Throwable =>
//                t.printStackTrace()
//                return null
//        }
//    }

    override
    def tick(code: String, trading_day : Int) : (Seq[MarketQuote], String) = {
        var params = Map[String, Any]( "code" -> code )
        if (trading_day > 0 )
            params += "tradingday" -> trading_day

        val r = client.call("dapi.tst", params, 6000)

        val (data, msg) = JsonHelper.extractResultMapList(r)

        if (data == null)
            return (null, msg)


        val ticks = _convert[Seq[MarketQuote]](data)
        //val ticks = _convertTicks(data)
        if (ticks != null)
            return (ticks, "")
        else
            return (null, "unknown error")
    }

    override
    def bar (code : String, cycle : String, trading_day: Int) : (Seq[Bar], String) = {
        var params = Map[String, Any]( "code" -> code )
        if (cycle != null && cycle.nonEmpty )
            params += "cycle" -> cycle
        if (trading_day > 0 )
            params += "tradingday" -> trading_day

        val r = client.call("dapi.tsi", params, 6000)

        val (data, msg) = JsonHelper.extractResult[Map[String, List[_]]](r)
        if (data == null)
            return (null, msg)

        val bars = _convert[Seq[Bar]](data)
        if (bars != null)
            (bars, "")
        else
            (null, "unknown error")
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
