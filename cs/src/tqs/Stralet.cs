using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using TQuant.Api;
using TQuant.Api.Impl;
using TQuant.Stralet.Impl;
//using Newtonsoft.Json.Linq;
//using Newtonsoft.Json;
using System.Text.Json;

namespace TQuant.Stralet
{
    public interface ILogger
    {
        void Info(String format, params object[] objs);
        void Warn(String format, params object[] objs);
        void Error(String format, params object[] objs);
        void Fatal(String format, params object[] objs);
    }

    public interface IStraletContext
    {
        ILogger Logger { get; }

        Dictionary<String, Object> Props { get; }

        Int32 TradingDay { get; }

        TQuant.Stralet.FinDateTime CurTime { get; }

        DateTime CurDateTime { get; }

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
        private IStraletContext ctx;
        internal void _SetContext(IStraletContext sc)
        {
            this.ctx = sc;
        }
        public IStraletContext Context { get { return ctx; } }

        public virtual void OnInit() { }
        public virtual void OnFini() { }
        public virtual void OnQuote  (MarketQuote quote) { }
        public virtual void OnBar    (String cycle, Bar bar) { }
        public virtual void OnOrder  (Order order) { }
        public virtual void OnTrade  (Trade trade) { }
        public virtual void OnTimer  (long id, long data) { }
        public virtual void OnEvent  (String name, long data) { }
        public virtual void OnAccountStatus(AccountInfo account) { }

    }

    class LoggerImpl : ILogger
    {
        StraletContextImpl context;

        public LoggerImpl(StraletContextImpl context)
        {
            this.context = context;
        }
        public void Debug(string format, params object[] objs)
        {
            context.Log(LogSeverity.INFO, String.Format(format, objs));
        }

        public void Info(string format, params object[] objs)
        {
            context.Log(LogSeverity.INFO, String.Format(format, objs));
        }

        public void Error(string format, params object[] objs)
        {
            context.Log(LogSeverity.ERROR, String.Format(format, objs));
        }

        public void Warn(string format, params object[] objs)
        {
            context.Log(LogSeverity.WARNING, String.Format(format, objs));
        }

        public void Fatal(string format, params object[] objs)
        {
            string msg = String.Format(format, objs);
            context.Log(LogSeverity.ERROR, msg);
            throw new Exception(msg);
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

            this.Logger = new LoggerImpl(this);

            IntPtr str = TqsDll.tqs_sc_get_properties(h);
            string properties = Marshal.PtrToStringAnsi(str);
            //Props = JsonConvert.DeserializeObject<Dictionary<string, object>>(properties);
            Props = JsonSerializer.Deserialize<Dictionary<string, object>>(properties);
        }

        public Dictionary<string, object> Props { get; }

        public Int32 TradingDay { get { return trading_day; } }

        public FinDateTime CurTime     { get { return TqsDll.tqs_sc_cur_time(this.handle); } }
        public DateTime    CurDateTime { get { return TqsDll.tqs_sc_cur_time(this.handle).AsDateTime();} }

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
            Log(LogSeverity.INFO, str);
        }

        public void Log(LogSeverity severity, String str)
        {
            int l = 0;
            switch (severity)
            {
                case LogSeverity.INFO: l = 0; break;
                case LogSeverity.WARNING: l = 1; break;
                case LogSeverity.ERROR: l = 2; break;
                case LogSeverity.FATAL: l = 3; break;
            }
            TqsDll.tqs_sc_log(this.handle, l, str);
        }

        public ILogger Logger { get; }

        public void Stop()
        {
            throw new NotImplementedException();
        }

        public String Mode { get; }
    }

    class StraletWrap
    {
        TqsDll.DotNetStralet wrap = new TqsDll.DotNetStralet();
        //StraletContextImpl ctx;
        internal IntPtr handle;
        Stralet stralet;

        public StraletWrap(Stralet stralet)
        {
            this.stralet = stralet;

            wrap.OnInit           = this.OnInit          ;
            wrap.OnFini           = this.OnFini          ;
            wrap.OnQuote          = this.OnQuote         ;
            wrap.OnBar            = this.OnBar           ;
            wrap.OnOrder          = this.OnOrder         ;
            wrap.OnTrade          = this.OnTrade         ;
            wrap.OnTimer          = this.OnTimer         ;
            wrap.OnEvent          = this.OnEvent         ;
            wrap.OnAccountStatus  = this.OnAccountStatus ;

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

        internal void OnInit(IntPtr ctx)
        {
            stralet._SetContext(new StraletContextImpl(ctx));
            stralet.OnInit();
        }

        internal void OnFini()
        {
            stralet.OnFini();
            stralet = null;
            handle = IntPtr.Zero;
        }

        internal void OnQuote(IntPtr p)
        {
            stralet.OnQuote(Marshal.PtrToStructure<MarketQuote>(p));
        }

        internal void OnBar(string cycle, IntPtr p)
        {
            //var bar_wrap = Marshal.PtrToStructure<Impl.BarWrap>(p);
            stralet.OnBar(cycle, Marshal.PtrToStructure<Bar>(p));
        }

        internal void OnOrder(IntPtr p)
        {
            stralet.OnOrder(Marshal.PtrToStructure<Order>(p));
        }

        internal void OnTrade(IntPtr p)
        {
            stralet.OnTrade(Marshal.PtrToStructure<Trade>(p));
        }

        internal void OnTimer(Int64 id, IntPtr data)
        {
            //var timer_wrap = Marshal.PtrToStructure<Impl.TimerWrap>(evt_data);
            stralet.OnTimer(id, data.ToInt64());
        }

        internal void OnEvent(string name, IntPtr data)
        {
            //var event_wrap = Marshal.PtrToStructure<Impl.EventWrap>(evt_data);
            stralet.OnEvent(name, data.ToInt64());
        }

        internal void OnAccountStatus(IntPtr p)
        {
            stralet.OnAccountStatus(Marshal.PtrToStructure<AccountInfo>(p));
        }
    }
}
