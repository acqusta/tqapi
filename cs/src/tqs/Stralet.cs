using System;
using TQuant.Api;
using TQuant.Stralet.Impl;

namespace TQuant
{
    namespace Stralet
    {
        public enum LogLevel
        {
            INFO,
            WARNING,
            ERROR,
            FATAL
        }

        public class StraletContext
        {
            IntPtr handle;
            int trading_day;

            public StraletContext(IntPtr h)
            {
                this.handle = h;
                this.trading_day = TqsDll.tqs_sc_trading_day(h);
            }

            public Int32 TradingDay { get { return trading_day; } }
            public DateTime CurTime { get { return TqsDll.tqs_sc_cur_time(this.handle); } }
            //public DateTime CurTimeAsSysDT { }

            IntPtr SaveEventData(Object data)
            {
                return IntPtr.Zero;
            }

            Object GetEventData(IntPtr ptr)
            {
                return null;
            }

            IntPtr SaveTimerData(Int32 timer_id, Object data)
            {
                return IntPtr.Zero;
            }

            Object GetTimerData(Int32 timer_id, IntPtr ptr)
            {
                return null;
            }

            void RemoveTimerData(Int32 timer_id)
            { }

            public void PostEvent(String evt, Object data)
            {
                TqsDll.tqs_sc_post_event(this.handle, evt, SaveEventData(data));
            }

            public void SetTimer(Stralet stralet, Int32 id, Int32 delay, Object data)
            {
                TqsDll.tqs_sc_set_timer(this.handle, stralet._Handle, id, delay, SaveTimerData(id, data));
            }

            public void KillTimer(Stralet stralet, Int32 id)
            {
                TqsDll.tqs_sc_kill_timer(this.handle, stralet._Handle, id);
                RemoveTimerData(id);
            }

            public DataApi DataApi(String source)
            {
                return null;
            }

            public TradeApi TradeApi
            {
                get
                {
                    return null;
                }
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

            //virtual string get_parameter(const char* name, const char* def_value) = 0;

            //        virtual string mode() = 0;

            //virtual void register_algo(AlgoStralet* algo) = 0;
            //virtual void unregister_algo(AlgoStralet* algo) = 0;

        }

        public class Stralet
        {
            StraletWrap wrap;
            StraletContext ctx;

            public Stralet()
            {
                wrap = new StraletWrap(this);
            }

            internal IntPtr _Handle { get { return wrap.handle; } }

            internal void _OnInit(StraletContext sc)
            {
                this.ctx = sc;
                OnInit(sc);
            }

            internal void _OnFini()
            {
                OnFini();
                this.ctx = null;
                this.wrap = null;
            }

            public StraletContext Context { get { return ctx; } }

            public virtual void OnInit(StraletContext sc) { }
            public virtual void OnFini() { }
            public virtual void OnQuote(MarketQuote quote) { }
            public virtual void OnBar(Bar bar) { }
            public virtual void OnTimer(Int32 id, Object data) { }
            public virtual void OnEvent(String evt, Object data) { }
            public virtual void OnOrderStatus(OrderStatus order) { }
            public virtual void OnOrderTrade(Trade trade) { }
            public virtual void OnAccountStatus(AccountInfo account) { }
        }

        class StraletWrap
        {
            TqsDll.DotNetStralet wrap = new TqsDll.DotNetStralet();
            StraletContext ctx;
            internal IntPtr handle;

            Object GetObjectByID(IntPtr id)
            {
                return null;
            }

            public StraletWrap(Stralet stralet)
            {
                wrap.OnInit = (sc) =>
                {
                    ctx = new StraletContext(sc);
                    stralet._OnInit(ctx);
                };
                wrap.OnFini = stralet._OnFini;
                wrap.OnQuote = stralet.OnQuote;
                wrap.OnBar = stralet.OnBar;
                wrap.OnTimer = (id, data) =>
                {
                    stralet.OnTimer(id, GetObjectByID(data));
                };

                wrap.OnEvent = (evt, data) =>
                {
                    stralet.OnEvent(evt, GetObjectByID(data));
                };

                wrap.OnOrderStatus = stralet.OnOrderStatus;
                wrap.OnTrade = stralet.OnOrderTrade;
                wrap.OnAccountStatus = stralet.OnAccountStatus;

                handle = TqsDll.tqs_create_stralet(wrap);
            }

            ~StraletWrap()
            {
                TqsDll.tqs_destroy_stralet(handle);
            }
        }
    }
}
