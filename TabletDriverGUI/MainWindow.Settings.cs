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
    public partial class MainWindow : Window
    {


        #region Driver configuration stuff

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
            comboBoxSmoothingRate.SelectedIndex = config.SmoothingInterval - 1;
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
            // Noise filter
            //
            checkBoxNoiseFilter.IsChecked = config.NoiseFilterEnabled;
            textNoiseBuffer.Text = Utils.GetNumberString(config.NoiseFilterBuffer);
            textNoiseThreshold.Text = Utils.GetNumberString(config.NoiseFilterThreshold);
            if (config.NoiseFilterEnabled)
            {
                textNoiseBuffer.IsEnabled = true;
                textNoiseThreshold.IsEnabled = true;
            }
            else
            {
                textNoiseBuffer.IsEnabled = false;
                textNoiseThreshold.IsEnabled = false;
            }


            //
            // Anti-smoothing filter
            //
            checkBoxAntiSmoothing.IsChecked = config.AntiSmoothingEnabled;
            textAntiSmoothingShape.Text = Utils.GetNumberString(config.AntiSmoothingShape, "0.00");
            textAntiSmoothingCompensation.Text = Utils.GetNumberString(config.AntiSmoothingCompensation, "0.00");
            checkBoxAntiSmoothingIgnoreWhenDragging.IsChecked = config.AntiSmoothingIgnoreWhenDragging;
            if (config.AntiSmoothingEnabled)
            {
                textAntiSmoothingShape.IsEnabled = true;
                textAntiSmoothingCompensation.IsEnabled = true;
                checkBoxAntiSmoothingIgnoreWhenDragging.IsEnabled = true;
            }
            else
            {
                textAntiSmoothingShape.IsEnabled = false;
                textAntiSmoothingCompensation.IsEnabled = false;
                checkBoxAntiSmoothingIgnoreWhenDragging.IsEnabled = false;
            }

            //
            // Automatic restart
            //
            checkBoxAutomaticRestart.IsChecked = config.AutomaticRestart;

            //
            // Run at startup
            //
            checkBoxRunAtStartup.IsChecked = config.RunAtStartup;


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



            // Smoothing filter
            config.SmoothingEnabled = (bool)checkBoxSmoothing.IsChecked;
            config.SmoothingInterval = comboBoxSmoothingRate.SelectedIndex + 1;
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

            // Noise filter
            config.NoiseFilterEnabled = (bool)checkBoxNoiseFilter.IsChecked;
            if (Utils.ParseNumber(textNoiseBuffer.Text, out val))
                config.NoiseFilterBuffer = (int)val;
            if (Utils.ParseNumber(textNoiseThreshold.Text, out val))
                config.NoiseFilterThreshold = val;
            if (config.NoiseFilterEnabled)
            {
                textNoiseBuffer.IsEnabled = true;
                textNoiseThreshold.IsEnabled = true;
            }
            else
            {
                textNoiseBuffer.IsEnabled = false;
                textNoiseThreshold.IsEnabled = false;
            }

            // Anti-smoothing filter
            config.AntiSmoothingEnabled = (bool)checkBoxAntiSmoothing.IsChecked;
            if (Utils.ParseNumber(textAntiSmoothingShape.Text, out val))
                config.AntiSmoothingShape = val;
            if (Utils.ParseNumber(textAntiSmoothingCompensation.Text, out val))
                config.AntiSmoothingCompensation = val;
            config.AntiSmoothingIgnoreWhenDragging = (bool)checkBoxAntiSmoothingIgnoreWhenDragging.IsChecked;
            if (config.AntiSmoothingEnabled)
            {
                textAntiSmoothingShape.IsEnabled = true;
                textAntiSmoothingCompensation.IsEnabled = true;
                checkBoxAntiSmoothingIgnoreWhenDragging.IsEnabled = true;
            }
            else
            {
                textAntiSmoothingShape.IsEnabled = false;
                textAntiSmoothingCompensation.IsEnabled = false;
                checkBoxAntiSmoothingIgnoreWhenDragging.IsEnabled = false;
            }

            //
            // Automatic restart
            //
            config.AutomaticRestart = (bool)checkBoxAutomaticRestart.IsChecked;


            //
            // Run at startup
            //
            oldValue = config.RunAtStartup;
            config.RunAtStartup = (bool)checkBoxRunAtStartup.IsChecked;
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

        #endregion



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



        #region Canvas stuff

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


        //
        // Canvas mouse up
        //
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


        //
        // Canvas mouse move
        //
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

        #endregion



        //
        // TextBox setting changed
        //
        private void TextChanged(object sender, TextChangedEventArgs e)
        {
            UpdateSettingsToConfiguration();
        }


        //
        // Checkbox setting changed
        //
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


        //
        // Selection settings changed
        //
        private void ItemSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            UpdateSettingsToConfiguration();
        }


        //
        // Window size changed
        //
        private void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (!IsLoaded || isLoadingSettings) return;
            if (WindowState != WindowState.Maximized)
            {
                config.WindowWidth = (int)e.NewSize.Width;
                config.WindowHeight = (int)e.NewSize.Height;
            }
        }


        //
        // Monitor combobox clicked -> create new monitor list
        //
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


        //
        // Monitor selected -> change screen map
        //
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


    }
}
