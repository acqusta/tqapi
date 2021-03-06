package test;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.stralet.*;

public class StraletDemo extends Stralet {

    @Override
    public void onInit() {
        StraletContext ctx = getContext();

        ctx.getLogger().info("onInit %d", ctx.getTradingDay());

        DataApi dapi = this.getContext().getDataApi();
        dapi.subscribe( new String[] { "000001.SH", "399001.SH", "000001.SZ", "600000.SH", "RB1905.SHF", "T1906.CFE", "IF1904.CFE"});
    }

    @Override
    public void onFini() {
        StraletContext ctx = getContext();

        ctx.getLogger().info("onFini %d", ctx.getTradingDay());
    }

    @Override
    public void onQuote(DataApi.MarketQuote quote) {
        StraletContext ctx = getContext();
        ctx.getLogger().info("onQuote %s %d %d %f %d", quote.code, quote.date, quote.time, quote.last, quote.volume);
    }

    @Override
    public void onBar(String cycle, DataApi.Bar bar) {
        StraletContext ctx = getContext();
        ctx.getLogger().info("onBar %s %d %d %f %d", bar.code, bar.date, bar.time, bar.close, bar.volume);
    }

    static void runBackTest() throws Exception {
        BackTest.Config cfg = new BackTest.Config();
        //cfg.dapi_addr  = "ipc://tqc_10001";
        cfg.dapi_addr = "tcp://192.168.50.132:10002";
        cfg.data_level = "tk";
        cfg.begin_date = 20201029;
        cfg.end_date   = 20201029;

        BackTest.run(cfg, new StraletCreator() {
            @Override
            public Stralet createStralet() {
                return new StraletDemo();
            }});
    }

    static void runRealTime() throws Exception {
        RealTime.Config cfg = new RealTime.Config();
        //cfg.dapi_addr = "ipc://10002";
        cfg.dapi_addr = "tcp://192.168.50.132:10002";
//        cfg.dapi_addr = "ipc://10002";
//        cfg.tapi_addr = "ipc://10202";

        RealTime.run(cfg, new StraletCreator() {
            @Override
            public Stralet createStralet() {
                return new StraletDemo();
            }});
    }

    public static void main(String[] args) throws Exception {

        runBackTest();
        //runRealTime();
    }
}
