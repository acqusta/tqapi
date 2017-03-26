package test

import xtz.tquant.api.scala.TQuantApi
import xtz.tquant.api.scala.DataApi._
import scala.collection.JavaConverters._

object DataApiDemo extends App{


    // 初始化 api
    val api = new TQuantApi("tcp://127.0.0.1:10001")
    val dapi = api.data_api

    def init() {
        dapi.setCallback(new Callback() {
            override
            def onMarketQuote(q : MarketQuote) {
                println("quote: " + q.code + " " + q.date + " " + q.time + " "
                        + q.open + " " + q.high + " " + q.low + " " + q.close + " "
                        + q.last + " " + q.volume)
            }
        })
    }

    def test() {
        testQuote()
        testSubscribe()
    }

    def testQuote() {

        try {
            val (q, msg) = dapi.quote("rb1705.SHF")

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

    def testSubscribe() {

        val codes = Array( "000001.SH", "399001.SZ", "cu1705.SHF", "CF705.CZC", "rb1705.SHF")

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
    test()
}
