using System;
using System.Collections.Generic;
using TQuant.Api;
using TQuant.Stralet;

namespace Test
{
    class StraletDemo : Stralet
    {        
        public override void OnInit(IStraletContext sc)
        {
            sc.Logger.Info("OnInit: " + sc.TradingDay);
            sc.DataApi.Subscribe( new String[]{ "000001.SH", "600000.SH","399001.SZ"});
        }

        public override void OnFini()
        {
            //ctx.Log("OnFini: " + ctx.TradingDay);
        }
        public override void OnQuote(MarketQuote quote)
        {
            //var str = String.Format("on_quote {0} {1} {2} {3}", quote.code, quote.time, quote.last, quote.volume);
            //ctx.Log(str);
        }
        public override void OnBar(String cyle, Bar bar) { }
        public override void OnTimer(Int32 id, Int64 data) { }
        public override void OnEvent(String evt, Object data) { }
        public override void OnOrderStatus(Order order) { }
        public override void OnOrderTrade(Trade trade) { }
        public override void OnAccountStatus(AccountInfo account) { }
    }


    class Demo
    {
        static Stralet CreateStralet()
        {
            return new StraletDemo();
        }

        public static int Main(string[] args)
        {
            //return MyIfSpreadTradeMain.Main(args);
            //return MyIfSpreadTrade2Main.Main(args);
            return MyIfSpreadTrade3Stralet.MyIfSpreadTrade3Main.Main(args);
            //return TestFSM.Main(args);

            BackTestConfig cfg = new BackTestConfig();
            cfg.data_level = "tk";
            cfg.begin_date = 20180101;
            cfg.end_date   = 20180501;

            TQuant.Stralet.BackTest.Run(cfg, CreateStralet);
            return 1;
        }
    }
    
}
