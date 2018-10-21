using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using TQuant.Api;
using TQuant.Api.Impl;
using TQuant.Stralet.Impl;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;

namespace TQuant.Stralet
{
    public interface ILoggingAdapter
    {
        void Debug(String format, params object[] objs);
        void Info(String format, params object[] objs);
        void Warning(String format, params object[] objs);
        void Error(String format, params object[] objs);
        void Error(Exception e, String format, params object[] objs);
    }

    public interface IStraletContext
    {
        ILoggingAdapter Logger { get; }

        Dictionary<String, Object> Props { get; }

        Int32 TradingDay { get; }

        TQuant.Stralet.FinDataTime CurTime { get; }

        void PostEvent(String evt, long data);

        void SetTimer(long id, long delay, long data = 0);

        void KillTimer(long id);

        DataApi DataApi { get; }

        TradeApi TradeApi { get; }

        String Mode { get; }

        void Stop();
    }

    public abstract class Stralet
    {
        //public Stralet()
        //{
        //}

        private IStraletContext ctx;
        internal void _SetContext(IStraletContext sc)
        {
            this.ctx = sc;
        }
        public IStraletContext Context { get { return ctx; } }

        public virtual void OnInit() { }
        public virtual void OnFini() { }
        public virtual void OnQuote(MarketQuote quote) { }
        public virtual void OnBar(String cycle, Bar bar) { }
        public virtual void OnOrder(Order order) { }
        public virtual void OnTrade(Trade trade) { }
        public virtual void OnTimer(long id, long data) { }
        public virtual void OnEvent(String name, long data) { }
        public virtual void OnAccountStatus(AccountInfo account) { }

    }

    class LogginAdpterImpl : ILoggingAdapter
    {
        StraletContextImpl context;

        public LogginAdpterImpl(StraletContextImpl context)
        {
            this.context = context;
        }
        public void Debug(string format, params object[] objs)
        {
            context.Log(LogLevel.INFO, String.Format(format, objs));
        }

        public void Info(string format, params object[] objs)
        {
            context.Log(LogLevel.INFO, String.Format(format, objs));
        }

        public void Error(string format, params object[] objs)
        {
            context.Log(LogLevel.ERROR, String.Format(format, objs));
        }

        public void Error(Exception e, string format, params object[] objs)
        {
            context.Log(LogLevel.ERROR, String.Format(format, objs));
            throw e;
        }

        public void Warning(string format, params object[] objs)
        {
            context.Log(LogLevel.WARNING, String.Format(format, objs));
        }
    }

    public class StraletContextImpl : IStraletContext
    {
        private IntPtr handle;
        private int trading_day;

        public StraletContextImpl(IntPtr h)
        {
            this.handle = h;
            this.trading_day = TqsDll.tqs_sc_trading_day(h);

            IntPtr mode = TqsDll.tqs_sc_mode(this.handle);
            this.Mode = Marshal.PtrToStringAnsi(mode);

            TradeApi = new TradeApiImpl(TqsDll.tqs_sc_trade_api(this.handle), false);
            DataApi  = new DataApiImpl(TqsDll.tqs_sc_data_api(this.handle), false);

            this.Logger = new LogginAdpterImpl(this);

            IntPtr str = TqsDll.tqs_sc_get_properties(h);
            string properties = Marshal.PtrToStringAnsi(str);
            Props = JsonConvert.DeserializeObject<Dictionary<string, object>>(properties);
        }

        public Dictionary<string, object> Props { get; }

        public Int32 TradingDay { get { return trading_day; } }

        public FinDataTime CurTime { get { return TqsDll.tqs_sc_cur_time(this.handle); } }

        public void PostEvent(String evt, long data)
        {
            TqsDll.tqs_sc_post_event(this.handle, evt, new IntPtr(data));
        }

        public void SetTimer(long id, long delay, long data = 0)
        {
            TqsDll.tqs_sc_set_timer(this.handle, id, delay, new IntPtr(data));
        }

        public void KillTimer(long id)
        {
            TqsDll.tqs_sc_kill_timer(this.handle, id);
        }

        public DataApi DataApi { get; }

        public TradeApi TradeApi { get; }

        public void Log(String str)
        {
            Log(LogLevel.INFO, str);
        }

        public void Log(LogLevel level, String str)
        {
            int l = 0;
            switch (level)
            {
                case LogLevel.INFO: l = 0; break;
                case LogLevel.WARNING: l = 1; break;
                case LogLevel.ERROR: l = 2; break;
                case LogLevel.FATAL: l = 3; break;
            }
            TqsDll.tqs_sc_log(this.handle, l, str);
        }

        public ILoggingAdapter Logger { get; }

        public void Stop()
        {
            throw new NotImplementedException();
        }

        public String Mode { get; }
    }

    class StraletWrap
    {
        TqsDll.DotNetStralet wrap = new TqsDll.DotNetStralet();
        StraletContextImpl ctx;
        internal IntPtr handle;
        Stralet stralet;

        public StraletWrap(Stralet stralet)
        {
            this.stralet = stralet;

            //wrap.OnDestroy = () =>
            //{
            //    this.handle = IntPtr.Zero;
            //    //stralet._OnDestroy();
            //};

            wrap.OnEvent = (evt, data) =>
            {
                _OnEvent(evt, data);
            };

            handle = TqsDll.tqs_stralet_create(ref wrap);
        }

        ~StraletWrap()
        {
            if (handle != IntPtr.Zero)
            {
                TqsDll.tqs_stralet_destroy(handle);
                stralet = null;
            }
        }

        internal void _OnEvent(Int32 evt_id, IntPtr evt_data)
        {
            switch (evt_id)
            {
                case STRALET_EVENT_ID.ON_INIT:
                    {
                        ctx = new StraletContextImpl(evt_data);
                        stralet._SetContext(ctx);

                        stralet.OnInit();
                        break;
                    }
                case STRALET_EVENT_ID.ON_FINI:
                    {
                        stralet.OnFini();
                        stralet = null;
                        handle = IntPtr.Zero;
                        break;
                    }
                case STRALET_EVENT_ID.ON_QUOTE:
                    {
                        stralet.OnQuote(Marshal.PtrToStructure<MarketQuote>(evt_data));
                        break;
                    }
                case STRALET_EVENT_ID.ON_BAR:
                    {
                        var bar_wrap = Marshal.PtrToStructure<Impl.BarWrap>(evt_data);
                        stralet.OnBar(bar_wrap.cycle, Marshal.PtrToStructure<Bar>(bar_wrap.bar));
                        break;
                    }
                case STRALET_EVENT_ID.ON_TIMER:
                    {
                        var timer_wrap = Marshal.PtrToStructure<Impl.TimerWrap>(evt_data);
                        stralet.OnTimer(timer_wrap.id, timer_wrap.data.ToInt64());
                        break;
                    }
                case STRALET_EVENT_ID.ON_EVENT:
                    {
                        var event_wrap = Marshal.PtrToStructure<Impl.EventWrap>(evt_data);
                        stralet.OnEvent(event_wrap.name, event_wrap.data.ToInt64());
                        break;
                    }
                case STRALET_EVENT_ID.ON_ORDER:
                    {
                        stralet.OnOrder(Marshal.PtrToStructure<Order>(evt_data));
                        break;
                    }
                case STRALET_EVENT_ID.ON_TRADE:
                    {
                        stralet.OnTrade(Marshal.PtrToStructure<Trade>(evt_data));
                        break;
                    }
                case STRALET_EVENT_ID.ON_ACCOUNT_STATUS:
                    {
                        stralet.OnAccountStatus(Marshal.PtrToStructure<AccountInfo>(evt_data));
                        break;
                    }
            }
        }
    }
}
