using System;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Timers;
using System.IO.Pipes;
using System.Windows;
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

        // Pipe
        System.Threading.Thread pipeInputThread = null;
        System.Threading.Thread pipeStateThread = null;

        NamedPipeClientStream pipeStreamInput;
        NamedPipeClientStream pipeStreamOutput;
        NamedPipeClientStream pipeStreamState;

        StreamReader pipeInputReader = null;
        StreamWriter pipeOutputWriter = null;
        BinaryReader pipeStateReader = null;

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
        public void SendCommand(string line)
        {
            if (running)
            {
                processService.StandardInput.WriteLine(line);
            }
        }

        //
        // Send command to the driver service through a named pipe
        //
        public void SendPipeCommand(string line)
        {
            if (running)
            {
                if (pipeOutputWriter != null)
                {
                    pipeOutputWriter.WriteLine(line);
                    pipeOutputWriter.Flush();
                }
            }
        }

        //
        // Add text to console
        //
        public void ConsoleAddLine(string line)
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
                if (line.Contains("[STATUS]"))
                {

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


                //ConsoleAddLine(e.Data);
                MessageReceived?.Invoke(this, new DriverEventArgs(DriverEventType.Message, e.Data, ""));
            }
        }


        //
        // Pipe input thread
        //
        private void RunPipeInputThread()
        {
            StringBuilder stringBuilder = new StringBuilder();
            char[] buffer = new char[1024];


            while (running)
            {
                pipeStreamInput = new NamedPipeClientStream(".", "TabletDriverOutput", PipeDirection.InOut);
                pipeStreamOutput = new NamedPipeClientStream(".", "TabletDriverInput", PipeDirection.InOut);

                //ConsoleAddLine("Connecting...");
                try
                {
                    pipeStreamInput.Connect();
                    pipeStreamOutput.Connect();
                }
                catch (Exception)
                {
                    // ConsoleAddLine("Input pipe connect error! " + ex.Message);
                    break;
                }
                //ConsoleAddLine("Connected!");

                pipeInputReader = new StreamReader(pipeStreamInput);
                pipeOutputWriter = new StreamWriter(pipeStreamOutput);


                while (running && pipeStreamInput.IsConnected)
                {
                    //ConsoleAddLine("Read!");
                    int bytesRead = 0;
                    try
                    {
                        bytesRead = pipeInputReader.Read(buffer, 0, buffer.Length);
                    }
                    catch (Exception)
                    {
                        // ConsoleAddLine("Can't read input pipe!");
                        break;
                    }
                    if (bytesRead == 0) continue;
                    //ConsoleAddLine("Read: " + bytesRead);

                    for (int i = 0; i < bytesRead; i++)
                    {
                        char c = buffer[i];
                        if (c == '\n' || c == '\r' || c == 0)
                        {
                            if (stringBuilder.Length > 0)
                            {
                                ConsoleAddLine(stringBuilder.ToString());
                            }
                            stringBuilder.Clear();
                        }
                        else
                        {
                            stringBuilder.Append(c);
                        }
                    }

                }

                ConsoleAddLine("Input pipe disconnected!");

                // Close input reader
                try
                {
                    pipeInputReader.Close();
                    pipeInputReader.Dispose();
                    pipeInputReader = null;
                }
                catch (Exception) { }

                // Close input stream
                try
                {
                    pipeStreamInput.Close();
                    pipeStreamInput.Dispose();
                    pipeStreamInput = null;
                }
                catch (Exception) { }



                // Close output writer
                try
                {
                    pipeOutputWriter.Close();
                    pipeOutputWriter.Dispose();
                    pipeOutputWriter = null;
                }
                catch (Exception) { }

                // Close output stream
                try
                {
                    pipeStreamOutput.Close();
                    pipeStreamOutput.Dispose();
                    pipeStreamOutput = null;
                }
                catch (Exception) { }

            }

        }


        //
        // Pipe state thread
        //
        private void RunPipeStateThread()
        {
            int size = 0;
            byte[] bytes;
            GCHandle gcHandle;
            TabletState readState;

            while (running)
            {
                if (!running) break;
                pipeStreamState = new NamedPipeClientStream(".", "TabletDriverState", PipeDirection.InOut);

                try
                {
                    pipeStreamState.Connect();
                }
                catch (Exception)
                {
                    // ConsoleAddLine("State pipe connection error! " + ex.Message);
                    break;
                }
                pipeStateReader = new BinaryReader(pipeStreamState);


                while (running && pipeStreamState.IsConnected)
                {
                    try
                    {

                        size = Marshal.SizeOf(typeof(TabletState));
                        bytes = pipeStateReader.ReadBytes(size);
                        gcHandle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
                        readState = (TabletState)Marshal.PtrToStructure(gcHandle.AddrOfPinnedObject(), typeof(TabletState));
                        gcHandle.Free();
                    }
                    catch (Exception)
                    {
                        //ConsoleAddLine("Can't read state pipe! " + ex.Message);
                        size = 0;
                        break;
                    }
                    if (size > 0)
                    {
                        tabletState = readState;
                        //ConsoleAddLine("State X: " + tabletState.inputX + ", Y: " + tabletState.inputY);
                    }


                }

                ConsoleAddLine("State pipe disconnected!");

                // Close state reader
                try
                {
                    pipeStateReader.Close();
                    pipeStateReader.Dispose();
                    pipeStateReader = null;
                }
                catch (Exception) { }


                // Close state stream
                try
                {
                    pipeStreamState.Close();
                    pipeStreamState.Dispose();
                    pipeStreamState = null;
                }
                catch (Exception) { }

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

                    // Pipe input thread
                    pipeInputThread = new System.Threading.Thread(new System.Threading.ThreadStart(RunPipeInputThread));
                    pipeInputThread.Start();

                    // Pipe state thread
                    pipeStateThread = new System.Threading.Thread(new System.Threading.ThreadStart(RunPipeStateThread));
                    pipeStateThread.Start();


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

            // Close pipe reader
            try
            {
                if (pipeInputReader != null)
                {
                    pipeInputReader.Close();
                    pipeInputReader.Dispose();
                }
            }
            catch (Exception e) { Debug.WriteLine("Pipe input reader error! " + e.Message); }

            // Close pipe input stream
            try
            {
                if (pipeStreamInput != null)
                {
                    pipeStreamInput.Close();
                    pipeStreamInput.Dispose();
                }
            }
            catch (Exception e) { Debug.WriteLine("Pipe input stream error! " + e.Message); }

            // Close pipe writer
            try
            {
                if (pipeOutputWriter != null)
                {
                    pipeOutputWriter.Close();
                    pipeOutputWriter.Dispose();
                }

            }
            catch (Exception e) { Debug.WriteLine("Pipe writer error! " + e.Message); }


            // Close pipe output stream
            try
            {
                if (pipeStreamOutput != null)
                {
                    pipeStreamOutput.Close();
                    pipeStreamOutput.Dispose();
                }
            }
            catch (Exception e) { Debug.WriteLine("Pipe output stream error! " + e.Message); }


            // Close state reader
            try
            {
                if (pipeStateReader != null)
                {
                    pipeStateReader.Close();
                    pipeStateReader.Dispose();
                }

            }
            catch (Exception e) { Debug.WriteLine("Pipe state reader error! " + e.Message); }

            // Close pipe state stream
            try
            {
                if (pipeStreamState != null)
                {
                    pipeStreamState.Close();
                    pipeStreamState.Dispose();
                }
            }
            catch (Exception e) { Debug.WriteLine("Pipe state stream error! " + e.Message); }

            // Close pipe input thread
            try
            {
                if (pipeInputThread != null)
                {
                    pipeInputThread.Abort();
                }
            }
            catch (Exception e) { Debug.WriteLine("Pipe input thread error! " + e.Message); }

            // Close pipe state thread
            try
            {
                if (pipeStateThread != null)
                {
                    pipeStateThread.Abort();
                }
            }
            catch (Exception e) { Debug.WriteLine("Pipe state thread error! " + e.Message); }


            try
            {
                processService.CancelOutputRead();
                processService.Kill();
                processService.Dispose();
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
