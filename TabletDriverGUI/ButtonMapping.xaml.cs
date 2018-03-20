using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace TabletDriverGUI
{
    public enum ButtonActionEnum : uint
    {
        DISABLED = 0,
        MOUSE1 = 1,
        MOUSE2 = 2,
        MOUSE3 = 3,
        MOUSE4 = 4,
        MOUSE5 = 5,
        SHORTCUT = 6
    };

    /// <summary>
    /// Interaction logic for ButtonMapping.xaml
    /// </summary>
    public partial class ButtonMapping : Window
    {
        private Dictionary<string, uint> NbTabletButtonsMap;

        public ButtonMapping(Configuration config, string tabletName)
        {
            NbTabletButtonsMap = new Dictionary<string, uint>()
            {
                { "Wacom CTL-470", 0 },
                { "Wacom CTL-471", 0 },
                { "Wacom CTL-472", 0 },
                { "Wacom CTL-480", 4 },
                { "Wacom CTH-480", 4 },
                { "Wacom CTL-490", 4 },
                { "XP Pen G430", 0 },
                { "XP Pen G640", 0 },
                { "Huion 420", 0 },
                { "Huion H640P", 6 },
                { "Gaomon S56K", 0 },
            };

            //if (tabletName != null && NbTabletButtonsMap.ContainsKey(tabletName))
            //{
                InitializeComponent();
                //for (var i = 0; i < NbTabletButtonsMap[tabletName]; ++i)
                for (var i = 0; i < 4; ++i)
                {
                    ComboBox cb = new ComboBox
                    {
                        Name = "TabletButton" + (i + 1).ToString()
                    };
                    cb.DropDownClosed += new EventHandler(PromptShortcutWindowEvent);

                    GroupBox gb = new GroupBox
                    {
                        Header = "Tablet button " + (i + 1).ToString(),
                        Width = 110,
                        Margin = new Thickness(10, 0, 0, 0),
                        Content = cb
                    };
                    TabletButtonsContainer.Children.Add(gb);
                }

                // Create button map combobox items
                var tabletButtons = TabletButtonsContainer.Children;

                PenTipComboBox.Items.Add("Disabled");
                PenBottomComboBox.Items.Add("Disabled");
                PenTopComboBox.Items.Add("Disabled");
                for (var j = 0; j < tabletButtons.Count; ++j)
                {
                    var gb = tabletButtons[j] as GroupBox;
                    var cb = gb.Content as ComboBox;
                    cb.Items.Add("Disabled");
                    cb.SelectedIndex = 0;
                }
                for (int i = 1; i <= 5; ++i)
                {
                    PenTipComboBox.Items.Add("Mouse " + i);
                    PenBottomComboBox.Items.Add("Mouse " + i);
                    PenTopComboBox.Items.Add("Mouse " + i);
                    for (var j = 0; j < tabletButtons.Count; ++j)
                    {
                        var gb = tabletButtons[j] as GroupBox;
                        var cb = gb.Content as ComboBox;
                        cb.Items.Add("Mouse " + i);
                    }
                }
                PenTipComboBox.Items.Add("Shortcut");
                PenBottomComboBox.Items.Add("Shortcut");
                PenTopComboBox.Items.Add("Shortcut");
                for (var i = 0; i < tabletButtons.Count; ++i)
                {
                    var gb = tabletButtons[i] as GroupBox;
                    var cb = gb.Content as ComboBox;
                    cb.Items.Add("Shortcut");
                }

                PenTipComboBox.SelectedIndex = 0;
                PenBottomComboBox.SelectedIndex = 0;
                PenTopComboBox.SelectedIndex = 0;


                //
                // Buttons
                //
                if (config.ButtonMap.Count() == 3)
                {
                    PenTipComboBox.SelectedIndex = config.ButtonMap[0];
                    PenBottomComboBox.SelectedIndex = config.ButtonMap[1];
                    PenTopComboBox.SelectedIndex = config.ButtonMap[2];
                }
                else
                    config.ButtonMap = new int[] { 1, 2, 3 };

                CheckBoxDisablePenButtons.IsChecked = config.DisablePenButtons;
                CheckBoxDisableTabletButtons.IsChecked = config.DisableTabletButtons;
            //}
            //else
            //{
            //    throw new TabletNotRecognizedException("Tablet not recognized");
            //}
        }

        private void ButtonSet_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        private void CheckBoxDisablePenButtons_Checked(object sender, RoutedEventArgs e)
        {
            PenButtonsContainer.IsEnabled = false;
        }

        private void CheckBoxDisablePenButtons_Unchecked(object sender, RoutedEventArgs e)
        {
            PenButtonsContainer.IsEnabled = true;
        }

        private void CheckBoxDisableTabletButtons_Checked(object sender, RoutedEventArgs e)
        {
            TabletButtonsContainer.IsEnabled = false;
        }

        private void CheckBoxDisableTabletButtons_Unchecked(object sender, RoutedEventArgs e)
        {
            TabletButtonsContainer.IsEnabled = true;
        }

        private void PromptShortcutWindowEvent(object sender, EventArgs e)
        {
            PromptShortcutWindow(sender);
        }

        private void PromptShortcutWindow(object sender)
        {
            ComboBox cb = sender as ComboBox;

            if (cb.SelectedIndex == (uint)ButtonActionEnum.SHORTCUT)
            {
                ShortcutMapWindow shortcutMapWindow = new ShortcutMapWindow();

                shortcutMapWindow.ShowDialog();

                if (shortcutMapWindow.DialogResult == true)
                {
                    Console.WriteLine(shortcutMapWindow.PressedKey.ToString());
                    for (var i = 0; i < shortcutMapWindow.ModifierKey.Count; ++i)
                    {
                        Console.WriteLine(shortcutMapWindow.ModifierKey[i].ToString());
                    }
                }

                shortcutMapWindow.Close();
            }
        }
    }
}
