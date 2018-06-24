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
            Context.Logger.Info(String.Format("on_quote {0} {1} {2} {3}", quote.Code, quote.Time, quote.Last, quote.Volume));
        }
    }


    class Demo
    {
        static Stralet CreateStralet()
        {
            return new StraletDemo();
        }

        static int Test()
        {
            BackTest.Config cfg = new BackTest.Config();
            cfg.data_level = "1m";
            cfg.begin_date = 20180101;
            cfg.end_date = 20180102;

            TQuant.Stralet.BackTest.Run(cfg, CreateStralet);
            return 0;
        }

        static int Run()
        {
            RealTime.Config cfg = new RealTime.Config();

            TQuant.Stralet.RealTime.Run(cfg, CreateStralet);
            return 0;
        }

        public static int Main(string[] args)
        {
            if (args.Length >= 1 && args[0] == "realtime")
                return Run();
            else
                return Test();
        }
    }
    
}
