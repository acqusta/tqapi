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
        //IStraletRef Self { get; }

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
        private StraletWrap wrap;

        private StraletContextImpl ctx;

        public Stralet()
        {
            wrap = new StraletWrap(this);
        }

        internal IntPtr _Handle { get { return wrap.handle; } }

        internal void _SetContext(StraletContextImpl sc)
        {
            this.ctx = sc;
            //OnInit(sc);
        }

        internal void _OnDestroy()
        {
            this.ctx = null;
            this.wrap = null;
        }

        internal void _OnEvent(Int32 evt_id, IntPtr evt_data)
        {
            switch (evt_id)
            {
                case STRALET_EVENT_ID.ON_INIT:
                    {
                        OnEvent(StraletEvent.OnInit.Instance);
                        break;
                    }
                case STRALET_EVENT_ID.ON_FINI:
                    {
                        OnEvent(StraletEvent.OnFini.Instance);
                        break;
                    }
                case STRALET_EVENT_ID.ON_QUOTE:
                    {
                        OnEvent(new StraletEvent.OnQuote(Marshal.PtrToStructure<MarketQuote>(evt_data)));
                        break;
                    }
                case STRALET_EVENT_ID.ON_BAR:
                    {
                        var bar_wrap = Marshal.PtrToStructure<Impl.BarWrap>(evt_data);
                        OnEvent(new StraletEvent.OnBar(bar_wrap.cycle, Marshal.PtrToStructure<Bar>(bar_wrap.bar)));
                        break;
                    }
                case STRALET_EVENT_ID.ON_TIMER:
                    {
                        var timer_wrap = Marshal.PtrToStructure<Impl.TimerWrap>(evt_data);
                        OnEvent(new StraletEvent._OnTimer(timer_wrap.id, timer_wrap.data.ToInt64()));
                        break;
                    }
                case STRALET_EVENT_ID.ON_EVENT:
                    {
                        var event_wrap = Marshal.PtrToStructure<Impl.EventWrap>(evt_data);
                        OnEvent(new StraletEvent._OnEvent(event_wrap.name, event_wrap.data.ToInt64()));
                        break;
                    }
                case STRALET_EVENT_ID.ON_ORDER:
                    {
                        OnEvent(new StraletEvent.OnOrder(Marshal.PtrToStructure<Order>(evt_data)));
                        break;
                    }
                case STRALET_EVENT_ID.ON_TRADE:
                    {
                        OnEvent(new StraletEvent.OnTrade(Marshal.PtrToStructure<Trade>(evt_data)));
                        break;
                    }
                case STRALET_EVENT_ID.ON_ACCOUNT_STATUS:
                    {
                        //OnEvent(new StraletEvent.OnTrade(Marshal.PtrToStructure<Trade>(evt_data)));
                        break;
                    }
            }
        }
        public IStraletContext Context { get { return ctx; } }

        public abstract void OnEvent(Object evt);
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

        //public void AddStralet(Stralet stralet)
        //{
        //    stralet.SetContext(this);
        //}

        public String Mode { get; }
    }

    class StraletWrap
    {
        TqsDll.DotNetStralet wrap = new TqsDll.DotNetStralet();
        StraletContextImpl ctx;
        internal IntPtr handle;

        public StraletWrap(Stralet stralet)
        {
            wrap.SetContext = (sc) =>
            {
                ctx = new StraletContextImpl(sc);
                stralet._SetContext(ctx);
            };

            wrap.OnDestroy = () =>
            {
                this.handle = IntPtr.Zero;
                stralet._OnDestroy();
            };

            wrap.OnEvent = (evt, data) =>
            {
                stralet._OnEvent(evt, data);
            };

            handle = TqsDll.tqs_stralet_create(ref wrap);
        }

        ~StraletWrap()
        {
            if (handle != IntPtr.Zero)
                TqsDll.tqs_stralet_destroy(handle);
        }
    }
}
