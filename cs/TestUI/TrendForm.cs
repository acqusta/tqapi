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

namespace TestUI
{
    public partial class TrendForm : Form
    {
        public TrendForm()
        {
            InitializeComponent();
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void panelTrend_Paint(object sender, PaintEventArgs e)
        {
            PaintBarChart(e.Graphics);
        }

        private void PaintBarChart(Graphics g)
        {
            int width = this.panelTrend.Width;
            int height = this.panelTrend.Height;

            g.Clear(Color.Black);

            const int price_panel_width = 60;
            {
                // left price panel
                Pen pen = new Pen(Color.Gray, 1);
                pen.DashStyle = DashStyle.Custom;
                pen.DashPattern = new float[] { 1f, 1f };
                g.DrawLine(pen, price_panel_width, 0, price_panel_width, height);
                g.DrawLine(pen, width - price_panel_width, 0, width - price_panel_width, height);
            }

            var dapi = GlobalData.GetDataApi();
            dapi.Subscribe(new String[] { "000001.SH" });
            var bars = dapi.GetBar("000001.SH", "1m").Value;
            if (bars == null)
            {
                //MessageBox.Show("Can't get bar data: 000001.SH");
                return;
            }
            double max_price = -1.0;
            double min_price = 1e20;
            double center_price = dapi.GetQuote("000001.SH").Value.pre_close;
            foreach (var b in bars)
            {
                if (b.high > max_price) max_price = b.high;
                if (b.low < min_price) min_price = b.low;
            }
            {
                double tmp = Math.Max(Math.Abs(max_price - center_price), Math.Abs(min_price - center_price));
                max_price = center_price + tmp;
                min_price = center_price - tmp;
            }
        
            {
                int x_center = width / 2;
                int y_center = height / 2;
                // Assert h > 2*40;
                int brick_height = 40;
                for (int i = 1; ; i++)
                {
                    if (Math.Abs(height / (2 * i) - 40) < 10)
                    {
                        brick_height = height / (2 * i);
                        break;
                    }
                }

                int brick_width = (width - 2 * price_panel_width) / 8;

                {  // 画横线
                    Pen pen = new Pen(Color.Red, 1);
                    pen.DashStyle = DashStyle.Custom;
                    pen.DashPattern = new float[] { 1f, 1f };

                    // int[] y_lines = new int[h / brick_height];
                    int y = y_center - brick_height;
                    while (y > 0)
                    {
                        g.DrawLine(pen, price_panel_width, y, width - price_panel_width, y);
                        y -= brick_height;
                    }
                    y = y_center + brick_height;
                    while (y < height)
                    {
                        g.DrawLine(pen, price_panel_width, y, width - price_panel_width, y);
                        y += brick_height;
                    }

                    // 画竖线
                    // 半小时一根
                    for(int i = 1; i < 8; i++)
                    {
                        if (i != 4)
                        {
                            int x = price_panel_width + i * brick_width;
                            g.DrawLine(pen, x, 0, x, height);
                        }
                    }

                    // 画中央横线和竖线, 画两根加粗
                    g.DrawLine(pen, price_panel_width, y_center, width - price_panel_width, y_center);
                    g.DrawLine(pen, price_panel_width, y_center+1, width - price_panel_width, y_center+1);
                    g.DrawLine(pen, width/2, 0, width / 2, height);
                    g.DrawLine(pen, width / 2 + 1, 0, width / 2 +1, height);

                    // 画左边价格标签

                    {
                        Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

                        int price_count = height / brick_height + 1;
                        for (int i = 0; i < price_count; i++)
                        {
                            
                            Brush brush = (i < price_count / 2) ? Brushes.Red :
                                (i > price_count / 2) ? Brushes.Green : Brushes.White;
                            double price = max_price - (max_price - min_price) / price_count * i;
                            var tmp = String.Format("{0:F2}", price);
                            var size = g.MeasureString(tmp, font);
                            float y0 = i * brick_height - 3;// size.Height/2;
                            g.DrawString(tmp, font, brush, price_panel_width - size.Width, y0);

                            double rate = (price - center_price) / price * 100;
                            tmp = String.Format("{0:F2}%", rate);
                            size = g.MeasureString(tmp, font);
                            y0 = i * brick_height - size.Height / 2;
                            g.DrawString(tmp, font, brush, width-price_panel_width, y0);
                        }
                    }
                }

                {
                    PointF[] points = new PointF[bars.Length+1];
                    //for()
                    points[0] = new PointF(price_panel_width,
                        (float)((max_price - bars[0].open) / (max_price - min_price) * height));

                    for (int i = 0; i < bars.Length; i++)
                    {
                        points[i+1] = new PointF(
                            (float)(price_panel_width + (width-2*price_panel_width)*(i+1) * 1.0/bars.Length),
                            (float)((max_price - bars[i].close) / (max_price - min_price) * height));
                    }
                    Pen pen = new Pen(Color.White);
                    g.DrawLines(pen, points);
                }
            }
        }

        private void panelTrend_SizeChanged(object sender, EventArgs e)
        {
            panelTrend.Invalidate();
        }
    }
}
