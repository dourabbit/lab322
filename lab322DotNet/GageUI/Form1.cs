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
using System.IO;
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
        private static List<string> MsgHistory = new List<string>(); 
        private static void print(string str)
        {
            TextBox output = Form1.GetOutput();
            string[] buffer = str.Split('\r');
            if (buffer.Length > 1)
                MsgHistory[MsgHistory.Count - 1] = str;
            else
                MsgHistory.Add(str);
            
            String msg = "Testing Version";
            for (int i = 0; i < MsgHistory.Count; ++i)
                msg += '\n' + MsgHistory[i];

            if (output.InvokeRequired)
            {
                m_SetLabel setLabel = SetLabelText;
                Form1 form = Form1.GetForm();

                if (gage.State != Gage.GageState.Stop)
                    form.Invoke(setLabel, msg);
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
        private UInt16 _MAXLOOPs = 6000;
        private int _curLoopCount = 0;
        private int _drawedLoopCount = 0;
        private int _dataGridMaxRow = 50;
        private int _dataGridMaxCol = 120;
        private int _gridCellSize = 5;
        private UInt16 _dataLength4SingleChanl;
        private static Form1 singleton;
        private OutputDelegate outputFnc = print;
        private System.ComponentModel.BackgroundWorker bw;
        private Bitmap backBuffer;
        private List<Bitmap> gridsGraphs;
        private Bitmap background;
        private List<PictureBox> paintBoxes;
        private System.Drawing.Pen pen;
        private  List<System.Drawing.SolidBrush> brushes;
        private short[] RawDATA4Write;
        private byte[] byteArray;  
        private static UInt16 _numOfChannel = 5;
        public static TextBox GetOutput() { return singleton.OutputBox; }
        public static PictureBox GetPaint(int index) { return singleton.paintBoxes[index]; }
        public static Form1 GetForm() { return singleton; }
        public Form1()
        {
            InitializeComponent();

            OutputBox.TextChanged += OutputBox_TextChanged;
            paintBoxes = new List<PictureBox>();
            _settingRange = new List<short[]>();
            gridsGraphs = new List<Bitmap>();

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

                Bitmap grid = new Bitmap(_dataGridMaxCol,  _dataGridMaxRow);
                gridsGraphs.Add(grid);
                
                backBuffer = new Bitmap(paintBoxes[0].Width, paintBoxes[0].Height);
                range[0] = _initLeftRange; range[1] = _initRightRange;
                _settingRange.Add(range);
                flowLayoutPanel1.Controls.Add(paintBox);
            }


            background = new Bitmap(paintBoxes[0].Width, paintBoxes[0].Height);
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
       


            this.bw  = new System.ComponentModel.BackgroundWorker();
            this.bw.WorkerSupportsCancellation = false;
            this.bw.DoWork += new System.ComponentModel.DoWorkEventHandler(this.DoWork);
            this.bw.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.CompleteWork);

            singleton = this;



            //Draw graph
            System.Timers.Timer aTimer = new System.Timers.Timer();
            aTimer.Elapsed += new ElapsedEventHandler(Draw);
            aTimer.Interval = 333;
            aTimer.Enabled = true;

            System.Timers.Timer bTimer = new System.Timers.Timer();
            bTimer.Elapsed += new ElapsedEventHandler(DrawGrid);
            bTimer.Interval = 1000;
            bTimer.Enabled = true;

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

        }


        private void DrawGrid(object source, ElapsedEventArgs e)
        {

            for (int i = 0; i < _numOfChannel; ++i)
            {

                drawGrid2Img(i);

            }
            if (_drawedLoopCount < _curLoopCount)
                _drawedLoopCount = _curLoopCount;
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            gage = new Gage(outputFnc);

            gage.SetGage(Gage.GageState.Init);
            _dataLength4SingleChanl = Gage.length4SingleChannl;
            RawDATA4Write = new short[_MAXLOOPs * Gage.offset * _dataLength4SingleChanl];
            byteArray = new byte[1024];
            _maxiumValue = new ushort[_numOfChannel, _MAXLOOPs];
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (gage.State == Gage.GageState.Start)
                gage.SetGage(Gage.GageState.Stop);
        }

        private delegate void WorkerEventHandler(
            ref Gage gage,
            AsyncOperation asyncOp);

       private void DoWork(object sender, DoWorkEventArgs e) {
           Gage.GageState state = gage.GetGage();
           if(state == Gage.GageState.Pause)
               gage.SetGage(Gage.GageState.Pause);
           else
               gage.SetGage(Gage.GageState.Start);
       }
       /// <summary>
       /// Called by finishing the job
       /// </summary>
       /// <param name="sender"></param>
       /// <param name="e"></param>
       private void CompleteWork(object sender, RunWorkerCompletedEventArgs e)
       {

           if (gage.State == Gage.GageState.SysError)//Put error handler here
           {
               return;
           }



           if (_curLoopCount < _MAXLOOPs)
           {
               Array.Copy(gage.result, 0, this.RawDATA4Write, _curLoopCount*Gage.offset * _dataLength4SingleChanl, gage.result.Length);

               for (int channel = 0; channel < _numOfChannel; channel++) 
               {
                   for (int i = 0; i <  _dataLength4SingleChanl; i++)
                   {

                       if (i > _initLeftRange && i < _initRightRange)
                       {
                           short value = gage.result[channel + i * Gage.offset];
                           ushort maxValue = (ushort)(value < 0 ? -value : value);
                           maxValue = (maxValue == 32768) ? maxValue-- : maxValue;
                           _maxiumValue[channel, _curLoopCount] = maxValue > _maxiumValue[channel, _curLoopCount] ? maxValue : _maxiumValue[channel, _curLoopCount];
                       }

                   }
               
               }
               Gage.GageState state = gage.GetGage();
               if (state == Gage.GageState.Start || state == Gage.GageState.Pause)
                {
                    _curLoopCount++;
                    this.bw.RunWorkerAsync();
                }
           }
           else
           {
               _curLoopCount = 0;
               _drawedLoopCount = 0;
               Array.Clear(_maxiumValue, 0, _maxiumValue.Length);
               this.bw.RunWorkerAsync();
           }

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
               graphics.DrawImage(background, 0, 0);
               lock (gridsGraphs[chanelIndex])
                   graphics.DrawImage(gridsGraphs[chanelIndex], _gridGraphCorner.X, _gridGraphCorner.Y, _gridCellSize * _dataGridMaxCol, _gridCellSize * _dataGridMaxRow);


               int totalPoints = (data.Length / Gage.offset);
               float deltaX = (float)(_gageGraphSize.X - 2 * _gageGraphCorner.X) / (float)totalPoints;
               float offsetY = short.MaxValue;
               float scaleY = (float)(_gageGraphSize.Y - 2 * _gageGraphCorner.Y) / (float)(short.MaxValue * 2);
               int i = 0;
               while (chanelIndex + Gage.offset < data.Length)
               {
                   float x0,x1,y0,y1;
                   x0 = _gageGraphCorner.X + deltaX * i++;
                   x1 = _gageGraphCorner.X + deltaX * i;
                   y0 = _gageGraphCorner.Y + (data[chanelIndex] + offsetY) * scaleY;
                   y1 = _gageGraphCorner.Y + (data[chanelIndex + Gage.offset] + offsetY) * scaleY;
                   graphics.DrawLine(this.pen, x0,y0 ,x1 , y1);
                   chanelIndex += Gage.offset;
               }
           }
           using (var g = paintBox.CreateGraphics())
               g.DrawImage(backBuffer, 0, 0);
       }





        /// <summary>
        /// draw grid data, locking source 
       /// http://stackoverflow.com/questions/1060280/invalidoperationexception-object-is-currently-in-use-elsewhere-red-cross
        /// </summary>
        /// <param name="chanelIndex"></param>
       private void drawGrid2Img(int chanelIndex)
       {
                if (_drawedLoopCount < _curLoopCount)//_drawedLoopCount < _curLoopCount)
                {
                    int numOfGrid = _curLoopCount - _drawedLoopCount;

                   
                    lock (gridsGraphs[chanelIndex])
                    using (var g = Graphics.FromImage(gridsGraphs[chanelIndex]))
                    {
                       
                       g.Clear(Color.Empty);

                       lock (gridsGraphs[chanelIndex]) {

                           for (int col = 0; col < _dataGridMaxCol; col++)
                           {

                               for (int row = 0; row < _dataGridMaxRow; row++)
                               {
                                   int curIndex = row * _dataGridMaxCol + col;
                                   Color color;
                                   if (_curLoopCount < curIndex)
                                   {
                                       break;
                                       //color = Color.FromArgb(0,0,0,0);//reset of pixels are empty
                                   }
                                   else
                                   {
                                       ushort maxium = this._maxiumValue[chanelIndex, curIndex];
                                       double blend = (double)maxium / (double)short.MaxValue;
                                       color = Blend(Color.Blue, Color.Red, blend);
                                   }
                                   gridsGraphs[chanelIndex].SetPixel(col, row, color);
                               }
                           }
                       
                       }
                           // g.DrawImage(grid, _gridGraphCorner.X, _gridGraphCorner.Y, _gridCellSize * _dataGridMaxCol, _gridCellSize * _dataGridMaxRow);

                   }
               }


       }

      
       /// <summary>
       /// excuted by pressing start button
       /// </summary>
       /// <param name="sender"></param>
       /// <param name="e"></param>
        private void Start_Click(object sender, EventArgs e)
        {
            Gage.GageState state = gage.GetGage();
            if (state == Gage.GageState.Init)
             this.bw.RunWorkerAsync();
        }
        private void OutputBox_TextChanged(object sender, EventArgs e)
        {
            using (StreamWriter writer = File.AppendText("log.txt"))
            {
                writer.Write(DateTime.Now); writer.Write(" :: ");
                writer.WriteLine(MsgHistory[MsgHistory.Count-1]);
                writer.Flush();
                writer.Close();
            }
        }
        private void Write_Click(object sender, EventArgs e)
        {
             try
            {
                using (FileStream myFStream = new FileStream("temp.dat", FileMode.Create, FileAccess.Write))
                { 
                    BinaryWriter binWrit = new BinaryWriter(myFStream);
               
                    
                    //var byteArray = new byte[2 + 2 + 2 + RawDATA4Write.Length * 2];//header0 + header1 + header2
                    var headerBytes = new byte[6];
                    byte[] intBytes;
                    //Write Headers//if (BitConverter.IsLittleEndian) 
                    //UInt16 _MAXLOOPS, _numOfChannel, _dataLength4SingleChanl
                    intBytes = BitConverter.GetBytes(_MAXLOOPs);
                    Buffer.BlockCopy(intBytes, 0, headerBytes, 0, intBytes.Length);

                    intBytes = BitConverter.GetBytes(Gage.offset);
                    Buffer.BlockCopy(intBytes, 0, headerBytes, 2, intBytes.Length);

                    intBytes = BitConverter.GetBytes(_dataLength4SingleChanl);
                    Buffer.BlockCopy(intBytes, 0, headerBytes, 4, intBytes.Length);

                    myFStream.Write(headerBytes,0, headerBytes.Length);

                    

                    ////Write Data
                    var cacheSize = RawDATA4Write.Length / _MAXLOOPs;

                    var DataBytes = new byte[RawDATA4Write.Length * 2 / _MAXLOOPs];
                   
                    for (int i = 0; i < _MAXLOOPs; i++) {
                        Buffer.BlockCopy(RawDATA4Write, cacheSize * i, DataBytes, 0, cacheSize);
                        myFStream.Write(DataBytes, 0, DataBytes.Length);
                    };
                    //Buffer.BlockCopy(RawDATA4Write, 0, byteArray, 6, RawDATA4Write.Length);
                    //myFStream.Write(byteArray, 0, byteArray.Length);
                    myFStream.Close();
                }
            }
             catch (Exception err)
             {
                 // Error
                 print("Exception caught in process: " + err.ToString());
             }
        }

    }
}
