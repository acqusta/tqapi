package xtz.tquant.api.scala

import xtz.tquant.api.java.JsonRpc
import xtz.tquant.api.scala.impl.{DataApiImpl, TradeApiImpl}

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
    def tradeApi : TradeApi = _trade_api

    /**
     *  取交易接口
     *
     * @return
     */
    def dataApi : DataApi = _data_api

}
