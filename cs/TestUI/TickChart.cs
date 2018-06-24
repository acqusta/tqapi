using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using TQuant.Api;

namespace TestUI
{
    public partial class TickChart : UserControl
    {
        String code;
        int trading_day;

        int brick_h_count = 0;
        int brick_v_count = 0;
        float brick_width = 0;
        float brick_height = 0;

        const int titlebar_height = 20;
        const int timebar_height = 20;
        const int sidebar_width = 60;

        double max_price = 0.0;
        double min_price = 0.0;

        MarketQuote[] ticks;

        public TickChart()
        {
            InitializeComponent();
        }

        public void SetData(String code, int trading_day, MarketQuote[] quotes)
        {
            this.code = code;
            this.trading_day = trading_day;
            this.ticks = quotes;

            this.Invalidate();
        }

        protected override void OnPaint(PaintEventArgs pe)
        {
            base.OnPaint(pe);
            Graphics g = pe.Graphics;

            //g.DrawString("TrendChart", DefaultFont, new SolidBrush(ForeColor), Width / 2, Height / 2);
            DrawTickChart(pe.Graphics);
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

        private void DrawTickChart(Graphics g)
        {
            int width = this.Width;
            int height = this.Height;

            // 固定大小
            this.brick_width = 100;
            this.brick_height = 60;
            int max_time_interval = (int)((Width - 2*sidebar_width - 10) *1.0 / brick_width * 60 * 1000);
            var end_time = IntTimeToMillis(ticks[ticks.Length - 1].Time);

            var begin_pos = 0;

            max_price = -1;
            min_price = 1e10;
            for (int i = ticks.Length - 1; i >= 0; i--)
            {
                //if (ticks[i].time > 113000000 && ticks[i].time < 130000000) continue;

                max_price = Math.Max(ticks[i].Last, max_price);
                min_price = Math.Min(ticks[i].Last, min_price);

                //if (ticks[i].time < 1130000)
                //{
                    if (IntTimeToMillis(ticks[i].Time) < end_time - max_time_interval)
                    {
                        begin_pos = i;
                        break;
                    }
                //}
                //else //if (ticks[i].time < 130000000)
                //{
                //    if (IntTimeToMillis(ticks[i].time) < end_time - max_time_interval)
                //    {
                //        begin_pos = i;
                //        break;
                //    }

                //}
            }

            g.Clear(Color.Black);
            DrawLines(g);

            DrawTitleBar(g);

            DrawTimeBar(g, begin_pos);

            DrawPriceLabel(g);

            DrawTickLine(g, begin_pos);
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
            int begin_time = ticks[begin_pos].Time;
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            var y = Height - timebar_height;
            Brush brush = Brushes.White;
            DateTime time = new DateTime(2018, 4, 25, 
                (begin_time / 10000000), (begin_time / 100000) % 100,
                (begin_time / 1000) % 100, (begin_time % 1000));

            for (int i = 0; i < brick_h_count; i++)
            {
                var str = time.ToLongTimeString();
                var size = g.MeasureString(str, font);
                var x = sidebar_width + i * brick_width - size.Width / 2;
                g.DrawString(str, font, brush, x, y + 3);
                time = time.AddMinutes(1);
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
            if ((h / 2) % 40 < 10)
            {
                brick_v_count = (int)(h / 2 / 40) * 2;
                if (brick_v_count < 6) brick_v_count = 6;
                brick_height = h * 1f / brick_v_count;
            }
            else
            {
                brick_v_count = (int)(h / 2 / 40 + 1) * 2;
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
            // 画左边价格标签
            Font font = new Font("SimSun", 9F, System.Drawing.FontStyle.Regular);

            int price_v_count = brick_v_count - 2;
            for (int i = 0; i < price_v_count + 1; i++)
            {

                Brush brush = Brushes.White;

                double price = max_price - (max_price - min_price) / price_v_count * i;

                var tmp = String.Format("{0:F2}", price);
                var size = g.MeasureString(tmp, font);
                float y0 = titlebar_height + i * brick_height - size.Height / 2 + 1;
                g.DrawString(tmp, font, brush, sidebar_width - size.Width, y0);
            }
        }

        private void DrawTickLine(Graphics g, int begin_pos)
        {
            var points = new PointF[2 * (ticks.Length - begin_pos + 1)];

            float x0 = sidebar_width;
            float y0 = brick_height * (brick_v_count - 2) + titlebar_height;
            var begin_time = IntTimeToMillis(ticks[begin_pos].Time);

            Pen yellow_pen = new Pen(Color.Yellow);

            int real_length = 0;
            for (int i = begin_pos; i < ticks.Length; i++) 
            {
                var tick = ticks[i];
                var p = new PointF(
                        (float)(x0 + (IntTimeToMillis(tick.Time) - begin_time) *1f / 60000 * brick_width),
                        (float)(y0 - (tick.Last - min_price) / (max_price - min_price) * brick_height * (brick_v_count - 2))
                    );

                if (real_length != 0)
                {
                    var last_p = points[real_length-1];
                    if (Math.Abs(last_p.Y - p.Y) > 0.0001)
                    {
                        points[real_length] = new PointF(p.X, last_p.Y);
                        real_length += 1;
                    }
                }
                points[real_length] = p;
                real_length += 1;
            }
            PointF[] new_points = new PointF[real_length];
            Array.Copy(points, new_points, real_length);
            Pen pen = new Pen(Color.White);
            g.DrawLines(pen, new_points);
        }


        private void DrawVolumeBar(Graphics g, int begin_pos)
        {
            float x0 = sidebar_width;
            float y0 = Height - timebar_height;

            var begin_time = IntTimeToMillis(ticks[begin_pos].Time);

            long max_vol = 0;
            long last_vol = ticks[begin_pos].Volume;
            for (int i = begin_pos + 1; i < ticks.Length; i++)
            {
                max_vol = Math.Max(ticks[i].Volume - last_vol, max_vol);
                last_vol = ticks[i].Volume;
            }

            Pen yellow_pen = new Pen(Color.Yellow);
            last_vol = ticks[begin_pos].Volume;
            for (int i = begin_pos + 1; i < ticks.Length; i++)
            {
                var tick = ticks[i];
                float x = (float)(x0 + (IntTimeToMillis(tick.Time) - begin_time) * 1f / 60000 * brick_width);
                float y = (float)(y0 - (tick.Volume - last_vol) * 1f / max_vol * brick_height * 2);
                g.DrawLine(yellow_pen, x, y, x, y0);
                last_vol = tick.Volume;
            }
        }

        private void TickChart_SizeChanged(object sender, EventArgs e)
        {
            this.Invalidate();
        }
    }

}
