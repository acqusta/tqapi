using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace TQuant
{
    namespace Api
    {
        namespace Impl
        {
            class TqapiDll
            {
                public struct CallResultWrap
                {
                    public String msg;
                    public IntPtr value;
                    public Int32 element_size;
                    public Int32 element_count;
                    public Int32 value_type;
                }

                [DllImport("tqapi.dll", EntryPoint = "tqapi_set_params", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tqapi_set_params(String key, String value);

                [DllImport("tqapi.dll", EntryPoint = "dapi_create", CallingConvention =CallingConvention.Cdecl)]
                public static extern IntPtr dapi_create(String addr);

                [DllImport("tqapi.dll", EntryPoint = "dapi_destroy", CallingConvention = CallingConvention.Cdecl)]
                public static extern void dapi_destroy(IntPtr h);

                [DllImport("tqapi.dll", EntryPoint = "tapi_create", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_create(String addr);

                [DllImport("tqapi.dll", EntryPoint = "tapi_destroy", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tapi_destroy(IntPtr h);

                [DllImport("tqapi.dll", EntryPoint = "dapi_get_bar", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr dapi_get_bar(IntPtr h, String code, String cycle,
                                                         Int32 trading_day, Boolean align);

                [DllImport("tqapi.dll", EntryPoint = "dapi_get_daily_bar", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr dapi_get_daily_bar(IntPtr h, String code, String cycle,
                                                               Boolean align);

                [DllImport("tqapi.dll", EntryPoint = "dapi_get_quote", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr dapi_get_quote(IntPtr h, String code);

                [DllImport("tqapi.dll", EntryPoint = "dapi_get_tick", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr dapi_get_tick(IntPtr h, String code, Int32 trading_day);

                [DllImport("tqapi.dll", EntryPoint = "destroy_callresult", CallingConvention = CallingConvention.Cdecl)]
                public static extern void destroy_callresult(IntPtr h);

                [DllImport("tqapi.dll", EntryPoint = "dapi_subscribe", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr dapi_subscribe(IntPtr h, String codes);

                [DllImport("tqapi.dll", EntryPoint = "dapi_unsubscribe", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr dapi_unsubscribe(IntPtr h, String codes);

                [DllImport("tqapi.dll", EntryPoint = "tapi_query_balance", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_query_balance(IntPtr h, String account_id);

                [DllImport("tqapi.dll", EntryPoint = "tapi_query_positions", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_query_positions(IntPtr h, String account_id, String codes);

                [DllImport("tqapi.dll", EntryPoint = "tapi_query_orders", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_query_orders(IntPtr h, String account_id, String codes);

                [DllImport("tqapi.dll", EntryPoint = "tapi_query_trades", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_query_trades(IntPtr h, String account_id, String codes);

                [DllImport("tqapi.dll", EntryPoint = "tapi_query_account_status", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_query_account_status(IntPtr h);

                [DllImport("tqapi.dll", EntryPoint = "tapi_query", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_query(IntPtr h, String account_id, String command, String data);

                [DllImport("tqapi.dll", EntryPoint = "tapi_place_order", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_place_order(IntPtr h, String account_id,
                    String code, Double price, Int64 size, String action, String price_type, Int32 order_id);

                [DllImport("tqapi.dll", EntryPoint = "tapi_cancel_order1", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_cancel_order1(IntPtr h, String account_id,
                    String code, String entrust_no, Int32 order_id);

                [DllImport("tqapi.dll", EntryPoint = "tapi_cancel_order2", CallingConvention = CallingConvention.Cdecl)]
                public static extern IntPtr tapi_cancel_order2(IntPtr h, String account_id,
                    String code, Int32 order_id);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void DataApiOnMarketQuote(MarketQuote quote);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void DataApiOnBar(String cycle, Bar bar);

                [DllImport("tqapi.dll", EntryPoint = "dapi_set_callback", CallingConvention = CallingConvention.Cdecl)]
                public static extern void dapi_set_callback(IntPtr h, DataApiOnMarketQuote on_quote, DataApiOnBar on_bar);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void TradeApiOnOrderStatus(Order order);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void TradeApiOnOrderTrade(Trade trade);

                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate void TradeApiOnAccountStatus(AccountInfo account);

                [DllImport("tqapi.dll", EntryPoint = "tapi_set_callback", CallingConvention = CallingConvention.Cdecl)]
                public static extern void tapi_set_callback(IntPtr h,
                                                            TradeApiOnOrderStatus on_order_status,
                                                            TradeApiOnOrderTrade on_order_trade,
                                                            TradeApiOnAccountStatus on_account_status);

                public static T[] CopyArray<T>(CallResultWrap r)
                {
                    T[] ar = new T[r.element_count];
                    for (int i = 0; i < r.element_count; i++)
                    {
                        IntPtr ins = new IntPtr(r.value.ToInt64() + i * r.element_size);
                        ar[i] = Marshal.PtrToStructure<T>(ins);
                    }
                    return ar;
                }
            }

            class DataApiImpl : DataApi
            {
                IntPtr handle;
                bool is_owner;

                TqapiDll.DataApiOnMarketQuote on_quote;
                TqapiDll.DataApiOnBar on_bar;

                public event OnMarketQuoteHandler OnMarketQuote;

                public event OnBarHandler OnBar;

                public DataApiImpl(IntPtr handle, bool is_owner)
                {
                    this.is_owner = is_owner;
                    this.handle = handle;
                    on_quote = (quote)      => { if (OnMarketQuote != null) OnMarketQuote(quote); };
                    on_bar   = (cycle, bar) => { if (OnBar != null) OnBar(cycle, bar);};

                    TqapiDll.dapi_set_callback(this.handle, on_quote, on_bar);
                }

                ~DataApiImpl()
                {
                    if (this.is_owner && this.handle != IntPtr.Zero)
                    {
                        TqapiDll.dapi_set_callback(this.handle, null, null);
                        TqapiDll.dapi_destroy(this.handle);
                        this.handle = IntPtr.Zero;
                    }
                }

                public CallResult<Bar[]> GetBar(string code, string cycle, int trading_day, bool align)
                {
                    IntPtr r = TqapiDll.dapi_get_bar(handle, code, cycle, trading_day, align);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<Bar[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<Bar[]>(TqapiDll.CopyArray<Bar>(cr));
                    else
                        ret = new CallResult<Bar[]>(cr.msg);
                    
                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<DailyBar[]> GetDailyBar(string code, string price_adj, bool align)
                {
                    IntPtr r = TqapiDll.dapi_get_daily_bar(handle, code, price_adj, align);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<DailyBar[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<DailyBar[]>(TqapiDll.CopyArray<DailyBar>(cr));
                    else
                        ret = new CallResult<DailyBar[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<MarketQuote> GetQuote(string code)
                {
                    IntPtr r = TqapiDll.dapi_get_quote(handle, code);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<MarketQuote> ret;
                    if (cr.value != IntPtr.Zero)
                        ret = new CallResult<MarketQuote>(Marshal.PtrToStructure<MarketQuote>(cr.value));
                    else
                        ret = new CallResult<MarketQuote>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<MarketQuote[]> GetTick(string code, int trading_day)
                {
                    IntPtr r = TqapiDll.dapi_get_tick(handle, code, trading_day);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<MarketQuote[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<MarketQuote[]>(TqapiDll.CopyArray<MarketQuote>(cr));
                    else
                        ret = new CallResult<MarketQuote[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<string[]> Subscribe(string[] codes)
                {
                    string str = "";
                    if (codes != null && codes.Length > 0)
                    {
                        foreach (var s in codes) str += s + ",";
                    }

                    IntPtr r = TqapiDll.dapi_subscribe(handle, str);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<string[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<string[]>(TqapiDll.CopyArray<string>(cr));
                    else
                        ret = new CallResult<string[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<string[]> UnSubscribe(string[] codes)
                {
                    string str = "";
                    if (codes != null && codes.Length > 0)
                    {
                        foreach (var s in codes) str += s + ",";
                    }

                    IntPtr r = TqapiDll.dapi_unsubscribe(handle, str);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);
                    CallResult<string[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<string[]>(TqapiDll.CopyArray<string>(cr));
                    else
                        ret = new CallResult<string[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }
            }

            class TradeApiImpl : TradeApi
            {
                IntPtr handle;
                bool is_owner;

                TqapiDll.TradeApiOnOrderStatus on_order_status;
                TqapiDll.TradeApiOnOrderTrade on_order_trade;
                TqapiDll.TradeApiOnAccountStatus on_account_status;

                public event OnOrderStatusHandler OnOrderStatus;
                public event OnOrderTradeHandler OnOrderTrade;
                public event OnAccountStatusHandler OnAccountStatus;

                public TradeApiImpl(IntPtr handle, bool is_owner)
                {                    
                    this.handle = handle;
                    this.is_owner = is_owner;
                    on_order_status = (order)     => { if (OnOrderStatus != null) OnOrderStatus(order); };
                    on_order_trade = (trade)      => { if (OnOrderTrade  != null) OnOrderTrade(trade); };
                    on_account_status = (account) => { if (OnAccountStatus != null) OnAccountStatus(account); };

                    TqapiDll.tapi_set_callback(this.handle, on_order_status, on_order_trade, on_account_status);
                }

                ~TradeApiImpl()
                {
                    if (this.is_owner && this.handle != IntPtr.Zero)
                    {
                        TqapiDll.tapi_set_callback(this.handle, null, null, null);
                        TqapiDll.tapi_destroy(this.handle);
                        this.handle = IntPtr.Zero;
                    }
                }

                public CallResult<bool> CancelOrder(string account_id, string code, int order_id)
                {
                    IntPtr r = TqapiDll.tapi_cancel_order2(this.handle, account_id, code, order_id);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<bool> ret;
                    if (cr.value != IntPtr.Zero)
                        ret = new CallResult<bool>(Marshal.ReadInt32(cr.value) == 1);
                    else
                        ret = new CallResult<bool>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<bool> CancelOrder(string account_id, string code, string entrust_no, int order_id = 0)
                {
                    IntPtr r = TqapiDll.tapi_cancel_order1(this.handle, account_id, code, entrust_no, order_id);
                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<bool> ret;
                    if (cr.value != IntPtr.Zero)
                        ret = new CallResult<bool>(Marshal.ReadByte(cr.value) != 0);
                    else
                        ret = new CallResult<bool>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<OrderID> PlaceOrder(string account_id, string code, double price, long size, string action, string price_type, int order_id)
                {
                    IntPtr r = TqapiDll.tapi_place_order(this.handle, account_id,
                        code, price, size, action, price_type, order_id);

                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<OrderID> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<OrderID>(Marshal.PtrToStructure<OrderID>(cr.value));
                    else
                        ret = new CallResult<OrderID>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;

                }

                public CallResult<string> Query(string account_id, string command, string data)
                {
                    IntPtr r = TqapiDll.tapi_query(this.handle, account_id, command, data);

                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<string> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<string>(Marshal.PtrToStringAuto(cr.value));
                    else
                        ret = new CallResult<string>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<AccountInfo[]> QueryAccountStatus()
                {
                    IntPtr r = TqapiDll.tapi_query_account_status(this.handle);

                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<AccountInfo[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<AccountInfo[]>(TqapiDll.CopyArray<AccountInfo>(cr));
                    else
                        ret = new CallResult<AccountInfo[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<Balance> QueryBalance(string account_id)
                {
                    IntPtr r = TqapiDll.tapi_query_balance(this.handle, account_id);

                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<Balance> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<Balance>(Marshal.PtrToStructure<Balance>(cr.value));
                    else
                        ret = new CallResult<Balance>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                private string codes_to_str(string[] codes)
                {
                    if (codes == null && codes.Length == 0)
                        return "";

                    string str = "";
                    for (var i = 0; i < codes.Length; i++) {
                        str += codes[i];
                        if (i < codes.Length - 1)
                            str += ",";
                    }

                    return str;
                }

                public CallResult<Order[]> QueryOrders(string account_id, string[] codes)
                {

                    IntPtr r = TqapiDll.tapi_query_orders(this.handle, account_id, codes_to_str(codes));

                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<Order[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<Order[]>(TqapiDll.CopyArray<Order>(cr));
                    else
                        ret = new CallResult<Order[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<Position[]> QueryPositions(string account_id, string[] codes)
                {
                    IntPtr r = TqapiDll.tapi_query_positions(this.handle, account_id, codes_to_str(codes));

                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<Position[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<Position[]>(TqapiDll.CopyArray<Position>(cr));
                    else
                        ret = new CallResult<Position[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }

                public CallResult<Trade[]> QueryTrades(string account_id, string[] codes)
                {
                    IntPtr r = TqapiDll.tapi_query_trades(this.handle, account_id, codes_to_str(codes));

                    var cr = Marshal.PtrToStructure<TqapiDll.CallResultWrap>(r);

                    CallResult<Trade[]> ret;
                    if (cr.value_type != 0)
                        ret = new CallResult<Trade[]>(TqapiDll.CopyArray<Trade>(cr));
                    else
                        ret = new CallResult<Trade[]>(cr.msg);

                    TqapiDll.destroy_callresult(r);
                    return ret;
                }
            }
        }
    }
}
