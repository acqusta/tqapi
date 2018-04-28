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

        private void btnGetTick_Click(object sender, EventArgs e)
        {
            string code = editCode.Text.Trim();
            if (code == "")
                return;

            int date = this.checkBoxUseTody.Checked? 0:
                dtpDate.Value.Year * 10000 + dtpDate.Value.Month * 100 + dtpDate.Value.Day;

            //if (dtpDate.Value.Date == DateTime.Today.Date)
            //    date = 0;

            listView1.Items.Clear();

            var dapi = GlobalData.GetDataApi();
            dapi.Subscribe(new String[] { code });
            var r = dapi.GetTick(code, date);
            if (r.Value == null)
            {
                MessageBox.Show("GetTick Error: " + r.Msg);
                return;
            }

            this.ticks = r.Value;

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

            lvitem_cache = new ListViewItem[r.Value.Length];

            for (int i = 0; i < r.Value.Length; i++)
            {
                var tick = r.Value[i];

                string sdate = String.Format("{0}-{1:D2}-{2:D2}",
                    tick.date / 10000, (tick.date / 100) % 100, (tick.date % 100));

                string stime = String.Format("{0}:{1:D2}:{2:D2}.{3:D3}",
                    tick.time / 10000000, (tick.time / 100000) % 100,
                    (tick.time / 1000) % 100, (tick.time % 1000));

                ListViewItem item = new ListViewItem();
                item.Text = i.ToString();
                item.SubItems.Add(sdate);
                item.SubItems.Add(stime);
                item.SubItems.Add(String.Format(price_fmt, tick.pre_close));
                item.SubItems.Add(String.Format(price_fmt, tick.open));
                item.SubItems.Add(String.Format(price_fmt, tick.high));
                item.SubItems.Add(String.Format(price_fmt, tick.low));
                item.SubItems.Add(String.Format(price_fmt, tick.last));
                item.SubItems.Add(String.Format("{0:N0}", tick.volume / 100));
                item.SubItems.Add(String.Format("{0:N0}", (long)tick.turnover));
                item.SubItems.Add(String.Format("{0:N0}", (tick.volume - volume)/100));
                // item.SubItems.Add(String.Format("{0:N0}", (long)(tick.turnover - turnover)));
                if (tick.volume != volume)
                    item.SubItems.Add(String.Format(price_fmt, (tick.turnover - turnover) / (tick.volume - volume)));
                else
                    item.SubItems.Add("");

                if (dt != DataType.INDEX) { 
                    if (dt != DataType.FUTURE)
                    {
                        item.SubItems.Add(String.Format(price_fmt, tick.ask5)); item.SubItems.Add(String.Format("{0}", tick.ask_vol5 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.ask4)); item.SubItems.Add(String.Format("{0}", tick.ask_vol4 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.ask3)); item.SubItems.Add(String.Format("{0}", tick.ask_vol3 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.ask2)); item.SubItems.Add(String.Format("{0}", tick.ask_vol2 / 100));
                    }

                    item.SubItems.Add(String.Format(price_fmt, tick.ask1)); item.SubItems.Add(String.Format("{0}", tick.ask_vol1 / 100));
                    item.SubItems.Add(String.Format(price_fmt, tick.bid1)); item.SubItems.Add(String.Format("{0}", tick.ask_vol1 / 100));

                    if (dt != DataType.FUTURE)
                    {
                        item.SubItems.Add(String.Format(price_fmt, tick.bid2)); item.SubItems.Add(String.Format("{0}", tick.ask_vol2 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.bid3)); item.SubItems.Add(String.Format("{0}", tick.ask_vol3 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.bid4)); item.SubItems.Add(String.Format("{0}", tick.ask_vol4 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.bid5)); item.SubItems.Add(String.Format("{0}", tick.ask_vol5 / 100));
                    }

                    if (dt == DataType.FUTURE)
                    {
                        item.SubItems.Add(String.Format("{0:N0}", tick.oi));
                        item.SubItems.Add(String.Format("{0:N0}", tick.pre_oi));
                        item.SubItems.Add(String.Format(price_fmt, tick.settle));
                        item.SubItems.Add(String.Format(price_fmt, tick.pre_settle));
                    }
                }
                volume   = tick.volume;
                turnover = tick.turnover;

                lvitem_cache[i] = item;
            }

            listView1.BeginUpdate();
            SetHeader(dt);
            listView1.VirtualListSize = r.Value.Length;

            listView1.EnsureVisible(r.Value.Length - 1);
            foreach (ColumnHeader col in listView1.Columns)
                col.Width = -1;
            listView1.EndUpdate();
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

        //void listView1_CacheVirtualItems(object sender, CacheVirtualItemsEventArgs e)
        //{

        //}

        //void listView1_SearchForVirtualItem(object sender, SearchForVirtualItemEventArgs e)
        //{
        //}
    }
}
