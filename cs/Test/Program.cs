using System;
using System.IO;
using System.Threading;
using System.Collections.Generic;

using TQuant.Api;


namespace Test
{
    class Program
    {
        static void TestDataApi(DataApi dapi)
        {
            {
                var r = dapi.GetBar("000001.SH", "1m", 0, true);
                if (r.Value != null)
                {
                    foreach (var bar in r.Value)
                        Console.WriteLine(bar.Code + "|"
                            + bar.Date + "|" + bar.Time + "|"
                            + bar.Open + "|" + bar.High + "|"
                            + bar.Low + "|" + bar.Close + "|"
                            + bar.Volume + "|" + bar.Turnover);
                    Console.WriteLine("bars : " + r.Value.Length);
                }
                else
                {
                    Console.WriteLine("GetBar error:" + r.Msg);
                }
            }
            {
                var r = dapi.GetTick("600000.SH");
                if (r.Value != null)
                {
                    foreach (var tick in r.Value)
                        Console.WriteLine(tick.Code + "|"
                            + tick.Date + "|" + tick.Time + "|"
                            + tick.Open + "|" + tick.High + "|"
                            + tick.Low + "|" + tick.Close + "|"
                            + tick.Volume + "|" + tick.Turnover);
                    Console.Write("ticks: " + r.Value.Length);
                }
            }
            {
                var r = dapi.GetDailyBar("000001.SH", "", true);
                if (r.Value != null)
                {
                    foreach (var bar in r.Value)
                        Console.WriteLine(bar.Code + "|"
                            + bar.Date + "|"
                            + bar.Open + "|" + bar.High + "|"
                            + bar.Low + "|" + bar.Close + "|"
                            + bar.Volume + "|" + bar.Turnover);
                    Console.WriteLine("dailybars : " + r.Value.Length);
                }
                else
                {
                    Console.WriteLine("GetBar error:" + r.Msg);
                }
            }

        }

        static void OnAccountStatus(AccountInfo account)
        {
            Console.WriteLine("on_account: " + account.AccountId + "," + account.Status);
        }

        static void OnOrderStatus(Order order)
        {
            Console.WriteLine("on_order: "
                        + order.AccountId + ","
                        + order.Code + ","
                        + order.EntrustAction + ","
                        + order.EntrustPrice + ","
                        + order.EntrustSize + ","
                        + order.EntrustDate + ","
                        + order.EntrustTime + ","
                        + order.EntrustNo + ","
                        + order.FillPrice + ","
                        + order.FillSize + ","
                        + order.Status + ","
                        + order.StatusMsg
                        );
        }

        static void OnOrderTrade(Trade trade)
        {
            Console.WriteLine("on_trade: "
                        + trade.AccountId + ","
                        + trade.FillDate + ","
                        + trade.FillTime + ","
                        + trade.Code + ","
                        + trade.EntrustAction + ","
                        + trade.EntrustNo + ","
                        + trade.FillPrice + ","
                        + trade.FillSize + ","
                        + trade.FillNo
                        );
        }

        static void TestTradeApi(TradeApi tapi)
        {
            tapi.OnAccountStatus += OnAccountStatus;
            tapi.OnOrderStatus += OnOrderStatus;
            tapi.OnOrderTrade  += OnOrderTrade;

            {
                var r = tapi.QueryAccountStatus();
                if (r.Value != null)
                {
                    foreach (var act in r.Value)
                    {
                        Console.WriteLine("account: " + act.AccountId + ","
                            + act.Broker + "," + act.AccountId + "," + act.AccountType);
                    }
                }
                else
                {
                    Console.WriteLine("QueryAccountStatus error: " + r.Msg);
                }
            }

            {
                var r = tapi.QueryBalance("glsc");
                if (r.Value != null)
                {
                    var bal = r.Value;
                    Console.WriteLine("balance: "
                        + bal.InitBalance + ","
                        + bal.EnableBalance + ","
                        + bal.Margin + ","
                        + bal.FloatPnl + ","
                        + bal.ClosePnl
                        );
                }
                else
                {
                    Console.WriteLine("QueryBalance error: " + r.Msg);
                }
            }
            {
                var r = tapi.QueryPositions("glsc");
                if (r.Value != null)
                {
                    foreach (var pos in r.Value)
                    {
                        Console.WriteLine("position: "
                            + pos.Code + ","
                            + pos.Name + ","
                            + pos.Side + ","
                            + pos.InitSize + ","
                            + pos.EnableSize + ","
                            + pos.CurrentSize + ","
                            + pos.TodaySize
                            );
                    }
                }
                else
                {
                    Console.WriteLine("QueryPositions error: " + r.Msg);
                }
            }
            {
                var r = tapi.QueryOrders("glsc");
                if (r.Value != null)
                {
                    foreach (var ord in r.Value)
                    {
                        Console.WriteLine("order: "
                            + ord.AccountId + ","
                            + ord.Code + ","
                            + ord.EntrustAction + ","
                            + ord.EntrustPrice + ","
                            + ord.EntrustSize + ","
                            + ord.EntrustDate + ","
                            + ord.EntrustTime + ","
                            + ord.EntrustNo + ","
                            + ord.FillPrice + ","
                            + ord.FillSize + ","
                            + ord.Status + ","
                            + ord.StatusMsg
                            );
                    }
                }
                else
                {
                    Console.WriteLine("QueryOrders error: " + r.Msg);
                }
            }
            {
                var r = tapi.QueryTrades("glsc");
                if (r.Value != null)
                {
                    foreach (var trade in r.Value)
                    {
                        Console.WriteLine("order: "
                            + trade.AccountId + ","
                            + trade.FillDate + ","
                            + trade.FillTime + ","
                            + trade.Code + ","
                            + trade.EntrustAction + ","
                            + trade.EntrustNo + ","
                            + trade.FillPrice + ","
                            + trade.FillSize + ","
                            + trade.FillNo
                            );
                    }
                }
                else
                {
                    Console.WriteLine("QueryTrades error: " + r.Msg);
                }
            }

            {
                var r = tapi.PlaceOrder("glsc", "000001.SH", 10.0, 100, "Buy");
                if (r.Value != null)
                {
                    var oid = r.Value;
                    Console.WriteLine("PlaceOrder result: " + oid.EntrustNo + "," + oid.OrderId);
                }
                else
                {
                    Console.WriteLine("PlaceOrder error: " + r.Msg);
                }

                if (r.Value != null)
                {
                    var r2 = tapi.CancelOrder("glsc", "000001.SH", r.Value.EntrustNo);
                    if (r2.Value)
                    {
                        Console.WriteLine("CancelOrder result: " + r2.Value);
                    }
                    else
                    {
                        Console.WriteLine("CancelOrder error: " + r2.Msg);
                    }
                }
            }

            while(true)
            {
                Thread.Sleep(1000);
            }

        }

        static void OnMarketQuote(MarketQuote quote)
        {
            Console.WriteLine("on_quote: " + quote.Date + "," + quote.Time + ","
                + quote.Code + "," + quote.Last + "," + quote.Volume);
        }

        static void OnBar(String cycle, Bar bar)
        {
            Console.WriteLine("on_bar: " + cycle + "," + bar.Date + "," + bar.Time + ","
                + bar.Code + "," + bar.Open + "," + bar.High + "," + bar.Low + "," + bar.Close);
        }

        static void TestDataApi2(DataApi dapi)
        {
            dapi.Subscribe(new string[] { 
                //"CU1806.SHF", "RB1810.SHF",
                "000001.SH", "399001.SZ",
                "600000.SH", "000001.SZ"
            });

            dapi.OnBar += OnBar;
            dapi.OnMarketQuote += OnMarketQuote;

            while (true)
            {
                Thread.Sleep(1000);
            }
        }

        static void PerfTest(DataApi dapi)
        {
            //var code = "RB1810.SHF";
            var code = "600000.SH";
            dapi.Subscribe(new string[] { code });

            long begin_time = DateTime.Now.Ticks;

            int count = 3000;
            for (var i = 0; i < count; i ++)
            {
                if ( i % 100 == 0) Console.WriteLine(i);
                var r = dapi.GetTick(code, 20180612);
                //var r = dapi.GetBar(code, "1m");
                if (r.Value == null)
                    Console.WriteLine("GetTick error: " + r.Msg);
            }

            long end_time = DateTime.Now.Ticks;

            Console.WriteLine("Used time: " + (end_time - begin_time) / 10000.0  + "ms");
            Console.WriteLine("Each time: " + (end_time - begin_time) /10000.0 / count + "ms");
            Console.Read();
        }

        static void PerfTest2(DataApi dapi)
        {
            var codes = new List<String>();

            var result = CSVReader.ParseCSV(File.ReadAllText(@"d:\\tmp\\000300.SH.csv"));
            foreach (var line in result)
            {
                codes.Add(line[3]);
            }
            codes.RemoveAt(0);

            dapi.Subscribe(new String[] { "000001.SH" });
            dapi.Subscribe(codes.ToArray());

            var begin_time = DateTime.Now;
            foreach (var bar in dapi.GetDailyBar("000001.SH").Value)
            {
                if (bar.Date < 20180101 || bar.Date > 20180501) continue;
                foreach (var code in codes)
                    dapi.GetBar(code, "1m", bar.Date);

            }
            var end_time = DateTime.Now;

            Console.Out.WriteLine(String.Format("time: {0}", (end_time - begin_time).TotalSeconds));            
        }

        struct CodeMapping
        {
            public string code;
            public int    date;
            public string target_code;
        };

        static Dictionary<string, CodeMapping[]> g_code_maping_map = new Dictionary<string, CodeMapping[]>();

        static void LoadCodeMapping()
        {
            var result = CSVReader.ParseCSV(File.ReadAllText(@"d:\\tquant\\tqc\\tmp\\code_mapping.csv"));
            result.RemoveAt(0);

            var code_mapping_map = new Dictionary<string, List<CodeMapping>>();
            foreach (var line in result)
            {
                CodeMapping mapping;
                mapping.code = line[0];
                mapping.date = int.Parse(line[1]);
                mapping.target_code = line[2];
                if (!code_mapping_map.ContainsKey(mapping.code))
                    code_mapping_map[mapping.code] = new List<CodeMapping>();
                code_mapping_map[mapping.code].Insert(0, mapping);
            }

            foreach( var e in code_mapping_map)
            {
                g_code_maping_map[e.Key] = e.Value.ToArray();
            }
        }

        static string GetCode(string code, int date)
        {
            if (!g_code_maping_map.ContainsKey(code))
                return code;

            var mappings = g_code_maping_map[code];
            foreach (var mapping in mappings)
            {
                if (mapping.date <= date)
                    return mapping.target_code;
            }

            return mappings[0].target_code;
        }

        static void PerfTest3(DataApi dapi)
        {
            LoadCodeMapping();

            //String code = "IF.CFE";
            String code = "RB.SHF";
            dapi.Subscribe(new String[] { code });


            var begin_time = DateTime.Now;
            int total_count = 0;
            int date_count = 0;

            var begin_date = new System.DateTime(2017, 1, 1);
            var end_date = new System.DateTime(2018, 1, 1);
            for (var date = begin_date; date < end_date; date = date.AddDays(1)) {

                int i_date = date.Year * 10000 + date.Month * 100 + date.Day;
                string real_code = GetCode(code, i_date);
                //auto ticks = dapi->bar(real_code, "1m", bar.date, false);
                var ticks = dapi.GetTick(real_code, i_date);
                if (ticks.Value != null)
                {
                    date_count++;
                    total_count += ticks.Value.Length;
                }
                else
                    Console.WriteLine(String.Format("tick error: {0}, {1}, {2}", real_code, i_date, ticks.Msg));

            }

            var end_time = DateTime.Now;

            var used_time = end_time - begin_time;
            Console.Out.WriteLine(String.Format("used time     : {0} milliseconds", (int)used_time.TotalMilliseconds));
            Console.Out.WriteLine(String.Format("total records : {0}", total_count));
            Console.Out.WriteLine(String.Format("total date    : {0}", date_count));
            Console.Out.WriteLine(String.Format("ticks per day : {0}", total_count / date_count));
            Console.Out.WriteLine(String.Format("time per day  : {0}", used_time.TotalMilliseconds * 1.0/ date_count));
        }

        static int Main(string[] args)
        {
            if (true)
            {
                var dapi = TQuantApi.CreateDataApi("ipc://tqc_10001");
                //var tapi = TQuantApi.CreateTradeApi("ipc://tqc_10001");
                //TestDataApi(dapi);
                //TestDataApi2(dapi);
                //TestTradeApi(tapi);
                PerfTest(dapi);
                //PerfTest2(dapi);
            }

            if (false)
            {
                TQuantApi.SetParams("embed_path", "d:/tquant/");
                var dapi = TQuantApi.CreateDataApi("embed://tkapi/file://d:/tquant/tqc?hisdata_only=true");
                PerfTest3(dapi);
            }
            return 0;
        }
    }
}
