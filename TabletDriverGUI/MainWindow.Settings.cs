using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;

namespace TabletDriverGUI
{
    public partial class MainWindow : Window
    {
        private CheckBox[] checkboxAntiSmoothingSettingEnabled;
        private TextBox[] textAntiSmoothingVelocity;
        private TextBox[] textAntiSmoothingShape;
        private TextBox[] textAntiSmoothingCompensation;

        List<Configuration.Preset> presetsNoiseReduction;
        List<Configuration.Preset> presetsAntiSmoothing;

        WindowTabletView tabletView;


        //
        // Create setting UI components
        //
        private void CreateSettingElements()
        {

            //
            // Create tablet button map WrapPanel items
            //
            for (int i = 0; i < 16; i++)
            {
                GroupBox groupBox = new GroupBox
                {
                    Width = 90,
                    Header = "Button " + (i + 1).ToString()
                };
                Button button = new Button
                {
                    Height = 22,
                    Content = "",
                    Padding = new Thickness(2, 0, 2, 0),
                    ToolTip = "Empty",
                    Background = Brushes.White
                };
                button.Click += ButtonMap_Click;
                button.ToolTipOpening += ButtonMap_ToolTipOpening;

                groupBox.Content = button;
                wrapPanelTabletButtons.Children.Add(groupBox);
            }
            CheckBox checkBox = new CheckBox
            {
                Content = "Disable buttons"
            };
            checkBox.Checked += CheckboxChanged;
            checkBox.Unchecked += CheckboxChanged;
            checkBox.VerticalAlignment = VerticalAlignment.Bottom;
            checkBox.Margin = new Thickness(5, 5, 5, 10);
            wrapPanelTabletButtons.Children.Add(checkBox);



            //
            // Smoothing rate ComboBox
            //
            comboBoxSmoothingRate.Items.Clear();
            for (int i = 1; i <= 8; i++)
            {
                comboBoxSmoothingRate.Items.Add((1000.0 / i).ToString("0") + " Hz");
            }
            comboBoxSmoothingRate.SelectedIndex = 3;



            //
            // Noise reduction presets
            //

            presetsNoiseReduction = new List<Configuration.Preset>
            {
                new Configuration.Preset("Wacom 470/471", (conf) =>
                {
                    conf.NoiseFilterBuffer = 10;
                    conf.NoiseFilterThreshold = 0.4;
                }),
                new Configuration.Preset("Wacom 472", (conf) =>
                {
                    conf.NoiseFilterBuffer = 10;
                    conf.NoiseFilterThreshold = 0.3;
                }),
                new Configuration.Preset("Wacom 480", (conf) =>
                {
                    conf.NoiseFilterBuffer = 10;
                    conf.NoiseFilterThreshold = 0.5;
                }),
            };
            comboBoxNoiseReductionPresets.Items.Clear();
            foreach (var preset in presetsNoiseReduction)
            {
                comboBoxNoiseReductionPresets.Items.Add(preset.Name);
            }
            comboBoxNoiseReductionPresets.SelectionChanged += (sender, e) =>
            {
                int index = comboBoxNoiseReductionPresets.SelectedIndex;
                if (index >= 0 && index < presetsNoiseReduction.Count)
                {
                    Configuration.Preset preset = presetsNoiseReduction[comboBoxNoiseReductionPresets.SelectedIndex];
                    preset.Action(config);
                    LoadSettingsFromConfiguration();
                    SetStatus("Noise reduction filter preset '" + preset.Name + "' loaded!");
                }
            };







            //
            // Anti-smoothing presets
            //
            presetsAntiSmoothing = new List<Configuration.Preset>
            {
                new Configuration.Preset("Clear", (conf) => { }),
                new Configuration.Preset("Simple", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.5, 10);
                }),

                new Configuration.Preset("Gaomon S56K", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 10);
                }),

                new Configuration.Preset("Huion 420", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 15);
                }),

                new Configuration.Preset("Huion H420", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 15);
                }),

                new Configuration.Preset("Huion H430P", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 50);
                    conf.SetAntiSmoothingSetting(1, true, 70, 0.1, 30);
                    conf.SetAntiSmoothingSetting(2, true, 150, 0.1, 20);
                }),

                new Configuration.Preset("Huion H640P", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 50);
                    conf.SetAntiSmoothingSetting(1, true, 70, 0.1, 30);
                    conf.SetAntiSmoothingSetting(2, true, 150, 0.1, 20);
                }),

                new Configuration.Preset("VEIKK A50", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 10);
                }),


                new Configuration.Preset("VEIKK S640", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 0);
                    conf.SetAntiSmoothingSetting(1, true, 50, 1.5, 5);
                    conf.SetAntiSmoothingSetting(2, true, 100, 1.5, 10);
                }),

                new Configuration.Preset("Wacom 490 Hover", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 25);
                    conf.SetAntiSmoothingSetting(1, true, 100, 0.1, 20);
                    conf.AntiSmoothingOnlyWhenHover = true;
                }),

                new Configuration.Preset("XP-Pen G430", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 0);
                    conf.SetAntiSmoothingSetting(1, true, 50, 1.5, 5);
                    conf.SetAntiSmoothingSetting(2, true, 200, 1.5, 10);
                }),

                new Configuration.Preset("XP-Pen G640", (conf) =>
                {
                    conf.SetAntiSmoothingSetting(0, true, 0, 0.1, 0);
                    conf.SetAntiSmoothingSetting(1, true, 50, 1.5, 5);
                    conf.SetAntiSmoothingSetting(2, true, 200, 1.5, 10);
                }),

            };


            //
            // Anti-smoothing preset combobox
            //
            comboBoxAntiSmoothingPresets.Items.Clear();
            foreach (var preset in presetsAntiSmoothing)
            {
                comboBoxAntiSmoothingPresets.Items.Add(preset.Name);
            }
            comboBoxAntiSmoothingPresets.SelectionChanged += (sender, e) =>
            {
                int index = comboBoxAntiSmoothingPresets.SelectedIndex;
                if (index >= 0 && index < presetsAntiSmoothing.Count)
                {
                    Configuration.Preset preset = presetsAntiSmoothing[comboBoxAntiSmoothingPresets.SelectedIndex];
                    config.AntiSmoothingOnlyWhenHover = false;
                    config.AntiSmoothingDragMultiplier = 1.0;
                    config.ClearAntiSmoothingSettings();
                    preset.Action(config);
                    LoadSettingsFromConfiguration();
                    SetStatus("Anti-smoothing filter preset '" + preset.Name + "' loaded!");
                }
            };


            //
            // Anti-smoothing filter settings
            //
            int antiSmoothingSettingCount = 5;
            checkboxAntiSmoothingSettingEnabled = new CheckBox[antiSmoothingSettingCount];
            textAntiSmoothingVelocity = new TextBox[antiSmoothingSettingCount];
            textAntiSmoothingShape = new TextBox[antiSmoothingSettingCount];
            textAntiSmoothingCompensation = new TextBox[antiSmoothingSettingCount];

            if (config.AntiSmoothingSettings.Length < antiSmoothingSettingCount)
            {
                Configuration.AntiSmoothingSetting[] newSettings = new Configuration.AntiSmoothingSetting[antiSmoothingSettingCount];
                for (int i = 0; i < antiSmoothingSettingCount; i++)
                {
                    if (i < config.AntiSmoothingSettings.Length)
                    {
                        newSettings[i] = config.AntiSmoothingSettings[i];
                    }
                    else
                    {
                        newSettings[i] = new Configuration.AntiSmoothingSetting
                        {
                            Enabled = false,
                            Velocity = 0,
                            Shape = 0.5,
                            Compensation = 0
                        };
                    }
                }
                config.AntiSmoothingSettings = newSettings;
            }


            //
            // Loop through anti-smoothing settings
            //
            for (int i = 0; i < antiSmoothingSettingCount; i++)
            {

                // Enabled
                checkboxAntiSmoothingSettingEnabled[i] = new CheckBox
                {
                    Width = 20,
                    Height = 25,
                    Margin = new Thickness(1),
                    VerticalContentAlignment = VerticalAlignment.Center,
                    HorizontalAlignment = HorizontalAlignment.Right
                };
                checkboxAntiSmoothingSettingEnabled[i].Checked += CheckboxChanged;
                checkboxAntiSmoothingSettingEnabled[i].Unchecked += CheckboxChanged;
                stackPanelAntiSmoothingEnabled.Children.Add(checkboxAntiSmoothingSettingEnabled[i]);


                // Velocity 
                Grid grid = new Grid();
                Label label = new Label { Content = "mm/s", HorizontalAlignment = HorizontalAlignment.Right };
                textAntiSmoothingVelocity[i] = new TextBox
                {
                    Width = 80,
                    Height = 25,
                    Padding = new Thickness(2),
                    Margin = new Thickness(1)
                };
                textAntiSmoothingVelocity[i].TextChanged += TextChanged;
                textAntiSmoothingVelocity[i].DataContext = checkboxAntiSmoothingSettingEnabled[i];
                textAntiSmoothingVelocity[i].SetBinding(TextBox.IsEnabledProperty, "IsChecked");
                grid.Children.Add(textAntiSmoothingVelocity[i]);
                grid.Children.Add(label);
                stackPanelAntiSmoothingVelocity.Children.Add(grid);


                // Shape 
                textAntiSmoothingShape[i] = new TextBox
                {
                    Width = 60,
                    Height = 25,
                    Padding = new Thickness(2),
                    Margin = new Thickness(1)
                };
                textAntiSmoothingShape[i].TextChanged += TextChanged;
                textAntiSmoothingShape[i].DataContext = checkboxAntiSmoothingSettingEnabled[i];
                textAntiSmoothingShape[i].SetBinding(TextBox.IsEnabledProperty, "IsChecked");
                stackPanelAntiSmoothingShape.Children.Add(textAntiSmoothingShape[i]);


                // Compensation 
                grid = new Grid();
                label = new Label { Content = "ms", HorizontalAlignment = HorizontalAlignment.Right };
                textAntiSmoothingCompensation[i] = new TextBox
                {
                    Width = 100,
                    Height = 25,
                    Padding = new Thickness(2),
                    Margin = new Thickness(1)
                };
                textAntiSmoothingCompensation[i].TextChanged += TextChanged;
                textAntiSmoothingCompensation[i].DataContext = checkboxAntiSmoothingSettingEnabled[i];
                textAntiSmoothingCompensation[i].SetBinding(TextBox.IsEnabledProperty, "IsChecked");

                grid.Children.Add(textAntiSmoothingCompensation[i]);
                grid.Children.Add(label);
                stackPanelAntiSmoothingCompensation.Children.Add(grid);

            }


            //
            // Tablet view window
            //
            tabletView = null;

        }



        #region Configuration Load, Update, Init

        //
        // Load settings from configuration
        //
        private void LoadSettingsFromConfiguration()
        {
            if (isLoadingSettings) return;
            isLoadingSettings = true;

            //
            // Tablet area
            //
            textTabletAreaWidth.Text = Utils.GetNumberString(config.SelectedTabletArea.Width);
            textTabletAreaHeight.Text = Utils.GetNumberString(config.SelectedTabletArea.Height);
            textTabletAreaX.Text = Utils.GetNumberString(config.SelectedTabletArea.X);
            textTabletAreaY.Text = Utils.GetNumberString(config.SelectedTabletArea.Y);
            checkBoxForceAspect.IsChecked = config.ForceAspectRatio;


            //
            // Positioning & Mode
            //
            comboBoxOutputPositioning.SelectedIndex = (int)config.Positioning;

            switch (config.Mode)
            {
                case Configuration.OutputModes.Standard:
                    radioOutputModeStandard.IsChecked = true;
                    break;
                case Configuration.OutputModes.WindowsInk:
                    radioOutputModeWindowsInk.IsChecked = true;
                    break;
                case Configuration.OutputModes.Compatibility:
                    radioOutputModeCombatibility.IsChecked = true;
                    break;
                default:
                    break;
            }



            //
            // Windows Ink pressure
            //
            if (config.Mode == Configuration.OutputModes.WindowsInk)
                groupBoxWindowsInkSettings.IsEnabled = true;
            else
                groupBoxWindowsInkSettings.IsEnabled = false;


            //
            // Rotation
            //
            textTabletAreaRotation.Text = Utils.GetNumberString(config.TabletAreas[0].Rotation);
            checkBoxInvert.IsChecked = config.Invert;

            //
            // Desktop size
            //
            if (config.AutomaticDesktopSize)
            {
                textDesktopWidth.Text = Utils.GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = Utils.GetNumberString(GetVirtualDesktopSize().Height);
                config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                config.DesktopSize.Height = GetVirtualDesktopSize().Height;
                config.DesktopSize.X = config.DesktopSize.Width / 2.0;
                config.DesktopSize.Y = config.DesktopSize.Height / 2.0;
                textDesktopWidth.IsEnabled = false;
                textDesktopHeight.IsEnabled = false;
            }
            else
            {
                textDesktopWidth.Text = Utils.GetNumberString(config.DesktopSize.Width);
                textDesktopHeight.Text = Utils.GetNumberString(config.DesktopSize.Height);
                config.DesktopSize.X = config.DesktopSize.Width / 2.0;
                config.DesktopSize.Y = config.DesktopSize.Height / 2.0;
            }
            checkBoxAutomaticDesktopSize.IsChecked = config.AutomaticDesktopSize;


            //
            // Screen area
            //
            textScreenAreaWidth.Text = Utils.GetNumberString(config.SelectedScreenArea.Width, "0");
            textScreenAreaHeight.Text = Utils.GetNumberString(config.SelectedScreenArea.Height, "0");
            textScreenAreaX.Text = Utils.GetNumberString(config.SelectedScreenArea.X - config.SelectedScreenArea.Width / 2.0, "0");
            textScreenAreaY.Text = Utils.GetNumberString(config.SelectedScreenArea.Y - config.SelectedScreenArea.Height / 2.0, "0");


            //
            // Move screen areas to valid positions
            //
            for (int i = 0; i < config.GetAreaCount(); i++)
                config.ScreenAreas[i].MoveInside(config.DesktopSize);

            textScreenAreaX.Text = Utils.GetNumberString(config.SelectedScreenArea.X - config.SelectedScreenArea.Width / 2.0);
            textScreenAreaY.Text = Utils.GetNumberString(config.SelectedScreenArea.Y - config.SelectedScreenArea.Height / 2.0);


            //
            // Force aspect ratio
            //
            if (config.ForceAspectRatio)
            {
                for (int i = 0; i < config.GetAreaCount(); i++)
                {
                    config.TabletAreas[i].Height = config.TabletAreas[i].Width / (config.ScreenAreas[i].Width / config.ScreenAreas[i].Height);
                }

                textTabletAreaHeight.Text = Utils.GetNumberString(config.SelectedTabletArea.Height);
                textTabletAreaHeight.IsEnabled = false;
            }


            //
            // Move tablet areas to valid positions
            //
            for (int i = 0; i < config.GetAreaCount(); i++)
                config.TabletAreas[i].MoveInside(config.TabletFullArea);

            textTabletAreaX.Text = Utils.GetNumberString(config.SelectedTabletArea.X);
            textTabletAreaY.Text = Utils.GetNumberString(config.SelectedTabletArea.Y);



            //
            // Pen buttons
            //
            if (config.ButtonMap.Count() == 3)
            {
                buttonPenButton1.Content = config.ButtonMap[0];
                buttonPenButton2.Content = config.ButtonMap[1];
                buttonPenButton3.Content = config.ButtonMap[2];
            }
            else
            {
                config.ButtonMap = new string[] { "MOUSE1", "MOUSE2", "MOUSE3" };
            }
            checkBoxDisableButtons.IsChecked = config.DisableButtons;


            //
            // Tablet buttons
            //
            if (config.TabletButtonMap.Count() == 16)
            {
                for (int i = 0; i < 16; i++)
                {
                    GroupBox box = (GroupBox)wrapPanelTabletButtons.Children[i];
                    Button button = (Button)(box.Content);
                    button.Content = config.TabletButtonMap[i];
                }
            }
            else
            {
                config.TabletButtonMap = new string[16];
                for (int i = 0; i < 16; i++) config.TabletButtonMap[i] = "";
            }
            if (wrapPanelTabletButtons.Children.Count == 17)
            {
                ((CheckBox)wrapPanelTabletButtons.Children[16]).IsChecked = config.DisableTabletButtons;
            }


            //
            // Pressure
            //
            sliderPressureSensitivity.Value = config.PressureSensitivity;
            sliderPressureDeadzoneLow.Value = config.PressureDeadzoneLow;
            sliderPressureDeadzoneHigh.Value = config.PressureDeadzoneHigh;


            //
            // Scroll
            //
            textScrollSensitivity.Text = Utils.GetNumberString(config.ScrollSensitivity);
            textScrollAcceleration.Text = Utils.GetNumberString(config.ScrollAcceleration);
            checkBoxScrollStopCursor.IsChecked = config.ScrollStopCursor;
            checkBoxScrollDrag.IsChecked = config.ScrollDrag;


            //
            // Smoothing filter
            //
            checkBoxSmoothing.IsChecked = config.SmoothingEnabled;
            textSmoothingLatency.Text = Utils.GetNumberString(config.SmoothingLatency);
            comboBoxSmoothingRate.SelectedIndex = config.SmoothingInterval - 1;
            checkBoxSmoothingOnlyWhenButtons.IsChecked = config.SmoothingOnlyWhenButtons;


            //
            // Noise filter
            //
            checkBoxNoiseFilter.IsChecked = config.NoiseFilterEnabled;
            textNoiseBuffer.Text = Utils.GetNumberString(config.NoiseFilterBuffer);
            textNoiseThreshold.Text = Utils.GetNumberString(config.NoiseFilterThreshold);

            //
            // Anti-smoothing filter
            //
            checkBoxAntiSmoothing.IsChecked = config.AntiSmoothingEnabled;
            checkBoxAntiSmoothingOnlyWhenHover.IsChecked = config.AntiSmoothingOnlyWhenHover;
            textAntiSmoothingDragMultiplier.Text = Utils.GetNumberString(config.AntiSmoothingDragMultiplier);
            for (int i = 0; i < config.AntiSmoothingSettings.Length; i++)
            {
                checkboxAntiSmoothingSettingEnabled[i].IsChecked = config.AntiSmoothingSettings[i].Enabled;
                textAntiSmoothingVelocity[i].Text = Utils.GetNumberString(config.AntiSmoothingSettings[i].Velocity);
                textAntiSmoothingShape[i].Text = Utils.GetNumberString(config.AntiSmoothingSettings[i].Shape, "0.000");
                textAntiSmoothingCompensation[i].Text = Utils.GetNumberString(config.AntiSmoothingSettings[i].Compensation);
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
            foreach (string command in config.CustomCommands)
            {
                if (command.Trim().Length > 0)
                    tmp += command.Trim() + "\n";
            }
            textCustomCommands.Text = tmp;

            //
            // Debugging
            //
            checkBoxDebugging.IsChecked = config.DebuggingEnabled;

            // Settings loaded
            isLoadingSettings = false;

            // Update canvases
            UpdateCanvasElements();

            // Update binding data context
            DataContext = null;
            DataContext = config;

        }


        //
        // Update settings to configuration
        //
        private void UpdateSettingsToConfiguration()
        {
            if (isLoadingSettings)
                return;

            bool oldValue;


            //
            // Tablet area
            //
            if (Utils.ParseNumber(textTabletAreaWidth.Text, out double value))
                config.SelectedTabletArea.Width = value;
            if (Utils.ParseNumber(textTabletAreaHeight.Text, out value))
                config.SelectedTabletArea.Height = value;
            if (Utils.ParseNumber(textTabletAreaX.Text, out value))
                config.SelectedTabletArea.X = value;
            if (Utils.ParseNumber(textTabletAreaY.Text, out value))
                config.SelectedTabletArea.Y = value;
            if (Utils.ParseNumber(textTabletAreaRotation.Text, out value))
                config.TabletAreas[0].Rotation = value;

            // Update secondary area rotations
            for (int i = 1; i < config.GetAreaCount(); i++)
            {
                config.TabletAreas[i].Rotation = config.TabletAreas[0].Rotation;
            }


            config.Invert = (bool)checkBoxInvert.IsChecked;
            config.ForceAspectRatio = (bool)checkBoxForceAspect.IsChecked;

            //
            // Output positioning and mode
            //
            if (comboBoxOutputPositioning.SelectedIndex >= 0)
                config.Positioning = (Configuration.OutputPositioning)comboBoxOutputPositioning.SelectedIndex;
            else
                config.Positioning = Configuration.OutputPositioning.Absolute;

            if (radioOutputModeStandard.IsChecked == true) config.Mode = Configuration.OutputModes.Standard;
            else if (radioOutputModeWindowsInk.IsChecked == true) config.Mode = Configuration.OutputModes.WindowsInk;
            else if (radioOutputModeCombatibility.IsChecked == true) config.Mode = Configuration.OutputModes.Compatibility;

            //
            // Force the tablet areas to be inside of the full area
            //
            for (int i = 0; i < config.GetAreaCount(); i++)
                config.TabletAreas[i].MoveInside(config.TabletFullArea);


            //
            // Screen area
            //
            if (Utils.ParseNumber(textScreenAreaWidth.Text, out value))
                config.SelectedScreenArea.Width = value;
            if (Utils.ParseNumber(textScreenAreaHeight.Text, out value))
                config.SelectedScreenArea.Height = value;
            if (Utils.ParseNumber(textScreenAreaX.Text, out value))
                config.SelectedScreenArea.X = value + config.SelectedScreenArea.Width / 2.0;
            if (Utils.ParseNumber(textScreenAreaY.Text, out value))
                config.SelectedScreenArea.Y = value + config.SelectedScreenArea.Height / 2.0;



            //
            // Desktop size
            //
            if (Utils.ParseNumber(textDesktopWidth.Text, out value))
                config.DesktopSize.Width = value;
            if (Utils.ParseNumber(textDesktopHeight.Text, out value))
                config.DesktopSize.Height = value;
            config.AutomaticDesktopSize = (bool)checkBoxAutomaticDesktopSize.IsChecked;
            if (config.AutomaticDesktopSize == true)
            {
                textDesktopWidth.Text = Utils.GetNumberString(GetVirtualDesktopSize().Width);
                textDesktopHeight.Text = Utils.GetNumberString(GetVirtualDesktopSize().Height);
                config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                config.DesktopSize.Height = GetVirtualDesktopSize().Height;
            }


            //
            // Force aspect ratio
            //
            if (config.ForceAspectRatio)
            {
                config.SelectedTabletArea.Height = config.SelectedTabletArea.Width / (config.SelectedScreenArea.Width / config.SelectedScreenArea.Height);
                textTabletAreaHeight.Text = Utils.GetNumberString(config.SelectedTabletArea.Height);
            }


            //
            // Button map 
            //
            config.ButtonMap[0] = buttonPenButton1.Content.ToString();
            config.ButtonMap[1] = buttonPenButton2.Content.ToString();
            config.ButtonMap[2] = buttonPenButton3.Content.ToString();
            config.DisableButtons = (bool)checkBoxDisableButtons.IsChecked;


            //
            // Tablet button map
            //
            for (int i = 0; i < 16; i++)
            {
                GroupBox box = (GroupBox)wrapPanelTabletButtons.Children[i];
                Button button = (Button)(box.Content);
                config.TabletButtonMap[i] = button.Content.ToString();
            }
            if (wrapPanelTabletButtons.Children.Count == 17)
            {
                config.DisableTabletButtons = (bool)(((CheckBox)wrapPanelTabletButtons.Children[16]).IsChecked);
            }


            //
            // Pressure sensitivity
            //
            config.PressureSensitivity = sliderPressureSensitivity.Value;
            config.PressureDeadzoneLow = sliderPressureDeadzoneLow.Value;
            config.PressureDeadzoneHigh = sliderPressureDeadzoneHigh.Value;


            //
            // Scroll
            //
            if (Utils.ParseNumber(textScrollSensitivity.Text, out value))
                config.ScrollSensitivity = value;
            if (Utils.ParseNumber(textScrollAcceleration.Text, out value))
                config.ScrollAcceleration = value;
            config.ScrollStopCursor = (bool)checkBoxScrollStopCursor.IsChecked;
            config.ScrollDrag = (bool)checkBoxScrollDrag.IsChecked;


            //
            // Smoothing filter
            //
            config.SmoothingEnabled = (bool)checkBoxSmoothing.IsChecked;
            config.SmoothingInterval = comboBoxSmoothingRate.SelectedIndex + 1;
            if (Utils.ParseNumber(textSmoothingLatency.Text, out value))
                config.SmoothingLatency = value;
            config.SmoothingOnlyWhenButtons = (bool)checkBoxSmoothingOnlyWhenButtons.IsChecked;


            //
            // Noise filter
            //
            config.NoiseFilterEnabled = (bool)checkBoxNoiseFilter.IsChecked;
            if (Utils.ParseNumber(textNoiseBuffer.Text, out value))
                config.NoiseFilterBuffer = (int)value;
            if (Utils.ParseNumber(textNoiseThreshold.Text, out value))
                config.NoiseFilterThreshold = value;


            //
            // Anti-smoothing filter
            //
            config.AntiSmoothingEnabled = (bool)checkBoxAntiSmoothing.IsChecked;
            config.AntiSmoothingOnlyWhenHover = (bool)checkBoxAntiSmoothingOnlyWhenHover.IsChecked;
            if (Utils.ParseNumber(textAntiSmoothingDragMultiplier.Text, out value))
                config.AntiSmoothingDragMultiplier = value;

            for (int i = 0; i < config.AntiSmoothingSettings.Length; i++)
            {
                config.AntiSmoothingSettings[i].Enabled = (bool)checkboxAntiSmoothingSettingEnabled[i].IsChecked;

                if (Utils.ParseNumber(textAntiSmoothingVelocity[i].Text, out value))
                    config.AntiSmoothingSettings[i].Velocity = value;
                if (Utils.ParseNumber(textAntiSmoothingShape[i].Text, out value))
                    config.AntiSmoothingSettings[i].Shape = value;
                if (Utils.ParseNumber(textAntiSmoothingCompensation[i].Text, out value))
                    config.AntiSmoothingSettings[i].Compensation = value;

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


            //
            // Custom commands
            //
            List<string> commandList = new List<string>();
            foreach (string command in textCustomCommands.Text.Split('\n'))
                if (command.Trim().Length > 0)
                    commandList.Add(command.Trim());
            config.CustomCommands = commandList.ToArray();


            //
            // Debugging
            //
            config.DebuggingEnabled = (bool)checkBoxDebugging.IsChecked;


            // Update canvases
            UpdateCanvasElements();

        }


        //
        // Initialize configuration
        //
        private void InitializeConfiguration()
        {
            isLoadingSettings = true;
            Width = config.WindowWidth;
            Height = config.WindowHeight;
            isLoadingSettings = false;

            //
            // Convert old configuration format area values
            //
            if (config.ScreenArea != null)
            {
                config.ScreenAreas[0].Width = config.ScreenArea.Width;
                config.ScreenAreas[0].Height = config.ScreenArea.Height;
                config.ScreenAreas[0].X = config.ScreenArea.X + config.ScreenArea.Width / 2.0;
                config.ScreenAreas[0].Y = config.ScreenArea.Y + config.ScreenArea.Height / 2.0;
                config.ScreenArea = null;
            }
            if (config.TabletArea != null)
            {
                config.TabletAreas[0].Set(config.TabletArea);
                config.TabletArea = null;
            }

            //
            // Conversion from config format V1
            //
            if (config.ConfigVersion <= 1)
            {

                //
                // Convert old button map
                //
                bool isOld = true;
                if (config.ButtonMap.Length == 3)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        if (config.ButtonMap[i] != "1" && config.ButtonMap[i] != "2" && config.ButtonMap[i] != "3")
                        {
                            isOld = false;
                        }
                    }
                    if (isOld)
                    {
                        for (int i = 0; i < 3; i++)
                            config.ButtonMap[i] = "MOUSE" + config.ButtonMap[i].ToString();
                    }
                }
            }

            // Fix screen area array length
            if (config.ScreenAreas.Length != config.GetMaxAreaCount())
            {
                Area[] newScreenAreas = new Area[config.GetMaxAreaCount()];
                for (int i = 0; i < config.GetMaxAreaCount(); i++)
                {
                    if (i < config.ScreenAreas.Length)
                    {
                        newScreenAreas[i] = config.ScreenAreas[i];
                    }
                    else
                    {
                        newScreenAreas[i] = new Area(0, 0, 0, 0);
                    }
                }
                config.ScreenAreas = newScreenAreas;
            }

            // Fix tablet area array length
            if (config.TabletAreas.Length != config.GetMaxAreaCount())
            {
                Area[] newTabletAreas = new Area[config.GetMaxAreaCount()];
                for (int i = 0; i < config.GetMaxAreaCount(); i++)
                {
                    if (i < config.TabletAreas.Length)
                    {
                        newTabletAreas[i] = config.TabletAreas[i];
                    }
                    else
                    {
                        newTabletAreas[i] = new Area(0, 0, 0, 0);
                    }
                }
                config.TabletAreas = newTabletAreas;

            }


            // Invalid screen area -> Set defaults
            for (int i = 0; i < config.GetAreaCount(); i++)
            {
                if (config.ScreenAreas[i].Width == 0 || config.ScreenAreas[i].Height == 0)
                {
                    config.DesktopSize.Width = GetVirtualDesktopSize().Width;
                    config.DesktopSize.Height = GetVirtualDesktopSize().Height;
                    config.ScreenAreas[i].Width = config.DesktopSize.Width;
                    config.ScreenAreas[i].Height = config.DesktopSize.Height;
                    config.ScreenAreas[i].X = config.DesktopSize.Width / 2.0;
                    config.ScreenAreas[i].Y = config.DesktopSize.Height / 2.0;
                }

                FixTabletAreaDimensions(config.TabletAreas[i], config.ScreenAreas[i]);
            }


            // Primary area is always enabled
            config.ScreenAreas[0].IsEnabled = true;

            // Set selected area to primary area
            config.SelectedScreenArea = config.ScreenAreas[0];
            config.SelectedTabletArea = config.TabletAreas[0];

            // Reset tablet name
            config.TabletName = "";

            // Load settings from configuration
            LoadSettingsFromConfiguration();

            // Update the settings back to the configuration
            UpdateSettingsToConfiguration();

            // Set run at startup
            SetRunAtStartup(config.RunAtStartup);

            // Set window data context
            this.DataContext = config;

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

                //
                // Enable/Disable Windows Ink pressure settings
                //
                if (config.Mode == Configuration.OutputModes.WindowsInk)
                    groupBoxWindowsInkSettings.IsEnabled = true;
                else
                    groupBoxWindowsInkSettings.IsEnabled = false;

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

            // Disable tablet area height when aspect ratio is forced
            if (checkBoxForceAspect.IsChecked == true)
                textTabletAreaHeight.IsEnabled = false;
            else
                textTabletAreaHeight.IsEnabled = true;


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

            // Debugging checkbox
            if (sender == checkBoxDebugging)
            {
                if (checkBoxDebugging.IsChecked == true)
                {
                    driver.SendCommand("Debug true");
                }
                else
                {
                    driver.SendCommand("Debug false");
                }
            }

            UpdateSettingsToConfiguration();

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
            UpdateCanvasElements();
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
            if (sender != comboBoxMonitor) return;
            if (e.AddedItems.Count <= 0) return;

            System.Windows.Forms.Screen[] screens = GetAvailableScreens();
            Vector minimumScreenPosition = GetMinimumScreenPosition(screens);

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

                // Monitors
                if (index >= 0 && index < screens.Length && screens.Length > 1)
                {
                    textScreenAreaX.Text = Utils.GetNumberString(screens[index].Bounds.X - minimumScreenPosition.X);
                    textScreenAreaY.Text = Utils.GetNumberString(screens[index].Bounds.Y - minimumScreenPosition.Y);
                    textScreenAreaWidth.Text = Utils.GetNumberString(screens[index].Bounds.Width);
                    textScreenAreaHeight.Text = Utils.GetNumberString(screens[index].Bounds.Height);
                }

            }
            comboBoxMonitor.Text = "";
            comboBoxMonitor.SelectedIndex = -1;
            UpdateSettingsToConfiguration();
        }


        //
        // Button mapping click
        //
        private void ButtonMap_Click(object sender, RoutedEventArgs e)
        {
            Button button = (Button)sender;
            //MessageBox.Show(button.Content.ToString());

            bool isPenButton = false;

            if (sender == buttonPenButton1) isPenButton = true;
            else if (sender == buttonPenButton2) isPenButton = true;
            else if (sender == buttonPenButton3) isPenButton = true;


            WindowButtonMapping windowButtonMapping = new WindowButtonMapping(button, isPenButton);
            windowButtonMapping.ShowDialog();
            if (windowButtonMapping.DialogResult == true)
            {
                button.Content = windowButtonMapping.Result.ToUpper();
                UpdateSettingsToConfiguration();
            }

        }


        //
        // Button map tooltip opening
        //
        private void ButtonMap_ToolTipOpening(object sender, ToolTipEventArgs e)
        {
            Button button = (Button)sender;
            if (button.Content.ToString() == "")
            {
                button.ToolTip = "Empty";
            }
            else
            {
                button.ToolTip = button.Content;
            }
        }


        //
        // Main Menu Click
        //
        private void MainMenuClick(object sender, RoutedEventArgs e)
        {

            //
            // Save settings
            //
            if (sender == mainMenuSaveSettings)
            {
                SaveSettings(sender, e);
            }

            //
            // Import settings
            //
            else if (sender == mainMenuImport)
            {
                OpenFileDialog dialog = new OpenFileDialog
                {
                    InitialDirectory = Directory.GetCurrentDirectory(),
                    Filter = "XML File|*.xml",
                    Title = "Import settings"
                };
                if (dialog.ShowDialog() == true)
                {
                    try
                    {
                        Configuration tmpConfig = Configuration.CreateFromFile(dialog.FileName);
                        config = tmpConfig;
                        InitializeConfiguration();
                        Application.Current.Dispatcher.Invoke(() =>
                        {
                            LoadSettingsFromConfiguration();
                            UpdateSettingsToConfiguration();
                        });
                        SetStatus("Settings imported!");

                    }
                    catch (Exception)
                    {
                        MessageBox.Show("Settings import failed!", "ERROR!",
                            MessageBoxButton.OK, MessageBoxImage.Error);
                    }
                }
            }

            //
            // Export settings
            //
            else if (sender == mainMenuExport)
            {
                SaveFileDialog dialog = new SaveFileDialog
                {
                    InitialDirectory = Directory.GetCurrentDirectory(),
                    AddExtension = true,
                    DefaultExt = "xml",
                    Filter = "XML File|*.xml",
                    Title = "Export settings"

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

            //
            // Reset to default
            //
            else if (sender == mainMenuResetToDefault)
            {
                WindowMessageBox messageBox = new WindowMessageBox(
                              "Are you sure?", "Reset to default settings?",
                              "Yes", "No");
                messageBox.ShowDialog();
                if (messageBox.DialogResult == true)
                {

                    config = null;
                    isFirstStart = true;
                    config = new Configuration();

                    // Initialize configuration
                    InitializeConfiguration();

                    // Restart driver
                    StopDriver();
                    StartDriver();
                }

            }

            //
            // Exit GUI only
            //
            else if (sender == mainMenuExitGUI)
            {
                driver.DoNotKill = true;
                Close();
            }



            //
            // Exit
            //
            else if (sender == mainMenuExit)
            {
                Close();
            }



            //
            // Open current directory
            //
            else if (sender == mainMenuOpenCurrentDirectory)
            {
                try
                {
                    Process.Start("explorer.exe", ".");
                }
                catch (Exception)
                {
                    MessageBox.Show("Couldn't open TabletDriver folder!", "Error!", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }


            //
            // Open config folder
            //
            else if (sender == mainMenuOpenConfig)
            {
                try
                {
                    Process.Start("explorer.exe", "config");
                }
                catch (Exception)
                {
                    MessageBox.Show("Couldn't open config folder!", "Error!", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }

            //
            // Open tools folder
            //
            else if (sender == mainMenuOpenTools)
            {
                try
                {
                    Process.Start("explorer.exe", "tools");
                }
                catch (Exception)
                {
                    MessageBox.Show("Couldn't open tools folder!", "Error!", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }


            //
            // Tablet View
            //
            else if (sender == mainMenuTabletView)
            {
                if (tabletView != null)
                    tabletView.Close();

                tabletView = new WindowTabletView(config, driver)
                {
                    WindowStartupLocation = WindowStartupLocation.CenterOwner,
                    Owner = Application.Current.MainWindow,
                };
                tabletView.ShowDialog();
                tabletView.Close();
                tabletView = null;
                GC.Collect();
                GC.WaitForPendingFinalizers();

            }


            //
            // Tablet View Settings
            //
            else if (sender == mainMenuTabletViewSettings)
            {

                WindowTabletViewSettings tabletViewSettings = new WindowTabletViewSettings(config)
                {
                    WindowStartupLocation = WindowStartupLocation.CenterOwner,
                    Owner = Application.Current.MainWindow,
                };
                tabletViewSettings.ShowDialog();
            }


            //
            // Update desktop image
            //
            else if (sender == mainMenuUpdateDesktopImage)
            {
                DispatcherTimer timer = new DispatcherTimer
                {
                    Interval = new TimeSpan(0, 0, 0, 0, 200)
                };
                timer.Tick += (s, ev) =>
                {
                    imageDesktopScreenshot.Source = null;
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                    UpdateDesktopImage();
                    timer.Stop();
                };
                timer.Start();
            }


            //
            // Fit window to content
            //
            else if (sender == mainMenuFitToContent)
            {
                SizeToContent = SizeToContent.WidthAndHeight;
                UpdateLayout();
                SizeToContent = SizeToContent.Manual;
            }

        }


        //
        // Tab selection changed
        //
        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            //
            // Update canvas elements when changed to area tab
            //
            if (tabControl.SelectedItem == tabArea)
            {
                if (IsLoaded)
                {
                    DispatcherTimer timer = new DispatcherTimer
                    {
                        Interval = new TimeSpan(0, 0, 0, 0, 100)
                    };
                    timer.Tick += (snd, ev) =>
                    {
                        UpdateCanvasElements();
                        timer.Stop();
                    };
                    timer.Start();
                }
            }

            //
            // Console tab is selected
            //
            if (tabControl.SelectedItem == tabConsole)
            {
                ConsoleBufferToText();
            }
        }

    }
}
