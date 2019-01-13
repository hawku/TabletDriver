using System;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Timers;
using System.Text;
using System.Runtime.InteropServices;

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
        private Dictionary<string, string> commands;
        public Dictionary<string, string> Commands { get { return commands; } }

        // Pipe stuff
        NamedPipeClient pipeInput;
        NamedPipeClient pipeOutput;
        NamedPipeClient pipeState;
        byte[] stateBytes;
        StringBuilder messageBuilder;


        [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Unicode)]
        [Serializable]
        public struct TabletState
        {
            public int index;

            public int inputButtons;
            public double inputX;
            public double inputY;
            public double inputPressure;
            public double inputVelocity;

            public int outputButtons;
            public double outputX;
            public double outputY;
            public double outputPressure;
        }
        public TabletState tabletState;


        // Other variables
        private readonly string servicePath;
        private Process processService;
        private Timer timerWatchdog;
        private bool running;
        private readonly object locker = new object();

        public bool DoNotKill;

        public bool IsRunning
        {
            get
            {
                lock (locker)
                {
                    return running;
                }
            }
            set
            {
                lock (locker)
                {
                    running = value;
                }
            }
        }

        //
        // Constructor
        //
        public TabletDriver(string servicePath)
        {
            this.servicePath = servicePath;
            processService = null;
            DoNotKill = false;
            timerWatchdog = new Timer(2000);
            timerWatchdog.Elapsed += TimerWatchdog_Elapsed;

            ConsoleMaxLines = 300;
            ConsoleBuffer = new List<string>(ConsoleMaxLines);
            mutexConsoleUpdate = new System.Threading.Mutex();

            commands = new Dictionary<string, string>();

            messageBuilder = new StringBuilder();

            pipeInput = new NamedPipeClient("TabletDriverOutput");
            pipeOutput = new NamedPipeClient("TabletDriverInput");
            pipeState = new NamedPipeClient("TabletDriverState");

            stateBytes = new byte[Marshal.SizeOf(typeof(TabletState))];
            pipeInput.MessageReceived += PipeInput_MessageReceived;
            pipeState.MessageReceived += PipeState_MessageReceived;

            pipeInput.Connected += (snd, e) => { pipeInput.WriteMessage("\n"); };
            pipeOutput.Connected += (snd, e) =>
            {
                pipeOutput.WriteMessage("\n");
            };
            pipeState.Connected += (snd, e) => { pipeState.WriteMessage("\n"); };


            // Invoke driver started event
            Started?.Invoke(this, new EventArgs());


        }


        //
        // Driver watchdog
        //
        private void TimerWatchdog_Elapsed(object sender, ElapsedEventArgs e)
        {
            if (IsRunning)
            {
                //Console.WriteLine("ID: " + processDriver.Id);
                if (processService.HasExited)
                {
                    RaiseError("Driver watchdog detected a service shutdown!");
                    Stop();
                }

                processService.Refresh();
                if (processService != null && !processService.HasExited)
                {
                    switch (processService.PriorityClass)
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
        }

        //
        // Send command to the driver service
        //
        public void SendCommand(string command)
        {
            if (IsRunning)
            {

                if (pipeOutput.IsRunning)
                {
                    pipeOutput.WriteMessage(command);
                }
                else
                {
                    processService.StandardInput.WriteLine(command);
                }
            }
        }

        //
        // Add text to console
        //
        public void ConsoleAddLine(string line)
        {
            try
            {
                mutexConsoleUpdate.WaitOne();
                ConsoleBuffer.Add(line);
                HasConsoleUpdated = true;

                // Limit console buffer size
                if (ConsoleBuffer.Count >= ConsoleMaxLines)
                    ConsoleBuffer.RemoveRange(0, ConsoleBuffer.Count - ConsoleMaxLines);
            }
            catch (Exception) { }

            try { mutexConsoleUpdate.ReleaseMutex(); } catch (Exception) { }
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
            string result = null;

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

                string[,] commandMatrix = new string[rows, columns];
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

                for (row = 0; row < rows; row++)
                {
                    if (row != 0)
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
                    ConsoleAddLine("");
                    ConsoleAddLine("Commands: ");
                    ConsoleAddLine(commandsString);
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
            ConsoleAddLine("ERROR! " + e.Data);
            ErrorReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Error, e.Data, ""));
        }

        //
        // Driver service standard output data received
        //
        private void ProcessService_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (e.Data == null)
            {
                if (IsRunning)
                    Stop();
            }
            else
            {
                //ProcessDriverMessage(e.Data + "\n");
            }
        }


        //
        // Received input pipe message
        //
        private void PipeInput_MessageReceived(object sender, NamedPipeClient.NamedPipeEventArgs e)
        {

            string stringMessage = Encoding.UTF8.GetString(e.Message.Data, 0, e.Message.Length);
            //ConsoleAddLine("Pipe: '" + stringMessage + "'");
            ProcessDriverMessage(stringMessage);

        }

        //
        // Received state pipe message
        //
        private void PipeState_MessageReceived(object sender, NamedPipeClient.NamedPipeEventArgs e)
        {
            GCHandle gcHandle;
            TabletState readState;

            // Convert bytes to TabletState
            if (e.Message.Length == stateBytes.Length)
            {
                gcHandle = GCHandle.Alloc(e.Message.Data, GCHandleType.Pinned);
                readState = (TabletState)Marshal.PtrToStructure(gcHandle.AddrOfPinnedObject(), typeof(TabletState));
                gcHandle.Free();
                tabletState = readState;
            }
        }


        //
        // Process driver message
        //
        private void ProcessDriverMessage(string messageData)
        {
            //ConsoleAddLine("Message data: '" + messageData + "'");

            // Add message data to stringbuilder
            messageBuilder.Append(messageData);


            // Find a line
            string line = "";
            int index;
            int startIndex = 0;
            for (index = 0; index < messageBuilder.Length; index++)
            {
                char c = messageBuilder[index];
                if (c == '\n')
                {
                    //ConsoleAddLine("New line at " + index);
                    if (index > 0 && index > startIndex + 1)
                    {
                        line = messageBuilder.ToString(startIndex, index - startIndex + 1).Trim();
                        ProcessDriverMessageLine(line);
                        startIndex = index;
                    }
                }
            }

            // Remove lines from stringbuilder
            if (startIndex < messageBuilder.Length)
            {
                messageBuilder.Remove(0, startIndex + 1);
            }
            else
            {
                messageBuilder.Clear();
            }


        }

        //
        // Process driver message line
        //
        private void ProcessDriverMessageLine(string line)
        {
            //ConsoleAddLine("Message line: '" + line + "'");

            // Status line?
            if (line.Contains("[STATUS]"))
            {

                // Parse status variable and value
                Match match = Regex.Match(line, "^.+\\[STATUS\\] ([^ ]+) (.*?)$");
                if (match.Success)
                {
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
            }

            ConsoleAddLine(line);
            MessageReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Message, line, ""));
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

                    IsRunning = true;
                    timerWatchdog.Start();

                    // Named pipes
                    pipeInput.Start();
                    pipeOutput.Start();
                    pipeState.Start();

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
            if (!IsRunning) return;
            timerWatchdog.Stop();
            IsRunning = false;

            // Stop named pipe clients
            pipeInput.Stop();
            pipeOutput.Stop();
            pipeState.Stop();


            // Kill service process
            Console.WriteLine("Killing TabletDriverService");
            try
            {
                if (!DoNotKill)
                {
                    processService.CancelOutputRead();
                    processService.Kill();
                    processService.Dispose();
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine("Service process error! " + e.Message);
            }

            Stopped?.Invoke(this, new EventArgs());

            System.Threading.Thread.Sleep(10);

        }


    }
}
