using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using TQuant.Api;

namespace TestUI
{
    public partial class TrendForm : Form
    {
        String code = "000001.SH";
        int   trading_day = 20170323;
        TrendChart  trend_chart;
        TickChart   tick_chart;
        UserControl top_chart;
        BarChart bar_chart;

        public TrendForm(String code, int trading_day)
        {
            this.code = code;
            this.trading_day = trading_day;
            InitializeComponent();
            this.DoubleBuffered = true;
            var dapi = GlobalData.GetDataApi();
            dapi.Subscribe(new String[] { code });

            this.toolStripButton1_Click(this, null);
        }

        private void toolStripButton1_Click(object sender, EventArgs e)
        {
            var dapi = GlobalData.GetDataApi();
            var bars = dapi.GetBar(code, "1m", trading_day).Value;
            if (bars == null || bars.Length == 0)
                return;

            double[] prices = new double[bars.Length];
            long[] volumes = new long[bars.Length];
            int[] times = new int[bars.Length];

            for (int i = 0; i < bars.Length; i++)
            {
                prices[i]  = bars[i].close;
                times[i]   = bars[i].time;
                volumes[i] = bars[i].volume;
            }

            double pre_close = 0.0;
            var q = dapi.GetQuote(code).Value;
            if (q != null && q.trading_day == bars[0].trading_day)
                pre_close = q.pre_close;

            var daily_bars = dapi.GetDailyBar(code).Value;
            for (int i = daily_bars.Length - 1; i >= 0; i--)
            {
                // FXIME: no pre_close in daily_bar!
                if (daily_bars[i].date < bars[0].trading_day)
                {
                    pre_close = daily_bars[i].close;
                    break;
                }
            }

            if (top_chart != null) top_chart.Visible = false;

            if (trend_chart == null)
            {
                trend_chart = new TrendChart();
                trend_chart.Parent = this.panelTrend;
                trend_chart.Dock = DockStyle.Fill;
            }

            trend_chart.SetData(code, trading_day, pre_close, prices, volumes, times);

            trend_chart.Visible = true;
            top_chart = trend_chart;
        }

        private void toolStripButton5_Click(object sender, EventArgs e)
        {
            var dapi = GlobalData.GetDataApi();
            var ticks = dapi.GetTick(code, trading_day).Value;
            if (ticks == null || ticks.Length == 0)
                return;

            if (top_chart != null) top_chart.Visible = false;

            if (tick_chart == null)
            {
                tick_chart = new TickChart();
                tick_chart.Parent = this.panelTrend;
                tick_chart.Dock = DockStyle.Fill;
            }

            tick_chart.SetData(code, ticks[0].trading_day, ticks);

            tick_chart.Visible = true;
            top_chart = tick_chart;
        }

        void SetVisibleChart(UserControl chart)
        {
            if (top_chart == chart) return;
            if (top_chart != null) top_chart.Visible = false;
            top_chart = chart;
            chart.Visible = true;
        }

        private void toolStripButton4_Click(object sender, EventArgs e)
        {
            //var dapi = GlobalData.GetDataApi();
            //var bars = dapi.GetBar(code, "1m", trading_day).Value;
            //if (bars == null || bars.Length == 0)
            //    return;

            var dapi = GlobalData.GetDataApi();
            var ticks = dapi.GetTick(code, trading_day).Value;
            if (ticks == null || ticks.Length == 0)
                return;

            var bars = BuildBar(ticks, 60);

            if (bar_chart == null)
            {
                bar_chart = new BarChart();
                bar_chart.Parent = this.panelTrend;
                bar_chart.Dock = DockStyle.Fill;
            }

            bar_chart.SetData(code, bars, 60);
            SetVisibleChart(bar_chart);
        }

        static int bartime(int time, int cycle)
        {
            time /= 1000;
            int seconds = time / 10000 * 3600 + (time % 10000) / 100 * 60 + (time % 60);
            seconds = (seconds / cycle + 1) * cycle;
            if (seconds >= 3600 * 24) {
                seconds %= 3600 * 24;
            }
            return (seconds / 3600 * 10000 + (seconds % 3600) / 60 * 100 + (seconds % 60)) * 1000;
        }

        public static Bar[] BuildBar(MarketQuote[] ticks, int cycle)
        {
            List<Bar> bars = new List<Bar>();
            Bar cur_bar = null;

            long total_volume = 0;
            double total_turnover = 0.0;

            double high1 = 0.0;
            double high2 = 0.0;
            double low1 = 1e8;
            double low2 = 1e8;
            foreach (var tick in ticks)
            {
                if (tick.volume == 0) continue;

                int bar_date = tick.trading_day;
                int bar_time = bartime(tick.time, cycle);

                high2 = tick.high;
                low2 = tick.low;

                if (cur_bar != null)
                {
                    if (bar_date < cur_bar.date || (bar_date == cur_bar.date && bar_time < cur_bar.time))
                        continue;

                    // Ignore quotes having same volume and less volume
                    if (total_volume >= tick.volume)
                        continue;

                    if (cur_bar.date == bar_date && cur_bar.time == bar_time)
                    {
                        //cur_bar.open = cur_bar.open;
                        cur_bar.high      = Math.Max(cur_bar.high, tick.last);
                        cur_bar.low       = Math.Min(cur_bar.low, tick.last);
                        cur_bar.close     = tick.last;
                        cur_bar.volume   += tick.volume - total_volume;
                        cur_bar.turnover += tick.turnover - total_turnover;
                        cur_bar.oi        = tick.oi;

                        total_volume = tick.volume;
                        total_turnover    = tick.turnover;

                        continue;
                    }
                }

                // create new bar

                if (cur_bar != null)
                {
                    if (Math.Abs(high1 - high2) > 0.000001)
                        if (high2 > cur_bar.high) cur_bar.high = high2;

                    if (Math.Abs(low1 - low2) > 0.000001)
                        if (low2 < cur_bar.low) cur_bar.low = low2;
                }

                Bar bar = new Bar();
                bar.trading_day = tick.trading_day;
                bar.date        = bar_date;
                bar.time        = bar_time;
                bar.close       = bar.high = bar.low = bar.open = tick.last;
                bar.volume      = tick.volume - total_volume;
                bar.turnover    = tick.turnover - total_turnover;
                bar.oi          = tick.oi;
                bar.code        = tick.code;

                total_volume   = tick.volume;
                total_turnover = tick.turnover;

                high2 = high1 = tick.high;
                low2 = low1 = tick.low;

                cur_bar = bar;

                bars.Add(bar);
            }

            return bars.ToArray();
        }

        private void toolStripButton2_Click(object sender, EventArgs e)
        {
            var dapi = GlobalData.GetDataApi();
            var ticks = dapi.GetTick(code, trading_day).Value;
            if (ticks == null || ticks.Length == 0)
                return;

            var bars = BuildBar(ticks, 15);
            if (bar_chart == null)
            {
                bar_chart = new BarChart();
                bar_chart.Parent = this.panelTrend;
                bar_chart.Dock = DockStyle.Fill;
            }

            bar_chart.SetData(code, bars, 15);
            SetVisibleChart(bar_chart);
        }

        private void toolStripButton3_Click(object sender, EventArgs e)
        {
            var dapi = GlobalData.GetDataApi();
            var ticks = dapi.GetTick(code, trading_day).Value;
            if (ticks == null || ticks.Length == 0)
                return;

            var bars = BuildBar(ticks, 30);
            if (bar_chart == null)
            {
                bar_chart = new BarChart();
                bar_chart.Parent = this.panelTrend;
                bar_chart.Dock = DockStyle.Fill;
            }

            bar_chart.SetData(code, bars, 30);
            SetVisibleChart(bar_chart);
        }
    }
}
