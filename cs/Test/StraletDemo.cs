﻿using System;
using System.Collections.Generic;
using TQuant.Api;
using TQuant.Stralet;

namespace Test
{
    class StraletDemo : Stralet
    {
        public override void OnInit()
        {
            Context.Logger.Info(String.Format("OnInit: {0}", Context.TradingDay));
            Context.DataApi.Subscribe( new String[]{ "000001.SH", "600000.SH","399001.SZ"});
        }

        public override void OnFini()
        {
            Context.Logger.Info(String.Format("OnFini: {0}", Context.TradingDay));
        }
        public override void OnQuote(MarketQuote quote)
        {
            Context.Logger.Info(String.Format("OnQuote {0} {1} {2} {3}", quote.Code, quote.Time, quote.Last, quote.Volume));
        }
        public override void OnBar(string cycle, Bar bar)
        {
            Context.Logger.Info(String.Format("OnBar {0} {1} {2} {3}", bar.Code, bar.Time, bar.Close, bar.Volume));
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
