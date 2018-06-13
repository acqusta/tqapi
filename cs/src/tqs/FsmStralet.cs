using System;
using System.Collections.Generic;
using TQuant.Api;

namespace TQuant.Stralet
{
    namespace StraletEvent
    {
        public class OnInit
        {
            public static OnInit Instance { get; } = new OnInit();
        }

        public class OnFini
        {
            public static OnFini Instance { get; } = new OnFini();
        }

        public class OnQuote
        {
            public MarketQuote Quote { get; }

            public OnQuote(MarketQuote quote)
            {
                this.Quote = quote;
            }
        }

        public class OnBar
        {
            public Bar Bar { get; }
            public String Cycle { get; }

            public OnBar(String cycle, Bar bar)
            {
                this.Cycle = cycle;
                this.Bar = bar;
            }
        }

        public class OnOrder
        {
            public Order Order { get; }
            public OnOrder(Order order) { this.Order = order; }
        }

        public class OnTrade
        {
            public Trade Trade { get; }
            public OnTrade(Trade trade) { this.Trade = trade; }
        }

        internal class _OnTimer
        {
            public long Id { get; }
            public long Data { get; }

            public _OnTimer(long id, long data)
            {
                this.Id = id;
                this.Data = data;
            }
        }

        internal class _OnEvent
        {
            public string Name { get; }
            public long Data { get; }

            public _OnEvent(string name, long data)
            {
                this.Name = name;
                this.Data = data;
            }
        }
    }

    class FsmStraletWrap : TQuant.Stralet.Stralet
    {
        private IFsmStralet stralet;

        public FsmStraletWrap(IFsmStralet stralet)
        {
            this.stralet = stralet;
        }

        public override void OnEvent(Object evt)
        {
            stralet.Receive(evt);
        }

    }

    public interface IFsmStralet
    {
        //void SetContext(IStraletContext ctx);
        Stralet Stralet { get; }
        bool Receive(object message);
    }

    public interface IFsmStraletContext<TState, TDate> : IStraletContext, IFsmContext
    { }

    class FsmStraletContextImpl<TState, TData> : FsmContextImpl<TState, TData>, IStraletContext, IFsmStraletContext<TState, TData>
    {
        private FsmStralet<TState, TData> stralet;
        private IStraletContext sc { get { return stralet.stralet.Context; } }

        public FsmStraletContextImpl(FsmStralet<TState, TData> stralet) : base(stralet)
        {
            this.stralet = stralet;
        }

        public FinDataTime CurTime {  get { return sc.CurTime; } }

        public DataApi DataApi { get { return sc.DataApi; } }

        public ILoggingAdapter Logger { get { return this.sc.Logger; } }

        public string Mode {  get { return this.sc.Mode; } }
        
        public Dictionary<string, object> Props { get { return this.sc.Props; } }

        public TradeApi TradeApi { get { return this.sc.TradeApi; } }

        public int TradingDay { get { return this.sc.TradingDay; } }

        public void KillTimer(long id)
        {
            this.sc.KillTimer(id);
        }

        public void PostEvent(string evt, long data)
        {
            this.sc.PostEvent(evt, data);
        }

        public void SetTimer(long id, long delay, long data = 0)
        {
            this.sc.SetTimer(id, delay, data);
        }

        public void Stop()
        {
            this.sc.Stop();
        }
    }

    class FsmContextImpl<TState, TData> : IFsmContext
    {
        class Timer
        {
            public Timer(string name, object message, int interval, bool repeat, int id)
            {
                Repeat = repeat;
                Message = message;
                Interval = interval;
                Name = name;
                Id = id;
                OnTimer = new BaseFsm<TState, TData>.OnTimer(Name, Message);
            }

            public BaseFsm<TState, TData>.OnTimer OnTimer { get; }
            public string Name { get; }
            public object Message { get; }
            public bool Repeat { get; }
            public int Id { get; }
            public int Interval { get; }
        }

        private FsmStralet<TState, TData> stralet;
        private readonly IDictionary<long, Timer> id2timers = new Dictionary<long, Timer>();
        private readonly IDictionary<String, Timer> name2timers = new Dictionary<String, Timer>();
        private int timer_id = 0;

        private Dictionary<long, object> post_object_map = new Dictionary<long, object>();
        private long post_object_next_id = 0;

        public FsmContextImpl(FsmStralet<TState, TData> stralet)
        {
            this.stralet = stralet;
        }

        public void KillTimer(string name)
        {
            Timer timer;
            if (name2timers.TryGetValue(name, out timer))
            {
                name2timers.Remove(name);
                id2timers.Remove(timer.Id);
                this.stralet.Context.KillTimer(timer.Id);
            }
        }

        public void SetTimer(string name, object data, TimeSpan timeout, bool repeat)
        {
            Timer timer;
            if (name2timers.TryGetValue(name, out timer))
            {
                name2timers.Remove(name);
                id2timers.Remove(timer.Id);
                this.stralet.Context.KillTimer(timer.Id);
            }
            timer = new Timer(name, data, (int)timeout.TotalMilliseconds, repeat, ++this.timer_id);

            id2timers[timer.Id] = timer;
            name2timers[timer.Name] = timer;
            this.stralet.Context.SetTimer(timer.Id, timer.Interval, 0);
        }

        public bool IsTimerActive(string name)
        {
            return name2timers.ContainsKey(name);
        }

        public void ClearTimers()
        {
            foreach (var timer in this.id2timers.Values)
                this.stralet.Context.KillTimer(timer.Id);
            this.id2timers.Clear();
            this.name2timers.Clear();
        }

        public void PostEvent(object msg)
        {
            long id = ++post_object_next_id;
            post_object_map[id] = msg;

            this.stralet.Context.PostEvent("__post_evt_id__", id);
        }


        public void Stop(Reason reason)
        {
            throw new NotImplementedException();
        }

        public object Filter(object message)
        {
            if (message is StraletEvent._OnTimer)
            {
                var on_timer = message as StraletEvent._OnTimer;
                Timer timer;
                if (id2timers.TryGetValue(on_timer.Id, out timer))// && timer.Id == on_timer.Data)
                {
                    if (!timer.Repeat)
                    {
                        id2timers.Remove(timer.Id);
                        name2timers.Remove(timer.Name);
                        this.stralet.Context.KillTimer(timer.Id);
                    }

                    return timer.OnTimer;
                }
                else
                {
                    return message;
                }
            }
            else if (message is StraletEvent._OnEvent)
            {
                var on_event = message as StraletEvent._OnEvent;
                if (on_event.Name == "__post_evt_id__")
                {
                    long id = on_event.Data;
                    object real_event;
                    if (this.post_object_map.TryGetValue(id, out real_event))
                    {
                        this.post_object_map.Remove(id);
                        return real_event;
                    }
                    else
                    {
                        return null;
                    }
                }
                return message;
            }
            else
            {
                return message;
            }
        }

    }

    public class FsmStralet<TState, TData> : BaseFsm<TState, TData>, IFsmStralet
    {
        internal FsmStraletWrap stralet;
        private FsmStraletContextImpl<TState, TData> fsm_context;

        protected FsmStralet()
        {
            this.stralet = new FsmStraletWrap(this);
            this.fsm_context = new FsmStraletContextImpl<TState, TData>(this);
            base.SetFsmContext(fsm_context);
        }

        public Stralet Stralet { get { return stralet; } }

        public IFsmStraletContext<TState, TData> Context { get { return fsm_context; } }

        public bool Receive(object message)
        {
            message = fsm_context.Filter(message);
            return message != null && base.Receive(message);
        }
    }
}

