package test;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.stralet.*;

public class StraletDemo extends Stralet {
    StraletDemo() {
        System.out.println("HelloWorld");
    }
    @Override
    public void onInit() {
        StraletContext ctx = getContext();

        ctx.getLogger().info("onInit %d", ctx.getTradingDay());

        DataApi dapi = this.getContext().getDataApi();
        dapi.subscribe( new String[] { "000001.SH", "600000.SH"});
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

    public static void main(String[] args) throws Exception {
        BackTest.Config cfg = new BackTest.Config();
        cfg.dapi_addr  = "ipc://tqc_10001";
        cfg.data_level = "tk";
        cfg.begin_date = 20190103;
        cfg.end_date   = 20190331;

        BackTest.run(cfg, new StraletCreator() {
            @Override
            public Stralet createStralet() {
               return new StraletDemo();
            }});
    }
}
