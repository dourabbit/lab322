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
        private Point _gageGraphCorner = new Point(30, 50);
        private Point _gageGraphSize = new Point(50, 50);
        private Point _gridGraphCorner = new Point(20, 20);
        private Point _gridGraphSize = new Point(50, 50);

        private List<short[]> _settingRange;
        private ushort[,] _maxiumValue; 


        private static Gage gage;
        private const int _initLeftRange = 2000;
        private const int _initRightRange = 5000;
        private int _MAXLOOPs = 6000;
        private int _curLoopCount = 0;
        private int _drawedLoopCount = 0;
        private int _dataGridMaxRow = 50;
        private int _dataGridMaxCol = 120;
        private int _gridCellSize = 5;
        private uint _dataLength4SingleChanl;
        private static Form1 singleton;
        private OutputDelegate output = print;
        private System.ComponentModel.BackgroundWorker bw;
        private Bitmap backBuffer;
        private List<Bitmap> backgrounds;
        private List<PictureBox> paintBoxes;
        private System.Drawing.Pen pen;
        private  List<System.Drawing.SolidBrush> brushes;
        private short[,,] result; 
        private static int _numOfChannel = 5;
        public static TextBox GetOutput() { return singleton.OutputBox; }
        public static PictureBox GetPaint(int index) { return singleton.paintBoxes[index]; }
        public static Form1 GetForm() { return singleton; }
        public Form1()
        {
             
            InitializeComponent();
            paintBoxes = new List<PictureBox>();
            _settingRange = new List<short[]>();
            backgrounds = new List<Bitmap>();

            brushes = new List<SolidBrush>();

            brushes.Add(new System.Drawing.SolidBrush(System.Drawing.Color.Gray));
            brushes.Add(new System.Drawing.SolidBrush(System.Drawing.Color.Black));

            _gageGraphSize = new Point(250, 300);
            _gridGraphSize = new Point(500, 300);

            _gridGraphCorner.X += _gageGraphCorner.X + _gageGraphSize.X;
            _gridGraphCorner.Y = 20;
            pen = new Pen(Color.White, 0.2f);

            

            for (int i = 0; i < _numOfChannel; ++i)
            { 
                PictureBox paintBox = new PictureBox();
                paintBox.Location = new System.Drawing.Point(0, 0);
                paintBox.Name = "paintBox";
                paintBox.Size = new System.Drawing.Size(950, 300);
                paintBox.TabIndex = 3;
                paintBox.TabStop = false;
                paintBoxes.Add(paintBox);
                short[] range = new short[2];

                Bitmap background = new Bitmap(paintBoxes[0].Width, paintBoxes[0].Height);
                backgrounds.Add(background);
                using (var g = Graphics.FromImage(background))
                {
                    float x0, y0;

                    for (int row = 0; row < _dataGridMaxRow; row++)
                    {
                        int index = row % 2 == 0 ? 0 : 1;
                        for (int col = 0; col < _dataGridMaxCol; col++)
                        {
                            index ^= 1;
                            x0 = _gridGraphCorner.X + _gridCellSize * col;
                            y0 = _gridGraphCorner.Y + _gridCellSize * row;
                            g.FillRectangle(brushes[index], (int)x0, (int)y0, 4, 4);
                        }
                    }
                }
                backBuffer = new Bitmap(paintBoxes[0].Width, paintBoxes[0].Height);
                range[0] = _initLeftRange; range[1] = _initRightRange;
                _settingRange.Add(range);
                flowLayoutPanel1.Controls.Add(paintBox);
            }


           

       


            this.bw  = new System.ComponentModel.BackgroundWorker();
            this.bw.WorkerSupportsCancellation = false;
            this.bw.DoWork += new System.ComponentModel.DoWorkEventHandler(this.DoWork);
            this.bw.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.CompleteWork);

            singleton = this;



            //Draw graph
            System.Timers.Timer aTimer = new System.Timers.Timer();
            aTimer.Elapsed += new ElapsedEventHandler(Draw);
            // Set the Interval to 5 seconds.
            aTimer.Interval = 80;
            aTimer.Enabled = true;


            this.WindowState = FormWindowState.Maximized;
        }
        /// <summary>
        /// Draw graph
        /// </summary>
        /// <param name="source"></param>
        /// <param name="e"></param>
        private void Draw(object source, ElapsedEventArgs e)
        {

            for (int i = 0; i < _numOfChannel; ++i)
            {
                PictureBox paintBox = Form1.GetPaint(i);
                try
                {
                    if (paintBox.InvokeRequired)
                        drawData(paintBox, gage.result,i);
                    else
                        drawData(paintBox, gage.result,i);


                  
                }
                catch (Exception exception) { 
            
            
                }
            }
            if (_drawedLoopCount < _curLoopCount)
                _drawedLoopCount++;
           // _drawedLoopCount = _curLoopCount;
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            gage = new Gage(output);
            gage.Initialize();
            _dataLength4SingleChanl = gage.resultSize / 8;
            result = new short[_MAXLOOPs, _numOfChannel, _dataLength4SingleChanl];
            _maxiumValue = new ushort[_numOfChannel, _MAXLOOPs];
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

       public Color Blend(Color backColor, Color color , double amount)
       {
           byte r = (byte)((color.R * amount) + backColor.R * (1 - amount));
           byte g = (byte)((color.G * amount) + backColor.G * (1 - amount));
           byte b = (byte)((color.B * amount) + backColor.B * (1 - amount));
           return Color.FromArgb(r, g, b);
       }

       
       //http://stackoverflow.com/questions/11606022/paint-event-of-a-picturebox-is-causing-flicking-where-else-can-i-do-this
       private void drawData(PictureBox paintBox, short[] data, int chanelIndex)
       {
           if (data==null||data.Length == 0) return;

           using (var graphics = Graphics.FromImage(backBuffer))
           {
               graphics.Clear(Color.Black);

               if (false)//_drawedLoopCount < _curLoopCount)
               {
                  int numOfGrid = _curLoopCount - _drawedLoopCount;
                 // Console.WriteLine(numOfGrid.ToString());
                  using (var g = Graphics.FromImage(backgrounds[chanelIndex]))
                  {
                      int drawIndex = _drawedLoopCount;
                      for (int a = 0; a < numOfGrid; ++a)
                      {
                          float x, y;
                          int col = (_drawedLoopCount+a) % _dataGridMaxCol;
                          int row = (_drawedLoopCount + a - col) / _dataGridMaxCol;
                          x = _gridGraphCorner.X + _gridCellSize * col;
                          y = _gridGraphCorner.Y + _gridCellSize * row;
                          ushort maxium = this._maxiumValue[chanelIndex, row * _dataGridMaxCol + col];
                          double blend = (double)maxium / (double)short.MaxValue;
                          Color color = Blend(Color.Blue, Color.Red, blend);
                          using (SolidBrush brush = new SolidBrush(color))
                          {
                              g.FillRectangle(brush, (int)x, (int)y, _gridCellSize, _gridCellSize);
                          }
                          drawIndex++;
                      
                      
                      }

                  }
              }
              
                    graphics.DrawImage(backgrounds[chanelIndex], 0, 0);


               const int offset = 8 * 5;
               int totalPoint = (data.Length / offset);
               float deltaX = (float)(_gageGraphSize.X - 2 * _gageGraphCorner.X) / (float)totalPoint;
               float offsetY = short.MaxValue;
               float scaleY = (float)(_gageGraphSize.Y - 2 * _gageGraphCorner.Y) / (float)(short.MaxValue * 2);
               int i = 0;
               while (chanelIndex + offset < data.Length)
               {
                   float x0,x1,y0,y1;
                   x0 = _gageGraphCorner.X + deltaX * i++;
                   x1 = _gageGraphCorner.X + deltaX * i;
                   y0 = _gageGraphCorner.Y + (data[chanelIndex] + offsetY) * scaleY;
                   y1 = _gageGraphCorner.Y + (data[chanelIndex + offset] + offsetY) * scaleY;
                   graphics.DrawLine(this.pen, x0,y0 ,x1 , y1);
                   chanelIndex += offset;
               }
           }
           using (var g = paintBox.CreateGraphics())
               g.DrawImage(backBuffer, 0, 0);
       }


        /// <summary>
        /// Called by finishing the job
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
       private void CompleteWork( object sender,RunWorkerCompletedEventArgs e){

           if (_curLoopCount < _MAXLOOPs)
           {

               for (int channel = 0; channel < _numOfChannel; channel++)
               {
                   for (int i = 0; i < _dataLength4SingleChanl; ++i)
                   {
                       const int offset = 8;
                       short value =  gage.result[channel + i * offset];
                       this.result[_curLoopCount, channel, i] = value;

                       if (i > _initLeftRange && i < _initRightRange) {
                           ushort maxValue = (ushort)(value < 0 ? -value : value);
                           maxValue = (maxValue == 32768) ? maxValue-- : maxValue;
                           _maxiumValue[channel, _curLoopCount] = maxValue > _maxiumValue[channel, _curLoopCount] ? maxValue : _maxiumValue[channel, _curLoopCount];
                       }
                   }
               }

             // Draw(null,null);

               if (gage.isRunning)
               {
                   _curLoopCount++;
                   this.bw.RunWorkerAsync();
               }
           }
           else{
               //_curLoopCount = 0;
           }
          
         }
       /// <summary>
       /// excuted by pressing start button
       /// </summary>
       /// <param name="sender"></param>
       /// <param name="e"></param>
        private void Start_Click(object sender, EventArgs e)
        {

            if (gage.isRunning) return;
            gage.Start();
            this.bw.RunWorkerAsync();
        }

    }
}
