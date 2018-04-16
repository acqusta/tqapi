using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace tquant
{
    namespace api
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct MarketQuote
        {
            public String code;
            public Int32  date;
            public Int32  time;
            public Int64  recv_time;
            public Int32  trading_day;
            public Double open;
            public Double high;
            public Double low;
            public Double close;
            public Double last;
            public Double high_limit;
            public Double low_limit;
            public Double pre_close;
            public Int64  volume;
            public Double turnover;
            public Double ask1;
            public Double ask2;
            public Double ask3;
            public Double ask4;
            public Double ask5;
            public Double bid1;
            public Double bid2;
            public Double bid3;
            public Double bid4;
            public Double bid5;
            public Int64  ask_vol1;
            public Int64  ask_vol2;
            public Int64  ask_vol3;
            public Int64  ask_vol4;
            public Int64  ask_vol5;
            public Int64  bid_vol1;
            public Int64  bid_vol2;
            public Int64  bid_vol3;
            public Int64  bid_vol4;
            public Int64  bid_vol5;
            public Double settle;
            public Double pre_settle;
            public Int64  oi;
            public Int64  pre_oi;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public class Bar
        {
            public String code;
            public Int32  date;
            public Int32  time;
            public Int32  trading_day;
            public Double open;
            public Double high;
            public Double low;
            public Double close;
            public Int64  volume;
            public Double turnover;
            public Int64  oi;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public class DailyBar
        {
            public String code;
            public Int32  date;
            public Double open;
            public Double high;
            public Double low;
            public Double close;
            public Int64  volume;
            public Double turnover;
            public Int64  oi;
            public Double settle;
            public Double pre_close;
            public Double pre_settle;
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

        public interface DataApiCallback
        {
            void OnMarketQuote(MarketQuote quote);
            void OnBar(String cycle, Bar bar);
        }

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
            CallResult<DailyBar[]> GetDailyBar(String code, String price_adj, bool align);

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
            void SetCallback(DataApiCallback callback);
        }
    }
}
