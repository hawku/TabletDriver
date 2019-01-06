using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TabletDriverGUI
{
    public partial class MainWindow : Window
    {

        //
        // Start the driver
        //
        void StartDriver()
        {

            if (running) return;

            // Try to start the driver
            try
            {
                running = true;

                // Console timer
                timerConsoleUpdate.Start();

                // Pen position timer
                //timerUpdatePenPositions.Start();

                driver.Start(config.DriverPath, config.DriverArguments);
                if (!driver.IsRunning)
                {
                    SetStatus("Can't start the driver! Check the console!");
                    driver.ConsoleAddLine("ERROR! Can't start the driver!");
                }
                else
                {
                    SetStatus("Driver starting...");
                }
            }

            // Start failed
            catch (Exception e)
            {
                SetStatus("Can't start the driver! Check the console!");
                driver.ConsoleAddLine("ERROR! Can't start the driver!\n  " + e.Message);
            }
        }


        //
        // Stop the driver
        //
        void StopDriver()
        {
            if (!running) return;
            running = false;

            //timerUpdatePenPositions.Stop();

            driver.Stop();
            timerConsoleUpdate.Stop();
        }


        //
        // Send settings to the driver
        //
        private void SendSettingsToDriver()
        {
            if (!driver.IsRunning) return;

            // Clear setting commands list
            settingCommands.Clear();

            //
            // Desktop size
            //
            settingCommands.Add("DesktopSize " + textDesktopWidth.Text + " " + textDesktopHeight.Text);


            //
            // Screen and tablet areas
            //
            int areaIndex = 0;
            for (int i = 0; i < config.GetAreaCount(); i++)
            {
                if (config.ScreenAreas[i].IsEnabled)
                {
                    // Screen area
                    settingCommands.Add("ScreenArea " +
                        Utils.GetNumberString(config.ScreenAreas[i].Width) + " " + Utils.GetNumberString(config.ScreenAreas[i].Height) + " " +
                        Utils.GetNumberString(config.ScreenAreas[i].X) + " " + Utils.GetNumberString(config.ScreenAreas[i].Y) + " " +
                        areaIndex
                    );

                    // Inverted tablet area
                    if (config.Invert)
                    {
                        settingCommands.Add("TabletArea " +
                            Utils.GetNumberString(config.TabletAreas[i].Width) + " " +
                            Utils.GetNumberString(config.TabletAreas[i].Height) + " " +
                            Utils.GetNumberString(config.TabletFullArea.Width - config.TabletAreas[i].X) + " " +
                            Utils.GetNumberString(config.TabletFullArea.Height - config.TabletAreas[i].Y) + " " +
                            areaIndex
                        );
                        settingCommands.Add(
                            "Rotate " + Utils.GetNumberString(config.TabletAreas[0].Rotation + 180) + " " +
                            areaIndex
                        );

                    }

                    // Normal tablet area
                    else
                    {
                        settingCommands.Add("TabletArea " +
                            Utils.GetNumberString(config.TabletAreas[i].Width) + " " +
                            Utils.GetNumberString(config.TabletAreas[i].Height) + " " +
                            Utils.GetNumberString(config.TabletAreas[i].X) + " " +
                            Utils.GetNumberString(config.TabletAreas[i].Y) + " " +
                            areaIndex
                        );
                        settingCommands.Add(
                            "Rotate " + Utils.GetNumberString(config.TabletAreas[0].Rotation) + " " +
                            areaIndex
                        );
                    }
                    areaIndex++;
                }



            }
            settingCommands.Add("ScreenMapCount " + areaIndex);


            //
            // Output Mode
            //
            switch (config.Mode)
            {
                case Configuration.OutputModes.Standard:

                    // Windows 8, 8.1, 10
                    if (VersionHelper.IsWindows8OrGreater())
                    {
                        if (config.Positioning == Configuration.OutputPositioning.Absolute)
                            settingCommands.Add("OutputMode Absolute");
                        else
                            settingCommands.Add("OutputMode Relative");
                    }

                    // Windows 7
                    else
                    {
                        if (config.Positioning == Configuration.OutputPositioning.Absolute)
                            settingCommands.Add("OutputMode SendInputAbsolute");
                        else
                            settingCommands.Add("OutputMode SendInputRelative");
                    }
                    break;

                case Configuration.OutputModes.WindowsInk:
                    if (config.Positioning == Configuration.OutputPositioning.Absolute)
                        settingCommands.Add("OutputMode DigitizerAbsolute");
                    else
                        settingCommands.Add("OutputMode DigitizerRelative");
                    break;
                case Configuration.OutputModes.Compatibility:

                    // Windows 8, 8.1, 10
                    if (VersionHelper.IsWindows8OrGreater())
                    {
                        if (config.Positioning == Configuration.OutputPositioning.Absolute)
                            settingCommands.Add("OutputMode SendInputAbsolute");
                        else
                            settingCommands.Add("OutputMode SendInputRelative");
                    }

                    // Windows 7
                    else
                    {
                        if (config.Positioning == Configuration.OutputPositioning.Absolute)
                            settingCommands.Add("OutputMode Absolute");
                        else
                            settingCommands.Add("OutputMode Relative");
                    }
                    break;
                default:
                    break;
            }

            //
            // Relative positioning sensitivity
            //
            settingCommands.Add("RelativeSensitivity " +
                 Utils.GetNumberString(config.ScreenAreas[0].Width / config.TabletAreas[0].Width) +
                 " " +
                 Utils.GetNumberString(config.ScreenAreas[0].Height / config.TabletAreas[0].Height)
             );


            //
            // Pen button map
            //
            if (config.DisableButtons)
            {
                settingCommands.Add("ClearButtonMap");
            }
            else
            {
                settingCommands.Add("ClearButtonMap");
                int button = 1;
                foreach (string key in config.ButtonMap)
                {
                    settingCommands.Add("ButtonMap " + button + " \"" + key + "\"");
                    button++;
                }
            }


            //
            // Tablet button map
            //
            if (config.DisableTabletButtons)
            {
                settingCommands.Add("ClearAuxButtonMap");
            }
            else
            {
                settingCommands.Add("ClearAuxButtonMap");
                int button = 1;
                foreach (string key in config.TabletButtonMap)
                {
                    if (key != "")
                    {
                        settingCommands.Add("AuxButtonMap " + button + " \"" + key + "\"");
                    }
                    button++;
                }
            }


            //
            // Pressure
            //
            settingCommands.Add("PressureSensitivity " + Utils.GetNumberString(config.PressureSensitivity));
            settingCommands.Add("PressureDeadzone " +
                Utils.GetNumberString(config.PressureDeadzoneLow) + " " +
                Utils.GetNumberString(config.PressureDeadzoneHigh)
            );


            //
            // Scroll
            //
            settingCommands.Add("ScrollSensitivity " + Utils.GetNumberString(config.ScrollSensitivity));
            settingCommands.Add("ScrollAcceleration " + Utils.GetNumberString(config.ScrollAcceleration));
            settingCommands.Add("ScrollStopCursor " + (config.ScrollStopCursor ? "true" : "false"));
            settingCommands.Add("ScrollDrag " + (config.ScrollDrag ? "true" : "false"));


            //
            // Smoothing filter
            //
            if (config.SmoothingEnabled)
            {
                settingCommands.Add(
                    "Smoothing " + Utils.GetNumberString(config.SmoothingLatency) + " " +
                    (config.SmoothingOnlyWhenButtons ? "true" : "false")
                );
                settingCommands.Add("FilterTimerInterval " + Utils.GetNumberString(config.SmoothingInterval));
            }
            else
            {
                settingCommands.Add("Smoothing 0");
                settingCommands.Add("FilterTimerInterval 10");
            }


            //
            // Noise filter
            //
            if (config.NoiseFilterEnabled)
            {
                settingCommands.Add("Noise " + Utils.GetNumberString(config.NoiseFilterBuffer) + " " + Utils.GetNumberString(config.NoiseFilterThreshold));
            }
            else
            {
                settingCommands.Add("Noise 0");
            }


            //
            // Anti-smoothing filter
            //
            if (config.AntiSmoothingEnabled)
            {
                settingCommands.Add(
                    "AntiSmoothing " +
                    (config.AntiSmoothingOnlyWhenHover ? "true" : "false") + " " +
                    Utils.GetNumberString(config.AntiSmoothingDragMultiplier)
                );

                Configuration.AntiSmoothingSetting[] settings;
                settings = (Configuration.AntiSmoothingSetting[])config.AntiSmoothingSettings.Clone();

                // Sort
                Array.Sort(settings, (a, b) =>
                {
                    if (a.Velocity > b.Velocity) return 1;
                    if (b.Velocity > a.Velocity) return -1;
                    return 0;
                });
                foreach (var setting in settings)
                {
                    if (setting.Enabled)
                    {
                        settingCommands.Add("AntiSmoothingAdd " +
                            Utils.GetNumberString(setting.Velocity) + " " +
                            Utils.GetNumberString(setting.Shape, "0.000") + " " +
                            Utils.GetNumberString(setting.Compensation)
                        );
                    }
                }

            }
            else
            {
                settingCommands.Add("AntiSmoothing off");
            }


            //
            // Debugging
            //
            if (config.DebuggingEnabled)
            {
                settingCommands.Add("Debug true");
            }
            else
            {
                settingCommands.Add("Debug false");
            }


            //
            // Custom commands
            //
            if (config.CustomCommands.Length > 0)
            {
                foreach (string command in config.CustomCommands)
                {
                    string tmp = command.Trim();
                    if (tmp.Length > 0)
                    {
                        settingCommands.Add(tmp);
                    }
                }
            }


            //
            // Send commands to the driver
            //
            foreach (string command in settingCommands)
            {
                // Skip comments
                if (command.StartsWith("#")) continue;

                driver.SendCommand(command);
            }


            //
            // Write settings to usersettings.cfg
            //
            try
            {
                File.WriteAllLines("config\\usersettings.cfg", settingCommands.ToArray());
            }
            catch (Exception)
            {
            }

        }


        //
        // Driver message received
        //
        private void OnDriverMessageReceived(object sender, TabletDriver.DriverEventArgs e)
        {
            //ConsoleAddText(e.Message);
        }


        //
        // Driver error received
        //
        private void OnDriverErrorReceived(object sender, TabletDriver.DriverEventArgs e)
        {
            SetStatusWarning(e.Message);
        }


        //
        // Driver status message received
        //
        private void OnDriverStatusReceived(object sender, TabletDriver.DriverEventArgs e)
        {
            string variableName = e.Message;
            string parameters = e.Parameters;
            Application.Current.Dispatcher.Invoke(() =>
            {
                ProcessStatusMessage(variableName, parameters);
            });
        }
        // Process driver status message
        private void ProcessStatusMessage(string variableName, string parameters)
        {

            //
            // Startup commands request
            //
            if(variableName == "startup_request")
            {
                SendStartupCommands();
            }


            //
            // Settings request
            //
            else if (variableName == "settings_request")
            {
                SendSettingsToDriver();
            }

            //
            // Tablet Name
            //
            else if (variableName == "tablet")
            {
                string tabletName = parameters;
                string title = "TabletDriverGUI - " + tabletName;
                Regex regex = new Regex("\\([^\\)]+\\)");
                config.TabletName = regex.Replace(tabletName, "");
                Title = title;

                // Limit notify icon text length
                if (tabletName.Length > 63)
                {
                    notifyIcon.Text = tabletName.Substring(0, 63);
                }
                else
                {
                    notifyIcon.Text = tabletName;
                }
                SetStatus("Connected to " + tabletName);
            }


            //
            // Tablet width
            //
            else if (variableName == "width")
            {
                if (Utils.ParseNumber(parameters, out double val))
                {
                    config.TabletFullArea.Width = val;
                    config.TabletFullArea.X = val / 2.0;
                    if (isFirstStart)
                    {
                        config.TabletAreas[0].Width = config.TabletFullArea.Width;
                        config.TabletAreas[0].X = config.TabletFullArea.X;
                        FixTabletAreaDimensions(config.TabletAreas[0], config.ScreenAreas[0]);
                        SendSettingsToDriver();
                    }
                    LoadSettingsFromConfiguration();
                    UpdateSettingsToConfiguration();
                }
            }


            //
            // Tablet height
            //
            else if (variableName == "height")
            {
                if (Utils.ParseNumber(parameters, out double val))
                {
                    config.TabletFullArea.Height = val;
                    config.TabletFullArea.Y = val / 2.0;
                    if (isFirstStart)
                    {
                        config.TabletAreas[0].Height = config.TabletFullArea.Height;
                        config.TabletAreas[0].Y = config.TabletFullArea.Y;
                        FixTabletAreaDimensions(config.TabletAreas[0], config.ScreenAreas[0]);
                        SendSettingsToDriver();
                    }
                    LoadSettingsFromConfiguration();
                    UpdateSettingsToConfiguration();

                }
            }


            //
            // Tablet measurement to tablet area
            //
            else if (variableName == "measurement" && isEnabledMeasurementToArea)
            {
                string[] stringValues = parameters.Split(' ');
                int valueCount = stringValues.Count();
                if (valueCount >= 4)
                {
                    double minimumX = 10000;
                    double minimumY = 10000;
                    double maximumX = -10000;
                    double maximumY = -10000;
                    for (int i = 0; i < valueCount; i += 2)
                    {
                        if (
                            Utils.ParseNumber(stringValues[i], out double x)
                            &&
                            Utils.ParseNumber(stringValues[i + 1], out double y)
                        )
                        {
                            // Find limits
                            if (x > maximumX) maximumX = x;
                            if (x < minimumX) minimumX = x;
                            if (y > maximumY) maximumY = y;
                            if (y < minimumY) minimumY = y;
                        }
                    }

                    double areaWidth = maximumX - minimumX;
                    double areaHeight = maximumY - minimumY;
                    double centerX = minimumX + areaWidth / 2.0;
                    double centerY = minimumY + areaHeight / 2.0;

                    config.SelectedTabletArea.Width = areaWidth;
                    config.SelectedTabletArea.Height = areaHeight;
                    config.SelectedTabletArea.X = centerX;
                    config.SelectedTabletArea.Y = centerY;
                    LoadSettingsFromConfiguration();
                    UpdateSettingsToConfiguration();


                }
                isEnabledMeasurementToArea = false;
                buttonDrawArea.IsEnabled = true;
                SetStatus("");
            }


            //
            // Tablet buttons
            //
            else if (variableName == "aux_buttons")
            {
                if (Utils.ParseNumber(parameters, out double test))
                {
                    tabletButtonCount = (int)test;
                    if (tabletButtonCount > 0)
                    {
                        for (int i = 0; i < 16; i++)
                        {
                            GroupBox box = (GroupBox)wrapPanelTabletButtons.Children[i];
                            if (i >= tabletButtonCount)
                            {
                                box.Visibility = Visibility.Collapsed;
                            }
                            else
                            {
                                box.Visibility = Visibility.Visible;
                            }
                        }
                        groupBoxTabletButtons.Visibility = Visibility.Visible;

                    }
                    if (isFirstStart)
                        SendSettingsToDriver();
                }

            }

            //
            // Driver started
            //
            else if (variableName == "started")
            {
                if (parameters.Trim() == "1" || parameters.Trim().ToLower() == "true")
                {
                    isFirstStart = false;
                }
            }
        }


        //
        // Driver Started
        //
        private void OnDriverStarted(object sender, EventArgs e)
        {
          
        }

        private void SendStartupCommands()
        {
            // Debugging commands
            if (config.DebuggingEnabled)
            {
                driver.SendCommand("HIDList");
            }

            driver.SendCommand("GetCommands");
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
            driver.SendCommand("Echo");
        }


        //
        // Driver Stopped
        //
        private void OnDriverStopped(object sender, EventArgs e)
        {
            if (running)
            {

                // Automatic restart?
                if (config.AutomaticRestart)
                {
                    SetStatus("Driver stopped. Restarting! Check console !!!");
                    driver.ConsoleAddLine("Driver stopped. Restarting!");

                    // Run in the main application thread
                    Application.Current.Dispatcher.Invoke(() =>
                    {
                        driver.Stop();
                        timerRestart.Start();
                    });

                }
                else
                {
                    SetStatus("Driver stopped!");
                    driver.ConsoleAddLine("Driver stopped!");
                }

                // Run in the main application thread
                Application.Current.Dispatcher.Invoke(() =>
                {
                    Title = "TabletDriverGUI";
                    notifyIcon.Text = "No tablet found";
                    groupBoxTabletButtons.Visibility = Visibility.Collapsed;
                });

            }
        }


        //
        // Driver restart timer tick
        //
        private void TimerRestart_Tick(object sender, EventArgs e)
        {
            if (running)
            {
                driver.Start(config.DriverPath, config.DriverArguments);
            }
            timerRestart.Stop();
        }


        //
        // Restart Driver button click
        //
        private void RestartDriverClick(object sender, RoutedEventArgs e)
        {
            if (running)
            {
                StopDriver();
            }
            StartDriver();
        }


    }
}
