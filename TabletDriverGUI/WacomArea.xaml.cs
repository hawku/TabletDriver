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
    public partial class WacomArea : Window
    {
        public WacomArea()
        {
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            InitializeComponent();
        }

        private void ButtonSet_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

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
