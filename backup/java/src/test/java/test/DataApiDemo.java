package test;

import java.util.List;

import xtz.tquant.api.java.TQuantApi;
import xtz.tquant.api.java.DataApi;
import xtz.tquant.api.java.DataApi.*;

public class DataApiDemo {

    TQuantApi api = null;
    DataApi dapi = null;

    DataApiDemo() {

        api = new TQuantApi("tcp://127.0.0.1:10001");
        dapi = api.getDataApi();

        dapi.setCallback(new Callback() {
            @Override
            public void onMarketQuote(MarketQuote q) {
                System.out.printf("quote: %s %d %d %.4f %.4f %.4f %.4f %.4f %d %.4f\n",
                        q.code, q.date, q.time,
                        q.open, q.high, q.low, q.close,
                        q.last, q.volume, q.turnover);
            }

            @Override
            public void onBar(String cycle, Bar bar) {

            }
        });
    }

    void testQuote() {

        try {
            DataApi.CallResult<DataApi.MarketQuote> result = dapi.quote("000001.SH");

            if ( result.value !=null) {
                DataApi.MarketQuote q = result.value;
                System.out.printf("quote: %s %d %d %.4f %.4f %.4f %.4f %.4f %d %.4f\n",
                        q.code, q.date, q.time,
                        q.open, q.high, q.low, q.close,
                        q.last, q.volume, q.turnover);
            } else {
                System.out.print("quote error: " + result.msg);
            }
        }catch (Throwable t) {
            t.printStackTrace();
        }
    }

    void testBar() {

        try {
            DataApi.CallResult<List<DataApi.Bar>> result = dapi.bar("000001.SH", "1m", 0, "",false);

            if ( result.value !=null) {
                for ( DataApi.Bar bar : result.value) {
                    System.out.printf("bar: %s %d %d %.4f %.4f %.4f %.4f %d %.4f\n",
                            bar.code, bar.date, bar.time,
                            bar.open, bar.high, bar.low, bar.close,
                            bar.volume, bar.turnover);
                }
            } else {
                System.out.print("bar error: " + result.msg);
            }
        }catch (Throwable t) {
            t.printStackTrace();
        }
    }

    void testTick() {

        try {
            DataApi.CallResult<List<DataApi.MarketQuote>> result = dapi.tick("000001.SH", 0);

            if (result.value != null) {
                for ( DataApi.MarketQuote q : result.value) {
                    System.out.printf("tick: %s %d %d %.4f %.4f %.4f %.4f %.4f %d %.4f\n",
                            q.code, q.date, q.time,
                            q.open, q.high, q.low, q.close,
                            q.last, q.volume, q.turnover);
                }
            } else {
                System.out.println("quote error: " + result.msg);
            }
        }catch (Throwable t) {
            t.printStackTrace();
        }

        long t = System.currentTimeMillis();

        for ( int i = 0; i < 100; i++)
            dapi.tick("000001.SH", 0);

        System.out.println("tick time:" + (System.currentTimeMillis() -t ) / 100);

    }
    void testSubscribe() {

        {
            String[] codes = new String[] { "000001.SH", "399001.SZ", "cu1705.SHF", "CF705.CZC", "rb1801.SHF" };

            DataApi.CallResult<List<String>> result = dapi.subscribe(codes);

            if (result.value != null) {
                for ( String s : result.value) {
                    System.out.println("Subscribed: " + s);
                }
            } else {
                System.out.println("subscribe return error: " + result.msg);
            }
        }
        try {
            while (true) {
                System.out.println("-------------------------");
                DataApi.CallResult<List<String>> r = dapi.subscribe(null);
                if (r.value != null) {
                    for ( String s : r.value) {
                        System.out.println("Subscribed: " + s);
                    }
                }
                Thread.sleep(2*1000);
            }
        } catch ( Throwable t) {
            t.printStackTrace();
        }

    }

    void test() {
        testSubscribe();
        testQuote();
        testBar();
        testTick();
    }

    public static void main(String[] args) {

        new DataApiDemo().test();
    }
}
