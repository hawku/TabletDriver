using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Timers;

namespace TabletDriverGUI
{
    public class TabletDriver
    {
        // Event handlers
        public delegate void DriverEventHandler(object sender, DriverEventArgs e);
        public event DriverEventHandler MessageReceived;
        public event DriverEventHandler ErrorReceived;
        public event EventHandler Started;
        public event EventHandler Stopped;
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

        // Console stuff
        public List<string> ConsoleBuffer;
        public bool HasConsoleUpdated;
        private readonly int ConsoleMaxLines;
        private System.Threading.Mutex mutexConsoleUpdate;

        // Other variables
        private readonly string servicePath;
        private Process processService;
        private Timer timerWatchdog;
        private bool running;
        public bool IsRunning { get { return running; } }

        //
        // Constructor
        //
        public TabletDriver(string servicePath)
        {
            this.servicePath = servicePath;
            this.processService = null;
            timerWatchdog = new Timer(2000);
            timerWatchdog.Elapsed += TimerWatchdog_Elapsed;

            ConsoleMaxLines = 300;
            ConsoleBuffer = new List<string>(ConsoleMaxLines);
            mutexConsoleUpdate = new System.Threading.Mutex();
        }

        //
        // Driver watchdog
        //
        private void TimerWatchdog_Elapsed(object sender, ElapsedEventArgs e)
        {
            if (running)
            {
                //Console.WriteLine("ID: " + processDriver.Id);
                if (processService.HasExited)
                {
                    RaiseError("Driver watchdog detected a service shutdown!");
                    Stop();
                }

                processService.Refresh();
                switch(processService.PriorityClass)
                {
                    case ProcessPriorityClass.High:
                    case ProcessPriorityClass.RealTime:
                        break;
                    default:
                        RaiseError("TabletDriverService priority too low! Run the GUI as an administrator or change the priority to high!");
                        break;
                }
            }
        }

        //
        // Send command to the driver service
        //
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
        // Console mutex lock
        //
        public void ConsoleLock()
        {
            mutexConsoleUpdate.WaitOne();
        }
        // Console mutex unlock
        public void ConsoleUnlock()
        {
            mutexConsoleUpdate.ReleaseMutex();
        }


        //
        // Raise error message
        //
        private void RaiseError(string text)
        {
            ErrorReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Error, text));
        }

        //
        // Driver service disposed
        //
        private void ProcessService_Disposed(object sender, EventArgs e)
        {
            Stop();
        }

        //
        // Driver service exited
        //
        private void ProcessService_Exited(object sender, EventArgs e)
        {
            Stop();
        }

        //
        // Driver service error received
        //
        private void ProcessService_ErrorDataReceived(object sender, DataReceivedEventArgs e)
        {
            ConsoleAddText("ERROR! " + e.Data);
            ErrorReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Error, e.Data));
        }

        //
        // Driver service data received
        //
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
        // Start the driver service
        //
        public void Start(string processPath, string arguments)
        {

            if (!File.Exists(processPath))
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

                // Start process
                if (processService.Start())
                {
                    processService.BeginOutputReadLine();

                    // Set process priority
                    try
                    {
                        processService.PriorityClass = ProcessPriorityClass.High;
                    }
                    catch (Exception)
                    {
                    }

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
        // Stop the driver service
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
