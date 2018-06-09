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
                        Console.WriteLine(bar.code + "|"
                            + bar.date + "|" + bar.time + "|"
                            + bar.open + "|" + bar.high + "|"
                            + bar.low + "|" + bar.close + "|"
                            + bar.volume + "|" + bar.turnover);
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
                        Console.WriteLine(tick.code + "|"
                            + tick.date + "|" + tick.time + "|"
                            + tick.open + "|" + tick.high + "|"
                            + tick.low + "|" + tick.close + "|"
                            + tick.volume + "|" + tick.turnover);
                    Console.Write("ticks: " + r.Value.Length);
                }
            }
            {
                var r = dapi.GetDailyBar("000001.SH", "", true);
                if (r.Value != null)
                {
                    foreach (var bar in r.Value)
                        Console.WriteLine(bar.code + "|"
                            + bar.date + "|"
                            + bar.open + "|" + bar.high + "|"
                            + bar.low + "|" + bar.close + "|"
                            + bar.volume + "|" + bar.turnover);
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
            Console.WriteLine("on_account: " + account.account_id + "," + account.status);
        }

        static void OnOrderStatus(Order order)
        {
            Console.WriteLine("on_order: "
                        + order.account_id + ","
                        + order.code + ","
                        + order.entrust_action + ","
                        + order.entrust_price + ","
                        + order.entrust_size + ","
                        + order.entrust_date + ","
                        + order.entrust_time + ","
                        + order.entrust_no + ","
                        + order.fill_price + ","
                        + order.fill_size + ","
                        + order.status + ","
                        + order.status_msg
                        );
        }

        static void OnOrderTrade(Trade trade)
        {
            Console.WriteLine("on_trade: "
                        + trade.account_id + ","
                        + trade.fill_date + ","
                        + trade.fill_time + ","
                        + trade.code + ","
                        + trade.entrust_action + ","
                        + trade.entrust_no + ","
                        + trade.fill_price + ","
                        + trade.fill_size + ","
                        + trade.fill_no
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
                        Console.WriteLine("account: " + act.account_id + ","
                            + act.broker + "," + act.account_id + "," + act.account_type);
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
                        + bal.init_balance + ","
                        + bal.enable_balance + ","
                        + bal.margin + ","
                        + bal.float_pnl + ","
                        + bal.close_pnl
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
                            + pos.code + ","
                            + pos.name + ","
                            + pos.side + ","
                            + pos.init_size + ","
                            + pos.enable_size + ","
                            + pos.current_size + ","
                            + pos.today_size
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
                            + ord.account_id + ","
                            + ord.code + ","
                            + ord.entrust_action + ","
                            + ord.entrust_price + ","
                            + ord.entrust_size + ","
                            + ord.entrust_date + ","
                            + ord.entrust_time + ","
                            + ord.entrust_no + ","
                            + ord.fill_price + ","
                            + ord.fill_size + ","
                            + ord.status + ","
                            + ord.status_msg
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
                            + trade.account_id + ","
                            + trade.fill_date + ","
                            + trade.fill_time + ","
                            + trade.code + ","
                            + trade.entrust_action + ","
                            + trade.entrust_no + ","
                            + trade.fill_price + ","
                            + trade.fill_size + ","
                            + trade.fill_no
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
                    Console.WriteLine("PlaceOrder result: " + oid.entrust_no + "," + oid.order_id);
                }
                else
                {
                    Console.WriteLine("PlaceOrder error: " + r.Msg);
                }

                if (r.Value != null)
                {
                    var r2 = tapi.CancelOrder("glsc", "000001.SH", r.Value.entrust_no);
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
            Console.WriteLine("on_quote: " + quote.date + "," + quote.time + ","
                + quote.code + "," + quote.last + "," + quote.volume);
        }

        static void OnBar(String cycle, Bar bar)
        {
            Console.WriteLine("on_bar: " + cycle + "," + bar.date + "," + bar.time + ","
                + bar.code + "," + bar.open + "," + bar.high + "," + bar.low + "," + bar.close);
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
                var r = dapi.GetTick(code, 0);
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
                if (bar.date < 20180101 || bar.date > 20180501) continue;
                foreach (var code in codes)
                    dapi.GetBar(code, "1m", bar.date);

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
