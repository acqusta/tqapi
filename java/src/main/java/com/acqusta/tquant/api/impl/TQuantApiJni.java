package com.acqusta.tquant.api.impl;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.api.TradeApi;

public class TQuantApiJni {

    static {
        //System.load("D:\\work\\github\\tqc-api\\c\\build\\java\\src\\main\\native\\Release\\tqapi_jni.dll");
        System.load("/Users/terryxu/work/tquant/tqc-api/c/build/dist/java/libtqapi_jni.dylib");
    }

    public static native long create(String addr) throws Exception;

    public static native void destroy(long handle);

    public static native long getTradeApi(long handle);

    public static native long getDataApi(long handle);

    long handle = 0;

    TQuantApiJni(String addr) throws  Exception {
        this.handle = create(addr);
    }

    @Override
    protected void finalize() throws Throwable {
        if (handle != 0) {
            destroy(handle);
            handle = 0;
        }
        super.finalize();
    }
}

class TradeApiJni {

    static native TradeApi.AccountInfo[] queryAccountStatus  (long handle);

    static native TradeApi.Balance queryBalance (long handle, String account_id);

    static native TradeApi.Order[] queryOrders(long handle, String account_id);

    static native TradeApi.Trade[] queryTrades(long handle, String account_id);

    static native TradeApi.Position[] queryPositions(long handle, String account_id);

    static native TradeApi.OrderID placeOrder(long handle, String account_id, String code, double price, long size, String action, int order_id);

    static native boolean cancelOrder(long handle, String account_id, String code, int order_id);

    static native boolean cancelOrder(long handle, String account_id, String code, String entrust_no);

    static native String query(long handle, String account_id, String command, String params);

    static native void setCallback(long handle, TradeApi.Callback callback);
}

class DataApiJni {

    static native DataApi.MarketQuote[] getTick(long handle, String code, int trading_day);

    static native DataApi.Bar[] getBar (long handle, String code, String cycle, int trading_day, boolean align);

    static native DataApi.DailyBar[] getDailyBar (long handle, String code, String price_adj, boolean align);

    static native DataApi.MarketQuote getQuote (long handle, String code);

    static native String[] subscribe(long handle, String[] codes);

    static native String[] unsubscribe(long handle, String[] codes);

    static native void setCallback(long handle, DataApi.Callback callback);
}

class JniHelper {

    static public TradeApi.AccountInfo createAccountInfo(String account_id, String broker, String account,
                                                         String status, String msg, String account_type) {

        TradeApi.AccountInfo obj = new TradeApi.AccountInfo();
        obj.account_id   = account_id;
        obj.broker       = broker;
        obj.account      = account;
        obj.status       = status;
        obj.msg          = msg;
        obj.account_type = account_type;

        return obj;
    }

    static public TradeApi.Balance createBalance(String account_id, String fund_account,
                                                 double init_balance, double enable_balance, double margin,
                                                 double float_pnl, double close_pnl) {
        TradeApi.Balance obj = new TradeApi.Balance();
        obj.account_id = account_id;
        obj.fund_account = fund_account;
        obj.init_balance = init_balance;
        obj.enable_balance = enable_balance;
        obj.margin = margin;
        obj.float_pnl = float_pnl;
        obj.close_pnl = close_pnl;
        return obj;
    }

    static public TradeApi.Order createOrder(String account_id, String code, String name,
                                             String entrust_no, String entrust_action,
                                             int entrust_date, int entrust_time,
                                             double entrust_price, long entrust_size,
                                             double fill_price, long fill_size,
                                             String status, String status_msg,
                                             int order_id) {
        TradeApi.Order obj = new TradeApi.Order();

        obj.account_id     = account_id          ;
        obj.code           = code                ;
        obj.name           = name                ;
        obj.entrust_no     = entrust_no          ;
        obj.entrust_action = entrust_action      ;
        obj.entrust_price  = entrust_price       ;
        obj.entrust_size   = entrust_size        ;
        obj.entrust_date   = entrust_date        ;
        obj.entrust_time   = entrust_time        ;
        obj.fill_price     = fill_price          ;
        obj.fill_size      = fill_size           ;
        obj.status         = status              ;
        obj.status_msg     = status_msg          ;
        obj.order_id       = order_id            ;
        return obj;
    }

    static public TradeApi.Trade createTrade(String account_id, String code, String name,
                                             String entrust_no, String entrust_action,
                                             String fill_no,
                                             double fill_price, long fill_size,
                                             int fill_date, int fill_time) {
        TradeApi.Trade obj = new TradeApi.Trade();
        obj.account_id     = account_id     ;
        obj.code           = code           ;
        obj.name           = name           ;
        obj.entrust_no     = entrust_no     ;
        obj.entrust_action = entrust_action ;
        obj.fill_no        = fill_no        ;
        obj.fill_size      = fill_size      ;
        obj.fill_price     = fill_price     ;
        obj.fill_date      = fill_date      ;
        obj.fill_time      = fill_time      ;
        return obj;
    }

    static public TradeApi.Position createPosition(String account_id, String code, String name,
                                                long current_size, long enable_size, long init_size,
                                                long today_size, long frozen_size,
                                                String side,
                                                double cost, double cost_price, double last_price,
                                                double float_pnl, double close_pnl,
                                                double margin, double commission) {
        TradeApi.Position obj = new TradeApi.Position();
        obj.account_id    = account_id   ;
        obj.code          = code         ;
        obj.name          = name         ;
        obj.current_size  = current_size ;
        obj.enable_size   = enable_size  ;
        obj.init_size     = init_size    ;
        obj.today_size    = today_size   ;
        obj.frozen_size   = frozen_size  ;
        obj.side          = side         ;
        obj.cost          = cost         ;
        obj.cost_price    = cost_price   ;
        obj.last_price    = last_price   ;
        obj.float_pnl     = float_pnl    ;
        obj.close_pnl     = close_pnl    ;
        obj.margin        = margin       ;
        obj.commission    = commission   ;
        return obj;
    }

    static public TradeApi.OrderID createOrderID(String entrust_no, int order_id) {
        TradeApi.OrderID obj = new TradeApi.OrderID();
        obj.entrust_no    = entrust_no   ;
        obj.order_id      = order_id     ;
        return obj;
    }


    static public DataApi.MarketQuote createMarketQuote(
                String code         ,
                int    date         ,
                int    time         ,
                int    trading_day  ,
                double open         ,
                double high         ,
                double low          ,
                double close        ,
                double last         ,
                double high_limit   ,
                double low_limit    ,
                double pre_close    ,
                long   volume       ,
                double turnover     ,
                double ask1         ,
                double ask2         ,
                double ask3         ,
                double ask4         ,
                double ask5         ,
                double ask6         ,
                double ask7         ,
                double ask8         ,
                double ask9         ,
                double ask10        ,
                double bid1         ,
                double bid2         ,
                double bid3         ,
                double bid4         ,
                double bid5         ,
                double bid6         ,
                double bid7         ,
                double bid8         ,
                double bid9         ,
                double bid10        ,
                long   ask_vol1     ,
                long   ask_vol2     ,
                long   ask_vol3     ,
                long   ask_vol4     ,
                long   ask_vol5     ,
                long   ask_vol6     ,
                long   ask_vol7     ,
                long   ask_vol8     ,
                long   ask_vol9     ,
                long   ask_vol10    ,
                long   bid_vol1     ,
                long   bid_vol2     ,
                long   bid_vol3     ,
                long   bid_vol4     ,
                long   bid_vol5     ,
                long   bid_vol6     ,
                long   bid_vol7     ,
                long   bid_vol8     ,
                long   bid_vol9     ,
                long   bid_vol10    ,
                double settle       ,
                double pre_settle   ,
                long   oi           ,
                long   pre_oi       ) {

        DataApi.MarketQuote obj = new DataApi.MarketQuote();
        obj.code         = code         ;
        obj.date         = date         ;
        obj.time         = time         ;
        obj.trading_day  = trading_day  ;
        obj.open         = open         ;
        obj.high         = high         ;
        obj.low          = low          ;
        obj.close        = close        ;
        obj.last         = last         ;
        obj.high_limit   = high_limit   ;
        obj.low_limit    = low_limit   ;
        obj.pre_close    = pre_close    ;
        obj.volume       = volume       ;
        obj.turnover     = turnover     ;
        obj.ask1         = ask1         ;
        obj.ask2         = ask2         ;
        obj.ask3         = ask3         ;
        obj.ask4         = ask4         ;
        obj.ask5         = ask5         ;
        obj.ask6         = ask6         ;
        obj.ask7         = ask7         ;
        obj.ask8         = ask8         ;
        obj.ask9         = ask9         ;
        obj.ask10        = ask10        ;
        obj.bid1         = bid1         ;
        obj.bid2         = bid2         ;
        obj.bid3         = bid3         ;
        obj.bid4         = bid4         ;
        obj.bid5         = bid5         ;
        obj.bid6         = bid6         ;
        obj.bid7         = bid7         ;
        obj.bid8         = bid8         ;
        obj.bid9         = bid9         ;
        obj.bid10        = bid10        ;
        obj.ask_vol1     = ask_vol1     ;
        obj.ask_vol2     = ask_vol2     ;
        obj.ask_vol3     = ask_vol3     ;
        obj.ask_vol4     = ask_vol4     ;
        obj.ask_vol5     = ask_vol5     ;
        obj.ask_vol6     = ask_vol6     ;
        obj.ask_vol7     = ask_vol7     ;
        obj.ask_vol8     = ask_vol8     ;
        obj.ask_vol9     = ask_vol9     ;
        obj.ask_vol10    = ask_vol10    ;
        obj.bid_vol1     = bid_vol1     ;
        obj.bid_vol2     = bid_vol2     ;
        obj.bid_vol3     = bid_vol3     ;
        obj.bid_vol4     = bid_vol4     ;
        obj.bid_vol5     = bid_vol5     ;
        obj.bid_vol6     = bid_vol6     ;
        obj.bid_vol7     = bid_vol7     ;
        obj.bid_vol8     = bid_vol8     ;
        obj.bid_vol9     = bid_vol9     ;
        obj.bid_vol10    = bid_vol10    ;
        obj.settle       = settle       ;
        obj.pre_settle   = pre_settle   ;
        obj.oi           = oi           ;
        obj.pre_oi       = pre_oi       ;

        return obj;
    }


    static public DataApi.Bar createBar(
            String code              ,
            int    date              ,
            int    time              ,
            int    trading_day       ,
            double open              ,
            double high              ,
            double low               ,
            double close             ,
            long   volume            ,
            double turnover          ,
            long   oi                ) {
        DataApi.Bar obj = new DataApi.Bar();
        obj.code        = code        ;
        obj.date        = date        ;
        obj.time        = time        ;
        obj.trading_day = trading_day ;
        obj.open        = open        ;
        obj.high        = high        ;
        obj.low         = low         ;
        obj.close       = close       ;
        obj.volume      = volume      ;
        obj.turnover    = turnover    ;
        obj.oi          = oi          ;

        return obj;
    }

    static public DataApi.DailyBar createDailyBar(
            String code              ,
            int    date              ,
            double open              ,
            double high              ,
            double low               ,
            double close             ,
            long   volume            ,
            double turnover          ,
            long   oi                ,
            double settle            ,
            double pre_close         ,
            double pre_settle        ) {

        DataApi.DailyBar obj = new DataApi.DailyBar();
        obj.code        = code        ;
        obj.date        = date        ;
        obj.open        = open        ;
        obj.high        = high        ;
        obj.low         = low         ;
        obj.close       = close       ;
        obj.volume      = volume      ;
        obj.turnover    = turnover    ;
        obj.oi          = oi          ;
        obj.settle      = settle      ;
        obj.pre_close   = pre_close   ;
        obj.pre_settle  = pre_settle  ;

        return obj;
    }
}

