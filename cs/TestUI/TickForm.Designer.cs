namespace TestUI
{
    partial class TickForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.panel1 = new System.Windows.Forms.Panel();
            this.checkBoxUseTody = new System.Windows.Forms.CheckBox();
            this.buttonShowChart = new System.Windows.Forms.Button();
            this.btnGetTick = new System.Windows.Forms.Button();
            this.dtpDate = new System.Windows.Forms.DateTimePicker();
            this.editCode = new System.Windows.Forms.TextBox();
            this.listView1 = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.buttonExportCsv = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.AutoSize = true;
            this.panel1.Controls.Add(this.buttonExportCsv);
            this.panel1.Controls.Add(this.checkBoxUseTody);
            this.panel1.Controls.Add(this.buttonShowChart);
            this.panel1.Controls.Add(this.btnGetTick);
            this.panel1.Controls.Add(this.dtpDate);
            this.panel1.Controls.Add(this.editCode);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(1211, 45);
            this.panel1.TabIndex = 0;
            // 
            // checkBoxUseTody
            // 
            this.checkBoxUseTody.AutoSize = true;
            this.checkBoxUseTody.Location = new System.Drawing.Point(412, 20);
            this.checkBoxUseTody.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.checkBoxUseTody.Name = "checkBoxUseTody";
            this.checkBoxUseTody.Size = new System.Drawing.Size(59, 19);
            this.checkBoxUseTody.TabIndex = 4;
            this.checkBoxUseTody.Text = "当日";
            this.checkBoxUseTody.UseVisualStyleBackColor = true;
            this.checkBoxUseTody.CheckedChanged += new System.EventHandler(this.checkBoxUseTody_CheckedChanged);
            // 
            // buttonShowChart
            // 
            this.buttonShowChart.Location = new System.Drawing.Point(632, 11);
            this.buttonShowChart.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.buttonShowChart.Name = "buttonShowChart";
            this.buttonShowChart.Size = new System.Drawing.Size(100, 29);
            this.buttonShowChart.TabIndex = 3;
            this.buttonShowChart.Text = "走势图";
            this.buttonShowChart.UseVisualStyleBackColor = true;
            this.buttonShowChart.Click += new System.EventHandler(this.buttonShowChart_Click);
            // 
            // btnGetTick
            // 
            this.btnGetTick.Location = new System.Drawing.Point(524, 12);
            this.btnGetTick.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnGetTick.Name = "btnGetTick";
            this.btnGetTick.Size = new System.Drawing.Size(100, 29);
            this.btnGetTick.TabIndex = 2;
            this.btnGetTick.Text = "提取 Tick";
            this.btnGetTick.UseVisualStyleBackColor = true;
            this.btnGetTick.Click += new System.EventHandler(this.btnGetTick_Click);
            // 
            // dtpDate
            // 
            this.dtpDate.Location = new System.Drawing.Point(225, 15);
            this.dtpDate.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.dtpDate.Name = "dtpDate";
            this.dtpDate.Size = new System.Drawing.Size(176, 25);
            this.dtpDate.TabIndex = 1;
            // 
            // editCode
            // 
            this.editCode.Location = new System.Drawing.Point(15, 16);
            this.editCode.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.editCode.Name = "editCode";
            this.editCode.Size = new System.Drawing.Size(184, 25);
            this.editCode.TabIndex = 0;
            this.editCode.Text = "000001.SH";
            // 
            // listView1
            // 
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3});
            this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView1.GridLines = true;
            this.listView1.Location = new System.Drawing.Point(0, 45);
            this.listView1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(1211, 433);
            this.listView1.TabIndex = 1;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            // 
            // buttonExportCsv
            // 
            this.buttonExportCsv.Location = new System.Drawing.Point(740, 11);
            this.buttonExportCsv.Margin = new System.Windows.Forms.Padding(4);
            this.buttonExportCsv.Name = "buttonExportCsv";
            this.buttonExportCsv.Size = new System.Drawing.Size(100, 29);
            this.buttonExportCsv.TabIndex = 5;
            this.buttonExportCsv.Text = "导入Excel";
            this.buttonExportCsv.UseVisualStyleBackColor = true;
            // 
            // TickForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1211, 478);
            this.Controls.Add(this.listView1);
            this.Controls.Add(this.panel1);
            this.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.Name = "TickForm";
            this.Text = "TickForm";
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button btnGetTick;
        private System.Windows.Forms.DateTimePicker dtpDate;
        private System.Windows.Forms.TextBox editCode;
        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.Button buttonShowChart;
        private System.Windows.Forms.CheckBox checkBoxUseTody;
        private System.Windows.Forms.Button buttonExportCsv;
    }
}