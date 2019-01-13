using System;
using System.IO.Pipes;
using System.Text;
using System.Threading.Tasks;

namespace TabletDriverGUI
{
    class NamedPipeClient
    {
        readonly string pipeName;
        NamedPipeClientStream pipeStream;

        bool isRunning;
        readonly object lockObject = new object();

        public bool IsRunning
        {
            get
            {
                lock (lockObject) { return isRunning; }
            }
            set
            {
                lock (lockObject) { isRunning = value; }
            }
        }

        byte[] bufferRead;


        public delegate void NamedPipeEventHandler(object sender, NamedPipeEventArgs e);
        public event NamedPipeEventHandler MessageReceived;
        public event EventHandler Connected;
        public event EventHandler Disconnected;
        public class NamedPipeEventArgs : EventArgs
        {
            public Message Message;
            public NamedPipeEventArgs(Message message)
            {
                Message = message;
            }
        }

        public class Message
        {
            public byte[] Data;
            public int Length;
            public Message()
            {
                Length = 0;
            }
        }

        public class TaskResult
        {
            public bool IsSuccess { get; set; }
            public string ErrorMessage { get; set; }
        }


        //
        // Constructor
        //
        public NamedPipeClient(string pipeName)
        {
            this.pipeName = pipeName;
            bufferRead = new byte[10240];
        }

        //
        // Start
        //
        public bool Start()
        {
            if (!IsRunning)
            {
                IsRunning = true;
                Console.WriteLine("new NamedPipeClientStream " + pipeName);
                pipeStream = new NamedPipeClientStream(".", pipeName, PipeDirection.InOut, PipeOptions.Asynchronous);
                Console.WriteLine("Connecting to " + pipeName + "...");
                pipeStream.Connect(5000);
                if (pipeStream.IsConnected)
                {
                    Console.WriteLine("Connected to " + pipeName + "!");
                    OnConnected();
                    BeginRead(new Message());
                }
                else
                {
                    Console.WriteLine("Couldn't connect to " + pipeName + "!");
                    return false;
                }
                return true;
            }
            return false;
        }


        //
        // Stop
        //
        public bool Stop()
        {
            IsRunning = false;

            try
            {
                pipeStream.Close();
                pipeStream.Dispose();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Pipe stop error: " + ex.Message);
            }

            OnDisconnected();

            return true;
        }


        //
        // Begin reading from the pipe
        //
        private void BeginRead(Message message)
        {
            try
            {
                //Console.WriteLine("BeginRead " + pipeName);
                pipeStream.BeginRead(bufferRead, 0, bufferRead.Length, EndReadCallBack, message);
            }
            catch (Exception ex)
            {
                Console.WriteLine("BeginRead error: " + ex.Message);
            }
        }

        //
        // End pipe read
        //
        private void EndReadCallBack(IAsyncResult result)
        {
            int bytesRead = pipeStream.EndRead(result);
            //Console.WriteLine("EndRead length " + pipeName + ": " + bytesRead);

            if (bytesRead > 0)
            {
                var message = (Message)result.AsyncState;

                // Set message values
                message.Length = bytesRead;
                message.Data = bufferRead;

                // Process message
                OnMessageReceived(message);

                // Start a new read
                BeginRead(new Message());
            }

            // Client disconnected
            else
            {
                if (IsRunning)
                {
                    OnDisconnected();
                    Stop();
                }
            }

        }

        //
        // End pipe write
        //
        private TaskResult EndWriteCallBack(IAsyncResult asyncResult)
        {
            pipeStream.EndWrite(asyncResult);
            pipeStream.Flush();

            return new TaskResult { IsSuccess = true };
        }


        //
        // Write message
        //
        public Task<TaskResult> WriteMessage(string message)
        {
            var taskCompletionSource = new TaskCompletionSource<TaskResult>();

            if (pipeStream != null && pipeStream.IsConnected)
            {
                var buffer = Encoding.UTF8.GetBytes(message);

                //Console.WriteLine("Writing message to " + pipeName + ": " + message);

                try
                {
                    pipeStream.BeginWrite(buffer, 0, buffer.Length, asyncResult => {
                        try
                        {
                            taskCompletionSource.SetResult(EndWriteCallBack(asyncResult));
                        }
                        catch (Exception ex)
                        {
                            taskCompletionSource.SetException(ex);
                        }
                    }, null);
                }
                catch (Exception e)
                {
                    Console.WriteLine("BeginWrite exception: " + e.Message);
                }
            }

            return taskCompletionSource.Task;
        }

        //
        // Pipe connected
        //
        void OnConnected()
        {
            Console.WriteLine(pipeName + " connected!");
            Connected?.Invoke(this, new EventArgs());
        }

        //
        // Pipe disconnected
        //
        void OnDisconnected()
        {
            Disconnected?.Invoke(this, new EventArgs());
        }

        //
        // Pipe message received
        //
        void OnMessageReceived(Message message)
        {
            MessageReceived?.Invoke(this, new NamedPipeEventArgs(message));
        }
    }
}
