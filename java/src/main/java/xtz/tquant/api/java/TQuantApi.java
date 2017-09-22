package xtz.tquant.api.java;

import xtz.tquant.api.java.impl.JsonRpc;
import xtz.tquant.api.java.impl.TradeApiImpl;
import xtz.tquant.api.java.impl.DataApiImpl;

public class TQuantApi {

    private JsonRpc.JsonRpcClient client;
    private TradeApiImpl trade_api;
    private DataApiImpl  data_api;

    /**
     * 初始化 TQuantApi
     * @param addr 服务器地址，取值通常为 tcp://127.0.0.1:10082
     */
    public TQuantApi(String addr) {

        this.client = new JsonRpc.JsonRpcClient();
        this.client.connect(addr);
        this.trade_api = new TradeApiImpl(client);
        this.data_api  = new DataApiImpl(client);

        client.setCallback(
                new JsonRpc.JsonRpcClient.Callback() {
                    @Override
                    public void onConnected() {
                        data_api.onConnected();
                        trade_api.onConnected();
                    }

                    @Override
                    public void onDisconnected() {
                        data_api.onDisconnected();
                        trade_api.onDisconnected();
                    }

                    @Override
                    public void onNotification(String event, Object value) {
                        if (event.startsWith("tapi.")) {
                            trade_api.onNotification(event, value);
                        } else if (event.startsWith("dapi.")) {
                            data_api.onNotification(event, value);
                        } else if (event.startsWith(".sys.")) {
                            data_api.onNotification(event, value);
                            trade_api.onNotification(event, value);
                        }
                    }
                });
    }

    /**
     * 取数据接口
     *
     * @return
     */
    public TradeApi getTradeApi() {
        return trade_api;
    }

    /**
     *  取交易接口
     *
     * @return
     */
    public DataApi  getDataApi() {
        return data_api;
    }


}
