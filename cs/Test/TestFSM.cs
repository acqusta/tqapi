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
        Initing,
        Locked,
        UnLocked,
        Stopped
    }

    public class ExampleFSMStralet : FsmStralet<State, IData>
    {
        public ExampleFSMStralet()
        {
            StartWith(State.Idle, Uninitialized.Instance);

            When(State.Idle, IdleFunctions);

            When(State.Initing, InitingFunctions);

            WhenUnhandled(UnhandledFunction);

            Initialize();
        }

        public State<State, IData> IdleFunctions(Event<IData> evt)
        {
            if (evt.FsmEvent is OnInit)
            {
                Context.DataApi.Subscribe(new String[] { "000001.SH" });
                SetTimer("CheckTimer", "checktime", TimeSpan.FromSeconds(60), true);
                return GoTo(State.Initing);
            }

            return null;
            
        }

        public State<State, IData> InitingFunctions(Event<IData> evt)
        {
            if (evt.FsmEvent is Timer)
            {
                var timer = evt.FsmEvent as Timer;
                Context.Logger.Info("OnTimer {0}", timer.Name);
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

        public static int Main(string[] args)
        {
            TQuant.Stralet.BackTestConfig cfg = new TQuant.Stralet.BackTestConfig();
            cfg.data_level = "tk";
            cfg.begin_date = 20180101;
            cfg.end_date   = 20180501;

            BackTest.Run(cfg, CreateStralet);
            return 1;
        }
    }
}
