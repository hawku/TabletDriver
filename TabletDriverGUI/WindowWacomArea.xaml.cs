using Microsoft.Win32;
using System;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for WacomArea.xaml
    /// </summary>
    public partial class WindowWacomArea : Window
    {
        private Configuration config;
        private Area tabletArea;

        public WindowWacomArea(Configuration configuration, Area TabletArea)
        {
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            Owner = Application.Current.MainWindow;
            InitializeComponent();

            config = configuration;
            tabletArea = TabletArea;
            LoadValues();

        }

        private void LoadValues()
        {
            textWacomLeft.Text = Utils.GetNumberString((tabletArea.X - tabletArea.Width / 2) * 100.0, "0");
            textWacomRight.Text = Utils.GetNumberString((tabletArea.X + tabletArea.Width / 2) * 100.0, "0");

            textWacomTop.Text = Utils.GetNumberString((tabletArea.Y - tabletArea.Height / 2) * 100.0, "0");
            textWacomBottom.Text = Utils.GetNumberString((tabletArea.Y + tabletArea.Height / 2) * 100.0, "0");
        }

        private void StoreValues()
        {
            if (
                Utils.ParseNumber(textWacomLeft.Text, out double left) &&
                Utils.ParseNumber(textWacomRight.Text, out double right) &&
                Utils.ParseNumber(textWacomTop.Text, out double top) &&
                Utils.ParseNumber(textWacomBottom.Text, out double bottom)
            )
            {
                double width, height;
                width = right - left;
                height = bottom - top;
                config.ForceAspectRatio = false;
                tabletArea.X = (left + width / 2.0) / 100.0;
                tabletArea.Y = (top + height / 2.0) / 100.0;
                tabletArea.Width = width / 100.0;
                tabletArea.Height = height / 100.0;
            }
            else
            {
                MessageBox.Show("Invalid values!", "Wacom area error!", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        //
        // Set button
        //
        private void ButtonSet_Click(object sender, RoutedEventArgs e)
        {
            StoreValues();
            DialogResult = true;
        }

        //
        // Cancel button
        //
        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        //
        // Force proportions
        //
        private void ButtonForceProportions_Click(object sender, RoutedEventArgs e)
        {
            if (Utils.ParseNumber(textWacomLeft.Text, out double left) &&
                Utils.ParseNumber(textWacomRight.Text, out double right) &&
                Utils.ParseNumber(textWacomTop.Text, out double top) &&
                Utils.ParseNumber(textWacomBottom.Text, out double bottom)
            )
            {
                double width, height;
                width = right - left;
                height = bottom - top;

                double screenAspectRatio = config.SelectedScreenArea.Width / config.SelectedScreenArea.Height;
                double tabletAspectRatio = width / height;

                // Tablet aspect ratio higher -> change width
                if (tabletAspectRatio >= screenAspectRatio)
                {
                    width = height * screenAspectRatio;

                }

                // Tablet aspect ratio lower -> change  height
                else
                {
                    height = width / screenAspectRatio;

                }

                right = left + width;
                bottom = top + height;

                textWacomRight.Text = Utils.GetNumberString(right, "0");
                textWacomBottom.Text = Utils.GetNumberString(bottom, "0");
            }
            else
            {
                MessageBox.Show("Invalid values!", "Wacom area error!", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        //
        // Load button
        //
        private void ButtonLoad_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog
            {
                InitialDirectory = Directory.GetCurrentDirectory(),
                Filter = "Wacom Backup(*.wacomprefs;*.tabletprefs)|*.wacomprefs;*.tabletprefs"
            };
            if (dialog.ShowDialog() == true)
            {
                LoadFromBackup(dialog.FileName);
            }
        }

        //
        // Load setting from Wacom backup
        //
        private void LoadFromBackup(string filepath)
        {
            try
            {
                string data = File.ReadAllText(filepath);
                data = data.Replace("&lt;", "<");
                data = data.Replace("&gt;", ">");

                double[] areaValues = new double[4] { 0, 0, 0, 0 };
                double[] lastAreaValues = new double[4];
                bool first = true;

                /*
                <TabletInputArea type="map">
                    <Extent type="map">
                        <X type="integer">15200</X>
                        <Y type="integer">9500</Y>
                        <Z type="integer">0</Z>
                    </Extent>
                    <Origin type="map">
                        <X type="integer">0</X>
                        <Y type="integer">0</Y>
                        <Z type="integer">0</Z>
                    </Origin>
                </TabletInputArea>
                */

                //
                // Regular expression
                //
                MatchCollection matches = Regex.Matches(
                    data,
                    "<TabletInputArea[^>]*>.*?" +
                    "<Extent.*?" +
                    "<X type=\"integer\">([0-9]+)</X>.*?" +
                    "<Y type=\"integer\">([0-9]+)</Y>.*?" +
                    "<Origin.*?" +
                    "<X type=\"integer\">([0-9]+)</X>.*?" +
                    "<Y type=\"integer\">([0-9]+)</Y>.*?" +
                    "</TabletInputArea>",
                    RegexOptions.Singleline | RegexOptions.IgnoreCase
                );

                //
                // Loop through regex matches
                //
                foreach (Match match in matches)
                {
                    if (
                        Utils.ParseNumber(match.Groups[1].ToString(), out areaValues[0]) &&
                        Utils.ParseNumber(match.Groups[2].ToString(), out areaValues[1]) &&
                        Utils.ParseNumber(match.Groups[3].ToString(), out areaValues[2]) &&
                        Utils.ParseNumber(match.Groups[4].ToString(), out areaValues[3])
                    )
                    {
                        // Stop at first different area value set
                        if (!first && !areaValues.SequenceEqual(lastAreaValues))
                            break;

                        Array.Copy(areaValues, lastAreaValues, 4);
                        first = false;
                    }
                }

                // Set text fields
                if (areaValues[0] != 0 && areaValues[1] != 0)
                {
                    textWacomLeft.Text = Utils.GetNumberString(areaValues[2]);
                    textWacomRight.Text = Utils.GetNumberString(areaValues[2] + areaValues[0]);
                    textWacomTop.Text = Utils.GetNumberString(areaValues[3]);
                    textWacomBottom.Text = Utils.GetNumberString(areaValues[3] + areaValues[1]);
                }

                // Show error
                else
                {
                    MessageBox.Show("Couldn't read the backup file!", "ERROR!",
                        MessageBoxButton.OK, MessageBoxImage.Error);
                }

            }

            // Exception
            catch (Exception)
            {
                MessageBox.Show("Couldn't read the backup file!", "ERROR!",
                    MessageBoxButton.OK, MessageBoxImage.Error);
            }

        }


    }
}
