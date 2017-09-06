package test

import xtz.tquant.api.scala.TQuantApi
import xtz.tquant.api.scala.DataApi._
import scala.collection.JavaConverters._

object DataApiDemo extends App{

    val api = new TQuantApi("tcp://127.0.0.1:10001")
    val dapi = api.dataApi

    def init() {
        dapi.setCallback(new Callback() {
            override
            def onMarketQuote(q : MarketQuote) {
                println("quote: " + q.code + " " + q.date + " " + q.time + " "
                        + q.open + " " + q.high + " " + q.low + " " + q.close + " "
                        + q.last + " " + q.volume)
            }

            override def onBar(cycle: String, bar: Bar): Unit = {

            }
        })
    }

    def test() {
        testSubscribe()
        testQuote()
        testBar()
        testTick()
    }

    def testQuote() {

        try {
            val (q, msg) = dapi.quote("000001.SH")

            if ( q !=null) {
                println("quote: " + q.code + " " + q.date + " " + q.time + " "
                        + q.open + " " + q.high + " " + q.low + " " + q.close + " "
                        + q.last + " " + q.volume)
            } else {
                println("quote error: " + msg)
            }
        }catch {
            case t: Throwable  => t.printStackTrace()
        }
    }

    def testBar(): Unit = {
        val (bars, msg) = dapi.bar("000001.SH", "1m", 0);
        if (bars != null ) {
            for ( b <- bars) {
                println("bar: " + b)
            }
        }
    }

    def testTick(): Unit = {

        val (ticks, msg) = dapi.tick("000001.SH", 0)
        if (ticks != null ) {
            for ( q <- ticks) {
                println("quote: " + q.code + " " + q.date + " " + q.time + " "
                  + q.open + " " + q.high + " " + q.low + " " + q.close + " "
                  + q.last + " " + q.volume)
            }
        }
        println("msg:", msg)

        val t = System.currentTimeMillis
        for ( i <- 1 to 100) dapi.tick("000001.SH", 20170821)
        println("tick time: ", (System.currentTimeMillis - t)/100)

    }

    def testSubscribe() {

        val codes = Array( "000001.SH", "399001.SZ", "IF1706.CFE")

        val (sub_codes, msg) = dapi.subscribe(codes)

        if (sub_codes != null) {
            for ( code <- sub_codes)
                println("Subscribed: " + code)
        } else {
            println("subscribe return error: " + msg)
        }

        try {
            Thread.sleep(10*1000)
        } catch {
            case t : Throwable => t.printStackTrace()
        }
    }

    init()
    //testTick()
    testBar()
}
