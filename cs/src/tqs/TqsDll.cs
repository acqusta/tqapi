﻿using System;
using System.Runtime.InteropServices;

namespace TQuant
{
    namespace Stralet
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct FinDateTime
        {
            public Int32 Date;
            public Int32 Time;

            public FinDateTime(Int32 Date, Int32 Time)
            {
                this.Date = Date;
                this.Time = Time;
            }

            public FinDateTime(DateTime dt)
            {
                this.Date = dt.Year * 10000 + dt.Month * 100 + dt.Day;
                this.Time = (dt.Hour * 10000 + dt.Minute * 100 + dt.Second) * 1000 + dt.Millisecond;
            }

            public DateTime AsDateTime()
            {
                int y = Date / 10000;
                int m = (Date / 100) % 100;
                int d = Date % 100;
                int MS = Time % 1000;
                Time /= 1000;
                int H = Time / 10000;
                int M = (Time / 100)% 100;
                int S = Time % 100;
                return new DateTime(y, m, d, H, M, S, MS);
            }
        }

        namespace Impl
        {
            //[StructLayout(LayoutKind.Sequential)]
            //struct TimerWrap
            //{
            //    public Int64  id;
            //    public IntPtr data;
            //};

            //[StructLayout(LayoutKind.Sequential)]
            //struct EventWrap
            //{
            //    public string name;
            //    public IntPtr data;
            //};

            //[StructLayout(LayoutKind.Sequential)]
            //struct BarWrap
            //{
            //    public string cycle;
            //    public IntPtr bar;
            //};

            //class STRALET_EVENT_ID
            //{
            //    public const int ZERO_ID  = 0;
            //    public const int ON_INIT  = 1;
            //    public const int ON_FINI  = 2;
            //    public const int ON_QUOTE = 3;
            //    public const int ON_BAR   = 4;
            //    public const int ON_TIMER = 5;
            //    public const int ON_EVENT = 6;
            //    public const int ON_ORDER = 7;
            //    public const int ON_TRADE = 8;
            //    public const int ON_ACCOUNT_STATUS = 9;
            //};

            class TqsDll
            {
                [DllImport("tqapi", EntryPoint = "tqs_sc_trading_day", CallingConvention =CallingConvention.Cdecl)]
                public static extern Int32 tqs_sc_trading_day(IntPtr h);

                [DllImport("tqapi", EntryPoint = "tqs_sc_cur_time", CallingConvention = CallingConvention.Cdecl)]
                public static extern FinDateTime tqs_sc_cur_time(IntPtr h);

                [DllImport("tqapi", EntryPoint = "tqs_sc_post_event", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_post_event(IntPtr h, String evt, IntPtr data);

                [DllImport("tqapi", EntryPoint = "tqs_sc_set_timer", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_set_timer(IntPtr h, Int64 id, Int64 delay, IntPtr data);

                [DllImport("tqapi", EntryPoint = "tqs_sc_kill_timer", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_kill_timer(IntPtr h, Int64 id);

                [DllImport("tqapi", EntryPoint = "tqs_sc_data_api", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_data_api(IntPtr h);

                [DllImport("tqapi", EntryPoint = "tqs_sc_trade_api", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_trade_api(IntPtr h);

                [DllImport("tqapi", EntryPoint = "tqs_sc_log", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_sc_log(IntPtr h, Int32 severity, String str);

                [DllImport("tqapi", EntryPoint = "tqs_sc_get_properties", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_get_properties(IntPtr h);

                [DllImport("tqapi", EntryPoint = "tqs_sc_mode", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_sc_mode(IntPtr h);

                [DllImport("tqapi", EntryPoint = "tqs_stralet_create", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_stralet_create(ref DotNetStralet stralet);

                [DllImport("tqapi", EntryPoint = "tqs_stralet_destroy", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqs_stralet_destroy(IntPtr stralet);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnDestroy();

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletSetContext(IntPtr sc);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnInit(IntPtr sc);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnFini ();

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnQuote(IntPtr q);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnBar  (string cycle, IntPtr bar);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnOrder(IntPtr p);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnTrade(IntPtr p);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnTimer(Int64 id, IntPtr data);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnEvent(string name, IntPtr data);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void StraletOnAccountStatus(IntPtr p);

                [StructLayout(LayoutKind.Sequential)]
                public struct DotNetStralet
                {
                    public StraletOnInit            OnInit           ;
                    public StraletOnFini            OnFini           ;
                    public StraletOnQuote           OnQuote          ;
                    public StraletOnBar             OnBar            ;
                    public StraletOnOrder           OnOrder          ;
                    public StraletOnTrade           OnTrade          ;
                    public StraletOnTimer           OnTimer          ;
                    public StraletOnEvent           OnEvent          ;
                    public StraletOnAccountStatus   OnAccountStatus  ;
                }

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate IntPtr StraletCreator();

                [DllImport("tqapi", EntryPoint = "tqs_bt_run", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_bt_run(String cfg, StraletCreator stralet_creator);

                [DllImport("tqapi", EntryPoint = "tqs_rt_run", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tqs_rt_run(String cfg, StraletCreator stralet_creator);
            }

            public enum LogSeverity
            {
                INFO,
                WARNING,
                ERROR,
                FATAL
            }
        }
    }
}
