using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GageWrapper;

using System.Runtime.InteropServices; 

namespace ConsoleApplication1
{

    class Program{
        //http://stackoverflow.com/questions/474679/capture-console-exit-c-sharp
        [DllImport("Kernel32")]
        private static extern bool SetConsoleCtrlHandler(EventHandler handler, bool add);
        private delegate bool EventHandler(CtrlType sig);
        static EventHandler _handler;

        enum CtrlType
        {
            CTRL_C_EVENT = 0,
            CTRL_BREAK_EVENT = 1,
            CTRL_CLOSE_EVENT = 2,
            CTRL_LOGOFF_EVENT = 5,
            CTRL_SHUTDOWN_EVENT = 6
        }
        static Gage gage;
        static bool isRunning = true;
        private static bool Handler(CtrlType sig)
        {
            switch (sig)
            {
                case CtrlType.CTRL_C_EVENT: isRunning = false; gage.Exit(); break;
                case CtrlType.CTRL_LOGOFF_EVENT: isRunning = false; gage.Exit(); break;
                case CtrlType.CTRL_SHUTDOWN_EVENT: isRunning = false; gage.Exit(); break;
                case CtrlType.CTRL_CLOSE_EVENT: isRunning = false; gage.Exit(); break;
                default:
                    break;
            }
            return true;
        }

      
        static void print(string str) {
            Console.Write(str);
        }
         static void Main(string[] args){
             _handler += new EventHandler(Handler);
             SetConsoleCtrlHandler(_handler, true);



             OutputDelegate output = print;
             gage = new Gage(output);
             gage.Initialize();
             gage.Start();
             while (isRunning){
                 gage.Capture();
             }
            
        }
    }
}
