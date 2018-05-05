using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Drawing;
using TQuant.Api;

namespace TestUI
{
    public partial class PositionForm : Form
    {
        public PositionForm()
        {
            InitializeComponent();

            listView1.Columns.Clear();
            listView1.Columns.Add("IDX");
            listView1.Columns.Add("Account ID");
            listView1.Columns.Add("CODE");
            listView1.Columns.Add("NAME");
            listView1.Columns.Add("CURRENT SIZE");
            listView1.Columns.Add("ENABLE SIZE");
            listView1.Columns.Add("INIT SIZE");
            listView1.Columns.Add("TODAY SIZE");
            listView1.Columns.Add("FROZEN SIZE");
            listView1.Columns.Add("SIDE");
            listView1.Columns.Add("COST");
            listView1.Columns.Add("COST PRICE");
            listView1.Columns.Add("LAST PRICE");
            listView1.Columns.Add("FLOAT PNL");
            listView1.Columns.Add("CLOSE PNL");
            listView1.Columns.Add("MARGIN");
            listView1.Columns.Add("COMMISSION");

            listView1.MultiSelect = false;
            listView1.FullRowSelect = true;
            listView1.View = View.Details;

            RefreshData();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            RefreshData();
        }

        void RefreshData()
        {
            TradeApi tapi = GlobalData.GetTradeApi();

            var accounts = tapi.QueryAccountStatus().Value;
            if (accounts == null) return;

            List<Position> positions = new List<Position>();

            foreach (var act in accounts)
            {
                var r = tapi.QueryPositions(act.account_id);
                if (r.Value == null)
                {
                    MessageBox.Show(r.Msg, "Error");
                    continue;
                }
                foreach (var p in r.Value)
                    positions.Add(p);                
            }

            listView1.BeginUpdate();
            listView1.Items.Clear();
            foreach (var pos in positions)
            {
                ListViewItem item = new ListViewItem();
                item.Text = ""; // i.ToString();
                item.SubItems.Add(pos.account_id);
                item.SubItems.Add(pos.code);
                item.SubItems.Add(pos.name);
                item.SubItems.Add(pos.current_size.ToString());
                item.SubItems.Add(pos.enable_size.ToString());
                item.SubItems.Add(pos.init_size.ToString());
                item.SubItems.Add(pos.today_size.ToString());
                item.SubItems.Add(pos.frozen_size.ToString());
                item.SubItems.Add(pos.side);
                item.SubItems.Add(pos.cost.ToString());
                item.SubItems.Add(pos.cost_price.ToString());
                item.SubItems.Add(pos.last_price.ToString());
                item.SubItems.Add(pos.float_pnl.ToString()).ForeColor = pos.last_price > pos.cost_price ? Color.Red : Color.Green;
                item.SubItems.Add(pos.close_pnl.ToString());
                item.SubItems.Add(pos.margin.ToString());
                item.SubItems.Add(pos.commission.ToString());

                listView1.Items.Add(item);
            }

            foreach (ColumnHeader col in listView1.Columns)
            {
                col.Width = -2;
                col.TextAlign = HorizontalAlignment.Right;
            }

            listView1.EndUpdate();
        }
    }
}
