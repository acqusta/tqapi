using System;
using System.Windows.Forms;
using System.Drawing;
using TQuant.Api;
using System.Drawing.Drawing2D;

namespace TestUI
{
    public partial class BarChart : UserControl
    {
        String code;
        int trading_day;

        int brick_h_count = 0;
        int brick_v_count = 0;
        float brick_width = 0;
        float brick_height = 0;
        int bar_cycle = 0;

        const int titlebar_height = 20;
        const int timebar_height = 20;
        const int sidebar_width = 60;
        const float bar_width = 5;

        double max_price = 0.0;
        double min_price = 0.0;

        Pen   red_pen = new Pen(Color.Red);
        Pen   green_pen = new Pen(Color.MediumTurquoise);
        Brush green_brush = Brushes.MediumTurquoise;


        Bar[] bars;


        public BarChart()
        {
            InitializeComponent();
        }

        public void SetData(String code, Bar[] bars, int bar_cycle)
        {
            this.code = code;
            this.trading_day = bars[0].trading_day;
            this.bars = bars;
            this.bar_cycle = bar_cycle;
            this.Invalidate();
        }

        protected override void OnPaint(PaintEventArgs pe)
        {
            base.OnPaint(pe);
            //pe.Graphics.DrawString("TrendChart", DefaultFont, new SolidBrush(ForeColor), Width / 2, Height / 2);
            DrawBarChart(pe.Graphics);
        }

        int IntTimeToMillis(int t)
        {
            int ms = t % 1000;
            t /= 1000;
            int h = t / 10000;
            int m = (t / 100) % 100;
            int s = t % 100;

            return (h * 3600 + m * 60 + s) * 1000 + ms;
        }

        private void DrawBarChart(Graphics g)
        {
            int width = this.Width;
            int height = this.Height;

            // 固定大小
            this.brick_width = 100;
            this.brick_height = 60;
            int max_bar_count = (int)((Width - 2 * sidebar_width - 10) * 1.0 / (bar_width + 2));

            max_price = -1;
            min_price = 1e10;
            int begin_pos = Math.Max(0, bars.Length - max_bar_count);

            for (int i = bars.Length - 1; i >= begin_pos; i--)
            {
                max_price = Math.Max(bars[i].high, max_price);
                min_price = Math.Min(bars[i].low, min_price);
            }

            g.Clear(Color.Black);
            DrawLines(g);

            DrawTitleBar(g);

            DrawTimeBar(g, begin_pos);

            DrawPriceLabel(g);

            DrawBars(g, begin_pos);
            DrawVolumeBar(g, begin_pos);
        }

        private void DrawTitleBar(Graphics g)
        {
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            Brush brush = Brushes.White;
            String str = String.Format("{0} - {1}", code, trading_day);
            g.DrawString(str, font, brush, sidebar_width, 2);
        }

        private void DrawTimeBar(Graphics g, int begin_pos)
        {
            int begin_time = bars[begin_pos].time;
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            var y = Height - timebar_height;
            Brush brush = Brushes.White;
            DateTime time = new DateTime(2018, 4, 25,
                (begin_time / 10000000), (begin_time / 100000) % 100,
                (begin_time / 1000) % 100, (begin_time % 1000));

            int interval = (int)(brick_width / (bar_width + 2) * bar_cycle);
            for (int i = 0; i < brick_h_count; i++)
            {
                var str = time.ToLongTimeString();
                var size = g.MeasureString(str, font);
                var x = sidebar_width + i * brick_width - size.Width / 2;
                g.DrawString(str, font, brush, x, y + 3);
                time = time.AddSeconds(interval);
                int hms = time.Hour * 10000 + time.Minute * 100 + time.Second;
                if (hms >113000 && hms < 130000)
                {
                    time = new DateTime(2018, 4, 25, 13,0,0).Add(time.Subtract (new DateTime(2018, 4, 25, 11, 30, 0)));
                }
            }
        }

        private void DrawLines(Graphics g)//, float x0, float y0, float w, float h)
        {
            float x0 = sidebar_width;
            float y0 = titlebar_height;
            float w = Width - 2 * sidebar_width;
            float h = Height - timebar_height - titlebar_height;

            Pen gray_pen = new Pen(Color.Gray, 1);
            gray_pen.DashStyle = DashStyle.Custom;
            gray_pen.DashPattern = new float[] { 1f, 1f };
            g.DrawLine(gray_pen, x0, y0, x0, y0 + h);
            g.DrawLine(gray_pen, x0 + w, y0, x0 + w, y0 + h);
            g.DrawLine(gray_pen, x0, y0, x0 + w, y0);
            g.DrawLine(gray_pen, x0, y0 + h, x0 + w, y0 + h);

            float x_center = x0 + w / 2;
            float y_center = y0 + h / 2;

            brick_h_count = (int)((w + brick_width - 1) / brick_width);

            brick_v_count = 0;
            brick_height = 0;
            if ((h / 2) % 60 < 10)
            {
                brick_v_count = (int)(h / 2 / 60) * 2;
                if (brick_v_count < 6) brick_v_count = 6;
                brick_height = h * 1f / brick_v_count;
            }
            else
            {
                brick_v_count = (int)(h / 2 / 60 + 1) * 2;
                if (brick_v_count < 6) brick_v_count = 6;
                brick_height = h / brick_v_count;
            }


            Pen red_pen = new Pen(Color.Red, 1);
            red_pen.DashStyle = DashStyle.Custom;
            red_pen.DashPattern = new float[] { 1f, 1f };

            //int price_center_pos = (brick_v_count - 2) / 2 + 1;
            int price_bottom_pos = brick_v_count - 2;
            for (int i = 1; i < brick_v_count; i++)
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
        }

        private void DrawPriceLabel(Graphics g)
        {
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            int price_v_count = brick_v_count - 2;
            for (int i = 0; i < price_v_count + 1; i++)
            {

                Brush brush = Brushes.White;

                double price = max_price - (max_price - min_price) / price_v_count * i;

                var tmp = String.Format("{0:F2}", price);
                var size = g.MeasureString(tmp, font);
                float y0 = titlebar_height + i * brick_height - size.Height / 2 + 1;
                //g.DrawString(tmp, font, brush, sidebar_width - size.Width, y0);
                g.DrawString(tmp, font, brush, Width - sidebar_width, y0);
            }
        }

        private void DrawBars(Graphics g, int begin_pos)
        {
            float bar_area_height = brick_height * (brick_v_count - 2);

            float x0 = sidebar_width;
            float y0 = bar_area_height + titlebar_height;

            for (int i = begin_pos; i < bars.Length; i++)
            {
                var bar = bars[i];
                float x        = x0 + (i - begin_pos) * (bar_width + 2) + bar_width/2;
                float y_top    = (float)(y0 - (bar.high - min_price) / (max_price - min_price) * bar_area_height);
                float y_bottom = (float)(y0 - (bar.low - min_price) / (max_price - min_price) * bar_area_height);

                float r_x_left  =  x - bar_width/2;
                float r_x_right = x + bar_width/2;
                float r_y_open  = (float)(y0 - (bar.open - min_price) / (max_price - min_price) * bar_area_height);
                float r_y_close = (float)(y0 - (bar.close - min_price) / (max_price - min_price) * bar_area_height);

                if (bar.open < bar.close)
                {
                    g.DrawLine(red_pen, x, y_top, x, y_bottom);
                    g.FillRectangle(Brushes.Black, r_x_left, r_y_close, bar_width, r_y_open - r_y_close);
                    g.DrawRectangle(red_pen, r_x_left, r_y_close, bar_width, r_y_open - r_y_close);
                }
                else
                {
                    g.DrawLine(green_pen, x, y_top, x, y_bottom);
                    g.FillRectangle(green_brush, r_x_left, r_y_open, bar_width, r_y_close - r_y_open);
                }
            }
        }

        private void DrawVolumeBar(Graphics g, int begin_pos)
        {
            float x0 = sidebar_width;
            float y0 = Height - timebar_height;

            var begin_time = IntTimeToMillis(bars[begin_pos].time);

            long max_vol = 0;
            for (int i = begin_pos; i < bars.Length; i++)
            {
                max_vol = Math.Max(bars[i].volume, max_vol);
            }

            for (int i = begin_pos; i < bars.Length; i++)
            {
                var bar = bars[i];
                float x = x0 + (i - begin_pos) * (bar_width + 2);
                float y_top = y0 - (float)(bar.volume * 1.0f / max_vol * brick_height * 2);

                if (bar.open < bar.close)
                    g.DrawRectangle(red_pen, x, y_top, bar_width - 1, y0 - y_top);
                else
                    g.FillRectangle(green_brush, x, y_top, bar_width - 1, y0 - y_top);
            }
        }

        private void BarChart_SizeChanged(object sender, EventArgs e)
        {
            this.Invalidate();
        }
    }

}
