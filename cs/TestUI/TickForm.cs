using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TestUI
{
    public partial class TickForm : Form
    {
        public TickForm()
        {
            InitializeComponent();
            SetHeader(false);
        }

        public void SetHeader(bool isFuture)
        {
            var headers = isFuture ?
                new String[]
                {
                    "Date", "Time", "PreClose", "Open", "High", "Low", "Last",
                    "Total Volume","Total Turnover", "Volume", "AvgPrice",
                    "Ask1", "AV1",
                    "Bid1", "BV1",
                    "OI", "PreOI", "Settle", "PreSettle"
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

            bool is_index = (code.StartsWith("0") && code.EndsWith(".SH") ||
                code.StartsWith("39") && code.EndsWith(".SZ"));

            bool is_stock = (code.StartsWith("6") && code.EndsWith(".SH")) ||
                (code.StartsWith("0") || code.StartsWith("2")) && code.EndsWith(".SZ");


            string price_fmt = is_stock ? "{0:N2}" : "{0:N4}";

            bool is_future = !(code.EndsWith(".SH") || code.EndsWith(".SZ"));

            listView1.BeginUpdate();
            {
                SetHeader(is_future);

                long volume = 0;
                double turnover = 0.0;

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

                    if (!is_index)
                    {
                        if (!is_future)
                        {
                            item.SubItems.Add(String.Format(price_fmt, tick.ask5)); item.SubItems.Add(String.Format("{0}", tick.ask_vol5 / 100));
                            item.SubItems.Add(String.Format(price_fmt, tick.ask4)); item.SubItems.Add(String.Format("{0}", tick.ask_vol4 / 100));
                            item.SubItems.Add(String.Format(price_fmt, tick.ask3)); item.SubItems.Add(String.Format("{0}", tick.ask_vol3 / 100));
                            item.SubItems.Add(String.Format(price_fmt, tick.ask2)); item.SubItems.Add(String.Format("{0}", tick.ask_vol2 / 100));
                        }

                        item.SubItems.Add(String.Format(price_fmt, tick.ask1)); item.SubItems.Add(String.Format("{0}", tick.ask_vol1 / 100));
                        item.SubItems.Add(String.Format(price_fmt, tick.bid1)); item.SubItems.Add(String.Format("{0}", tick.ask_vol1 / 100));
                        if (!is_future)
                        {
                            item.SubItems.Add(String.Format(price_fmt, tick.bid2)); item.SubItems.Add(String.Format("{0}", tick.ask_vol2 / 100));
                            item.SubItems.Add(String.Format(price_fmt, tick.bid3)); item.SubItems.Add(String.Format("{0}", tick.ask_vol3 / 100));
                            item.SubItems.Add(String.Format(price_fmt, tick.bid4)); item.SubItems.Add(String.Format("{0}", tick.ask_vol4 / 100));
                            item.SubItems.Add(String.Format(price_fmt, tick.bid5)); item.SubItems.Add(String.Format("{0}", tick.ask_vol5 / 100));
                        }
                        if (is_future)
                        {
                            item.SubItems.Add(String.Format("{0:N0}", tick.oi));
                            item.SubItems.Add(String.Format("{0:N0}", tick.pre_oi));
                            item.SubItems.Add(String.Format(price_fmt, tick.settle));
                            item.SubItems.Add(String.Format(price_fmt, tick.pre_settle));
                        }
                    }
                    listView1.Items.Add(item);
                    volume   = tick.volume;
                    turnover = tick.turnover;
                }
            }

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
    }
}
