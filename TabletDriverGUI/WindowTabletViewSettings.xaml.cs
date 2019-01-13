using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for TabletViewSettings.xaml
    /// </summary>
    public partial class WindowTabletViewSettings : Window
    {
        Configuration config;

        List<Configuration.Preset> presets;

        public WindowTabletViewSettings(Configuration config)
        {
            InitializeComponent();
            this.config = config;

            presets = new List<Configuration.Preset>
            {
                new Configuration.Preset("Default", (c) =>
                {
                }),

                new Configuration.Preset("OBS Overlay", (c) =>
                {
                    Configuration.TabletViewSettings settings = c.TabletView;
                    settings.BackgroundColor = "#000000";
                    settings.InfoColor = "#3333FF";
                    settings.InputColor = "#33FF33";
                    settings.OutputColor = "#FF3333";
                    settings.LatencyColor = "#FFFF33";
                    settings.DrawColor = "#FFFFFF";
                }),

                new Configuration.Preset("OBS Overlay 2", (c) =>
                {
                    Configuration.TabletViewSettings settings = c.TabletView;
                    settings.BackgroundColor = "#000000";
                    settings.InfoColor  = Utils.RGBToHexColor(50,100,255);
                    settings.InputColor = Utils.RGBToHexColor(190,255,0);
                    settings.OutputColor = Utils.RGBToHexColor(255,120,0);
                    settings.LatencyColor = Utils.RGBToHexColor(230,220,0);
                    settings.DrawColor = "#6666AA";
                    settings.InputTrailLength = 100;
                    settings.OutputTrailLength = 100;
                    settings.DrawLength = 0;
                    settings.Font = "Exo 2";
                    settings.FontSize = 25;
                    settings.OffsetText.X = 0;
                    settings.OffsetText.Y = 10;
                    settings.OffsetPressure.X = -15;
                    settings.OffsetPressure.Y = 12;
                    settings.FadeInOut = true;
                }),

                new Configuration.Preset("OBS Input Cursor Only", (c) =>
                {
                    Configuration.TabletViewSettings settings = c.TabletView;
                    settings.BackgroundColor = "#000000";
                    settings.InputColor = "#33FF33";
                    settings.OutputColor = "transparent";
                    settings.InputTrailLength = 100;
                    settings.OutputTrailLength = 0;
                    settings.OffsetText = new Point(0, -200);
                    settings.OffsetPressure = new Point(0, -200);
                    settings.FadeInOut = true;
                }),

                new Configuration.Preset("OBS Output Cursor Only", (c) =>
                {
                    Configuration.TabletViewSettings settings = c.TabletView;
                    settings.BackgroundColor = "#000000";
                    settings.InputColor = "transparent";
                    settings.OutputColor = "#FF3333";
                    settings.InputTrailLength = 0;
                    settings.OutputTrailLength = 100;
                    settings.OffsetText = new Point(0, -200);
                    settings.OffsetPressure = new Point(0, -200);
                    settings.FadeInOut = true;
                }),


            };

            // Fill combobox
            foreach (var preset in presets)
            {
                comboBoxPresets.Items.Add(preset);
            }
            comboBoxPresets.Focus();

            LoadValues(config);

            KeyDown += WindowTabletViewSettings_KeyDown;
        }

        private void WindowTabletViewSettings_KeyDown(object sender, KeyEventArgs e)
        {
            // Esc -> Cancel
            if (e.Key == Key.Escape)
            {
                ButtonCancel_Click(sender, null);
            }

        }

        public void LoadValues(Configuration config)
        {
            textBackgroundColor.Text = config.TabletView.BackgroundColor;
            textInfoColor.Text = config.TabletView.InfoColor;
            textInputColor.Text = config.TabletView.InputColor;
            textOutputColor.Text = config.TabletView.OutputColor;
            textLatencyColor.Text = config.TabletView.LatencyColor;
            textDrawColor.Text = config.TabletView.DrawColor;
            textInputTrailLength.Text = Utils.GetNumberString(config.TabletView.InputTrailLength);
            textOutputTrailLength.Text = Utils.GetNumberString(config.TabletView.OutputTrailLength);
            textDrawLength.Text = Utils.GetNumberString(config.TabletView.DrawLength);
            textFont.Text = config.TabletView.Font;
            textFontSize.Text = Utils.GetNumberString(config.TabletView.FontSize);
            textOffsetTextX.Text = Utils.GetNumberString(config.TabletView.OffsetText.X);
            textOffsetTextY.Text = Utils.GetNumberString(config.TabletView.OffsetText.Y);
            textOffsetPressureX.Text = Utils.GetNumberString(config.TabletView.OffsetPressure.X);
            textOffsetPressureY.Text = Utils.GetNumberString(config.TabletView.OffsetPressure.Y);
            checkBoxFadeInOut.IsChecked = config.TabletView.FadeInOut;
            checkBoxBorderless.IsChecked = config.TabletView.Borderless;
        }

        public void StoreValues()
        {

            config.TabletView.BackgroundColor = textBackgroundColor.Text;
            config.TabletView.InfoColor = textInfoColor.Text;
            config.TabletView.InputColor = textInputColor.Text;
            config.TabletView.OutputColor = textOutputColor.Text;
            config.TabletView.LatencyColor = textLatencyColor.Text;
            config.TabletView.DrawColor = textDrawColor.Text;
            config.TabletView.Font = textFont.Text;

            if (Utils.ParseNumber(textInputTrailLength.Text, out double value))
                config.TabletView.InputTrailLength = (int)value;
            if (config.TabletView.InputTrailLength < 0)
                config.TabletView.InputTrailLength = 0;

            if (Utils.ParseNumber(textOutputTrailLength.Text, out value))
                config.TabletView.OutputTrailLength = (int)value;
            if (config.TabletView.OutputTrailLength < 0)
                config.TabletView.OutputTrailLength = 0;

            if (Utils.ParseNumber(textDrawLength.Text, out value))
                config.TabletView.DrawLength = (int)value;
            if (config.TabletView.DrawLength < 0)
                config.TabletView.DrawLength = 0;

            if (Utils.ParseNumber(textFontSize.Text, out value))
                config.TabletView.FontSize = value;
            if (Utils.ParseNumber(textOffsetTextX.Text, out value))
                config.TabletView.OffsetText.X = value;
            if (Utils.ParseNumber(textOffsetTextY.Text, out value))
                config.TabletView.OffsetText.Y = value;
            if (Utils.ParseNumber(textOffsetPressureX.Text, out value))
                config.TabletView.OffsetPressure.X = value;
            if (Utils.ParseNumber(textOffsetPressureY.Text, out value))
                config.TabletView.OffsetPressure.Y = value;
            config.TabletView.FadeInOut = (bool)checkBoxFadeInOut.IsChecked;
            config.TabletView.Borderless = (bool)checkBoxBorderless.IsChecked;

        }

        private void ButtonSave_Click(object sender, RoutedEventArgs e)
        {
            StoreValues();
            DialogResult = true;
            Close();
        }

        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        private void ComboBoxPresets_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (comboBoxPresets.SelectedItem != null)
            {
                Configuration tmpConfig = new Configuration();
                Configuration.Preset preset = (Configuration.Preset)comboBoxPresets.SelectedItem;
                preset.Action(tmpConfig);
                LoadValues(tmpConfig);
            }
        }

        private void ComboBoxPresets_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                ButtonSave_Click(sender, null);
            }
        }
    }
}
