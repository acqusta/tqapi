using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TQuant
{
    namespace Api
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public class MarketQuote
        {
            public String Code;
#if X86
            public Int32 _padding_1;
#endif
            public Int32  Date;
            public Int32  Time;
            public Int64  RecvTime;
            public Int32  TradingDay;
            public Double Open;
            public Double High;
            public Double Low;
            public Double Close;
            public Double Last;
            public Double HighLimit;
            public Double LowLimit;
            public Double PreClose;
            public Int64  Volume;
            public Double Turnover;
            public Double Ask1;
            public Double Ask2;
            public Double Ask3;
            public Double Ask4;
            public Double Ask5;
            public Double Bid1;
            public Double Bid2;
            public Double Bid3;
            public Double Bid4;
            public Double Bid5;
            public Int64  AskVol1;
            public Int64  AskVol2;
            public Int64  AskVol3;
            public Int64  AskVol4;
            public Int64  AskVol5;
            public Int64  BidVol1;
            public Int64  BidVol2;
            public Int64  BidVol3;
            public Int64  BidVol4;
            public Int64  BidVol5;
            public Double Settle;
            public Double PreSettle;
            public Int64  Oi;
            public Int64  PreOi;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public class Bar
        {
            public String Code;
#if X86
            public Int32 _padding_1;
#endif
            public Int32  Date;
            public Int32  Time;
            public Int32  TradingDay;
            public Double Open;
            public Double High;
            public Double Low;
            public Double Close;
            public Int64  Volume;
            public Double Turnover;
            public Int64  Oi;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public class DailyBar
        {
            public String Code;
#if X86
            public Int32 _padding_1;
#endif
            public Int32  Date;
            public Double Open;
            public Double High;
            public Double Low;
            public Double Close;
            public Int64  Volume;
            public Double Turnover;
            public Int64  OI;
            public Double Settle;
            public Double PreClose;
            public Double PreSettle;
            //Double _padding;
        }

        public class CallResult<T>
        {
            public T      Value;
            public String Msg;

            public CallResult(String msg)
            {
                this.Msg = msg;
            }

            public CallResult(T value)
            {
                this.Value = value;
            }
        }

        public delegate void OnMarketQuoteHandler(MarketQuote quote);
        public delegate void OnBarHandler(String cycle, Bar bar);

        public interface DataApi
        {

            /**
            * 取某交易日的某个代码的 ticks
            *
            * 当tradingday为0，表示当前交易日
            *
            * @param code
            * @param trading_day
            * @return
            */
            CallResult<MarketQuote[]> GetTick(String code, int trading_day = 0);

            /**
            * 取某个代码的Bar
            *
            * 目前只支持分钟线
            *  当 cycle == "1m"时，返回trading_day的分钟线，trading_day=0表示当前交易日。
            *
            * @param code          证券代码
            * @param cycle         "1m""
            * @param trading_day   交易日
            * @param align         是否对齐
            * @return
            */
            CallResult<Bar[]> GetBar(String code, String cycle, int trading_day = 0, bool align=true);

            /**
            * 取某个代码的日线
            *
            *
            * @param code          证券代码
            * @param price_adj     价格复权，取值
            *                        back -- 后复权
            *                        forward -- 前复权
            * @param align         是否对齐
            * @return
            */
            CallResult<DailyBar[]> GetDailyBar(String code, String price_adj = "", bool align=true);

            /**
            * 取当前的行情快照
            *
            * @param code
            * @return
            */
            CallResult<MarketQuote> GetQuote(String code);

            /**
            * 订阅行情
            *
            * codes为新增的订阅列表，返回所有已经订阅的代码,包括新增的列表。如果codes为空，可以返回已订阅列表。
            *
            * @param codes
            * @return 所有已经订阅的代码
            */
            CallResult<String[]> Subscribe(String[] codes);

            /**
            * 取消订阅
            *
            * codes为需要取消的列表，返回所有还在订阅的代码。
            * 如果需要取消所有订阅，先通过 subscribe 得到所有列表，然后使用unscribe取消

            * @param codes
            * @return
            */
            CallResult<String[]> UnSubscribe(String[] codes);

            /**
            * 设置推送行情的回调函数
            *
            * 当订阅的代码列表中有新的行情，会通过该callback通知用户。
            *
            * @param callback
            */
            //void SetCallback(DataApiCallback callback);
            event OnMarketQuoteHandler OnMarketQuote;
            event OnBarHandler         OnBar;
        }
    }
}
