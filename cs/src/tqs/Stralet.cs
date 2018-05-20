using System;
using System.Collections.Generic;
using TQuant.Api;
using TQuant.Api.Impl;
using TQuant.Stralet.Impl;

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

        //Props Props { get; }
        Int32 TradingDay { get; }

        TQuant.Stralet.FinDataTime CurTime { get; }

        void PostEvent(String evt, Object data);

        void SetTimer(Stralet stralet, int id, int delay, long data = 0);

        //void SetTimer(String name, Int32 delay, bool repeated, Object data = null);

        void KillTimer(Stralet stralet, int id);

        DataApi GetDataApi(String source = "");

        DataApi DataApi { get; }

        TradeApi TradeApi { get; }

        void Stop();
    }

    //public interface IStralet
    //{
    //    IStraletContext Context { get; }
    //    void OnInit(IStraletContext sc);
    //    void OnFini();
    //    void OnQuote(MarketQuote quote);
    //    void OnBar(String cycle, Bar bar);
    //    void OnTimer(Int32 id, Int64 data);
    //    void OnEvent(String evt, Object data);
    //    void OnOrderStatus(Order order);
    //    void OnOrderTrade(Trade trade);
    //    void OnAccountStatus(AccountInfo account);
    //}

    public class Stralet //: IStralet
    {
        private StraletWrap wrap;

        private StraletContextImpl ctx;

        public Stralet()
        {
            wrap = new StraletWrap(this);
        }

        internal IntPtr _Handle { get { return wrap.handle; } }

        internal void _OnInit(StraletContextImpl sc)
        {
            this.ctx = sc;
            OnInit(sc);
        }

        internal void _OnDestroy()
        {
            this.ctx.Detach();
            this.ctx = null;
            this.wrap = null;
        }
        public IStraletContext Context { get { return ctx; } }

        public virtual void OnInit(IStraletContext sc) { }
        public virtual void OnFini() { }
        public virtual void OnQuote(MarketQuote quote) { }
        public virtual void OnBar(String cycle, Bar bar) { }
        public virtual void OnTimer(Int32 id, Int64 data) { }
        public virtual void OnEvent(String evt, Object data) { }
        public virtual void OnOrderStatus(Order order) { }
        public virtual void OnOrderTrade(Trade trade) { }
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
        private Dictionary<string, DataApiImpl> dapi_map = new Dictionary<string, DataApiImpl>();
        private TradeApiImpl tapi = null;

        public StraletContextImpl(IntPtr h)
        {
            this.handle = h;
            this.trading_day = TqsDll.tqs_sc_trading_day(h);

            this.Logger = new LogginAdpterImpl(this);
        }

        internal void Detach()
        {
            foreach (var dapi in dapi_map)
            {
                dapi.Value.Detach();
            }

            dapi_map.Clear();

            if (tapi != null)
            {
                tapi.Detach();
                tapi = null;
            }
        }

        public Int32 TradingDay { get { return trading_day; } }

        public FinDataTime CurTime { get { return TqsDll.tqs_sc_cur_time(this.handle); } }

        public void PostEvent(String evt, long data)
        {
            TqsDll.tqs_sc_post_event(this.handle, evt, new IntPtr(data));
        }

        public void SetTimer(Stralet stralet, Int32 id, Int32 delay, long data)
        {
            TqsDll.tqs_sc_set_timer(this.handle, stralet._Handle, id, delay, new IntPtr(data));
        }

        public void KillTimer(Stralet stralet, Int32 id)
        {
            TqsDll.tqs_sc_kill_timer(this.handle, stralet._Handle, id);
        }

        public DataApi GetDataApi(String source = "")
        {
            if (source == null) source = "";

            if (dapi_map.ContainsKey(source))
                return dapi_map[source];

            var h = TqsDll.tqs_sc_data_api(this.handle, source);
            if (h != IntPtr.Zero)
            {
                var dapi = new DataApiImpl(this, h);
                dapi_map[source] = dapi;
                return dapi;
            }
            else
            {
                return null;
            }
        }

        public DataApi DataApi
        {
            get
            {
                return GetDataApi();
            }
        }

        public TradeApi TradeApi
        {
            get
            {
                if (tapi != null) return tapi;

                var h = TqsDll.tqs_sc_trade_api(this.handle);
                if (h != IntPtr.Zero)
                    tapi = new TradeApiImpl(null, h);

                return tapi;
            }
        }

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

        public void PostEvent(string evt, object data)
        {
            throw new NotImplementedException();
        }
    }

    class StraletWrap
    {
        TqsDll.DotNetStralet wrap = new TqsDll.DotNetStralet();
        StraletContextImpl ctx;
        internal IntPtr handle;

        Object GetObjectByID(IntPtr id)
        {
            return null;
        }

        public StraletWrap(Stralet stralet)
        {
            wrap.OnInit = (sc) =>
            {
                ctx = new StraletContextImpl(sc);
                stralet._OnInit(ctx);
            };
            wrap.OnDestroy = () =>
            {
                this.handle = IntPtr.Zero;
                stralet._OnDestroy();
            };

            wrap.OnFini = stralet.OnFini;
            wrap.OnQuote = stralet.OnQuote;
            wrap.OnBar = stralet.OnBar;
            wrap.OnTimer = (id, data) =>
            {
                stralet.OnTimer(id, data.ToInt64());
            };

            wrap.OnEvent = (evt, data) =>
            {
                stralet.OnEvent(evt, GetObjectByID(data));
            };

            wrap.OnOrderStatus = stralet.OnOrderStatus;
            wrap.OnTrade = stralet.OnOrderTrade;
            wrap.OnAccountStatus = stralet.OnAccountStatus;

            handle = TqsDll.tqs_stralet_create(ref wrap);
        }

        ~StraletWrap()
        {
            if (handle != IntPtr.Zero)
                TqsDll.tqs_stralet_destroy(handle);
        }
    }
}
