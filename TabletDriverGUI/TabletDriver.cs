using System;
using System.Text.RegularExpressions;
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
        public event DriverEventHandler StatusReceived;
        public event EventHandler Started;
        public event EventHandler Stopped;
        public enum DriverEventType
        {
            Error,
            Message,
            Status
        }
        public class DriverEventArgs : EventArgs
        {
            public DriverEventType Type;
            public string Message;
            public string Parameters;
            public DriverEventArgs(DriverEventType type, string message, string parameters)
            {
                Type = type;
                Message = message;
                Parameters = parameters;
            }
        }


        // Console stuff
        public List<string> ConsoleBuffer;
        public bool HasConsoleUpdated;
        private readonly int ConsoleMaxLines;
        private System.Threading.Mutex mutexConsoleUpdate;
        private Dictionary<String, String> commands;

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

            commands = new Dictionary<string, string>();

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
        // Command name complete
        //
        public string CompleteCommandName(string inputText, bool showCommands)
        {
            List<string> commandsFound = new List<string>();
            string result = inputText;

            // Find commands
            foreach (var item in commands)
            {
                if (item.Key.StartsWith(inputText.ToLower()))
                {
                    commandsFound.Add(item.Value);
                }
            }
           

            // Only one command found
            if (commandsFound.Count == 1)
            {
                result = commandsFound[0];
            }

            // Multiple commands found
            else if (commandsFound.Count > 1)
            {

                // Sort 
                commandsFound.Sort();

                // Create a string from found commands
                string commandsString = "  | ";
                int maxWidth = 1;
                foreach (string command in commandsFound)
                {
                    if (command.Length > maxWidth)
                    {
                        maxWidth = command.Length;
                    }
                }
                int columns = (int)Math.Ceiling(100.0 / maxWidth);
                int rows = (int)Math.Ceiling((double)commandsFound.Count / columns);

                string[,] commandMatrix = new string[rows,columns];
                int row = 0;
                int column = 0;

                foreach (string command in commandsFound)
                {
                    if (rows == 1)
                    {
                        commandMatrix[row, column] = command + " | ";
                    }
                    else
                    {
                        commandMatrix[row, column] = command.PadRight(maxWidth) + " | ";
                    }
                    row++;
                    if (row >= rows)
                    {
                        row = 0;
                        column++;
                        if (column >= columns) break;
                    }
                }

                for(row = 0; row < rows; row++) {
                    if(row != 0)
                    commandsString += "\r\n  | ";
                    for (column = 0; column < columns; column++)
                    {
                        string commandString = commandMatrix[row, column];
                        if (commandString != null)
                        {
                            commandsString += commandString;
                        }
                    }
                }

                // Add commands to console output
                if (showCommands)
                {
                    ConsoleAddText("");
                    ConsoleAddText("Commands: ");
                    ConsoleAddText(commandsString);
                }

                // Fill input text
                string completedText = inputText;

                // Loop through commands
                foreach (string name in commandsFound)
                {
                    for (int i = 1; i <= name.Length; i++)
                    {
                        string completeTest = name.Substring(0, i);
                        bool completeOK = true;

                        // Check if the command complete is OK
                        foreach (string name2 in commandsFound)
                        {
                            // Not OK
                            if (!name2.ToLower().StartsWith(completeTest.ToLower()))
                            {
                                completeOK = false;
                                break;
                            }
                        }
                        if (completeOK)
                        {
                            completedText = completeTest;
                        }
                    }
                }

                result = completedText;
            }

            return result;
        }


        //
        // Raise error message
        //
        private void RaiseError(string text)
        {
            ErrorReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Error, text, ""));
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
            ErrorReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Error, e.Data, ""));
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
                string line = e.Data;

                // Status line?
                if (line.Contains("[STATUS]")) {

                    // Parse status variable and value
                    Match match = Regex.Match(line, "^.+\\[STATUS\\] ([^ ]+) (.*?)$");
                    if (!match.Success) return;

                    string variableName = match.Groups[1].ToString().ToLower();
                    string parameters = match.Groups[2].ToString();

                    //
                    // Commands status message
                    //
                    if (variableName == "commands")
                    {
                        string[] commandNames = parameters.Split(' ');
                        string lowerCaseName;
                        foreach (string name in commandNames)
                        {
                            lowerCaseName = name.Trim().ToLower();
                            if (lowerCaseName.Length > 0)
                            {
                                if (!commands.ContainsKey(lowerCaseName))
                                {
                                    commands.Add(lowerCaseName, name);
                                }
                            }
                        }
                    }

                    StatusReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Status, variableName, parameters));

                }


                ConsoleAddText(e.Data);
                MessageReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Message, e.Data, ""));
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
