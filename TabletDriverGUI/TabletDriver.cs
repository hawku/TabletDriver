using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Timers;

namespace TabletDriverGUI
{
    public class TabletDriver
    {
        public delegate void DriverEventHandler(object sender, DriverEventArgs e);

        public event DriverEventHandler MessageReceived;
        public event DriverEventHandler ErrorReceived;
        public event EventHandler Started;
        public event EventHandler Stopped;


        public List<string> ConsoleBuffer;
        public bool HasConsoleUpdated;
        private int ConsoleMaxLines;
        private Mutex mutexConsoleUpdate;


        public enum DriverEventType
        {
            Error,
            Message
        }
        public class DriverEventArgs : EventArgs
        {
            public DriverEventType Type;
            public string Message;
            public DriverEventArgs(DriverEventType type, string message)
            {
                Type = type;
                Message = message;
            }
        }

        private string servicePath;
        private Process processService;
        private System.Timers.Timer timerWatchdog;
        private bool running;

        public TabletDriver(string servicePath)
        {
            this.servicePath = servicePath;
            timerWatchdog = new System.Timers.Timer(1000);
            timerWatchdog.Elapsed += TimerWatchdog_Elapsed;

            ConsoleMaxLines = 300;
            ConsoleBuffer = new List<string>(ConsoleMaxLines);
            mutexConsoleUpdate = new Mutex();
        }

        // 
        public bool IsRunning { get { return running; } }


        // Watchdog timer
        private void TimerWatchdog_Elapsed(object sender, ElapsedEventArgs e)
        {
            if (running)
            {
                //Console.WriteLine("ID: " + processDriver.Id);
                if (processService.HasExited)
                {
                    RaiseError("Watchdog detected a driver service shutdown!");
                    Stop();
                }
            }
        }


        public void SendCommand(string line)
        {
            if (running)
                processService.StandardInput.WriteLine(line);
        }

        //
        // Add text to console
        //
        public void ConsoleAddText(string line)
        {
            mutexConsoleUpdate.WaitOne();
            ConsoleBuffer.Add(line);
            HasConsoleUpdated = true;

            // Limit console buffer size
            if (ConsoleBuffer.Count >= ConsoleMaxLines) 
                ConsoleBuffer.RemoveRange(0, ConsoleBuffer.Count - ConsoleMaxLines);

            mutexConsoleUpdate.ReleaseMutex();
        }


        //
        // Console lock
        //
        public void ConsoleLock()
        {
            mutexConsoleUpdate.WaitOne();
        }

        public void ConsoleUnlock()
        {
            mutexConsoleUpdate.ReleaseMutex();
        }


        private void RaiseError(string text)
        {
            ErrorReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Error, text));
        }

        //
        // Handle driver service process events
        //
        private void ProcessService_Disposed(object sender, EventArgs e)
        {
            Stop();
        }

        private void ProcessService_Exited(object sender, EventArgs e)
        {
            Stop();
        }

        private void ProcessService_ErrorDataReceived(object sender, DataReceivedEventArgs e)
        {
            ConsoleAddText("ERROR! " + e.Data);
            ErrorReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Error, e.Data));
        }

        private void ProcessService_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (e.Data == null)
            {
                if (running)
                    Stop();
            }
            else
            {
                ConsoleAddText(e.Data);
                MessageReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Message, e.Data));
            }
        }

        //
        // Start driver
        //
        public void Start(string processPath, string arguments)
        {

            if(!File.Exists(processPath))
            {
                throw new FileNotFoundException(processPath + " not found!");
            }

 
            // Try to start the driver
            try
            {
                // Create process start info
                ProcessStartInfo startInfo = new ProcessStartInfo
                {
                    FileName = processPath,
                    Arguments = arguments,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    RedirectStandardInput = true,
                    CreateNoWindow = true,
                    WindowStyle = ProcessWindowStyle.Normal
                };

                // Create process
                processService = new Process
                {
                    StartInfo = startInfo
                };
                processService.OutputDataReceived += ProcessService_OutputDataReceived;
                processService.ErrorDataReceived += ProcessService_ErrorDataReceived;
                processService.Exited += ProcessService_Exited;
                processService.Disposed += ProcessService_Disposed;


                if (processService.Start())
                {
                    //consoleHistory = new List<string>();
                    processService.BeginOutputReadLine();


                    running = true;
                    timerWatchdog.Start();

                    Started?.Invoke(this, new EventArgs());
                }

                // Start failed
                else
                {
                    throw new Exception("Can't start the driver service!");
                }
            }

            // Start failed
            catch (Exception e)
            {
                throw e;
            }
        }


        //
        // Stop driver
        //
        public void Stop()
        {
            if (!running) return;
            running = false;
            timerWatchdog.Stop();
            try
            {
                processService.CancelOutputRead();
                processService.Kill();
                processService.Dispose();
            }
            catch (Exception)
            {
            }
            Stopped?.Invoke(this, new EventArgs());
        }


    }
}
