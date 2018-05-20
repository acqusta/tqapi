using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using TQuant.Api;
using TQuant.Api.Impl;

namespace TQuant
{
    namespace Stralet
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct FinDataTime
        {
            public Int32 date;
            public Int32 time;
        }

        namespace Impl
        {
            class TqsDll
            {
                [DllImport("tqs.dll", EntryPoint = "tqs_sc_trading_day", CallingConvention =CallingConvention.Cdecl)]
                public static extern Int32 tqs_sc_trading_day(IntPtr h);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_cur_time", CallingConvention = CallingConvention.Cdecl)]
                public static extern FinDataTime tqs_sc_cur_time(IntPtr h);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_post_event", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqapi_get_data_api(IntPtr h, String source);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_post_event", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_post_event(IntPtr h, String evt, IntPtr data);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_set_timer", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_set_timer(IntPtr h, IntPtr stralet, Int32 id, Int32 delay, IntPtr data);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_data_api", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_kill_timer(IntPtr h, IntPtr stralet, Int32 id);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_data_api", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_data_api(IntPtr h, String source);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_trade_api", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_trade_api(IntPtr h);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_log", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_log(IntPtr h, Int32 level, String str);

                [DllImport("tqs.dll", EntryPoint = "tqs_get_parameter", CallingConvention = CallingConvention.Cdecl)]
                public static extern String tqs_get_parameter(IntPtr h, String name, String def_value);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_mode", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_mode(IntPtr h);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_register_algo", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_register_algo(IntPtr h, IntPtr algo);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_unregister_algo", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_unregister_algo(IntPtr h, IntPtr algo);

                [DllImport("tqs.dll", EntryPoint = "tqs_stralet_create", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_stralet_create(ref DotNetStralet stralet);

                [DllImport("tqs.dll", EntryPoint = "tqs_stralet_destroy", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_stralet_destroy(IntPtr stralet);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_register_stralet", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_bt_register_stralet(IntPtr h, IntPtr algo);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_unregister_algo", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_bt_unregister_algo(IntPtr h, IntPtr algo);


                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnDestroy();

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnInit(IntPtr sc);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnFini();

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnQuote(MarketQuote quote);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnBar(String cycle, Bar bar);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnTimer(Int32 id, IntPtr data);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnEvent(String evt, IntPtr data);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnOrderStatus(Order status);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnOrderTrade(Trade trade);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnAccountStatus(AccountInfo info);

                [StructLayout(LayoutKind.Sequential)]
                public struct DotNetStralet
                {
                    public StraletOnDestroy         OnDestroy;
                    public StraletOnInit            OnInit;
                    public StraletOnFini            OnFini;
                    public StraletOnQuote           OnQuote;
                    public StraletOnBar             OnBar;
                    public StraletOnTimer           OnTimer;
                    public StraletOnEvent           OnEvent;
                    public StraletOnOrderStatus     OnOrderStatus;
                    public StraletOnOrderTrade      OnTrade;
                    public StraletOnAccountStatus   OnAccountStatus;
                }

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate IntPtr BTRunCreateStralet();

                [DllImport("tqs.dll", EntryPoint = "tqs_bt_run", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_bt_run(String cfg, BTRunCreateStralet create_stralet);
            }

            public enum LogLevel
            {
                INFO,
                WARNING,
                ERROR,
                FATAL
            }

            //public class StraletContext
            //{
            //    IntPtr handle;
            //    int trading_day;

            //    Dictionary<string, DataApiImpl> dapi_map = new Dictionary<string, DataApiImpl>();
            //    TradeApiImpl tapi = null;

            //    public StraletContext(IntPtr h)
            //    {
            //        this.handle = h;
            //        this.trading_day = TqsDll.tqs_sc_trading_day(h);
            //        TqsDll.tqs_sc_cur_time(h);
            //    }

            //    internal void Detach()
            //    {
            //        foreach (var dapi in dapi_map)
            //        {
            //            dapi.Value.Detach();
            //        }

            //        dapi_map.Clear();

            //        if (tapi != null)
            //        {
            //            tapi.Detach();
            //            tapi = null;
            //        }
            //    }

            //    public Int32 TradingDay { get { return trading_day; } }
            //    public FinDataTime CurTime { get { return TqsDll.tqs_sc_cur_time(this.handle); } }

            //    public void PostEvent(String evt, long data)
            //    {
            //        TqsDll.tqs_sc_post_event(this.handle, evt, new IntPtr(data));
            //    }

            //    public void SetTimer(Stralet stralet, Int32 id, Int32 delay, long data)
            //    {
            //        TqsDll.tqs_sc_set_timer(this.handle, stralet._Handle, id, delay, new IntPtr(data));
            //    }

            //    public void KillTimer(Stralet stralet, Int32 id)
            //    {
            //        TqsDll.tqs_sc_kill_timer(this.handle, stralet._Handle, id);
            //    }

            //    public DataApi GetDataApi(String source = "")
            //    {
            //        if (source == null) source = "";

            //        if (dapi_map.ContainsKey(source))
            //            return dapi_map[source];

            //        var h = TqsDll.tqs_sc_data_api(this.handle, source);
            //        if (h != IntPtr.Zero)
            //        {
            //            var dapi = new DataApiImpl(this, h);
            //            dapi_map[source] = dapi;
            //            return dapi;
            //        }
            //        else
            //        {
            //            return null;
            //        }
            //    }

            //    public DataApi DataApi
            //    {
            //        get
            //        {
            //            return GetDataApi();
            //        }
            //    }

            //    public TradeApi TradeApi
            //    {
            //        get
            //        {
            //            if (tapi != null) return tapi;

            //            var h = TqsDll.tqs_sc_trade_api(this.handle);
            //            if (h != IntPtr.Zero)
            //                tapi = new TradeApiImpl(null, h);

            //            return tapi;
            //        }
            //    }

            //    public void Log(String str)
            //    {
            //        Log(LogLevel.INFO, str);
            //    }

            //    public void Log(LogLevel level, String str)
            //    {
            //        int l = 0;
            //        switch (level)
            //        {
            //            case LogLevel.INFO: l = 0; break;
            //            case LogLevel.WARNING: l = 1; break;
            //            case LogLevel.ERROR: l = 2; break;
            //            case LogLevel.FATAL: l = 3; break;
            //        }
            //        TqsDll.tqs_sc_log(this.handle, l, str);
            //    }

            //    //virtual string get_parameter(const char* name, const char* def_value) = 0;

            //    //        virtual string mode() = 0;

            //    //virtual void register_algo(AlgoStralet* algo) = 0;
            //    //virtual void unregister_algo(AlgoStralet* algo) = 0;

            //}



        }
    }
}
