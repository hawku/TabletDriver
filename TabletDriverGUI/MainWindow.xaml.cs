﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        public string Version = "0.0.11";

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
        private Ellipse ellipseTabletAreaCenter;
        private TextBlock textTabletAspectRatio;

        // Culture
        CultureInfo cultureEnglish;

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
            if (Process.GetProcessesByName(Process.GetCurrentProcess().ProcessName).Length > 1)
            {
                MessageBox.Show("Another instance of the TabletDriver application is already running. " +
                    "You can double click on the TabletDriver icon in your system tray to display it again.", "Another instance already running", MessageBoxButton.OK, MessageBoxImage.Warning);
                Application.Current.Shutdown();
            }

            //
            isLoadingSettings = true;

            // Initialize WPF
            InitializeComponent();


            // Limit frame rate
            Timeline.DesiredFrameRateProperty.OverrideMetadata(typeof(Timeline),
                new PropertyMetadata { DefaultValue = 10 }
            );


            // Version text
            textVersion.Text = this.Version;

            // Set culture to en-US to force decimal format and etc.
            CultureInfo.DefaultThreadCurrentCulture = new CultureInfo("en-US");
            cultureEnglish = new CultureInfo("en-US");
            cultureEnglish.NumberFormat.PerMilleSymbol = "";
            cultureEnglish.NumberFormat.NumberGroupSeparator = "";
            cultureEnglish.NumberFormat.PercentDecimalSeparator = ".";


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
            timerRestart = new DispatcherTimer();
            timerRestart.Interval = new TimeSpan(0, 0, 5);
            timerRestart.Tick += TimerRestart_Tick;

            // Statusbar timer
            timerStatusbar = new DispatcherTimer();
            timerStatusbar.Interval = new TimeSpan(0, 0, 5);
            timerStatusbar.Tick += TimerStatusbar_Tick;

            // Timer console update
            timerConsoleUpdate = new DispatcherTimer();
            timerConsoleUpdate.Interval = new TimeSpan(0, 0, 0, 0, 200);
            timerConsoleUpdate.Tick += TimerConsoleUpdate_Tick;


            // Create button map combobox items
            comboBoxButton1.Items.Clear();
            comboBoxButton2.Items.Clear();
            comboBoxButton3.Items.Clear();
            comboBoxButton1.Items.Add("Disabled");
            comboBoxButton2.Items.Add("Disabled");
            comboBoxButton3.Items.Add("Disabled");
            for (int i = 1; i <= 5; i++)
            {
                comboBoxButton1.Items.Add("Mouse " + i);
                comboBoxButton2.Items.Add("Mouse " + i);
                comboBoxButton3.Items.Add("Mouse " + i);
            }
            comboBoxButton1.SelectedIndex = 0;
            comboBoxButton2.SelectedIndex = 0;
            comboBoxButton3.SelectedIndex = 0;

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
            config.Write("Config.xml");
            Stop();
        }

        // Window loaded -> Start driver
        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // Load configuration
            try
            {
                config = Configuration.CreateFromFile("Config.xml");
                isLoadingSettings = true;
                Width = config.WindowWidth;
                Height = config.WindowHeight;
                isLoadingSettings = false;
            }
            catch (Exception)
            {
                driver.ConsoleAddText("New config created!");
                isFirstStart = true;
                config = new Configuration();
            }

            if (!config.DeveloperMode)
            {
                groupDesktopSize.Visibility = Visibility.Collapsed;
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

            // 
            UpdateSettingsToConfiguration();

            timerConsoleUpdate.Start();

            Start();
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
            System.Windows.Application.Current.Shutdown();
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
            textTabletAreaWidth.Text = GetNumberString(config.TabletArea.Width);
            textTabletAreaHeight.Text = GetNumberString(config.TabletArea.Height);
            textTabletAreaX.Text = GetNumberString(config.TabletArea.X);
            textTabletAreaY.Text = GetNumberString(config.TabletArea.Y);
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

            textTabletAreaRotation.Text = GetNumberString(config.TabletArea.Rotation);



            //
            // Force full area
            //
            if (config.ForceFullArea)
            {
                textTabletAreaWidth.IsEnabled = false;
                textTabletAreaHeight.IsEnabled = false;
                textTabletAreaX.IsEnabled = false;
                textTabletAreaY.IsEnabled = false;
            } else
            {
                textTabletAreaWidth.IsEnabled = true;
                textTabletAreaHeight.IsEnabled = true;
                textTabletAreaX.IsEnabled = true;
                textTabletAreaY.IsEnabled = true;
            }


            //
            // Screen area
            //
            textScreenAreaWidth.Text = GetNumberString(config.ScreenArea.Width, "0");
            textScreenAreaHeight.Text = GetNumberString(config.ScreenArea.Height, "0");
            textScreenAreaX.Text = GetNumberString(config.ScreenArea.X, "0");
            textScreenAreaY.Text = GetNumberString(config.ScreenArea.Y, "0");


            //
            // Desktop size
            //
            if (config.AutomaticDesktopSize)
            {
                textDesktopWidth.Text = GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = GetNumberString(GetVirtualDesktopSize().Height);
                config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                config.DesktopSize.Height = GetVirtualDesktopSize().Height;
                textDesktopWidth.IsEnabled = false;
                textDesktopHeight.IsEnabled = false;
            }
            else
            {
                textDesktopWidth.Text = GetNumberString(config.DesktopSize.Width);
                textDesktopHeight.Text = GetNumberString(config.DesktopSize.Height);
            }
            checkBoxAutomaticDesktopSize.IsChecked = config.AutomaticDesktopSize;


            // Force aspect ratio
            if (config.ForceAspectRatio)
            {
                config.TabletArea.Height = config.TabletArea.Width / (config.ScreenArea.Width / config.ScreenArea.Height);
                textTabletAreaHeight.Text = GetNumberString(config.TabletArea.Height);
                textTabletAreaHeight.IsEnabled = false;
            }


            //
            // Move tablet area to a valid position
            //
            config.TabletArea.MoveInside(config.TabletFullArea);
            textTabletAreaX.Text = GetNumberString(config.TabletArea.X);
            textTabletAreaY.Text = GetNumberString(config.TabletArea.Y);


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
            // Filter
            //
            checkBoxFilter.IsChecked = config.FilterEnabled;
            textFilterValue.Text = GetNumberString(config.FilterValue);
            if (config.FilterEnabled)
                textFilterValue.IsEnabled = true;
            else
                textFilterValue.IsEnabled = false;


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

            // Tablet area
            if (ParseNumber(textTabletAreaWidth.Text, out double value))
                config.TabletArea.Width = value;
            if (ParseNumber(textTabletAreaHeight.Text, out value))
                config.TabletArea.Height = value;
            if (ParseNumber(textTabletAreaX.Text, out value))
                config.TabletArea.X = value;
            if (ParseNumber(textTabletAreaY.Text, out value))
                config.TabletArea.Y = value;
            if (ParseNumber(textTabletAreaRotation.Text, out value))
                config.TabletArea.Rotation = value;

            config.ForceAspectRatio = (bool)checkBoxForceAspect.IsChecked;
            config.ForceFullArea = (bool)checkBoxForceFullArea.IsChecked;

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

                textTabletAreaWidth.Text = GetNumberString(config.TabletArea.Width);
                textTabletAreaHeight.Text = GetNumberString(config.TabletArea.Height);

                // Center
                //config.TabletArea.X = config.TabletFullArea.Width / 2.0;
                //config.TabletArea.Y = config.TabletFullArea.Height / 2.0;
            }

            config.TabletArea.MoveInside(config.TabletFullArea);

            // Screen area
            if (ParseNumber(textScreenAreaWidth.Text, out value))
                config.ScreenArea.Width = value;
            if (ParseNumber(textScreenAreaHeight.Text, out value))
                config.ScreenArea.Height = value;
            if (ParseNumber(textScreenAreaX.Text, out value))
                config.ScreenArea.X = value;
            if (ParseNumber(textScreenAreaY.Text, out value))
                config.ScreenArea.Y = value;


            // Desktop size
            if (ParseNumber(textDesktopWidth.Text, out value))
                config.DesktopSize.Width = value;
            if (ParseNumber(textDesktopHeight.Text, out value))
                config.DesktopSize.Height = value;
            config.AutomaticDesktopSize = (bool)checkBoxAutomaticDesktopSize.IsChecked;
            if (config.AutomaticDesktopSize == true)
            {
                textDesktopWidth.Text = GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = GetNumberString(GetVirtualDesktopSize().Height);
                config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                config.DesktopSize.Height = GetVirtualDesktopSize().Height;
            }


            // Force aspect ratio
            if (config.ForceAspectRatio)
            {
                config.TabletArea.Height = config.TabletArea.Width / (config.ScreenArea.Width / config.ScreenArea.Height);
                textTabletAreaHeight.Text = GetNumberString(config.TabletArea.Height);
            }


            // Button map 
            config.ButtonMap[0] = comboBoxButton1.SelectedIndex;
            config.ButtonMap[1] = comboBoxButton2.SelectedIndex;
            config.ButtonMap[2] = comboBoxButton3.SelectedIndex;
            config.DisableButtons = (bool)checkBoxDisableButtons.IsChecked;



            // Filter
            config.FilterEnabled = (bool)checkBoxFilter.IsChecked;
            if (ParseNumber(textFilterValue.Text, out value))
                config.FilterValue = value;
            if (config.FilterEnabled)
                textFilterValue.IsEnabled = true;
            else
                textFilterValue.IsEnabled = false;


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
        // String to Number
        //
        private bool ParseNumber(string str, out double value)
        {
            value = 0;
            if (double.TryParse(str, NumberStyles.AllowDecimalPoint | NumberStyles.AllowLeadingSign, cultureEnglish.NumberFormat, out double tmp))
            {
                value = tmp;
                return true;
            }
            return false;
        }

        //
        // Number to String
        //
        private string GetNumberString(double value)
        {
            return GetNumberString(value, "0.##");
        }
        private string GetNumberString(double value, string format)
        {
            return value.ToString(format, cultureEnglish.NumberFormat);
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
            textScreenAspectRatio = new TextBlock();
            textScreenAspectRatio.Text = "";
            textScreenAspectRatio.Foreground = Brushes.Black;
            textScreenAspectRatio.FontSize = 11;
            textScreenAspectRatio.FontWeight = FontWeights.Bold;
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
            // Tablet area center dot
            //
            ellipseTabletAreaCenter = new Ellipse
            {
                Fill = new SolidColorBrush(Color.FromRgb(100, 100, 100)),
            };
            canvasTabletArea.Children.Add(ellipseTabletAreaCenter);

            //
            // Tablet area aspect ratio text
            //
            textTabletAspectRatio = new TextBlock();
            textTabletAspectRatio.Text = "";
            textTabletAspectRatio.Foreground = Brushes.Black;
            textTabletAspectRatio.FontSize = 11;
            textTabletAspectRatio.FontWeight = FontWeights.Bold;
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
            textScreenAspectRatio.Text = GetNumberString(config.ScreenArea.Width / config.ScreenArea.Height, "0.###");
            Canvas.SetLeft(textScreenAspectRatio, offsetX + config.ScreenArea.X * scale + 3);
            Canvas.SetTop(textScreenAspectRatio, offsetY + config.ScreenArea.Y * scale + 2);



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
            // Tablet area center dot
            //
            ellipseTabletAreaCenter.Width = 6;
            ellipseTabletAreaCenter.Height = 6;
            Canvas.SetLeft(ellipseTabletAreaCenter, offsetX + config.TabletArea.X * scale - 3);
            Canvas.SetTop(ellipseTabletAreaCenter, offsetY + config.TabletArea.Y * scale - 3);


            //
            // Tablet area aspect ratio text
            //
            textTabletAspectRatio.Text = GetNumberString(config.TabletArea.Width / config.TabletArea.Height, "0.###");
            Canvas.SetLeft(textTabletAspectRatio, offsetX + (config.TabletArea.X - config.TabletArea.Width / 2) * scale + 2);
            Canvas.SetTop(textTabletAspectRatio, offsetY + (config.TabletArea.Y - config.TabletArea.Height / 2) * scale + 0);


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
            textScreenAreaX.Text = GetNumberString(config.ScreenArea.X, "0");
            textScreenAreaY.Text = GetNumberString(config.ScreenArea.Y, "0");
            textTabletAreaX.Text = GetNumberString(config.TabletArea.X);
            textTabletAreaY.Text = GetNumberString(config.TabletArea.Y);
            isLoadingSettings = false;
            canvasScreenMap.ReleaseMouseCapture();
            canvasTabletArea.ReleaseMouseCapture();
        }

        // Canvas mouse move
        private void Canvas_MouseMove(object sender, System.Windows.Input.MouseEventArgs e)
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
                if (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.LeftCtrl))
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
                textDesktopWidth.Text = GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = GetNumberString(GetVirtualDesktopSize().Height);
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
            config.WindowWidth = (int)e.NewSize.Width;
            config.WindowHeight = (int)e.NewSize.Height;
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
                textScreenAreaWidth.Text = GetNumberString(config.DesktopSize.Width);
                textScreenAreaHeight.Text = GetNumberString(config.DesktopSize.Height);
            }
            else if (index > 0)
            {
                index--;
                if (index >= 0 && index < screens.Length)
                {
                    textScreenAreaX.Text = GetNumberString(screens[index].Bounds.X - minX);
                    textScreenAreaY.Text = GetNumberString(screens[index].Bounds.Y - minY);
                    textScreenAreaWidth.Text = GetNumberString(screens[index].Bounds.Width);
                    textScreenAreaHeight.Text = GetNumberString(screens[index].Bounds.Height);
                }
            }
            UpdateSettingsToConfiguration();
        }

        //
        // Save settings
        //
        private void SaveSettings(object sender, RoutedEventArgs e)
        {
            config.Write("Config.xml");
            SendSettingsToDriver();
            SetStatus("Settings saved!");
        }

        //
        // Apply settings
        //
        private void ApplySettings(object sender, RoutedEventArgs e)
        {
            SendSettingsToDriver();
            SetStatus("Settings applied!");
        }

        #endregion



        #region Statusbar

        //
        // Update statusbar
        //
        private void SetStatus(string text)
        {
            System.Windows.Application.Current.Dispatcher.Invoke(() =>
            {
                textStatus.Text = text;
            });
            timerStatusbar.Stop();
            timerStatusbar.Start();
        }

        //
        // Statusbar timer tick
        //
        private void TimerStatusbar_Tick(object sender, EventArgs e)
        {
            System.Windows.Application.Current.Dispatcher.Invoke(() =>
            {
                textStatus.Text = "";
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
            System.Windows.Application.Current.Dispatcher.Invoke(() =>
            {
                ParseDriverMessage(e.Message);
            });
        }
        // Error
        private void OnDriverErrorReceived(object sender, TabletDriver.DriverEventArgs e)
        {
            //ConsoleAddText("ERROR! " + e.Message);
        }
        // Started
        private void OnDriverStarted(object sender, EventArgs e)
        {
            SendSettingsToDriver();
            driver.SendCommand("info");
            driver.SendCommand("start");
        }
        // Stopped
        private void OnDriverStopped(object sender, EventArgs e)
        {
            if (running)
            {
                SetStatus("Driver stopped. Restarting! Check console !!!");
                driver.ConsoleAddText("Driver stopped. Restarting!");

                // Run in the main application thread
                System.Windows.Application.Current.Dispatcher.Invoke(() =>
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
        // Parse driver message
        //
        private void ParseDriverMessage(string line)
        {

            //
            // Tablet Name
            //
            Match match;
            Regex regexTabletName = new Regex("^.*?\\[INFO\\] Tablet: (.*?)$");

            match = Regex.Match(line, "^.*?\\[INFO\\] Tablet: (.*?)$");
            if (match.Success)
            {
                string tabletName = match.Groups[1].ToString();
                Title = "TabletDriverGUI - " + tabletName;
                notifyIcon.Text = tabletName;
                SetStatus("Connected to " + tabletName);
            }
            double value;

            //
            // Tablet width
            //
            match = Regex.Match(line, "^.*?\\[INFO\\] Tablet width = ([0-9\\.]+) mm");
            if (match.Success)
            {
                if (ParseNumber(match.Groups[1].ToString(), out value))
                {
                    config.TabletFullArea.Width = value;
                    config.TabletFullArea.X = value / 2.0;
                    LoadSettingsFromConfiguration();
                    UpdateSettingsToConfiguration();
                    if (isFirstStart)
                    {
                        SendSettingsToDriver();
                    }
                }
            }

            //
            // Tablet height
            //
            match = Regex.Match(line, "^.*?\\[INFO\\] Tablet height = ([0-9\\.]+) mm");
            if (match.Success)
            {
                if (ParseNumber(match.Groups[1].ToString(), out value))
                {
                    config.TabletFullArea.Height = value;
                    config.TabletFullArea.Y = value / 2.0;
                    LoadSettingsFromConfiguration();
                    UpdateSettingsToConfiguration();
                    if (isFirstStart)
                    {
                        SendSettingsToDriver();
                    }

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
                GetNumberString(config.ScreenArea.Width) + " " + GetNumberString(config.ScreenArea.Height) + " " +
                GetNumberString(config.ScreenArea.X) + " " + GetNumberString(config.ScreenArea.Y)
            );

            // Tablet area
            driver.SendCommand("Area " +
                GetNumberString(config.TabletArea.Width) + " " + GetNumberString(config.TabletArea.Height) + " " +
                GetNumberString(config.TabletArea.X) + " " + GetNumberString(config.TabletArea.Y)
            );

            // Rotation
            driver.SendCommand("Rotate " + GetNumberString(config.TabletArea.Rotation));

            // Output Mode
            switch (config.OutputMode)
            {
                case Configuration.OutputModes.Absolute:
                    driver.SendCommand("Mode Absolute");
                    break;
                case Configuration.OutputModes.Relative:
                    driver.SendCommand("Mode Relative");
                    driver.SendCommand("Sensitivity " + GetNumberString(config.ScreenArea.Width / config.TabletArea.Width));
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

            // Filter
            if (config.FilterEnabled)
            {
                driver.SendCommand("Filter " + GetNumberString(config.FilterValue));
            }
            else
            {
                driver.SendCommand("Filter 0");
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
        private void TextConsoleInput_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
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
        private void TextConsoleInput_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
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


        #endregion

        //
        // Console output context menu
        //
        private void ConsoleMenuClick(object sender, RoutedEventArgs e)
        {
            string menuItemHeader = ((MenuItem)sender).Header.ToString();
            if (menuItemHeader.ToLower() == "copy all")
            {
                Clipboard.SetText(textConsole.Text);
                SetStatus("Console output copied to clipboard");
            }

        }

        //
        // Wacom Area
        //
        private void ButtonWacomArea_Click(object sender, RoutedEventArgs e)
        {
            WacomArea wacom = new WacomArea();
            wacom.textWacomLeft.Text = GetNumberString((config.TabletArea.X - config.TabletArea.Width / 2) * 100.0, "0");
            wacom.textWacomRight.Text = GetNumberString((config.TabletArea.X + config.TabletArea.Width / 2) * 100.0, "0");

            wacom.textWacomTop.Text = GetNumberString((config.TabletArea.Y - config.TabletArea.Height / 2) * 100.0, "0");
            wacom.textWacomBottom.Text = GetNumberString((config.TabletArea.Y + config.TabletArea.Height / 2) * 100.0, "0");

            wacom.ShowDialog();

            // Set button clicked
            if (wacom.DialogResult == true)
            {
                if (
                    ParseNumber(wacom.textWacomLeft.Text, out double left) &&
                    ParseNumber(wacom.textWacomRight.Text, out double right) &&
                    ParseNumber(wacom.textWacomTop.Text, out double top) &&
                    ParseNumber(wacom.textWacomBottom.Text, out double bottom)
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
    }
}