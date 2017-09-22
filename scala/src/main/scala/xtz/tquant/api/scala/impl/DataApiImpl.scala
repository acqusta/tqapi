package xtz.tquant.api.scala.impl

import com.fasterxml.jackson.annotation.JsonIgnoreProperties
import xtz.tquant.api.java.JsonRpc
import xtz.tquant.api.scala.DataApi
import xtz.tquant.api.scala.DataApi.{Bar, MarketQuote}

import scala.collection.mutable
import scala.concurrent.Future
import scala.concurrent.ExecutionContext.Implicits.global

object DataApiImpl {

    // Convert Tick and Bar to Shadow object then convert again. So MarketQuote and Bar won't declare fields as var.

    class MarketQuote_Shadow {
        var code        : String  = _    // 证券代码
        var date        : Int     = _    // 行情日期
        var time        : Int     = _    // 行情时间
        var trading_day : Int     = _    // 交易日
        var open        : Double  = _    // 开盘价
        var high        : Double  = _    // 最高价
        var low         : Double  = _    // 最低价
        var close       : Double  = _    // 收盘价
        var last        : Double  = _    // 最新价
        var high_limit  : Double  = _    // 涨停价
        var low_limit   : Double  = _    // 跌停价
        var pre_close   : Double  = _    // 昨收价
        var volume      : Long    = _    // 成交量
        var turnover    : Double  = _    // 成交金额
        var ask1        : Double  = _    // 卖一价
        var ask2        : Double  = _
        var ask3        : Double  = _
        var ask4        : Double  = _
        var ask5        : Double  = _
        var ask6        : Double  = _
        var ask7        : Double  = _
        var ask8        : Double  = _
        var ask9        : Double  = _
        var ask10       : Double  = _
        var bid1        : Double  = _    // 买一价
        var bid2        : Double  = _
        var bid3        : Double  = _
        var bid4        : Double  = _
        var bid5        : Double  = _
        var bid6        : Double  = _
        var bid7        : Double  = _
        var bid8        : Double  = _
        var bid9        : Double  = _
        var bid10       : Double  = _
        var ask_vol1    : Long    = _   // 卖一量
        var ask_vol2    : Long    = _
        var ask_vol3    : Long    = _
        var ask_vol4    : Long    = _
        var ask_vol5    : Long    = _
        var ask_vol6    : Long    = _
        var ask_vol7    : Long    = _
        var ask_vol8    : Long    = _
        var ask_vol9    : Long    = _
        var ask_vol10   : Long    = _
        var bid_vol1    : Long    = _   // 买一量
        var bid_vol2    : Long    = _
        var bid_vol3    : Long    = _
        var bid_vol4    : Long    = _
        var bid_vol5    : Long    = _
        var bid_vol6    : Long    = _
        var bid_vol7    : Long    = _
        var bid_vol8    : Long    = _
        var bid_vol9    : Long    = _
        var bid_vol10   : Long    = _
        var settle      : Double  = _   // 结算价
        var pre_settle  : Double  = _   // 昨结算价
        var oi          : Long    = _   // OpenInterest       未平仓量
        var pre_oi      : Long    = _   // Pre-OpenInterest   昨未平仓量
    }

    def _convertTicks(data: Map[String, List[_]]) : Seq[MarketQuote] = {
        try {
            val size = data.getOrElse("code", null).size
            val ticks = new Array[MarketQuote_Shadow](size)
            for (i <- 0 until  size) ticks(i) = new MarketQuote_Shadow()

            for ( (k,v) <- data) {
                k match {
                    case "code"         => val iter = v.iterator; ticks.foreach( _.code         = iter.next().asInstanceOf[String])
                    case  "date"        => val iter = v.iterator; ticks.foreach( _.date         = iter.next().asInstanceOf[Int   ])
                    case  "time"        => val iter = v.iterator; ticks.foreach( _.time         = iter.next().asInstanceOf[Int   ])
                    case  "trading_day" => val iter = v.iterator; ticks.foreach( _.trading_day  = iter.next().asInstanceOf[Int   ])
                    case  "open"        => val iter = v.iterator; ticks.foreach( _.open         = iter.next().asInstanceOf[Double])
                    case  "high"        => val iter = v.iterator; ticks.foreach( _.high         = iter.next().asInstanceOf[Double])
                    case  "low"         => val iter = v.iterator; ticks.foreach( _.low          = iter.next().asInstanceOf[Double])
                    case  "close"       => val iter = v.iterator; ticks.foreach( _.close        = iter.next().asInstanceOf[Double])
                    case  "last"        => val iter = v.iterator; ticks.foreach( _.last         = iter.next().asInstanceOf[Double])
                    case  "high_limit"  => val iter = v.iterator; ticks.foreach( _.high_limit   = iter.next().asInstanceOf[Double])
                    case  "low_limit"   => val iter = v.iterator; ticks.foreach( _.low_limit    = iter.next().asInstanceOf[Double])
                    case  "pre_close"   => val iter = v.iterator; ticks.foreach( _.pre_close    = iter.next().asInstanceOf[Double])
                    case  "volume"      => val iter = v.iterator; ticks.foreach( _.volume       = iter.next() match { case v:Int => v ; case v:Long => v} )
                    case  "turnover"    => val iter = v.iterator; ticks.foreach( _.turnover     = iter.next().asInstanceOf[Double])
                    case  "ask1"        => val iter = v.iterator; ticks.foreach( _.ask1         = iter.next().asInstanceOf[Double])
                    case  "ask2"        => val iter = v.iterator; ticks.foreach( _.ask2         = iter.next().asInstanceOf[Double])
                    case  "ask3"        => val iter = v.iterator; ticks.foreach( _.ask3         = iter.next().asInstanceOf[Double])
                    case  "ask4"        => val iter = v.iterator; ticks.foreach( _.ask4         = iter.next().asInstanceOf[Double])
                    case  "ask5"        => val iter = v.iterator; ticks.foreach( _.ask5         = iter.next().asInstanceOf[Double])
                    case  "ask6"        => val iter = v.iterator; ticks.foreach( _.ask6         = iter.next().asInstanceOf[Double])
                    case  "ask7"        => val iter = v.iterator; ticks.foreach( _.ask7         = iter.next().asInstanceOf[Double])
                    case  "ask8"        => val iter = v.iterator; ticks.foreach( _.ask8         = iter.next().asInstanceOf[Double])
                    case  "ask9"        => val iter = v.iterator; ticks.foreach( _.ask9         = iter.next().asInstanceOf[Double])
                    case  "ask10"       => val iter = v.iterator; ticks.foreach( _.ask10        = iter.next().asInstanceOf[Double])
                    case  "bid1"        => val iter = v.iterator; ticks.foreach( _.bid1         = iter.next().asInstanceOf[Double])
                    case  "bid2"        => val iter = v.iterator; ticks.foreach( _.bid2         = iter.next().asInstanceOf[Double])
                    case  "bid3"        => val iter = v.iterator; ticks.foreach( _.bid3         = iter.next().asInstanceOf[Double])
                    case  "bid4"        => val iter = v.iterator; ticks.foreach( _.bid4         = iter.next().asInstanceOf[Double])
                    case  "bid5"        => val iter = v.iterator; ticks.foreach( _.bid5         = iter.next().asInstanceOf[Double])
                    case  "bid6"        => val iter = v.iterator; ticks.foreach( _.bid6         = iter.next().asInstanceOf[Double])
                    case  "bid7"        => val iter = v.iterator; ticks.foreach( _.bid7         = iter.next().asInstanceOf[Double])
                    case  "bid8"        => val iter = v.iterator; ticks.foreach( _.bid8         = iter.next().asInstanceOf[Double])
                    case  "bid9"        => val iter = v.iterator; ticks.foreach( _.bid9         = iter.next().asInstanceOf[Double])
                    case  "bid10"       => val iter = v.iterator; ticks.foreach( _.bid10        = iter.next().asInstanceOf[Double])
                    case  "ask_vol1"    => val iter = v.iterator; ticks.foreach( _.ask_vol1     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol2"    => val iter = v.iterator; ticks.foreach( _.ask_vol2     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol3"    => val iter = v.iterator; ticks.foreach( _.ask_vol3     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol4"    => val iter = v.iterator; ticks.foreach( _.ask_vol4     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol5"    => val iter = v.iterator; ticks.foreach( _.ask_vol5     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol6"    => val iter = v.iterator; ticks.foreach( _.ask_vol6     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol7"    => val iter = v.iterator; ticks.foreach( _.ask_vol7     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol8"    => val iter = v.iterator; ticks.foreach( _.ask_vol8     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol9"    => val iter = v.iterator; ticks.foreach( _.ask_vol9     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "ask_vol10"   => val iter = v.iterator; ticks.foreach( _.ask_vol10    = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol1"    => val iter = v.iterator; ticks.foreach( _.bid_vol1     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol2"    => val iter = v.iterator; ticks.foreach( _.bid_vol2     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol3"    => val iter = v.iterator; ticks.foreach( _.bid_vol3     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol4"    => val iter = v.iterator; ticks.foreach( _.bid_vol4     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol5"    => val iter = v.iterator; ticks.foreach( _.bid_vol5     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol6"    => val iter = v.iterator; ticks.foreach( _.bid_vol6     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol7"    => val iter = v.iterator; ticks.foreach( _.bid_vol7     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol8"    => val iter = v.iterator; ticks.foreach( _.bid_vol8     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol9"    => val iter = v.iterator; ticks.foreach( _.bid_vol9     = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "bid_vol10"   => val iter = v.iterator; ticks.foreach( _.bid_vol10    = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "settle"      => val iter = v.iterator; ticks.foreach( _.settle       = iter.next().asInstanceOf[Double])
                    case  "pre_settle"  => val iter = v.iterator; ticks.foreach( _.pre_settle   = iter.next().asInstanceOf[Double])
                    case  "oi"          => val iter = v.iterator; ticks.foreach( _.oi           = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case  "pre_oi"      => val iter = v.iterator; ticks.foreach( _.pre_oi       = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case _ =>
                }
            }

            ticks map { x => MarketQuote (
                code        = x.code       ,
                date        = x.date       ,
                time        = x.time       ,
                trading_day = x.trading_day,
                open        = x.open       ,
                high        = x.high       ,
                low         = x.low        ,
                close       = x.close      ,
                last        = x.last       ,
                high_limit  = x.high_limit ,
                low_limit   = x.low_limit  ,
                pre_close   = x.pre_close  ,
                volume      = x.volume     ,
                turnover    = x.turnover   ,
                ask1        = x.ask1       ,
                ask2        = x.ask2       ,
                ask3        = x.ask3       ,
                ask4        = x.ask4       ,
                ask5        = x.ask5       ,
                ask6        = x.ask6       ,
                ask7        = x.ask7       ,
                ask8        = x.ask8       ,
                ask9        = x.ask9       ,
                ask10       = x.ask10      ,
                bid1        = x.bid1       ,
                bid2        = x.bid2       ,
                bid3        = x.bid3       ,
                bid4        = x.bid4       ,
                bid5        = x.bid5       ,
                bid6        = x.bid6       ,
                bid7        = x.bid7       ,
                bid8        = x.bid8       ,
                bid9        = x.bid9       ,
                bid10       = x.bid10      ,
                ask_vol1    = x.ask_vol1   ,
                ask_vol2    = x.ask_vol2   ,
                ask_vol3    = x.ask_vol3   ,
                ask_vol4    = x.ask_vol4   ,
                ask_vol5    = x.ask_vol5   ,
                ask_vol6    = x.ask_vol6   ,
                ask_vol7    = x.ask_vol7   ,
                ask_vol8    = x.ask_vol8   ,
                ask_vol9    = x.ask_vol9   ,
                ask_vol10   = x.ask_vol10  ,
                bid_vol1    = x.bid_vol1   ,
                bid_vol2    = x.bid_vol2   ,
                bid_vol3    = x.bid_vol3   ,
                bid_vol4    = x.bid_vol4   ,
                bid_vol5    = x.bid_vol5   ,
                bid_vol6    = x.bid_vol6   ,
                bid_vol7    = x.bid_vol7   ,
                bid_vol8    = x.bid_vol8   ,
                bid_vol9    = x.bid_vol9   ,
                bid_vol10   = x.bid_vol10  ,
                settle      = x.settle     ,
                pre_settle  = x.pre_settle ,
                oi          = x.oi         ,
                pre_oi      = x.pre_oi     ) }

        } catch {
            case t: Throwable =>
                t.printStackTrace()
                null
        }
    }

    class Bar_Shadow {
        var code            : String  = _  // 证券代码
        var date            : Int     = _  // 行情日期
        var time            : Int     = _  // 行情时间
        var trading_day     : Int     = _  // 交易日
        var open            : Double  = _  // bar的开盘价
        var high            : Double  = _  // bar的最高价
        var low             : Double  = _  // bar的最低价
        var close           : Double  = _  // bar的收盘价
        var volume          : Long    = _  // bar的成交量
        var turnover        : Double  = _  // bar的成交金额
        var oi              : Long    = _  // 持仓量，日线有效
    }

    def _convertBars(data: Map[String, List[_]]) : Seq[Bar] = {
        try {
            val size = data.getOrElse("code", null).size
            val bars = new Array[Bar_Shadow](size)
            for (i <- 0 until  size) bars(i) = new Bar_Shadow

            for ( (k,v) <- data) {
                k match {
                    case  "code"        => val iter = v.iterator; bars.foreach( _.code         = iter.next().asInstanceOf[String])
                    case  "date"        => val iter = v.iterator; bars.foreach( _.date         = iter.next().asInstanceOf[Int   ])
                    case  "time"        => val iter = v.iterator; bars.foreach( _.time         = iter.next().asInstanceOf[Int   ])
                    case  "trading_day" => val iter = v.iterator; bars.foreach( _.trading_day  = iter.next().asInstanceOf[Int   ])
                    case  "open"        => val iter = v.iterator; bars.foreach( _.open         = iter.next().asInstanceOf[Double])
                    case  "high"        => val iter = v.iterator; bars.foreach( _.high         = iter.next().asInstanceOf[Double])
                    case  "low"         => val iter = v.iterator; bars.foreach( _.low          = iter.next().asInstanceOf[Double])
                    case  "close"       => val iter = v.iterator; bars.foreach( _.close        = iter.next().asInstanceOf[Double])
                    case  "volume"      => val iter = v.iterator; bars.foreach( _.volume       = iter.next() match { case v:Int => v ; case v:Long => v} )
                    case  "turnover"    => val iter = v.iterator; bars.foreach( _.turnover     = iter.next().asInstanceOf[Double])
                    case  "oi"          => val iter = v.iterator; bars.foreach( _.oi           = iter.next() match { case v:Int => v ; case v:Long => v} ) //.asInstanceOf[Long  ])
                    case _ =>
                }
            }

            bars map { x => Bar(
                code         = x.code        ,
                date         = x.date        ,
                time         = x.time        ,
                trading_day  = x.trading_day ,
                open         = x.open        ,
                high         = x.high        ,
                low          = x.low         ,
                close        = x.close       ,
                volume       = x.volume      ,
                turnover     = x.turnover    ,
                oi           = x.oi          )
            }
        } catch {
            case t: Throwable =>
                t.printStackTrace()
                null
        }
    }

    case class BarInd( cycle : String, bar : Bar)

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class HeartBeat (
        sub_hash: Long = 0L
    )

    @JsonIgnoreProperties(ignoreUnknown = true)
    case class SubscribeResult (
        sub_codes  : Seq[String],
        sub_hash   : Long
    )
}

class DataApiImpl(client: JsonRpc.JsonRpcClient) extends DataApi {

    import DataApi._
    import DataApiImpl._

    var callback : Callback = _

    val sub_codes = mutable.HashSet[String]()
    var sub_hash = 0L

    def onConnected() = {
    }

    def onDisconnected() = {

    }

    def _convert[T: Manifest](data: Map[String, List[_]], errValue : T = null): T = {
        try {
            val size = data.getOrElse("code", null).size
            val bars = new Array[mutable.HashMap[String, Any]](size)
            for (i <- 0 until  size) bars(i) = new mutable.HashMap[String, Any]()
            for ( key <- data.keys) {
                val values = data(key)//.toArray
                var i = 0
                values.foreach( x=> { bars(i) += key -> x; i+=1} )
            }

            JsonHelper.convert[T](bars)
        }catch {
            case t: Throwable =>
                t.printStackTrace()
                errValue
        }
    }

    override
    def tick(code: String, trading_day : Int) : (Seq[MarketQuote], String) = {
        var params = Map[String, Any]( "code" -> code )
        if (trading_day > 0 )
            params += "trading_day" -> trading_day

        //        params += "_format" -> "rowset"

        val r = client.call("dapi.tst", params, 6000)

        val (data, msg) = JsonHelper.extractResultMapList(r)
        if (data == null)
            return (null, msg)
        val ticks = _convertTicks(data)

        // Test Code
        //        val ticks = JsonHelper.convert[Seq[MarketQuote]](r.result)
        //        val ticks = _convertTicks(r.result.asInstanceOf[List[Map[String,_]]])

        if (ticks != null)
            (ticks, "")
        else
            (null, "unknown error")
    }

    override
    def bar (code : String, cycle : String, trading_day: Int, price_adj: String) : (Seq[Bar], String) = {
        var params = Map[String, Any]( "code" -> code )
        if (cycle != null && cycle.nonEmpty )
            params += "cycle" -> cycle

        if (trading_day > 0 )
            params += "trading_day" -> trading_day

        if (price_adj!=null && price_adj.nonEmpty)
            params ++ "price_adj" -> price_adj

        val r = client.call("dapi.tsi", params, 6000)

        val (data, msg) = JsonHelper.extractResult[Map[String, List[_]]](r)
        if (data == null)
            return (null, msg)

        val bars = _convertBars(data)
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

        if (codes != null && codes.nonEmpty)
            this.sub_codes ++= codes

        var params = Map[String, Any]()

        if (codes != null)
            params += "codes" -> codes.mkString(",")
        else
            params += "codes" -> ""

        val r = client.call("dapi.tsq_sub", params, 6000)
        val (result, msg) = JsonHelper.extractResult[SubscribeResult](r)
        if (result != null) {
            if (codes != null && codes.nonEmpty)
                this.sub_hash = result.sub_hash
            (result.sub_codes, msg)
        } else {
            (null, msg)
        }
    }

    override
    def unsubscribe(codes: Seq[String]) : (Seq[String], String) = {

        if (codes != null && codes.nonEmpty)
            this.sub_codes --= codes

        var params = Map[String, Any]()

        if (codes != null)
            params += "codes" -> codes.mkString(",")
        else
            params += "codes" -> ""

        val r = client.call("dapi.tsq_unsub", params, 6000)
        val (result, msg) = JsonHelper.extractResult[SubscribeResult](r)
        if (result != null) {
            this.sub_hash = result.sub_hash
            (result.sub_codes, msg)
        } else {
            (null, msg)
        }
    }

    override
    def setCallback(callback : Callback) {
        this.callback = callback
    }

    def onHeartBeat(value : Any): Unit = {

        if (this.sub_codes.isEmpty) return

        val r = JsonHelper.convert[HeartBeat](value)

        if (r.sub_hash == this.sub_hash && this.sub_hash != 0)
            return

        var params = Map[String, Any]()
        params += "codes" -> sub_codes.mkString(",")

        Future {
            //println ("subscribe again: " + sub_codes)
            val r = client.call("dapi.tsq_sub", params, 2000)
            val (result, msg) = JsonHelper.extractResult[SubscribeResult](r)
            if (result != null) {
                //println("sbuscribe again: ", sub_hash)
                this.sub_hash = result.sub_hash
            } else {
                //println("subscribe error:", msg)
            }
        }
    }

    def onNotification(event: String, value : Any) : Unit = {

        try {
            event match {
                case ".sys.heartbeat" =>    onHeartBeat(value)

                case "dapi.quote" =>
                    if (this.callback != null) {
                        val q = JsonHelper.convert[MarketQuote](value)
                        if (q != null)
                            this.callback.onMarketQuote(q)
                    }
                case "dapi.bar" =>
                    if (this.callback != null) {
                        val ind = JsonHelper.convert[BarInd](value)
                        if (ind != null)
                            this.callback.onBar(ind.cycle, ind.bar)
                    }
                case _ =>
            }
        } catch {
            case t: Throwable => t.printStackTrace()
        }
    }
}
