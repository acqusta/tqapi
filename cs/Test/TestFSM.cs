using System;

using TQuant.Stralet;
using TQuant.Api;

namespace Test
{
    public interface IData { }

    public class Uninitialized : IData
    {
        public static Uninitialized Instance = new Uninitialized();

        private Uninitialized() { }
    }

    public enum State
    {
        Idle,
        Working,
        Stopped
    }

    public class ExampleFSMStralet : FsmStralet<State, IData>
    {
        public ExampleFSMStralet()
        {
            StartWith(State.Idle, Uninitialized.Instance);

            When(State.Idle, IdleFunctions);

            When(State.Working, WorkingFunctions);

            WhenUnhandled(UnhandledFunction);

            Initialize();
        }

        public State<State, IData> IdleFunctions(Event<IData> evt)
        {
            if (evt.FsmEvent is OnInit)
            {
                Context.Logger.Info("OnInit: {0} {1}", Context.TradingDay, Context.Mode);
                Context.DataApi.Subscribe(new String[] { "000001.SH", "RB.SHF" });
                SetTimer("CheckTimer", "checktime", TimeSpan.FromSeconds(60), true);
                return GoTo(State.Working);
            }

            return null;
            
        }

        public State<State, IData> WorkingFunctions(Event<IData> evt)
        {
            if (evt.FsmEvent is Timer)
            {
                var timer = evt.FsmEvent as Timer;
                Context.Logger.Info("OnTimer {0}", timer.Name);
                return Stay();
            }
            else if (evt.FsmEvent is OnQuote)
            {
                var quote = (evt.FsmEvent as OnQuote).Quote;
                Context.Logger.Info("OnQuoe {0} {1} {2} {3}", quote.time, quote.code, quote.last, quote.volume);
                return Stay();
            }
            else if (evt.FsmEvent is OnOrder)
            {
                var order = (evt.FsmEvent as OnOrder).Order;
                Context.Logger.Info("OnOrder {0} {1} {2} {3} {4} {5} {6}", 
                    order.account_id, order.code, order.entrust_action, order.entrust_no,
                    order.order_id, order.status, order.status_msg);
                return Stay();
            }
            else if (evt.FsmEvent is OnTrade)
            {
                var trade = (evt.FsmEvent as OnTrade).Trade;
                Context.Logger.Info("OnTrade {0} {1} {2} {3} {4} {5:F2} {6} {7}",
                    trade.account_id, trade.code, trade.entrust_action, trade.entrust_no, trade.fill_no,
                    trade.fill_price, trade.fill_size, trade.fill_time);
            }
            return null;
        }

        public State<State, IData> UnhandledFunction(Event<IData> evt)
        {
            if (evt.FsmEvent is Timer)
            {
                var timer = evt.FsmEvent as Timer;
                Context.Logger.Info("OnTimer {0}", timer.Name);
            }
            else if (evt.FsmEvent is OnFini)
            {
                Context.Logger.Info("OnFini");
                return GoTo(State.Stopped);
            }
            return null;
        }
    }

    public delegate Stralet CreateStralet();

    class TestFSM
    {
        static Stralet CreateStralet()
        {
            return new ExampleFSMStralet().Stralet;
        }

        static void Test()
        {
            var begin_time = DateTime.Now;
            BackTest.Config cfg = new BackTest.Config();
            cfg.dapi_addr = "ipc://tqc_10001";
            cfg.data_level = "tk";
            cfg.begin_date = 20180101;
            cfg.end_date = 20180516;

            BackTest.Run(cfg, CreateStralet);

            var used_time = System.DateTime.Now - begin_time;
            Console.WriteLine("used_time: " + used_time.TotalSeconds);
        }

        static void Run()
        {
            RealTime.Config cfg = new RealTime.Config();
            RealTime.Run(cfg, CreateStralet);
        }

        public static int Main(string[] args)
        {
            String mode = args.Length > 0 ? args[0] : "backtest";
            if (mode == "realtime")
                Run();
            else
                Test();

            return 1;
        }
    }
}
