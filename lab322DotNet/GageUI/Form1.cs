using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Timers;
using System.Activities;
using GageWrapper;

namespace GageUI
{
    public partial class Form1 : Form
    {
        private static void SetLabelText(string val)
        {
            Form1.GetOutput().Text = val;
        }
        delegate void m_SetLabel(string val);
        private static void print(string str)
        {
            TextBox output = Form1.GetOutput();
            String msg = str.ToString();


            if (output.InvokeRequired)
            {
                m_SetLabel setLabel = SetLabelText;
                Form1.GetForm().Invoke(setLabel, msg);
            }
            else
                output.Text = msg;
        }
        private static Gage gage;
        private static Form1 singleton;
        private OutputDelegate output = print;
        private System.ComponentModel.BackgroundWorker bw;
        public static TextBox GetOutput() { return singleton.OutputBox; }
        public static PictureBox GetPaint() { return singleton.paintBox; }
        public static Form1 GetForm() { return singleton; }
        public Form1()
        {
            this.bw  = new System.ComponentModel.BackgroundWorker();
            pen = new Pen(Color.Black, 0.2f);
           // this.Paint += new System.Windows.Forms.PaintEventHandler(this.Draw);
            this.bw.WorkerSupportsCancellation = false;
            this.bw.DoWork += new System.ComponentModel.DoWorkEventHandler(this.DoWork);
            this.bw.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.CompleteWork);
            InitializeComponent();
            singleton = this;

            System.Timers.Timer aTimer = new System.Timers.Timer();
            aTimer.Elapsed += new ElapsedEventHandler(Draw);
            // Set the Interval to 5 seconds.
            aTimer.Interval = 100;
            aTimer.Enabled = true;
        }

        private void Draw(object source, ElapsedEventArgs e)
        {

            PictureBox paintBox = Form1.GetPaint();
            try
            {
                if (paintBox.InvokeRequired)
                {
                    drawData(paintBox, gage.result);
                }
                else
                    drawData(paintBox, gage.result);
            }
            catch (Exception exception) { 
            
            
            }
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            gage = new Gage(output);
            gage.Initialize();
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (gage.isRunning)
                gage.Exit();
        }

        private delegate void WorkerEventHandler(
            ref Gage gage,
            AsyncOperation asyncOp);

       private void DoWork(object sender, DoWorkEventArgs e) {

           gage.Capture();
       }
       private void drawData(PictureBox paintBox, short[] data)
       {
           if (data==null||data.Length == 0) return;
           Graphics graphics = paintBox.CreateGraphics();
           graphics.Clear(Color.White);
           int offset = 8*5;
           int totalPoint = (data.Length/offset);
           float deltaX = paintBox.Width/ (float)totalPoint;
           float offsetY = short.MaxValue;
           float scaleY = (float)paintBox.Height / (float)(short.MaxValue*2);
           int index = 0, i = 0;
           while(index+offset<data.Length){

               graphics.DrawLine(this.pen, deltaX * i++, (data[index] + offsetY) * scaleY, deltaX * (i), (data[index + offset]+ offsetY) * scaleY);
               index += offset;
           }
       }
       private void CompleteWork(
            object sender,
            RunWorkerCompletedEventArgs e){


            //PictureBox paintBox = Form1.GetPaint();
            //if (paintBox.InvokeRequired)
            //{
            //    drawData(paintBox, gage.result);
            //}
            //else
            //    drawData(paintBox, gage.result);

            if(gage.isRunning)
                this.bw.RunWorkerAsync();
         }
       
        private void Start_Click(object sender, EventArgs e)
        {
            gage.Start();
            this.bw.RunWorkerAsync();
        }
    }
}
