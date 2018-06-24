using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using TQuant.Api;

namespace TestUI
{
    public partial class TickForm : Form
    {
        MarketQuote[] ticks;
        ListViewItem[] lvitem_cache = new ListViewItem[0];
        string code;

        public TickForm()
        {
            InitializeComponent();
            SetHeader(DataType.STOCK);

            //Create a simple ListView.
            //ListView listView1 = new ListView();
            //listView1.View = View.SmallIcon;
            listView1.VirtualMode = true;
            listView1.VirtualListSize = 0;

            //Hook up handlers for VirtualMode events.
            listView1.RetrieveVirtualItem += new RetrieveVirtualItemEventHandler(listView1_RetrieveVirtualItem);
            //listView1.CacheVirtualItems += new CacheVirtualItemsEventHandler(listView1_CacheVirtualItems);
            //listView1.SearchForVirtualItem += new SearchForVirtualItemEventHandler(listView1_SearchForVirtualItem);

            ////Search for a particular virtual item.
            ////Notice that we never manually populate the collection!
            ////If you leave out the SearchForVirtualItem handler, this will return null.
            //ListViewItem lvi = listView1.FindItemWithText("111111");

            ////Select the item found and scroll it into view.
            //if (lvi != null)
            //{
            //    listView1.SelectedIndices.Add(lvi.Index);
            //    listView1.EnsureVisible(lvi.Index);
            //}
        }

        enum DataType
        {
            FUTURE,
            INDEX,
            STOCK,
            OTHERS
        };

        void SetHeader(DataType dt)
        {
            var headers = dt == DataType.FUTURE ?
                new String[]
                {
                    "Date", "Time", "PreClose", "Open", "High", "Low", "Last",
                    "Total Volume","Total Turnover", "Volume", "AvgPrice",
                    "Ask1", "AV1",
                    "Bid1", "BV1",
                    "OI", "PreOI", "Settle", "PreSettle"
                } :
                dt == DataType.INDEX ?
                new String[]
                {
                    "Date", "Time", "PreClose", "Open", "High", "Low", "Last",
                    "Total Volume","Total Turnover", "Volume", "AvgPrice",
                } :
                new String[]
                {
                    "Date", "Time", "PreClose", "Open", "High", "Low", "Last",
                    "Total Volume","Total Turnover", "Volume", "AvgPrice",
                    "Ask5", "AV5", "Ask4", "AV4", "Ask3", "AV3", "Ask2", "AV2", "Ask1", "AV1",
                    "Bid1", "BV1", "Bid2", "BV2", "Bid3","BV3", "Bid4", "BV4", "Bid5", "BV5"
                };

            listView1.Columns.Clear();
            listView1.Columns.Add("IDX");

            foreach (var s in headers)
                listView1.Columns.Add(s).TextAlign = HorizontalAlignment.Right;

            listView1.MultiSelect = false;
            listView1.FullRowSelect = true;
            listView1.View = View.Details;
        }

        int trading_day = 0;
        private void btnGetTick_Click(object sender, EventArgs e)
        {
            string code = editCode.Text.Trim();
            if (code == "")
                return;

            trading_day = this.checkBoxUseTody.Checked ? 0 :
                dtpDate.Value.Year * 10000 + dtpDate.Value.Month * 100 + dtpDate.Value.Day;

            this.code = code;
            UpdateData();
        }

        void UpdateData()
        { 
            var dapi = GlobalData.GetDataApi();
            dapi.Subscribe(new String[] { code });
            var r = dapi.GetTick(code, this.trading_day);
            if (r.Value == null)
            {
                MessageBox.Show("GetTick Error: " + r.Msg);
                return;
            }

            this.ticks = r.Value;

            UpdateListView();
            UpdateTrendChart();
        }

        private void UpdateListView()
        {
            listView1.Items.Clear();

            DataType dt;
            if (code.StartsWith("0") && code.EndsWith(".SH") ||
                code.StartsWith("39") && code.EndsWith(".SZ"))
            {
                dt = DataType.INDEX;
            }
            else if ((code.StartsWith("6") && code.EndsWith(".SH")) ||
                (code.StartsWith("0") || code.StartsWith("2")) && code.EndsWith(".SZ"))
            {
                dt = DataType.STOCK;
            }
            else if (!(code.EndsWith(".SH") || code.EndsWith(".SZ")))
            {
                dt = DataType.FUTURE;
            }
            else
            {
                dt = DataType.OTHERS;
            }

            string price_fmt = dt == DataType.STOCK ? "{0:N2}" : "{0:N4}";

            long volume = 0;
            double turnover = 0.0;

            lvitem_cache = new ListViewItem[this.ticks.Length];

            for (int i = 0; i < this.ticks.Length; i++)
            {
                var tick = this.ticks[i];

                string sdate = String.Format("{0}-{1:D2}-{2:D2}",
                    tick.Date / 10000, (tick.Date / 100) % 100, (tick.Date % 100));

                string stime = String.Format("{0}:{1:D2}:{2:D2}.{3:D3}",
                    tick.Time / 10000000, (tick.Time / 100000) % 100,
                    (tick.Time / 1000) % 100, (tick.Time % 1000));

                ListViewItem item = new ListViewItem();
                item.Text = i.ToString();
                item.SubItems.Add(sdate);
                item.SubItems.Add(stime);
                item.SubItems.Add(String.Format(price_fmt, tick.PreClose));
                item.SubItems.Add(String.Format(price_fmt, tick.Open));
                item.SubItems.Add(String.Format(price_fmt, tick.High));
                item.SubItems.Add(String.Format(price_fmt, tick.Low));
                item.SubItems.Add(String.Format(price_fmt, tick.Last));
                item.SubItems.Add(String.Format("{0:N0}", tick.Volume / 100));
                item.SubItems.Add(String.Format("{0:N0}", (long)tick.Turnover));
                item.SubItems.Add(String.Format("{0:N0}", (tick.Volume - volume)/100));
                // item.SubItems.Add(String.Format("{0:N0}", (long)(tick.turnover - turnover)));
                if (tick.Volume != volume)
                    item.SubItems.Add(String.Format(price_fmt, (tick.Turnover - turnover) / (tick.Volume - volume)));
                else
                    item.SubItems.Add("");

                if (dt != DataType.INDEX) { 
                    if (dt != DataType.FUTURE)
                    {
                        item.SubItems.Add(String.Format(price_fmt, tick.Ask5)); item.SubItems.Add(String.Format("{0}", tick.AskVol5 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Ask4)); item.SubItems.Add(String.Format("{0}", tick.AskVol4 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Ask3)); item.SubItems.Add(String.Format("{0}", tick.AskVol3 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Ask2)); item.SubItems.Add(String.Format("{0}", tick.AskVol2 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Ask1)); item.SubItems.Add(String.Format("{0}", tick.AskVol1 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Bid1)); item.SubItems.Add(String.Format("{0}", tick.AskVol1 / 100));
                    }
                    else
                    {
                        item.SubItems.Add(String.Format(price_fmt, tick.Ask1)); item.SubItems.Add(String.Format("{0}", tick.AskVol1));
                        item.SubItems.Add(String.Format(price_fmt, tick.Bid1)); item.SubItems.Add(String.Format("{0}", tick.AskVol1));
                    }


                    if (dt != DataType.FUTURE)
                    {
                        item.SubItems.Add(String.Format(price_fmt, tick.Bid2)); item.SubItems.Add(String.Format("{0}", tick.AskVol2 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Bid3)); item.SubItems.Add(String.Format("{0}", tick.AskVol3 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Bid4)); item.SubItems.Add(String.Format("{0}", tick.AskVol4 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.Bid5)); item.SubItems.Add(String.Format("{0}", tick.AskVol5 / 100));
                    }

                    if (dt == DataType.FUTURE)
                    {
                        item.SubItems.Add(String.Format("{0:N0}", tick.Oi));
                        item.SubItems.Add(String.Format("{0:N0}", tick.PreOi));
                        item.SubItems.Add(String.Format(price_fmt, tick.Settle));
                        item.SubItems.Add(String.Format(price_fmt, tick.PreSettle));
                    }
                }
                volume   = tick.Volume;
                turnover = tick.Turnover;

                lvitem_cache[i] = item;
            }

            listView1.BeginUpdate();
            SetHeader(dt);
            listView1.VirtualListSize = this.ticks.Length;

            listView1.EnsureVisible(this.ticks.Length - 1);
            foreach (ColumnHeader col in listView1.Columns)
                col.Width = -1;
            listView1.EndUpdate();
        }

        private void UpdateTrendChart()
        {
            var bars = TrendForm.BuildBar(this.ticks, 60);
            double[] prices = new double[bars.Length];
            long[] volumes = new long[bars.Length];
            int[] times = new int[bars.Length];

            for (int i = 0; i < bars.Length; i++)
            {
                prices[i] = bars[i].Close;
                times[i] = bars[i].Time;
                volumes[i] = bars[i].Volume;
            }

            double pre_close = this.ticks[0].PreClose;
            int trading_day = this.ticks[0].TradingDay;
            trendChart1.SetData(code, trading_day, pre_close, prices, volumes, times);

        }

        private void buttonShowChart_Click(object sender, EventArgs e)
        {
            string code = editCode.Text.Trim();
            if (code == "")
                return;

            int date = this.checkBoxUseTody.Checked ? 0 :
                dtpDate.Value.Year * 10000 + dtpDate.Value.Month * 100 + dtpDate.Value.Day;

            TrendForm form = new TrendForm(code, date);
            form.Show();
        }

        private void checkBoxUseTody_CheckedChanged(object sender, EventArgs e)
        {
            dtpDate.Enabled = !this.checkBoxUseTody.Checked;
        }


        void listView1_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            if (e.ItemIndex < lvitem_cache.Length)
            {
                e.Item = lvitem_cache[e.ItemIndex];
            }
            else
            {
                int x = e.ItemIndex * e.ItemIndex;
                e.Item = new ListViewItem(x.ToString());
            }
        }

        private void trendChart1_Load(object sender, EventArgs e)
        {

        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (this.listView1.SelectedIndices.Count > 0)
            {
                int idx = this.listView1.SelectedIndices[0];
                if (idx < this.ticks.Length)
                {
                    var tick = this.ticks[idx];
                    trendChart1.SetCrossLinePos(tick.Last, tick.Time);
                }
            }
        }

        HashSet<int> trading_day_set = new HashSet<int>();

        bool IsTradingDay(DateTime dt)
        {
            int date = dt.Year * 10000 + dt.Month * 100 + dt.Day;
            if (trading_day_set.Count == 0)
            {
                var dapi = GlobalData.GetDataApi();
                dapi.Subscribe(new String[] { "000001.SH" });
                var bars = dapi.GetDailyBar("000001.SH").Value;
                if (bars != null)
                {
                    foreach (var b in bars)
                        trading_day_set.Add(b.Date);
                }
            }

            if (trading_day_set!=null)
                return trading_day_set.Contains(date);

            return (dt.DayOfWeek != DayOfWeek.Saturday && dt.DayOfWeek != DayOfWeek.Sunday);
        }

        private void buttonPrev_Click(object sender, EventArgs e)
        {
            string code = editCode.Text.Trim();
            if (code == "")
                return;

            if (this.checkBoxUseTody.Checked) return;

            DateTime dt = dtpDate.Value.AddDays(-1);
            while(!IsTradingDay(dt))
                dt = dt.AddDays(-1);

            dtpDate.Value = dt;

            trading_day = dtpDate.Value.Year * 10000 + dtpDate.Value.Month * 100 + dtpDate.Value.Day;

            this.code = code;
            UpdateData();

        }

        private void buttonNext_Click(object sender, EventArgs e)
        {
            string code = editCode.Text.Trim();
            if (code == "")
                return;

            if (this.checkBoxUseTody.Checked) return;

            DateTime dt = dtpDate.Value.AddDays(1);
            while (!IsTradingDay(dt) && dt.Date <= DateTime.Today)
                dt = dt.AddDays(1);

            if (dt.Date > DateTime.Today) dt = DateTime.Today;
            dtpDate.Value = dt;

            trading_day = dtpDate.Value.Year * 10000 + dtpDate.Value.Month * 100 + dtpDate.Value.Day;

            this.code = code;
            UpdateData();

        }

        //void listView1_CacheVirtualItems(object sender, CacheVirtualItemsEventArgs e)
        //{

        //}

        //void listView1_SearchForVirtualItem(object sender, SearchForVirtualItemEventArgs e)
        //{
        //}
    }
}
