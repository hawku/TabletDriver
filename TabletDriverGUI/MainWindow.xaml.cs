using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Threading;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        // Version
        public string Version = "0.2.4";

        // Console stuff
        private List<string> commandHistory;
        private int commandHistoryIndex;

        // Notify icon
        private System.Windows.Forms.NotifyIcon notifyIcon;
        public bool IsRealExit;

        // Driver
        private TabletDriver driver;
        private Dictionary<String, String> driverCommands;
        private List<string> settingCommands;
        private bool running;
        private int tabletButtonCount;

        // Timers
        private DispatcherTimer timerStatusbar;
        private DispatcherTimer timerRestart;
        private DispatcherTimer timerConsoleUpdate;
        private DispatcherTimer timerUpdatePenPositions;

        // Config
        private Configuration config;
        private string configFilename;
        private bool isFirstStart = false;
        private bool isLoadingSettings;

        // Measurement to area
        private bool isEnabledMeasurementToArea = false;

        //
        // Constructor
        //
        public MainWindow()
        {
            // Set the current directory as TabletDriverGUI.exe's directory.
            try { Directory.SetCurrentDirectory(AppDomain.CurrentDomain.BaseDirectory); } catch (Exception) { }

            //
            // Prevent triggering input field events
            //
            isLoadingSettings = true;

            // Initialize WPF
            InitializeComponent();

            // Version text
            textVersion.Text = this.Version;

            // Set culture to en-US to force decimal format and etc.
            CultureInfo.DefaultThreadCurrentCulture = new CultureInfo("en-US");


            // Create notify icon
            notifyIcon = new System.Windows.Forms.NotifyIcon
            {
                // Icon
                Icon = Properties.Resources.AppIcon,

                // Menu items
                ContextMenu = new System.Windows.Forms.ContextMenu(new System.Windows.Forms.MenuItem[]
                {
                    new System.Windows.Forms.MenuItem("TabletDriverGUI " + Version),
                    new System.Windows.Forms.MenuItem("Restart Driver", NotifyRestartDriver),
                    new System.Windows.Forms.MenuItem("Show", NotifyShowWindow),
                    new System.Windows.Forms.MenuItem("Exit", NotifyExit)
                })
            };
            notifyIcon.ContextMenu.MenuItems[0].Enabled = false;

            notifyIcon.Text = "";
            notifyIcon.MouseClick += NotifyIcon_Click;
            notifyIcon.Visible = true;
            IsRealExit = false;

            // Create command history list
            commandHistory = new List<string> { "" };
            commandHistoryIndex = 0;

            // Init setting commands list
            settingCommands = new List<string>();

            // Init tablet driver
            driver = new TabletDriver("TabletDriverService.exe");
            driverCommands = new Dictionary<string, string>();
            driver.MessageReceived += OnDriverMessageReceived;
            driver.ErrorReceived += OnDriverErrorReceived;
            driver.StatusReceived += OnDriverStatusReceived;
            driver.Started += OnDriverStarted;
            driver.Stopped += OnDriverStopped;
            running = false;


            // Restart timer
            timerRestart = new DispatcherTimer
            {
                Interval = new TimeSpan(0, 0, 5)
            };
            timerRestart.Tick += TimerRestart_Tick;

            // Statusbar timer
            timerStatusbar = new DispatcherTimer
            {
                Interval = new TimeSpan(0, 0, 5)
            };
            timerStatusbar.Tick += TimerStatusbar_Tick;

            // Timer console update
            timerConsoleUpdate = new DispatcherTimer
            {
                Interval = new TimeSpan(0, 0, 0, 0, 200)
            };
            timerConsoleUpdate.Tick += TimerConsoleUpdate_Tick;

            // Tooltip timeout
            ToolTipService.ShowDurationProperty.OverrideMetadata(
                typeof(DependencyObject), new FrameworkPropertyMetadata(60000));


            //
            // Hide tablet button mapping
            //
            groupBoxTabletButtons.Visibility = Visibility.Collapsed;


            // Ink canvas undo history
            inkCanvasUndoHistory = new StrokeCollection();

            // Ink canvas drawing attributes
            inkCanvasDrawingAttributes = new DrawingAttributes
            {
                Width = 10,
                Height = 10,
                Color = Color.FromRgb(0x55, 0x55, 0x55),
                StylusTip = StylusTip.Ellipse,
                FitToCurve = false
            };
            inkCanvas.DefaultDrawingAttributes = inkCanvasDrawingAttributes;

            // Process command line arguments
            ProcessCommandLineArguments();

            // Events
            Closing += MainWindow_Closing;
            Loaded += MainWindow_Loaded;
            SizeChanged += MainWindow_SizeChanged;

        }


        #region Window events

        // Window is closing -> Stop driver
        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            // Close tablet view
            if (tabletView != null)
                tabletView.Close();

            // Hide notify icon
            notifyIcon.Visible = false;

            // Write configuration to XML file
            try { config.Write(configFilename); }
            catch (Exception) { }

            // Stop driver
            StopDriver();
        }

        // Window loaded -> Start driver
        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {

            // Configuration filename
            configFilename = "config/config.xml";

            // Load configuration
            try
            {
                config = Configuration.CreateFromFile(configFilename);
            }
            catch (Exception)
            {
                driver.ConsoleAddLine("New config created!");
                isFirstStart = true;
                config = new Configuration();
            }

            // Create setting elements
            CreateSettingElements();

            // Create canvas elements
            CreateCanvasElements();

            // Initialize configuration
            InitializeConfiguration();


            // Hide the window if the GUI is started as minimized
            if (WindowState == WindowState.Minimized)
            {
                Hide();
            }

            //
            // Allow input field events
            //
            isLoadingSettings = false;

            // Start the driver
            StartDriver();

        }


        //
        // Window key down
        //
        private void Window_PreviewKeyDown(object sender, KeyEventArgs e)
        {

            // Control + S -> Save settings
            if (e.KeyboardDevice.Modifiers == ModifierKeys.Control && e.Key == Key.S)
            {
                SaveSettings(sender, null);
                e.Handled = true;
            }

            // Control + R -> Restart driver
            if (e.KeyboardDevice.Modifiers == ModifierKeys.Control && e.Key == Key.R)
            {
                RestartDriverClick(sender, null);
                e.Handled = true;
            }


            // Control + I -> Import settings
            if (e.KeyboardDevice.Modifiers == ModifierKeys.Control && e.Key == Key.I)
            {
                MainMenuClick(mainMenuImport, null);
                e.Handled = true;
            }

            // Control + E -> Export settings
            if (e.KeyboardDevice.Modifiers == ModifierKeys.Control && e.Key == Key.E)
            {
                MainMenuClick(mainMenuExport, null);
                e.Handled = true;
            }

            //
            // Move screen or tablet area with keys
            //
            if (canvasScreenMap.IsFocused || canvasTabletArea.IsFocused)
            {
                int deltaX = 0;
                int deltaY = 0;

                // Arrow keys to delta
                if (e.Key == Key.Left)
                    deltaX = -1;
                else if (e.Key == Key.Right)
                    deltaX = 1;
                else if (e.Key == Key.Up)
                    deltaY = -1;
                else if (e.Key == Key.Down)
                    deltaY = 1;
                if (deltaX == 0 && deltaY == 0)
                    return;

                // Control + arrow -> 10x movement
                if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
                {
                    deltaX *= 10;
                    deltaY *= 10;
                }

                // Screen map
                if (canvasScreenMap.IsFocused)
                {
                    SetStatus("Sender: " + sender);
                    config.SelectedScreenArea.X += deltaX;
                    config.SelectedScreenArea.Y += deltaY;
                    LoadSettingsFromConfiguration();
                    e.Handled = true;
                }

                // Tablet area
                else if (canvasTabletArea.IsFocused)
                {
                    config.SelectedTabletArea.X += deltaX;
                    config.SelectedTabletArea.Y += deltaY;
                    LoadSettingsFromConfiguration();
                    e.Handled = true;
                }
            }

        }


        //
        // Process command line arguments
        //
        void ProcessCommandLineArguments()
        {
            string[] args = Environment.GetCommandLineArgs();
            for (int i = 0; i < args.Length; i++)
            {
                // Skip values
                if (!args[i].StartsWith("-") && !args[i].StartsWith("/")) continue;

                // Remove '-' and '/' characters at the start of the argument
                string parameter = Regex.Replace(args[i], "^[\\-/]+", "").ToLower();

                //
                // Parameter: --hide
                //
                if (parameter == "hide")
                {
                    WindowState = WindowState.Minimized;
                }
            }
        }

        #endregion



        #region Notify icon stuff

        //
        // Notify icon click -> show window
        //
        private void NotifyIcon_Click(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // Is not mouse left button?
            if (e.Button != System.Windows.Forms.MouseButtons.Left)
                return;

            // Minimized -> Show window
            if (WindowState == WindowState.Minimized)
            {
                NotifyShowWindow(sender, e);
            }

            // Hide window
            else
            {
                NotifyHideWindow(sender, e);
            }
        }

        // Window minimizing -> minimize to taskbar
        protected override void OnStateChanged(EventArgs e)
        {
            if (WindowState == WindowState.Minimized)
            {
                Hide();
            }
        }


        // 'Restart driver' handler for taskbar menu
        void NotifyRestartDriver(object sender, EventArgs e)
        {
            RestartDriverClick(sender, null);
        }

        // 'Hide' handler for taskbar menu
        void NotifyHideWindow(object sender, EventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        // 'Show' handler for taskbar menu
        void NotifyShowWindow(object sender, EventArgs e)
        {
            Show();
            WindowState = WindowState.Normal;
            Activate();
        }

        // 'Exit' handler for taskbar menu
        void NotifyExit(object sender, EventArgs e)
        {
            IsRealExit = true;
            Application.Current.Shutdown();
        }

        #endregion



        #region Statusbar

        //
        // Update statusbar
        //
        private void SetStatus(string text)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                textStatus.Text = text;
            });
            timerStatusbar.Stop();
            timerStatusbar.Start();
        }

        //
        // Update statusbar
        //
        private void SetStatusWarning(string text)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                textStatusWarning.Text = text;
            });
            timerStatusbar.Stop();
            timerStatusbar.Start();
        }


        //
        // Statusbar warning text click
        //
        private void StatusWarning_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // Open Task Manager 
            if (textStatusWarning.Text.ToLower().Contains("priority"))
            {
                try { Process.Start("taskmgr.exe"); } catch (Exception) { }
            }
        }


        //
        // Statusbar timer tick
        //
        private void TimerStatusbar_Tick(object sender, EventArgs e)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                textStatus.Text = "";
                textStatusWarning.Text = "";
            });
            timerStatusbar.Stop();
        }

        #endregion




        #region WndProc

        //
        // Add WndProc hook
        //
        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);
            HwndSource source = PresentationSource.FromVisual(this) as HwndSource;
            source.AddHook(WndProc);
        }

        //
        // Process Windows messages
        //
        private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {

            // Show TabletDriverGUI
            if (msg == NativeMethods.WM_SHOWTABLETDRIVERGUI)
            {
                if (WindowState == WindowState.Minimized)
                {
                    NotifyShowWindow(null, null);
                }
                else
                {
                    Activate();
                }
            }

            return IntPtr.Zero;
        }


        #endregion


        private void MouseTest(object sender, MouseButtonEventArgs e)
        {
            SetStatus("Event: " + e.RoutedEvent.ToString() + ", Mouse at " + ((UIElement)sender).ToString() + "! " + e.ChangedButton.ToString() + " " + e.ButtonState.ToString());
        }

    }
}