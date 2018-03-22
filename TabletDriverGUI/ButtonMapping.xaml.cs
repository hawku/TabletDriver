using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
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
        public Dictionary<int, int[]> MacroButtonMap;

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
            MacroButtonMap = new Dictionary<int, int[]>();

            if (tabletName != null && NbTabletButtonsMap.ContainsKey(tabletName))
            {
                InitializeComponent();

                PenTipComboBox.DropDownClosed += new EventHandler(PromptShortcutWindowEvent);
                PenBottomComboBox.DropDownClosed += new EventHandler(PromptShortcutWindowEvent);
                PenTopComboBox.DropDownClosed += new EventHandler(PromptShortcutWindowEvent);

                for (var i = 0; i < NbTabletButtonsMap[tabletName]; ++i)
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
                PenTipComboBox.SelectedIndex = config.ButtonMap[0];
                PenBottomComboBox.SelectedIndex = config.ButtonMap[1];
                PenTopComboBox.SelectedIndex = config.ButtonMap[2];

                for (var i = 0; i < NbTabletButtonsMap[tabletName]; ++i)
                {
                    for (var j = 0; j < TabletButtonsContainer.Children.Count && j + 3 < config.ButtonMap.Length; ++j)
                    {
                        var gb = TabletButtonsContainer.Children[j] as GroupBox;
                        var cb = gb.Content as ComboBox;

                        cb.SelectedIndex = config.ButtonMap[3 + j];
                    }
                }

                CheckBoxDisablePenButtons.IsChecked = config.DisablePenButtons;
                CheckBoxDisableTabletButtons.IsChecked = config.DisableTabletButtons;
            }
            else
            {
                throw new TabletNotRecognizedException("Tablet not recognized");
            }
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

        private enum MapType : uint
        {
            MAPVK_VK_TO_VSC = 0x0,
            MAPVK_VSC_TO_VK = 0x1,
            MAPVK_VK_TO_CHAR = 0x2,
            MAPVK_VSC_TO_VK_EX = 0x3,
        }

        [DllImport("user32.dll")]
        private static extern int ToUnicode(
            uint wVirtKey,
            uint wScanCode,
            byte[] lpKeyState,
            [Out, MarshalAs(UnmanagedType.LPWStr, SizeParamIndex = 4)]
            StringBuilder pwszBuff,
            int cchBuff,
            uint wFlags);

        [DllImport("user32.dll")]
        private static extern bool GetKeyboardState(byte[] lpKeyState);

        [DllImport("user32.dll")]
        private static extern uint MapVirtualKey(uint uCode, MapType uMapType);

        private char GetVKFromModifierKey(ModifierKeys modifierKey)
        {
            char ch = '\0';

            switch (modifierKey)
            {
                case ModifierKeys.Control:
                    ch = (char)0x11;
                    break;
                case ModifierKeys.Shift:
                    ch = (char)0x10;
                    break;
                case ModifierKeys.Alt:
                    ch = (char)0x12;
                    break;
                case ModifierKeys.Windows:
                    ch = (char)0x5B;
                    break;
            }
            return ch;
        }

        private char GetCharFromKey(Key key)
        {
            char ch = '\0';

            int virtualKey = KeyInterop.VirtualKeyFromKey(key);
            byte[] keyboardState = new byte[256];
            GetKeyboardState(keyboardState);

            uint scanCode = MapVirtualKey((uint)virtualKey, MapType.MAPVK_VK_TO_VSC);
            StringBuilder stringBuilder = new StringBuilder(2);

            int result = ToUnicode((uint)virtualKey, scanCode, keyboardState, stringBuilder, stringBuilder.Capacity, 0);
            switch (result)
            {
                case -1:
                    break;
                case 0:
                    break;
                case 1:
                    {
                        ch = stringBuilder[0];
                        break;
                    }
                default:
                    {
                        ch = stringBuilder[0];
                        break;
                    }
            }
            return ch;
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
                    int idx = -1;
                    if (cb.Name == "PenTipComboBox")
                        idx = 0;
                    else if (cb.Name == "PenBottomComboBox")
                        idx = 1;
                    else if (cb.Name == "PenTopComboBox")
                        idx = 2;
                    else
                        idx = Int32.Parse(cb.Name.Remove(0, 12)) + 2;
                    List<int> l = new List<int>();

                    for (var i = 0; i < shortcutMapWindow.ModifierKey.Count; ++i)
                        l.Add(GetVKFromModifierKey(shortcutMapWindow.ModifierKey[i]));
                    l.Add(GetCharFromKey(shortcutMapWindow.PressedKey));
                    if (shortcutMapWindow.PressedKey == Key.None)
                        cb.SelectedIndex = (int)ButtonActionEnum.DISABLED;
                    MacroButtonMap[idx] = l.ToArray();
                }
                else
                    cb.SelectedIndex = (int)ButtonActionEnum.DISABLED;

                shortcutMapWindow.Close();
            }
        }
    }
}
