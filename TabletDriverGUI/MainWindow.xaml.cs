using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        // Version
        public string Version = "0.2.1";

        // Console stuff
        private List<string> commandHistory;
        private int commandHistoryIndex;

        // Notify icon
        private System.Windows.Forms.NotifyIcon notifyIcon;
        public bool IsRealExit;

        // Driver
        private TabletDriver driver;
        private Dictionary<String, String> driverCommands;
        private bool running;


        // Timers
        private DispatcherTimer timerStatusbar;
        private DispatcherTimer timerRestart;
        private DispatcherTimer timerConsoleUpdate;

        // Config
        private Configuration config;
        private string configFilename;
        private bool isFirstStart = false;
        private bool isLoadingSettings;

        // Screen map canvas elements
        private Rectangle[] rectangleMonitors;
        private Rectangle rectangleDesktop;
        private Rectangle rectangleScreenMap;
        private TextBlock textScreenAspectRatio;

        // Tablet area canvas elements
        private Polygon polygonTabletFullArea;
        private Polygon polygonTabletArea;
        private Polygon polygonTabletAreaArrow;
        private TextBlock textTabletAspectRatio;

        // Mouse drag
        private class MouseDrag
        {
            public bool IsMouseDown;
            public object Source;
            public Point OriginMouse;
            public Point OriginDraggable;
            public MouseDrag()
            {
                IsMouseDown = false;
                Source = null;
                OriginMouse = new Point(0, 0);
                OriginDraggable = new Point(0, 0);
            }
        }
        MouseDrag mouseDrag;

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
            notifyIcon.DoubleClick += NotifyIcon_DoubleClick;
            notifyIcon.Click += NotifyIcon_DoubleClick;
            notifyIcon.Visible = true;
            IsRealExit = false;

            // Create command history list
            commandHistory = new List<string> { "" };
            commandHistoryIndex = 0;

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
            // Buttom Map ComboBoxes
            //
            comboBoxButton1.Items.Clear();
            comboBoxButton2.Items.Clear();
            comboBoxButton3.Items.Clear();
            comboBoxButton1.Items.Add("Disable");
            comboBoxButton2.Items.Add("Disable");
            comboBoxButton3.Items.Add("Disable");
            for (int i = 1; i <= 5; i++)
            {
                comboBoxButton1.Items.Add("Mouse " + i);
                comboBoxButton2.Items.Add("Mouse " + i);
                comboBoxButton3.Items.Add("Mouse " + i);
            }
            comboBoxButton1.SelectedIndex = 0;
            comboBoxButton2.SelectedIndex = 0;
            comboBoxButton3.SelectedIndex = 0;


            //
            // Smoothing rate ComboBox
            //
            comboBoxSmoothingRate.Items.Clear();
            for (int i = 1; i <= 8; i++)
            {
                comboBoxSmoothingRate.Items.Add((1000.0 / i).ToString("0") + " Hz");
            }
            comboBoxSmoothingRate.SelectedIndex = 3;

            // Process command line arguments
            ProcessCommandLineArguments();

            // Events
            Closing += MainWindow_Closing;
            Loaded += MainWindow_Loaded;
            SizeChanged += MainWindow_SizeChanged;

            //
            // Allow input field events
            //
            isLoadingSettings = false;
        }


        #region Window events

        // Window is closing -> Stop driver
        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            notifyIcon.Visible = false;
            try
            {
                config.Write(configFilename);
            }
            catch (Exception)
            {
            }
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
                driver.ConsoleAddText("New config created!");
                isFirstStart = true;
                config = new Configuration();
            }
            isLoadingSettings = true;
            Width = config.WindowWidth;
            Height = config.WindowHeight;
            isLoadingSettings = false;


            if (!config.DeveloperMode)
            {
            }


            // Invalid config -> Set defaults
            if (config.ScreenArea.Width == 0 || config.ScreenArea.Height == 0)
            {
                config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                config.DesktopSize.Height = GetVirtualDesktopSize().Height;
                config.ScreenArea.Width = config.DesktopSize.Width;
                config.ScreenArea.Height = config.DesktopSize.Height;
                config.ScreenArea.X = 0;
                config.ScreenArea.Y = 0;
            }

            // Create canvas elements
            CreateCanvasElements();

            // Load settings from configuration
            LoadSettingsFromConfiguration();

            // Update the settings back to the configuration
            UpdateSettingsToConfiguration();


            // Set run at startup
            SetRunAtStartup(config.RunAtStartup);

            // Hide the window if the GUI is started as minimized
            if (WindowState == WindowState.Minimized)
            {
                Hide();
            }

            // Start the driver
            StartDriver();
        }


        //
        // Window key down
        //
        private void Window_PreviewKeyDown(object sender, KeyEventArgs e)
        {

            // Control + S -> Save settings
            if (e.Key == Key.S && e.KeyboardDevice.Modifiers == ModifierKeys.Control)
            {
                SaveSettings(sender, null);
                e.Handled = true;
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

        // Notify icon double click -> show window
        private void NotifyIcon_DoubleClick(object sender, EventArgs e)
        {
            if (WindowState == WindowState.Minimized)
            {
                NotifyShowWindow(sender, e);
            }
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



        #region Wacom / Draw area

        //
        // Wacom Area
        //
        private void ButtonWacomArea_Click(object sender, RoutedEventArgs e)
        {
            WacomArea wacom = new WacomArea();
            wacom.textWacomLeft.Text = Utils.GetNumberString((config.TabletArea.X - config.TabletArea.Width / 2) * 100.0, "0");
            wacom.textWacomRight.Text = Utils.GetNumberString((config.TabletArea.X + config.TabletArea.Width / 2) * 100.0, "0");

            wacom.textWacomTop.Text = Utils.GetNumberString((config.TabletArea.Y - config.TabletArea.Height / 2) * 100.0, "0");
            wacom.textWacomBottom.Text = Utils.GetNumberString((config.TabletArea.Y + config.TabletArea.Height / 2) * 100.0, "0");

            wacom.ShowDialog();

            // Set button clicked
            if (wacom.DialogResult == true)
            {
                if (
                    Utils.ParseNumber(wacom.textWacomLeft.Text, out double left) &&
                    Utils.ParseNumber(wacom.textWacomRight.Text, out double right) &&
                    Utils.ParseNumber(wacom.textWacomTop.Text, out double top) &&
                    Utils.ParseNumber(wacom.textWacomBottom.Text, out double bottom)
                )
                {
                    double width, height;
                    width = right - left;
                    height = bottom - top;
                    config.ForceAspectRatio = false;
                    config.ForceFullArea = false;
                    config.TabletArea.X = (left + width / 2.0) / 100.0;
                    config.TabletArea.Y = (top + height / 2.0) / 100.0;
                    config.TabletArea.Width = width / 100.0;
                    config.TabletArea.Height = height / 100.0;
                    LoadSettingsFromConfiguration();
                }
                else
                {
                    MessageBox.Show("Invalid values!", "Wacom area error!", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }

            wacom.Close();
        }


        //
        // Draw area
        //
        private void ButtonDrawArea_Click(object sender, RoutedEventArgs e)
        {
            if (!isEnabledMeasurementToArea)
            {
                isEnabledMeasurementToArea = true;
                driver.SendCommand("Measure 2");
                SetStatus("Click the top left and the bottom right corners of the area with your tablet pen!");
                buttonDrawArea.IsEnabled = false;
            }
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


    }
}