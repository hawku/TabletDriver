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
        public string Version = "0.1.2";

        // Console stuff
        private List<string> commandHistory;
        private int commandHistoryIndex;

        // Notify icon
        private System.Windows.Forms.NotifyIcon notifyIcon;
        public bool IsRealExit;

        // Driver
        private TabletDriver driver;
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

        //
        // Constructor
        //
        public MainWindow()
        {
            // Set the current directory as TabletDriverGUI.exe's directory.
            try { Directory.SetCurrentDirectory(AppDomain.CurrentDomain.BaseDirectory); } catch (Exception) { }

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
                    new System.Windows.Forms.MenuItem("Show", NotifyShowWindow),
                    new System.Windows.Forms.MenuItem("Exit", NotifyExit)
                })
            };
            notifyIcon.ContextMenu.MenuItems[0].Enabled = false;

            notifyIcon.Text = "";
            notifyIcon.DoubleClick += NotifyIcon_DoubleClick;
            notifyIcon.Visible = true;
            IsRealExit = false;

            // Create command history list
            commandHistory = new List<string> { "" };
            commandHistoryIndex = 0;

            // Init tablet driver
            driver = new TabletDriver("TabletDriverService.exe");
            driver.MessageReceived += OnDriverMessageReceived;
            driver.ErrorReceived += OnDriverErrorReceived;
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
            for (int i = 2; i <= 8; i++)
            {
                comboBoxSmoothingRate.Items.Add((1000.0 / i).ToString("0") + " Hz");
            }
            comboBoxSmoothingRate.SelectedIndex = 2;

            // Process command line arguments
            ProcessCommandLineArguments();

            // Events
            Closing += MainWindow_Closing;
            Loaded += MainWindow_Loaded;
            SizeChanged += MainWindow_SizeChanged;

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
            Stop();
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
            Start();
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
        }

        // 'Exit' handler for taskbar menu
        void NotifyExit(object sender, EventArgs e)
        {
            IsRealExit = true;
            Application.Current.Shutdown();
        }

        #endregion



        #region Setting handlers

        //
        // Load settings from configuration
        //
        private void LoadSettingsFromConfiguration()
        {
            isLoadingSettings = true;

            //
            // Tablet area
            //
            textTabletAreaWidth.Text = Utils.GetNumberString(config.TabletArea.Width);
            textTabletAreaHeight.Text = Utils.GetNumberString(config.TabletArea.Height);
            textTabletAreaX.Text = Utils.GetNumberString(config.TabletArea.X);
            textTabletAreaY.Text = Utils.GetNumberString(config.TabletArea.Y);
            checkBoxForceAspect.IsChecked = config.ForceAspectRatio;
            checkBoxForceFullArea.IsChecked = config.ForceFullArea;
            switch (config.OutputMode)
            {
                case Configuration.OutputModes.Absolute:
                    radioModeAbsolute.IsChecked = true;
                    break;
                case Configuration.OutputModes.Relative:
                    radioModeRelative.IsChecked = true;
                    break;
                case Configuration.OutputModes.Digitizer:
                    radioModeDigitizer.IsChecked = true;
                    break;
            }
            textTabletAreaRotation.Text = Utils.GetNumberString(config.TabletArea.Rotation);
            checkBoxInvert.IsChecked = config.Invert;


            //
            // Force full area
            //
            if (config.ForceFullArea)
            {
                textTabletAreaWidth.IsEnabled = false;
                textTabletAreaHeight.IsEnabled = false;
                textTabletAreaX.IsEnabled = false;
                textTabletAreaY.IsEnabled = false;
            }
            else
            {
                textTabletAreaWidth.IsEnabled = true;
                textTabletAreaHeight.IsEnabled = true;
                textTabletAreaX.IsEnabled = true;
                textTabletAreaY.IsEnabled = true;
            }


            //
            // Screen area
            //
            textScreenAreaWidth.Text = Utils.GetNumberString(config.ScreenArea.Width, "0");
            textScreenAreaHeight.Text = Utils.GetNumberString(config.ScreenArea.Height, "0");
            textScreenAreaX.Text = Utils.GetNumberString(config.ScreenArea.X, "0");
            textScreenAreaY.Text = Utils.GetNumberString(config.ScreenArea.Y, "0");


            //
            // Desktop size
            //
            if (config.AutomaticDesktopSize)
            {
                textDesktopWidth.Text = Utils.GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = Utils.GetNumberString(GetVirtualDesktopSize().Height);
                config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                config.DesktopSize.Height = GetVirtualDesktopSize().Height;
                textDesktopWidth.IsEnabled = false;
                textDesktopHeight.IsEnabled = false;
            }
            else
            {
                textDesktopWidth.Text = Utils.GetNumberString(config.DesktopSize.Width);
                textDesktopHeight.Text = Utils.GetNumberString(config.DesktopSize.Height);
            }
            checkBoxAutomaticDesktopSize.IsChecked = config.AutomaticDesktopSize;


            // Force aspect ratio
            if (config.ForceAspectRatio)
            {
                config.TabletArea.Height = config.TabletArea.Width / (config.ScreenArea.Width / config.ScreenArea.Height);
                textTabletAreaHeight.Text = Utils.GetNumberString(config.TabletArea.Height);
                textTabletAreaHeight.IsEnabled = false;
            }


            //
            // Move tablet area to a valid position
            //
            config.TabletArea.MoveInside(config.TabletFullArea);
            textTabletAreaX.Text = Utils.GetNumberString(config.TabletArea.X);
            textTabletAreaY.Text = Utils.GetNumberString(config.TabletArea.Y);


            //
            // Buttons
            //
            if (config.ButtonMap.Count() == 3)
            {
                comboBoxButton1.SelectedIndex = config.ButtonMap[0];
                comboBoxButton2.SelectedIndex = config.ButtonMap[1];
                comboBoxButton3.SelectedIndex = config.ButtonMap[2];
            }
            else
            {
                config.ButtonMap = new int[] { 1, 2, 3 };
            }
            checkBoxDisableButtons.IsChecked = config.DisableButtons;


            //
            // Smoothing filter
            //
            checkBoxSmoothing.IsChecked = config.SmoothingEnabled;
            textSmoothingLatency.Text = Utils.GetNumberString(config.SmoothingLatency);
            comboBoxSmoothingRate.SelectedIndex = config.SmoothingInterval - 2;
            if (config.SmoothingEnabled)
            {
                textSmoothingLatency.IsEnabled = true;
                comboBoxSmoothingRate.IsEnabled = true;
            }
            else
            {
                textSmoothingLatency.IsEnabled = false;
                comboBoxSmoothingRate.IsEnabled = false;
            }

            //
            // Run at startup
            //
            checkRunAtStartup.IsChecked = config.RunAtStartup;


            //
            // Custom commands
            //
            string tmp = "";
            foreach (string command in config.CommandsBefore)
            {
                if (command.Trim().Length > 0)
                    tmp += command.Trim() + "\n";
            }
            textCommandsBefore.Text = tmp;

            tmp = "";
            foreach (string command in config.CommandsAfter)
            {
                if (command.Trim().Length > 0)
                    tmp += command.Trim() + "\n";
            }
            textCommandsAfter.Text = tmp;


            // Update canvases
            UpdateCanvasElements();


            isLoadingSettings = false;
        }

        //
        // Update settings to configuration
        //
        private void UpdateSettingsToConfiguration()
        {
            if (isLoadingSettings)
                return;

            bool oldValue;

            // Tablet area
            if (Utils.ParseNumber(textTabletAreaWidth.Text, out double val))
                config.TabletArea.Width = val;
            if (Utils.ParseNumber(textTabletAreaHeight.Text, out val))
                config.TabletArea.Height = val;
            if (Utils.ParseNumber(textTabletAreaX.Text, out val))
                config.TabletArea.X = val;
            if (Utils.ParseNumber(textTabletAreaY.Text, out val))
                config.TabletArea.Y = val;
            if (Utils.ParseNumber(textTabletAreaRotation.Text, out val))
                config.TabletArea.Rotation = val;

            config.Invert = (bool)checkBoxInvert.IsChecked;
            config.ForceAspectRatio = (bool)checkBoxForceAspect.IsChecked;
            config.ForceFullArea = (bool)checkBoxForceFullArea.IsChecked;

            // Output Mode
            if (radioModeAbsolute.IsChecked == true) config.OutputMode = Configuration.OutputModes.Absolute;
            if (radioModeRelative.IsChecked == true) config.OutputMode = Configuration.OutputModes.Relative;
            if (radioModeDigitizer.IsChecked == true) config.OutputMode = Configuration.OutputModes.Digitizer;


            // Force full area
            if (config.ForceFullArea)
            {
                // Set tablet area size to full area
                config.TabletArea.Width = config.TabletFullArea.Width;
                config.TabletArea.Height = config.TabletFullArea.Height;

                // Force aspect
                if (config.ForceAspectRatio)
                    config.TabletArea.Height = config.TabletArea.Width / (config.ScreenArea.Width / config.ScreenArea.Height);

                // Fit area to full area
                config.TabletArea.ScaleInside(config.TabletFullArea);

                textTabletAreaWidth.Text = Utils.GetNumberString(config.TabletArea.Width);
                textTabletAreaHeight.Text = Utils.GetNumberString(config.TabletArea.Height);

            }

            // Force the tablet area to be inside of the full area
            config.TabletArea.MoveInside(config.TabletFullArea);

            // Screen area
            if (Utils.ParseNumber(textScreenAreaWidth.Text, out val))
                config.ScreenArea.Width = val;
            if (Utils.ParseNumber(textScreenAreaHeight.Text, out val))
                config.ScreenArea.Height = val;
            if (Utils.ParseNumber(textScreenAreaX.Text, out val))
                config.ScreenArea.X = val;
            if (Utils.ParseNumber(textScreenAreaY.Text, out val))
                config.ScreenArea.Y = val;


            // Desktop size
            if (Utils.ParseNumber(textDesktopWidth.Text, out val))
                config.DesktopSize.Width = val;
            if (Utils.ParseNumber(textDesktopHeight.Text, out val))
                config.DesktopSize.Height = val;
            config.AutomaticDesktopSize = (bool)checkBoxAutomaticDesktopSize.IsChecked;
            if (config.AutomaticDesktopSize == true)
            {
                textDesktopWidth.Text = Utils.GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = Utils.GetNumberString(GetVirtualDesktopSize().Height);
                config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                config.DesktopSize.Height = GetVirtualDesktopSize().Height;
            }


            // Force aspect ratio
            if (config.ForceAspectRatio)
            {
                config.TabletArea.Height = config.TabletArea.Width / (config.ScreenArea.Width / config.ScreenArea.Height);
                textTabletAreaHeight.Text = Utils.GetNumberString(config.TabletArea.Height);
            }


            // Button map 
            config.ButtonMap[0] = comboBoxButton1.SelectedIndex;
            config.ButtonMap[1] = comboBoxButton2.SelectedIndex;
            config.ButtonMap[2] = comboBoxButton3.SelectedIndex;
            config.DisableButtons = (bool)checkBoxDisableButtons.IsChecked;



            // Filter
            config.SmoothingEnabled = (bool)checkBoxSmoothing.IsChecked;
            config.SmoothingInterval = comboBoxSmoothingRate.SelectedIndex + 2;
            if (Utils.ParseNumber(textSmoothingLatency.Text, out val))
                config.SmoothingLatency = val;

            if (config.SmoothingEnabled)
            {
                textSmoothingLatency.IsEnabled = true;
                comboBoxSmoothingRate.IsEnabled = true;
            }
            else
            {
                textSmoothingLatency.IsEnabled = false;
                comboBoxSmoothingRate.IsEnabled = false;
            }

            //
            // Run at startup
            //
            oldValue = config.RunAtStartup;
            config.RunAtStartup = (bool)checkRunAtStartup.IsChecked;
            if (config.RunAtStartup != oldValue)
                SetRunAtStartup(config.RunAtStartup);


            // Custom commands
            List<string> commandList = new List<string>();
            foreach (string command in textCommandsBefore.Text.Split('\n'))
                if (command.Trim().Length > 0)
                    commandList.Add(command.Trim());
            config.CommandsBefore = commandList.ToArray();

            commandList.Clear();
            foreach (string command in textCommandsAfter.Text.Split('\n'))
                if (command.Trim().Length > 0)
                    commandList.Add(command.Trim());
            config.CommandsAfter = commandList.ToArray();



            UpdateCanvasElements();

        }

        //
        // Set run at startup
        //
        private void SetRunAtStartup(bool enabled)
        {
            try
            {
                string path = System.Reflection.Assembly.GetExecutingAssembly().Location;
                string entryName = "TabletDriverGUI";
                RegistryKey rk = Registry.CurrentUser.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", true);
                if (enabled)
                    rk.SetValue(entryName, "\"" + path + "\" --hide");
                else
                    rk.DeleteValue(entryName, false);

                rk.Close();
            }
            catch (Exception)
            {
            }
        }

        //
        // Get desktop size
        //
        System.Drawing.Rectangle GetVirtualDesktopSize()
        {
            System.Drawing.Rectangle rect = new System.Drawing.Rectangle();

            // Windows 8 or greater needed for the multiscreen absolute mode
            if (VersionHelper.IsWindows8OrGreater() || config.OutputMode == Configuration.OutputModes.Digitizer)
            {
                rect.Width = System.Windows.Forms.SystemInformation.VirtualScreen.Width;
                rect.Height = System.Windows.Forms.SystemInformation.VirtualScreen.Height;
            }
            else
            {
                rect.Width = System.Windows.Forms.Screen.PrimaryScreen.Bounds.Width;
                rect.Height = System.Windows.Forms.Screen.PrimaryScreen.Bounds.Height;

            }
            return rect;
        }


        //
        // Get available screens
        //
        System.Windows.Forms.Screen[] GetAvailableScreens()
        {
            System.Windows.Forms.Screen[] screens;

            // Windows 8 or greater needed for the multiscreen absolute mode
            if (VersionHelper.IsWindows8OrGreater() || config.OutputMode == Configuration.OutputModes.Digitizer)
                screens = System.Windows.Forms.Screen.AllScreens;
            else
                screens = new System.Windows.Forms.Screen[] { System.Windows.Forms.Screen.PrimaryScreen };
            return screens;
        }


        //
        // Create canvas elements
        //
        void CreateCanvasElements()
        {
            //
            // Screen map canvas
            //
            // Clear canvas
            canvasScreenMap.Children.Clear();


            // Monitor rectangles
            rectangleMonitors = new Rectangle[16];
            for (int i = 0; i < 16; i++)
            {
                rectangleMonitors[i] = new Rectangle
                {
                    Width = 10,
                    Height = 10,
                    Stroke = Brushes.Black,
                    StrokeThickness = 1.0,
                    Fill = Brushes.Transparent,
                    Visibility = Visibility.Hidden
                };
                canvasScreenMap.Children.Add(rectangleMonitors[i]);
            }

            //
            // Desktop area rectangle
            //
            rectangleDesktop = new Rectangle
            {
                Stroke = Brushes.Black,
                StrokeThickness = 2.0,
                Fill = Brushes.Transparent
            };
            canvasScreenMap.Children.Add(rectangleDesktop);


            //
            // Screen map area rectangle
            //
            Brush brushScreenMap = new SolidColorBrush(Color.FromArgb(50, 20, 20, 20));
            rectangleScreenMap = new Rectangle
            {
                Stroke = Brushes.Black,
                StrokeThickness = 2.0,
                Fill = brushScreenMap
            };
            canvasScreenMap.Children.Add(rectangleScreenMap);

            //
            // Screen aspect ratio text
            //
            textScreenAspectRatio = new TextBlock
            {
                Text = "",
                Foreground = Brushes.Black,
                FontSize = 13,
                FontWeight = FontWeights.Bold
            };
            canvasScreenMap.Children.Add(textScreenAspectRatio);



            //
            // Tablet area canvas
            //
            //
            // Clear
            canvasTabletArea.Children.Clear();

            //
            // Tablet full area polygon
            //
            polygonTabletFullArea = new Polygon
            {
                Stroke = new SolidColorBrush(Color.FromRgb(100, 100, 100)),
                StrokeThickness = 2.0,
                Points = new PointCollection
                {
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0)
                },
            };
            canvasTabletArea.Children.Add(polygonTabletFullArea);

            //
            // Tablet area polygon
            //
            polygonTabletArea = new Polygon
            {
                Stroke = new SolidColorBrush(Color.FromRgb(0, 0, 0)),
                Fill = new SolidColorBrush(Color.FromArgb(50, 20, 20, 20)),
                StrokeThickness = 2.0,
                Points = new PointCollection
                {
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0)
                },
            };
            canvasTabletArea.Children.Add(polygonTabletArea);


            //
            // Tablet area arrow polygon
            //
            polygonTabletAreaArrow = new Polygon
            {
                Fill = new SolidColorBrush(Color.FromArgb(50, 20, 20, 20)),
                Points = new PointCollection
                {
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0)
                },
            };
            canvasTabletArea.Children.Add(polygonTabletAreaArrow);


            //
            // Tablet area aspect ratio text
            //
            textTabletAspectRatio = new TextBlock
            {
                Text = "",
                Foreground = Brushes.Black,
                FontSize = 13,
                FontWeight = FontWeights.Bold
            };
            canvasTabletArea.Children.Add(textTabletAspectRatio);




            //
            // Canvas mouse drag
            //
            mouseDrag = new MouseDrag();
        }

        //
        // Update canvas elements
        //
        void UpdateCanvasElements()
        {
            UpdateScreenMapCanvas();
            UpdateTabletAreaCanvas();
        }

        //
        // Update screen map canvas elements
        //
        void UpdateScreenMapCanvas()
        {
            // Canvas element scaling
            double scaleX = (canvasScreenMap.ActualWidth - 2) / config.DesktopSize.Width;
            double scaleY = (canvasScreenMap.ActualHeight - 2) / config.DesktopSize.Height;
            double scale = scaleX;
            if (scaleX > scaleY)
                scale = scaleY;

            // Centered offset
            double offsetX = canvasScreenMap.ActualWidth / 2.0 - config.DesktopSize.Width * scale / 2.0;
            double offsetY = canvasScreenMap.ActualHeight / 2.0 - config.DesktopSize.Height * scale / 2.0;


            // Full desktop area
            rectangleDesktop.Width = config.DesktopSize.Width * scale;
            rectangleDesktop.Height = config.DesktopSize.Height * scale;
            Canvas.SetLeft(rectangleDesktop, offsetX);
            Canvas.SetTop(rectangleDesktop, offsetY);


            // Screen map area
            rectangleScreenMap.Width = config.ScreenArea.Width * scale;
            rectangleScreenMap.Height = config.ScreenArea.Height * scale;
            Canvas.SetLeft(rectangleScreenMap, offsetX + config.ScreenArea.X * scale);
            Canvas.SetTop(rectangleScreenMap, offsetY + config.ScreenArea.Y * scale);

            // Screen aspect ratio text
            textScreenAspectRatio.Text = Utils.GetNumberString(config.ScreenArea.Width / config.ScreenArea.Height, "0.###") + ":1";
            Canvas.SetLeft(textScreenAspectRatio, offsetX +
                (config.ScreenArea.X + config.ScreenArea.Width / 2.0) * scale -
                textScreenAspectRatio.ActualWidth / 2.0
            );
            Canvas.SetTop(textScreenAspectRatio, offsetY +
                (config.ScreenArea.Y + config.ScreenArea.Height / 2.0) * scale -
                textScreenAspectRatio.ActualHeight / 2.0
            );





            // Screens
            System.Windows.Forms.Screen[] screens = GetAvailableScreens();

            // Monitor minimums
            double minX = 99999;
            double minY = 99999;
            foreach (System.Windows.Forms.Screen screen in screens)
            {
                if (screen.Bounds.X < minX) minX = screen.Bounds.X;
                if (screen.Bounds.Y < minY) minY = screen.Bounds.Y;
            }


            // Monitor rectangles
            int rectangeIndex = 0;
            foreach (System.Windows.Forms.Screen screen in screens)
            {
                double x = screen.Bounds.X - minX;
                double y = screen.Bounds.Y - minY;

                rectangleMonitors[rectangeIndex].Visibility = Visibility.Visible;
                rectangleMonitors[rectangeIndex].Width = screen.Bounds.Width * scale;
                rectangleMonitors[rectangeIndex].Height = screen.Bounds.Height * scale;
                Canvas.SetLeft(rectangleMonitors[rectangeIndex], offsetX + x * scale);
                Canvas.SetTop(rectangleMonitors[rectangeIndex], offsetY + y * scale);

                rectangeIndex++;
                if (rectangeIndex >= 16) break;
            }

        }

        //
        // Update tablet area canvas elements
        //
        void UpdateTabletAreaCanvas()
        {
            double fullWidth = config.TabletFullArea.Width;
            double fullHeight = config.TabletFullArea.Height;

            // Canvas element scaling
            double scaleX = (canvasTabletArea.ActualWidth - 2) / fullWidth;
            double scaleY = (canvasTabletArea.ActualHeight - 2) / fullHeight;
            double scale = scaleX;
            if (scaleX > scaleY)
                scale = scaleY;


            double offsetX = canvasTabletArea.ActualWidth / 2.0 - fullWidth * scale / 2.0;
            double offsetY = canvasTabletArea.ActualHeight / 2.0 - fullHeight * scale / 2.0;

            //
            // Tablet full area
            //
            Point[] corners = config.TabletFullArea.Corners;
            for (int i = 0; i < 4; i++)
            {
                Point p = corners[i];
                p.X *= scale;
                p.Y *= scale;
                p.X += config.TabletFullArea.X * scale + offsetX;
                p.Y += config.TabletFullArea.Y * scale + offsetY;
                polygonTabletFullArea.Points[i] = p;
            }


            //
            // Tablet area
            //
            corners = config.TabletArea.Corners;
            for (int i = 0; i < 4; i++)
            {
                Point p = corners[i];
                p.X *= scale;
                p.Y *= scale;
                p.X += config.TabletArea.X * scale + offsetX;
                p.Y += config.TabletArea.Y * scale + offsetY;
                polygonTabletArea.Points[i] = p;
            }

            //
            // Tablet area arrow
            //
            polygonTabletAreaArrow.Points[0] = new Point(
                offsetX + config.TabletArea.X * scale,
                offsetY + config.TabletArea.Y * scale
            );

            polygonTabletAreaArrow.Points[1] = new Point(
                offsetX + corners[2].X * scale + config.TabletArea.X * scale,
                offsetY + corners[2].Y * scale + config.TabletArea.Y * scale
            );

            polygonTabletAreaArrow.Points[2] = new Point(
                offsetX + corners[3].X * scale + config.TabletArea.X * scale,
                offsetY + corners[3].Y * scale + config.TabletArea.Y * scale
            );


            //
            // Tablet area aspect ratio text
            //
            textTabletAspectRatio.Text = Utils.GetNumberString(config.TabletArea.Width / config.TabletArea.Height, "0.###") + ":1";
            Canvas.SetLeft(textTabletAspectRatio, offsetX + (config.TabletArea.X) * scale - textTabletAspectRatio.ActualWidth / 2.0);
            Canvas.SetTop(textTabletAspectRatio, offsetY + (config.TabletArea.Y) * scale - textTabletAspectRatio.ActualHeight / 2.0);


        }




        //
        // Canvas mouse events
        //
        //
        // Canvas mouse down
        private void Canvas_MouseDown(object sender, MouseButtonEventArgs e)
        {

            mouseDrag.IsMouseDown = true;
            mouseDrag.Source = (UIElement)sender;
            mouseDrag.OriginMouse = e.GetPosition((UIElement)mouseDrag.Source);

            // Screen Map
            if (mouseDrag.Source == canvasScreenMap)
            {
                // Reset monitor selection
                comboBoxMonitor.SelectedIndex = -1;

                mouseDrag.OriginDraggable = new Point(config.ScreenArea.X, config.ScreenArea.Y);
                canvasScreenMap.CaptureMouse();
            }

            // Tablet Area
            else if (mouseDrag.Source == canvasTabletArea)
            {
                mouseDrag.OriginDraggable = new Point(config.TabletArea.X, config.TabletArea.Y);
                canvasTabletArea.CaptureMouse();
            }
        }

        // Canvas mouse up
        private void Canvas_MouseUp(object sender, MouseButtonEventArgs e)
        {
            mouseDrag.IsMouseDown = false;
            LoadSettingsFromConfiguration();
            isLoadingSettings = true;
            textScreenAreaX.Text = Utils.GetNumberString(config.ScreenArea.X, "0");
            textScreenAreaY.Text = Utils.GetNumberString(config.ScreenArea.Y, "0");
            textTabletAreaX.Text = Utils.GetNumberString(config.TabletArea.X);
            textTabletAreaY.Text = Utils.GetNumberString(config.TabletArea.Y);
            isLoadingSettings = false;
            canvasScreenMap.ReleaseMouseCapture();
            canvasTabletArea.ReleaseMouseCapture();
        }

        // Canvas mouse move
        private void Canvas_MouseMove(object sender, MouseEventArgs e)
        {
            Point position;
            double dx, dy;
            double scaleX = 0, scaleY = 0, scale = 0;

            // Canvas mouse drag
            if (mouseDrag.IsMouseDown && mouseDrag.Source == sender)
            {
                position = e.GetPosition((UIElement)mouseDrag.Source);

                dx = position.X - mouseDrag.OriginMouse.X;
                dy = position.Y - mouseDrag.OriginMouse.Y;

                if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
                    dx = 0;
                if (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl))
                    dy = 0;

                // Screen map canvas
                if (mouseDrag.Source == canvasScreenMap)
                {
                    scaleX = config.DesktopSize.Width / canvasScreenMap.ActualWidth;
                    scaleY = config.DesktopSize.Height / canvasScreenMap.ActualHeight;
                    scale = scaleY;
                    if (scaleX > scaleY)
                        scale = scaleX;

                    config.ScreenArea.X = mouseDrag.OriginDraggable.X + dx * scale;
                    config.ScreenArea.Y = mouseDrag.OriginDraggable.Y + dy * scale;
                    UpdateScreenMapCanvas();
                }

                // Tablet area canvas
                else if (mouseDrag.Source == canvasTabletArea)
                {
                    scaleX = config.TabletFullArea.Width / canvasTabletArea.ActualWidth;
                    scaleY = config.TabletFullArea.Height / canvasTabletArea.ActualHeight;
                    scale = scaleY;
                    if (scaleX > scaleY)
                        scale = scaleX;

                    config.TabletArea.X = mouseDrag.OriginDraggable.X + dx * scale;
                    config.TabletArea.Y = mouseDrag.OriginDraggable.Y + dy * scale;

                    UpdateTabletAreaCanvas();
                }


            }
        }


        // TextBox setting changed
        private void TextChanged(object sender, TextChangedEventArgs e)
        {
            UpdateSettingsToConfiguration();
        }

        // Checkbox setting changed
        private void CheckboxChanged(object sender, RoutedEventArgs e)
        {
            if (isLoadingSettings) return;


            // Disable tablet area settings when full area is forced
            if (checkBoxForceFullArea.IsChecked == true)
            {
                textTabletAreaWidth.IsEnabled = false;
                textTabletAreaHeight.IsEnabled = false;
                textTabletAreaX.IsEnabled = false;
                textTabletAreaY.IsEnabled = false;
            }
            else
            {
                textTabletAreaWidth.IsEnabled = true;
                textTabletAreaX.IsEnabled = true;
                textTabletAreaY.IsEnabled = true;

                // Disable tablet area height when aspect ratio is forced
                if (checkBoxForceAspect.IsChecked == true)
                    textTabletAreaHeight.IsEnabled = false;
                else
                    textTabletAreaHeight.IsEnabled = true;


            }

            // Disable button map selection when buttons are disabled
            if (checkBoxDisableButtons.IsChecked == true)
            {
                comboBoxButton1.IsEnabled = false;
                comboBoxButton2.IsEnabled = false;
                comboBoxButton3.IsEnabled = false;
            }
            else
            {
                comboBoxButton1.IsEnabled = true;
                comboBoxButton2.IsEnabled = true;
                comboBoxButton3.IsEnabled = true;
            }

            // Disable desktop size settings when automatic is checked
            if (checkBoxAutomaticDesktopSize.IsChecked == true)
            {
                textDesktopWidth.Text = Utils.GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = Utils.GetNumberString(GetVirtualDesktopSize().Height);
                textDesktopWidth.IsEnabled = false;
                textDesktopHeight.IsEnabled = false;
            }
            else
            {
                textDesktopWidth.IsEnabled = true;
                textDesktopHeight.IsEnabled = true;
            }

            UpdateSettingsToConfiguration();

            if (sender == checkBoxForceFullArea)
            {
                LoadSettingsFromConfiguration();
                UpdateSettingsToConfiguration();
            }
        }

        // Selection settings changed
        private void ItemSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            UpdateSettingsToConfiguration();
        }

        // Window size changed
        private void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (!IsLoaded || isLoadingSettings) return;
            if (WindowState != WindowState.Maximized)
            {
                config.WindowWidth = (int)e.NewSize.Width;
                config.WindowHeight = (int)e.NewSize.Height;
            }
        }

        // Monitor combobox clicked -> create new monitor list
        private void ComboBoxMonitor_MouseDown(object sender, MouseButtonEventArgs e)
        {
            comboBoxMonitor.Items.Clear();


            System.Windows.Forms.Screen[] screens = GetAvailableScreens();
            if (screens.Length > 1)
            {
                comboBoxMonitor.Items.Add("Full desktop");
                foreach (System.Windows.Forms.Screen screen in screens)
                {
                    string name = screen.DeviceName;
                    if (screen.Primary)
                        name += " Main";

                    comboBoxMonitor.Items.Add(name);
                }
            }
            else
            {
                comboBoxMonitor.Items.Add(System.Windows.Forms.Screen.PrimaryScreen.DeviceName);
            }

        }

        // Monitor selected -> change screen map
        private void ComboBoxMonitor_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            System.Windows.Forms.Screen[] screens = GetAvailableScreens();
            double minX = 99999;
            double minY = 99999;
            foreach (System.Windows.Forms.Screen screen in screens)
            {
                if (screen.Bounds.X < minX) minX = screen.Bounds.X;
                if (screen.Bounds.Y < minY) minY = screen.Bounds.Y;
            }

            int index = comboBoxMonitor.SelectedIndex;
            if (index == 0)
            {
                textScreenAreaX.Text = "0";
                textScreenAreaY.Text = "0";
                textScreenAreaWidth.Text = Utils.GetNumberString(config.DesktopSize.Width);
                textScreenAreaHeight.Text = Utils.GetNumberString(config.DesktopSize.Height);
            }
            else if (index > 0)
            {
                index--;
                if (index >= 0 && index < screens.Length)
                {
                    textScreenAreaX.Text = Utils.GetNumberString(screens[index].Bounds.X - minX);
                    textScreenAreaY.Text = Utils.GetNumberString(screens[index].Bounds.Y - minY);
                    textScreenAreaWidth.Text = Utils.GetNumberString(screens[index].Bounds.Width);
                    textScreenAreaHeight.Text = Utils.GetNumberString(screens[index].Bounds.Height);
                }
            }
            UpdateSettingsToConfiguration();
        }

        //
        // Save settings
        //
        private void SaveSettings(object sender, RoutedEventArgs e)
        {
            try
            {
                config.Write(configFilename);
                SendSettingsToDriver();
                SetStatus("Settings saved!");
            }
            catch (Exception)
            {
                string dir = Directory.GetCurrentDirectory();
                MessageBox.Show("Error occured while saving the configuration.\n" +
                    "Make sure that it is possible to create and edit files in the '" + dir + "' directory.\n",
                    "ERROR!", MessageBoxButton.OK, MessageBoxImage.Error
                );
            }
        }

        //
        // Apply settings
        //
        private void ApplySettings(object sender, RoutedEventArgs e)
        {
            SendSettingsToDriver();
            SetStatus("Settings applied!");
        }


        //
        // Main Menu Click
        //
        private void MainMenuClick(object sender, RoutedEventArgs e)
        {

            // Import
            if (sender == mainMenuImport)
            {
                OpenFileDialog dialog = new OpenFileDialog
                {
                    InitialDirectory = Directory.GetCurrentDirectory(),
                    Filter = "XML File|*.xml"
                };
                if (dialog.ShowDialog() == true)
                {
                    try
                    {
                        Configuration tmpConfig = Configuration.CreateFromFile(dialog.FileName);
                        config = tmpConfig;
                        LoadSettingsFromConfiguration();
                        SetStatus("Settings imported!");
                    }
                    catch (Exception)
                    {
                        MessageBox.Show("Settings import failed!", "ERROR!",
                            MessageBoxButton.OK, MessageBoxImage.Error);
                    }
                }
            }

            // Export
            else if (sender == mainMenuExport)
            {
                SaveFileDialog dialog = new SaveFileDialog
                {
                    InitialDirectory = Directory.GetCurrentDirectory(),
                    AddExtension = true,
                    DefaultExt = "xml",
                    Filter = "XML File|*.xml"

                };
                if (dialog.ShowDialog() == true)
                {
                    try
                    {
                        UpdateSettingsToConfiguration();
                        config.Write(dialog.FileName);
                        SetStatus("Settings exported!");
                    }
                    catch (Exception)
                    {
                        MessageBox.Show("Settings export failed!", "ERROR!",
                            MessageBoxButton.OK, MessageBoxImage.Error);
                    }
                }

            }

            // Exit
            else if (sender == mainMenuExit)
            {
                Close();
            }

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



        #region Tablet Driver

        //
        // Driver event handlers
        //
        //
        // Message
        private void OnDriverMessageReceived(object sender, TabletDriver.DriverEventArgs e)
        {
            //ConsoleAddText(e.Message);
            Application.Current.Dispatcher.Invoke(() =>
            {
                ParseDriverStatus(e.Message);
            });
        }
        // Error
        private void OnDriverErrorReceived(object sender, TabletDriver.DriverEventArgs e)
        {
            SetStatusWarning(e.Message);

        }
        // Started
        private void OnDriverStarted(object sender, EventArgs e)
        {
            driver.SendCommand("HIDList");
            driver.SendCommand("Echo");
            driver.SendCommand("Echo   Driver version: " + Version);
            try { driver.SendCommand("echo   Windows version: " + Environment.OSVersion.VersionString); } catch (Exception) { }
            try
            {
                driver.SendCommand("Echo   Windows product: " +
                    Microsoft.Win32.Registry.GetValue(@"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion", "ProductName", "").ToString());
                driver.SendCommand("Echo   Windows release: " +
                    Microsoft.Win32.Registry.GetValue(@"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion", "ReleaseId", "").ToString());
            }
            catch (Exception)
            {
            }
            driver.SendCommand("Echo");
            driver.SendCommand("CheckTablet");
            SendSettingsToDriver();
            driver.SendCommand("Info");
            driver.SendCommand("Start");
            driver.SendCommand("Log Off");
            driver.SendCommand("LogDirect False");
            driver.SendCommand("Echo");
            driver.SendCommand("Echo Driver started!");
        }
        // Stopped
        private void OnDriverStopped(object sender, EventArgs e)
        {
            if (running)
            {
                SetStatus("Driver stopped. Restarting! Check console !!!");
                driver.ConsoleAddText("Driver stopped. Restarting!");

                // Run in the main application thread
                Application.Current.Dispatcher.Invoke(() =>
                {
                    Title = "TabletDriverGUI";
                    notifyIcon.Text = "";
                    driver.Stop();
                    timerRestart.Start();
                });

            }
        }
        // Driver restart timer
        private void TimerRestart_Tick(object sender, EventArgs e)
        {
            if (running)
            {
                driver.Start(config.DriverPath, config.DriverArguments);
            }
            timerRestart.Stop();
        }



        //
        // Parse driver status messages
        //
        private void ParseDriverStatus(string line)
        {
            // Status line?
            if (!line.Contains("[STATUS]")) return;

            // Parse status variable and value
            Match match = Regex.Match(line, "^.+\\[STATUS\\] ([^ ]+) (.*?)$");
            if (!match.Success) return;

            string variableName = match.Groups[1].ToString().ToLower();
            string stringValue = match.Groups[2].ToString();

            //
            // Tablet Name
            //
            if (variableName == "tablet")
            {
                Title = "TabletDriverGUI - " + stringValue;
                notifyIcon.Text = Title;
                SetStatus("Connected to " + stringValue);
            }

            //
            // Tablet width
            //
            if (variableName == "width")
            {
                if (Utils.ParseNumber(stringValue, out double val))
                {
                    config.TabletFullArea.Width = val;
                    config.TabletFullArea.X = val / 2.0;
                    LoadSettingsFromConfiguration();
                    UpdateSettingsToConfiguration();
                    if (isFirstStart)
                        SendSettingsToDriver();
                }
            }

            //
            // Tablet height
            //
            if (variableName == "height")
            {
                if (Utils.ParseNumber(stringValue, out double val))
                {
                    config.TabletFullArea.Height = val;
                    config.TabletFullArea.Y = val / 2.0;
                    LoadSettingsFromConfiguration();
                    UpdateSettingsToConfiguration();
                    if (isFirstStart)
                        SendSettingsToDriver();

                }
            }
        }

        //
        // Send settings to the driver
        //
        private void SendSettingsToDriver()
        {
            if (!driver.IsRunning) return;

            // Commands before settings
            if (config.CommandsBefore.Length > 0)
            {
                foreach (string command in config.CommandsBefore)
                {
                    string tmp = command.Trim();
                    if (tmp.Length > 0)
                    {
                        driver.SendCommand(tmp);
                    }
                }
            }


            // Desktop size
            driver.SendCommand("DesktopSize " + textDesktopWidth.Text + " " + textDesktopHeight.Text);


            // Screen area
            driver.SendCommand("ScreenArea " +
                Utils.GetNumberString(config.ScreenArea.Width) + " " + Utils.GetNumberString(config.ScreenArea.Height) + " " +
                Utils.GetNumberString(config.ScreenArea.X) + " " + Utils.GetNumberString(config.ScreenArea.Y)
            );


            //
            // Tablet area
            //
            // Inverted
            if (config.Invert)
            {
                driver.SendCommand("TabletArea " +
                    Utils.GetNumberString(config.TabletArea.Width) + " " +
                    Utils.GetNumberString(config.TabletArea.Height) + " " +
                    Utils.GetNumberString(config.TabletFullArea.Width - config.TabletArea.X) + " " +
                    Utils.GetNumberString(config.TabletFullArea.Height - config.TabletArea.Y)
                );
                driver.SendCommand("Rotate " + Utils.GetNumberString(config.TabletArea.Rotation + 180));
            }
            // Normal
            else
            {
                driver.SendCommand("TabletArea " +
                    Utils.GetNumberString(config.TabletArea.Width) + " " +
                    Utils.GetNumberString(config.TabletArea.Height) + " " +
                    Utils.GetNumberString(config.TabletArea.X) + " " +
                    Utils.GetNumberString(config.TabletArea.Y)
                );
                driver.SendCommand("Rotate " + Utils.GetNumberString(config.TabletArea.Rotation));
            }


            // Output Mode
            switch (config.OutputMode)
            {
                case Configuration.OutputModes.Absolute:
                    driver.SendCommand("Mode Absolute");
                    break;
                case Configuration.OutputModes.Relative:
                    driver.SendCommand("Mode Relative");
                    driver.SendCommand("Sensitivity " + Utils.GetNumberString(config.ScreenArea.Width / config.TabletArea.Width));
                    break;
                case Configuration.OutputModes.Digitizer:
                    driver.SendCommand("Mode Digitizer");
                    break;
            }


            // Button map
            if (config.DisableButtons)
            {
                driver.SendCommand("ButtonMap 0 0 0");
            }
            else
            {
                driver.SendCommand("ButtonMap " + String.Join(" ", config.ButtonMap));
            }

            // Smoothing filter
            if (config.SmoothingEnabled)
            {
                driver.SendCommand("Smoothing " + Utils.GetNumberString(config.SmoothingLatency));
                driver.SendCommand("SmoothingInterval " + Utils.GetNumberString(config.SmoothingInterval));
            }
            else
            {
                driver.SendCommand("Smoothing 0");
            }

            // Commands after settings
            if (config.CommandsAfter.Length > 0)
            {
                foreach (string command in config.CommandsAfter)
                {
                    string tmp = command.Trim();
                    if (tmp.Length > 0)
                    {
                        driver.SendCommand(tmp);
                    }
                }
            }
        }

        //
        // Start driver
        //
        void Start()
        {
            if (running) return;

            // Try to start the driver
            try
            {
                running = true;

                // Console timer
                timerConsoleUpdate.Start();

                driver.Start(config.DriverPath, config.DriverArguments);
                if (!driver.IsRunning)
                {
                    SetStatus("Can't start the driver! Check the console!");
                    driver.ConsoleAddText("ERROR! Can't start the driver!");
                }
            }

            // Start failed
            catch (Exception e)
            {
                SetStatus("Can't start the driver! Check the console!");
                driver.ConsoleAddText("ERROR! Can't start the driver!\n  " + e.Message);
            }
        }


        //
        // Stop driver
        //
        void Stop()
        {
            if (!running) return;
            running = false;
            driver.Stop();
            timerConsoleUpdate.Stop();
        }

        //
        // Restart Driver button click
        //
        private void RestartDriverClick(object sender, RoutedEventArgs e)
        {
            if (running)
            {
                Stop();
            }
            Start();
        }

        #endregion



        #region Console stuff

        //
        // Console buffer to text
        //
        private void ConsoleBufferToText()
        {
            StringBuilder stringBuilder = new StringBuilder();

            // Lock console
            driver.ConsoleLock();

            // Get console status
            if (!driver.HasConsoleUpdated)
            {
                driver.ConsoleUnlock();
                return;
            }
            driver.HasConsoleUpdated = false;

            // Create a string from buffer
            foreach (string line in driver.ConsoleBuffer)
            {
                stringBuilder.Append(line);
                stringBuilder.Append("\r\n");
            }

            // Unlock console
            driver.ConsoleUnlock();

            // Set output
            textConsole.Text = stringBuilder.ToString();

            // Scroll to end
            scrollConsole.ScrollToEnd();

        }

        // Console update timer tick
        private void TimerConsoleUpdate_Tick(object sender, EventArgs e)
        {
            ConsoleBufferToText();
        }


        //
        // Send a command to driver
        //
        private void ConsoleSendCommand(string line)
        {
            if (commandHistory.Last<string>() != line)
            {
                commandHistory.Add(line);
            }
            commandHistoryIndex = commandHistory.Count();
            textConsoleInput.Text = "";
            textConsoleInput.ScrollToEnd();
            try
            {
                driver.SendCommand(line);
            }
            catch (Exception e)
            {
                driver.ConsoleAddText("Error! " + e.Message);
            }
        }

        //
        // Console input key down
        //
        private void TextConsoleInput_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                string line = textConsoleInput.Text;
                ConsoleSendCommand(line);
            }
        }

        //
        // Console input preview key down
        //
        private void TextConsoleInput_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Up)
            {
                commandHistoryIndex--;
                if (commandHistoryIndex < 0) commandHistoryIndex = 0;
                textConsoleInput.Text = commandHistory[commandHistoryIndex];
                textConsoleInput.CaretIndex = textConsoleInput.Text.Length;
            }
            if (e.Key == Key.Down)
            {
                commandHistoryIndex++;
                if (commandHistoryIndex > commandHistory.Count() - 1)
                {
                    commandHistoryIndex = commandHistory.Count();
                    textConsoleInput.Text = "";
                }
                else
                {
                    textConsoleInput.Text = commandHistory[commandHistoryIndex];
                    textConsoleInput.CaretIndex = textConsoleInput.Text.Length;
                }
            }
        }


        //
        // Search rows
        //
        private List<string> SearchRows(List<string> rows, string search, int rowsBefore, int rowsAfter)
        {
            List<string> buffer = new List<string>(rowsBefore);
            List<string> output = new List<string>();
            int rowCounter = 0;

            foreach (string row in rows)
            {
                if (row.Contains(search))
                {
                    if (buffer.Count > 0)
                    {
                        foreach (string bufferLine in buffer)
                        {
                            output.Add(bufferLine);
                        }
                        buffer.Clear();
                    }
                    output.Add(row.Trim());
                    rowCounter = rowsAfter;
                }
                else if (rowCounter > 0)
                {
                    output.Add(row.Trim());
                    rowCounter--;
                }
                else
                {
                    buffer.Add(row);
                    if (buffer.Count > rowsBefore)
                    {
                        buffer.RemoveAt(0);
                    }
                }
            }
            return output;
        }


        //
        // Console output context menu
        //
        private void ConsoleMenuClick(object sender, RoutedEventArgs e)
        {


            // Copy all
            if (sender == menuCopyAll)
            {
                Clipboard.SetText(textConsole.Text);
                SetStatus("Console output copied to clipboard");
            }

            // Copy debug messages
            else if (sender == menuCopyDebug)
            {
                string clipboard = "";
                List<string> rows;
                driver.ConsoleLock();
                rows = SearchRows(driver.ConsoleBuffer, "[DEBUG]", 0, 0);
                driver.ConsoleUnlock();
                foreach (string row in rows)
                    clipboard += row + "\r\n";
                Clipboard.SetText(clipboard);
                SetStatus("Debug message copied to clipboard");
            }

            // Copy error messages
            else if (sender == menuCopyErrors)
            {
                string clipboard = "";
                List<string> rows;
                driver.ConsoleLock();
                rows = SearchRows(driver.ConsoleBuffer, "[ERROR]", 1, 1);
                driver.ConsoleUnlock();
                foreach (string row in rows)
                    clipboard += row + "\r\n";
                Clipboard.SetText(clipboard);
                SetStatus("Error message copied to clipboard");
            }

            // Start debug log
            else if (sender == menuStartDebug)
            {
                string logFilename = "debug_" + DateTime.Now.ToString("yyyy-MM-dd_hh_mm_ss") + ".txt";
                ConsoleSendCommand("log " + logFilename);
                ConsoleSendCommand("debug 1");
            }

            // Stop debug log
            else if (sender == menuStopDebug)
            {
                ConsoleSendCommand("log off");
                ConsoleSendCommand("debug 0");
            }

            // Open latest debug log
            else if (sender == menuOpenDebug)
            {
                try
                {
                    var files = Directory.GetFiles(".", "debug_*.txt").OrderBy(a => File.GetCreationTime(a));
                    if (files.Count() > 0)
                    {
                        string file = files.Last().ToString();
                        Process.Start(file);
                    }
                }
                catch (Exception)
                {
                }
            }

            // Run benchmark
            else if (sender == menuRunBenchmark)
            {
                ConsoleSendCommand("Benchmark");
            }

            // Copy Benchmark
            else if (sender == menuCopyBenchmark)
            {
                string clipboard = "";
                List<string> rows;
                driver.ConsoleLock();
                rows = SearchRows(driver.ConsoleBuffer, " [STATUS] BENCHMARK ", 0, 0);
                driver.ConsoleUnlock();
                foreach (string row in rows)
                {
                    Match m = Regex.Match(row, " BENCHMARK ([0-9\\.]+) ([0-9\\.]+) ([0-9\\.]+) (.*)$");
                    if (m.Success)
                    {
                        string tabletName = m.Groups[4].ToString();
                        string totalPackets = m.Groups[1].ToString();
                        string noiseWidth = m.Groups[2].ToString();
                        string noiseHeight = m.Groups[3].ToString();
                        clipboard =
                            "Tablet(" + tabletName + ") " +
                            "Noise(" + noiseWidth + " mm x " + noiseHeight + " mm) " +
                            "Packets(" + totalPackets + ")\r\n";
                    }
                }

                if (clipboard.Length > 0)
                {
                    Clipboard.SetText(clipboard);
                    SetStatus("Benchmark result copied to clipboard");
                }
            }

            // Open startup log
            else if (sender == menuOpenStartup)
            {
                if (File.Exists("startuplog.txt"))
                {
                    try { Process.Start("startuplog.txt"); } catch (Exception) { }
                }
                else
                {
                    MessageBox.Show(
                        "Startup log not found!\n" +
                        "Make sure that it is possible to create and edit files in the '" + Directory.GetCurrentDirectory() + "' directory.\n",
                        "Error!", MessageBoxButton.OK, MessageBoxImage.Error
                    );
                }
            }

            // Open driver folder
            else if (sender == menuOpenFolder)
            {
                try { Process.Start("."); } catch (Exception) { }
            }

            // Open GitHub page
            else if (sender == menuOpenGithub)
            {
                try { Process.Start("https://github.com/hawku/TabletDriver"); } catch (Exception) { }
            }

            // Open Latest URL
            else if (sender == menuOpenLatestURL)
            {
                Regex regex = new Regex("(http[s]?://.+?)($|\\s)", RegexOptions.IgnoreCase | RegexOptions.Multiline);
                MatchCollection matches = regex.Matches(textConsole.Text);
                if (matches.Count > 0)
                {
                    string url = matches[matches.Count - 1].Groups[0].ToString().Trim();
                    try { Process.Start(url); } catch (Exception) { }
                }
            }

            // Report a problem
            else if (sender == menuReportProblem)
            {
                try { Process.Start("https://github.com/hawku/TabletDriver/wiki/FAQ"); } catch (Exception) { }
            }


        }



        #endregion



        #region Wacom

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