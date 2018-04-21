using System;
using System.Threading;
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

        class MyTradeApiCallback : TradeApiCallback
        {
            public void OnAccountStatus(AccountInfo account)
            {
                Console.WriteLine("on_account: " + account.account_id + "," + account.status);
            }

            public void OnOrderStatus(Order order)
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

            public void OnOrderTrade(Trade trade)
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
        }

        static void TestTradeApi(TradeApi tapi)
        {
            tapi.SetCallback(new MyTradeApiCallback());

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
                    var r2 = tapi.CanceOrder("glsc", "000001.SH", r.Value.entrust_no);
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

        class MyDataApiCallback : DataApiCallback
        {
            public void OnMarketQuote(MarketQuote quote) {
                Console.WriteLine("on_quote: " + quote.date + "," + quote.time + ","
                    + quote.code + "," + quote.last + "," + quote.volume);
            }
            public void OnBar(String cycle, Bar bar) {
                Console.WriteLine("on_bar: " + cycle + "," + bar.date + "," + bar.time + ","
                    + bar.code + "," + bar.open + "," + bar.high + "," + bar.low + "," + bar.close);
            }
        }

        static void TestDataApi2(DataApi dapi)
        {
            dapi.Subscribe(new string[] { "CU1806.SHF", "RB1810.SHF",
                "000001.SH", "399001.SZ",
                "600000.SH", "000001.SZ"
            });
            dapi.SetCallback(new MyDataApiCallback());

            while(true)
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

            int count = 100000;
            for (var i = 0; i < count; i ++)
            {
                var r = dapi.GetTick(code, 20180416);
                //var r = dapi.GetBar(code, "1m", 20180416);
                if (r.Value == null)
                    Console.WriteLine("GetTick error: " + r.Msg);
                Thread.Sleep(10);
            }

            long end_time = DateTime.Now.Ticks;

            Console.WriteLine("Used time: " + (end_time - begin_time) /10000.0 / count + "ms");
            Console.Read();
        }

        static int Main(string[] args)
        {
            var tqapi = TQuantApi.Create("tcp://127.0.0.1:10001");
            var dapi = tqapi.GetDataApi();
            var tapi = tqapi.GetTradeApi();
            //TestDataApi(dapi);
            TestDataApi2(dapi);
            //TestTradeApi(tapi);
            PerfTest(dapi);
            return 0;
        }
    }
}
