package test;

import com.acqusta.tquant.api.TQuantApi;
import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.api.DataApi.*;

public class DataApiDemo {

    private TQuantApi api = null;
    private DataApi dapi = null;

    DataApiDemo() throws Exception {

        //api = new TQuantApi("tcp://127.0.0.1:10001");
        api = new TQuantApi("ipc://tqc_10001");
        dapi = api.getDataApi("");

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
                System.out.printf("bar: %s %d %d %.4f %.4f %.4f %.4f %.4f %d %.4f %d\n",
                        bar.code, bar.date, bar.time,
                        bar.open, bar.high, bar.low, bar.close,
                        bar.volume, bar.turnover, bar.oi);
            }
        });
    }

    void testQuote() {

        try {
            DataApi.CallResult<DataApi.MarketQuote> result = dapi.getQuote("000001.SH");

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
            DataApi.CallResult<DataApi.Bar[]> result = dapi.getBar("000001.SH", "1m", 0, false);

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
            DataApi.CallResult<DataApi.MarketQuote[]> result = dapi.getTick("000001.SH", 0);

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
            dapi.getTick("000001.SH", 0);

        System.out.println("tick time:" + (System.currentTimeMillis() -t ) / 100);
    }

    void testSubscribe() {

        {
            String[] codes = new String[] {
                    "000001.SH", "399001.SZ", "cu1705.SHF",
                    "CF705.CZC", "rb1801.SHF", "000999.SH" };

            DataApi.CallResult<String[]> result = dapi.subscribe(codes);

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
                DataApi.CallResult<String[]> r = dapi.subscribe(null);
                if (r.value != null) {
                    for ( String s : r.value) {
                        System.out.println("Subscribed: " + s);
                    }
                }
                Thread.sleep(2*1000);
                System.gc();
            }
        } catch ( Throwable t) {
            t.printStackTrace();
        }

    }

    void testPerf() {
        try {
            dapi.subscribe(new String[]{"IF.CFE"});
            CallResult<DailyBar[]> r = dapi.getDailyBar("IF.CFE", "", true);
            long begin_time = System.currentTimeMillis();
            int count  = 0;
            int dates = 0;
            for (DailyBar bar : r.value) {
                if (bar.date > 20171001) {
                    dates += 1;
                    DataApi.CallResult<DataApi.MarketQuote[]> result = dapi.getTick("IF.CFE", bar.date);
                    if (result.value != null) {
                        count += result.value.length;
                    } else {
                        System.out.println("error: " + bar.date + "," + result.msg);
                    }
                }
            }

            long end_time = System.currentTimeMillis();

            System.out.println("used time    : " +  (end_time - begin_time));
            System.out.println("count        : " + count);
            System.out.println("count per day: " + (count / dates));
            System.out.println("time per day : " + (end_time-begin_time)/dates);

        }catch (Throwable t) {
            t.printStackTrace();
        }
    }

    void testPerf2() {
        try {
            dapi.getQuote("000001.SH");
            long begin_time = System.currentTimeMillis();
            int count  = 0;
            int dates = 0;
            for( int i =0; i < 10000; i++) {
                CallResult<MarketQuote> r = dapi.getQuote("000001.SH");
                if (r.value != null) {
                    count += 1;//result.value.length;
                } else {
                    System.out.println("error: " + r.msg);
                }
            }

            long end_time = System.currentTimeMillis();

            System.out.println("used time    : " + (end_time - begin_time));
            System.out.println("time per call : " + (end_time - begin_time)/10000.0);

        }catch (Throwable t) {
            t.printStackTrace();
        }
    }

    void test() {
        testSubscribe();
        testQuote();
        testBar();
        testTick();
        testPerf();
        testPerf2();
    }

    public static void main(String[] args)  throws Exception {

        new DataApiDemo().test();
    }
}
