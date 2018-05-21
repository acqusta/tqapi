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
                public static extern IntPtr tqs_sc_mode(IntPtr h);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_register_algo", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_register_algo(IntPtr h, IntPtr algo);

                [DllImport("tqs.dll", EntryPoint = "tqs_sc_unregister_algo", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_unregister_algo(IntPtr h, IntPtr algo);

                [DllImport("tqs.dll", EntryPoint = "tqs_stralet_create", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_stralet_create(ref DotNetStralet stralet);

                [DllImport("tqs.dll", EntryPoint = "tqs_stralet_destroy", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_stralet_destroy(IntPtr stralet);


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
                public delegate IntPtr StraletCreator();

                [DllImport("tqs.dll", EntryPoint = "tqs_bt_run", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_bt_run(String cfg, StraletCreator stralet_creator);

                [DllImport("tqs.dll", EntryPoint = "tqs_rt_run", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_rt_run(String cfg, StraletCreator stralet_creator);
            }

            public enum LogLevel
            {
                INFO,
                WARNING,
                ERROR,
                FATAL
            }
        }
    }
}
