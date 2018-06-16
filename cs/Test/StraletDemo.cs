using System;
using System.Collections.Generic;
using TQuant.Api;
using TQuant.Stralet;
using TQuant.Stralet.StraletEvent;

namespace Test
{
    class StraletDemo : Stralet
    {
        public override void OnEvent(object evt)
        {
            if (evt is OnInit)
            {
                OnInit();
            }
            else if (evt is OnFini)
            {
                OnFini();
            }
            else if (evt is OnQuote)
            {
                OnQuote((evt as OnQuote).Quote);
            }
        }
        public void OnInit()
        {
            Context.Logger.Info(String.Format("OnInit: {0}", Context.TradingDay));
            Context.DataApi.Subscribe( new String[]{ "000001.SH", "600000.SH","399001.SZ"});
        }

        public void OnFini()
        {
            Context.Logger.Info(String.Format("OnFini: {0}", Context.TradingDay));
        }
        public void OnQuote(MarketQuote quote)
        {
            Context.Logger.Info(String.Format("on_quote {0} {1} {2} {3}", quote.code, quote.time, quote.last, quote.volume));
        }
    }


    class Demo
    {
        static Stralet CreateStralet()
        {
            return new StraletDemo();
        }

        static void Test()
        {
            BackTest.Config cfg = new BackTest.Config();
            cfg.data_level = "tk";
            cfg.begin_date = 20180101;
            cfg.end_date = 20180501;

            TQuant.Stralet.BackTest.Run(cfg, CreateStralet);
        }

        static void Run()
        {
            RealTime.Config cfg = new RealTime.Config();

            TQuant.Stralet.RealTime.Run(cfg, CreateStralet);
        }

        public static int Main(string[] args)
        {
            //return MyIfSpreadTradeMain.Main(args);
            //return MyIfSpreadTrade2Main.Main(args);
            //return MyIfSpreadTrade3Stralet.MyIfSpreadTrade3Main.Main(args);
            //return TestFSM.Main(args);

            Run();
            return 1;
        }
    }
    
}
