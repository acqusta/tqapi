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
        String code = "000001.SH";
        int trading_day = 20170323;
        int brick_h_count = 0;
        int brick_v_count = 0;
        float brick_width = 0;
        float brick_height = 0;

        public TrendForm(String code, int trading_day)
        {
            this.code = code;
            this.trading_day = trading_day;
            InitializeComponent();
            this.DoubleBuffered = true;
            var dapi = GlobalData.GetDataApi();
            dapi.Subscribe(new String[] { code });         
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void panelTrend_Paint(object sender, PaintEventArgs e)
        {
            DrawTrendChart(e.Graphics);
        }

        private void DrawTitleBar(Graphics g, int x, int y)
        {
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            Brush brush = Brushes.White;
            String str = String.Format("{0} - {1}", code, trading_day);
            g.DrawString(str, font, brush, x, y + 2);
        }

        private void DrawTimeBar(Graphics g, int width, int height, int sidebar_width, int timebar_height)
        {
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            var times = new String[] {
                "9:30", "10:00", "10:30", "11:00", "11:30",
                "13:30", "14:00", "14:30","15:00"
            };

            var y = height - timebar_height;
            Brush brush = Brushes.White;
            for (int i = 0; i < times.Length; i++)
            {
                var size = g.MeasureString(times[i], font);
                var x = sidebar_width + i * brick_width - size.Width / 2;
                g.DrawString(times[i], font, brush, x, y + 3);
            }
        }

        private void DrawLines(Graphics g, float x0, float y0, float w, float h)
        {
            Pen gray_pen = new Pen(Color.Gray, 1);
            gray_pen.DashStyle = DashStyle.Custom;
            gray_pen.DashPattern = new float[] { 1f, 1f };
            g.DrawLine(gray_pen, x0, y0, x0, y0+h);
            g.DrawLine(gray_pen, x0 + w, y0, x0 + w, y0 + h);
            g.DrawLine(gray_pen, x0, y0, x0 + w, y0);
            g.DrawLine(gray_pen, x0, y0 + h, x0 + w, y0 + h);

            float x_center = x0 + w / 2;
            float y_center = y0 + h / 2;

            brick_h_count = 8;
            brick_width = w / brick_h_count;

            brick_v_count = 0;
            brick_height = 0;
            if ((h/2) % 40 < 10)
            {
                brick_v_count = (int)(h /2 / 40) * 2;
                if (brick_v_count < 6) brick_v_count = 6;
                brick_height = h * 1f / brick_v_count;
            }
            else
            {
                brick_v_count = (int)(h /2 / 40 + 1) * 2;
                if (brick_v_count < 6) brick_v_count = 6;
                brick_height = h / brick_v_count;
            }


            Pen red_pen = new Pen(Color.Red, 1);
            red_pen.DashStyle = DashStyle.Custom;
            red_pen.DashPattern = new float[] { 1f, 1f };

            //int price_center_pos = (brick_v_count - 2) / 2 + 1;
            int price_bottom_pos = brick_v_count - 2;
            for(int i = 1; i < brick_v_count; i++)
            {
                float y = y0 + i * brick_height;
                var pen = i == price_bottom_pos ? gray_pen : red_pen;
                g.DrawLine(pen, x0, y, x0 + w, y);
            }

            for (int i = 1; i < brick_h_count; i++)
            {
                float x = x0 + i * brick_width;
                g.DrawLine(red_pen, x, y0, x, y0 + h);
            }

            // 画中央横线和竖线, 画两根加粗
            float center_y = y0 + h / 2;
            float center_x = x0 + w / 2;
            g.DrawLine(red_pen, x0, center_y + 1, x0 + w, center_y + 1);
            g.DrawLine(red_pen, center_x + 1, y0, center_x + 1, y0 + h);
        }

        private void DrawPriceLabel(Graphics g,
            int width, int height,
            int sidebar_width, int titlebar_height, int timebar_height,
            double max_price, double center_price, double min_price)
        {
            // 画左边价格标签
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            int price_v_count = brick_v_count - 2;
            for (int i = 0; i < price_v_count + 1; i++)
            {

                Brush brush = (i < price_v_count / 2) ? Brushes.Red :
                    (i > price_v_count / 2 ) ? Brushes.Green : Brushes.White;

                double price = (i != price_v_count / 2) ?
                    max_price - (max_price - min_price) / (price_v_count - 2 ) * i :
                    center_price;

                var tmp = String.Format("{0:F2}", price);
                var size = g.MeasureString(tmp, font);
                float y0 = titlebar_height + i * brick_height - size.Height/2 + 1;
                g.DrawString(tmp, font, brush, sidebar_width - size.Width, y0);

                double rate = i != price_v_count/2 ? (price - center_price) / price * 100 : 0;
                tmp = String.Format("{0:F2}%", rate);
                size = g.MeasureString(tmp, font);
                y0 = titlebar_height + i * brick_height - size.Height / 2 + 1;
                g.DrawString(tmp, font, brush, width - sidebar_width, y0);
            }
        }

        private void DrawTrendLine(Graphics g,
            TQuant.Api.Bar[] bars, double max_price, double min_price,
            float x0, float y0, float width, float height)
        {
            PointF[] points = new PointF[bars.Length + 1];

            points[0] = new PointF(x0,
                (float)((max_price - bars[0].open) / (max_price - min_price) * height));

            for (int i = 0; i < bars.Length; i++)
            {
                points[i + 1] = new PointF(
                    (float)(x0 + width * (i + 1) * 1.0 / bars.Length),
                    (float)(y0 + (max_price - bars[i].close) / (max_price - min_price) * height));
            }
            Pen pen = new Pen(Color.White);
            g.DrawLines(pen, points);
        }

        private void DrawVolumeBar(Graphics g, TQuant.Api.Bar[] bars, float x0, float y0, float width, float height)
        {
            PointF[] points = new PointF[bars.Length + 1];

            long max_vol = 0;
            foreach (var b in bars) if (b.volume > max_vol) max_vol = b.volume;

            if (max_vol == 0) return;

            Pen red_pen = new Pen(Color.Red);
            Pen green_pen = new Pen(Color.Green);
            Pen white_pen = new Pen(Color.White);

            for (int i = 0; i < bars.Length; i++)
            {
                var bar = bars[i];
                if (bar.volume == 0) continue;
                float x = x0 + width * (i + 1) / 240.0f;
                float y = y0 + height - height * (bar.volume) / max_vol;
                Pen pen;
                if (bar.close > bar.open) pen = red_pen;
                else if (bar.close < bar.open) pen = green_pen;
                else pen = white_pen;
                g.DrawLine(pen, x, y, x, y0 + height);
            }
        }

        private void DrawTrendChart(Graphics g)
        {
            int width = this.panelTrend.Width;
            int height = this.panelTrend.Height;

            var dapi = GlobalData.GetDataApi();
            var bars = dapi.GetBar(code, "1m", trading_day).Value;
            if (bars == null || bars.Length == 0)
                return;

            if (trading_day==0)
                trading_day = bars[0].trading_day;

            double max_price = -1.0;
            double min_price = 1e20;
            double center_price = bars[0].open;
            {
                var q = dapi.GetQuote(code).Value;
                if (q != null && q.trading_day == trading_day)
                {
                    center_price = q.pre_close;
                }
                else
                {
                    var daily_bars = dapi.GetDailyBar(code).Value;
                    for (int i =  daily_bars.Length -1; i >=0; i--)
                    {
                        // FXIME: no pre_close in daily_bar!
                        if (daily_bars[i].date < trading_day)
                        {
                            center_price = daily_bars[i].close;
                            break;
                        }
                    }
                }

            }

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


            const int titlebar_height = 20;
            const int timebar_height = 20;
            const int sidebar_width = 60;
            
            g.Clear(Color.Black);
            DrawTitleBar(g, sidebar_width, 0);
            DrawTimeBar(g, width, height, sidebar_width, timebar_height);
            DrawLines(g,
                sidebar_width, titlebar_height,
                width - 2* sidebar_width, height - 2* timebar_height);

            DrawPriceLabel(g, width, height,
                sidebar_width, titlebar_height, timebar_height,
                max_price, center_price, min_price);

            DrawTrendLine(g, bars, max_price, min_price,
                sidebar_width, titlebar_height,  // x,y
                width - 2 * sidebar_width,
                brick_height * (brick_v_count-2));

            DrawVolumeBar(g, bars,
                sidebar_width,    // x
                titlebar_height + (brick_v_count -2 )*brick_height,  // y
                width - 2 * sidebar_width,
                brick_height * 2);
        }

        private void panelTrend_SizeChanged(object sender, EventArgs e)
        {
            this.panelTrend.Invalidate();
        }
    }
}
