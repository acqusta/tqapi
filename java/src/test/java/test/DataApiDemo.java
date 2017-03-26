package test;

import java.util.List;

import xtz.tquant.api.java.TQuantApi;
import xtz.tquant.api.java.DataApi;
import xtz.tquant.api.java.DataApi.*;

public class DataApiDemo {

    TQuantApi api = null;
    DataApi dapi = null;

    DataApiDemo() {

        // 初始化 api
        api = new TQuantApi("tcp://127.0.0.1:10001");
        dapi = api.getDataApi();

        dapi.setCallback(new Callback() {
            @Override
            public void onMarketQuote(MarketQuote q) {
                System.out.println("quote: " + q.code + " " + q.date + " " + q.time + " "
                        + q.open + " " + q.high + " " + q.low + " " + q.close + " "
                        + q.last + " " + q.volume);
            }
        });
    }

    void test() {
        testQuote();
        testSubscribe();
    }

    void testQuote() {

        try {
            DataApi.CallResult<DataApi.MarketQuote> result = dapi.quote("rb1705.SHF");

            if ( result.value !=null) {
                DataApi.MarketQuote q = result.value;
                System.out.println("quote: " + q.code + " " + q.date + " " + q.time + " "
                        + q.open + " " + q.high + " " + q.low + " " + q.close + " "
                        + q.last + " " + q.volume);
            } else {
                System.out.print("quote error: " + result.msg);
            }
        }catch (Throwable t) {
            t.printStackTrace();
        }
    }

    void testSubscribe() {

        String[] codes = new String[] { "000001.SH", "399001.SZ", "cu1705.SHF", "CF705.CZC", "rb1705.SHF" };

        DataApi.CallResult<List<String>> result = dapi.subscribe(codes);

        if (result.value != null) {
            for ( String s : result.value) {
                System.out.println("Subscribed: " + s);
            }
        } else {
            System.out.println("subscribe return error: " + result.msg);
        }

        try {
            Thread.sleep(10*1000);
        } catch ( Throwable t) {
            t.printStackTrace();
        }

    }


    public static void main(String[] args) {

        new DataApiDemo().test();
    }
}
