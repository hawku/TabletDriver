using System;
using System.Collections.Generic;
using System.Globalization;
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
    /// <summary>
    /// Interaction logic for ButtonMapping.xaml
    /// </summary>
    public partial class ButtonMapping : Window
    {
        // WPF Button
        Button button;

        public string Result;

        public ButtonMapping()
        {
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            Owner = Application.Current.MainWindow;

            InitializeComponent();
            Result = "";
        }

        public ButtonMapping(Button button, bool isPenButton) : this()
        {
            this.button = button;

            // Tablet buttons don't need tip, barrel and eraser info
            if (!isPenButton)
            {
                ((ComboBoxItem)comboBoxMouse.Items[1]).Content = "Mouse 1 (Left)";
                ((ComboBoxItem)comboBoxMouse.Items[2]).Content = "Mouse 2 (Right)";
                ((ComboBoxItem)comboBoxMouse.Items[3]).Content = "Mouse 3 (Middle)";

                // Disable scroll
                ((ComboBoxItem)comboBoxMouse.Items[6]).Visibility = Visibility.Collapsed;
                ((ComboBoxItem)comboBoxMouse.Items[7]).Visibility = Visibility.Collapsed;
                ((ComboBoxItem)comboBoxMouse.Items[8]).Visibility = Visibility.Collapsed;
            }

            // Enable scroll
            else
            {
                ((ComboBoxItem)comboBoxMouse.Items[6]).Visibility = Visibility.Visible;
                ((ComboBoxItem)comboBoxMouse.Items[7]).Visibility = Visibility.Visible;
                ((ComboBoxItem)comboBoxMouse.Items[8]).Visibility = Visibility.Visible;
            }
            CheckButtonValue();
        }

        public void CheckButtonValue()
        {
            string keys = button.Content.ToString().ToUpper().Trim();

            if (keys.StartsWith("MOUSE"))
            {
                switch (keys)
                {
                    case "MOUSE1": comboBoxMouse.SelectedIndex = 1; break;
                    case "MOUSE2": comboBoxMouse.SelectedIndex = 2; break;
                    case "MOUSE3": comboBoxMouse.SelectedIndex = 3; break;
                    case "MOUSE4": comboBoxMouse.SelectedIndex = 4; break;
                    case "MOUSE5": comboBoxMouse.SelectedIndex = 5; break;
                    case "MOUSESCROLLV": comboBoxMouse.SelectedIndex = 6; break;
                    case "MOUSESCROLLH": comboBoxMouse.SelectedIndex = 7; break;
                    case "MOUSESCROLLB": comboBoxMouse.SelectedIndex = 8; break;
                    default: break;
                }
                comboBoxMouse.Focus();
            }
            else
            {
                textKeyboard.Focus();
            }

            textKeyboard.Text = keys;
            textCustom.Text = keys;
        }


        //
        // Mouse button changed
        //
        private void ComboBoxMouse_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!IsLoaded) return;
            string key = "";


            switch (comboBoxMouse.SelectedIndex)
            {
                case 1: key = "MOUSE1"; break;
                case 2: key = "MOUSE2"; break;
                case 3: key = "MOUSE3"; break;
                case 4: key = "MOUSE4"; break;
                case 5: key = "MOUSE5"; break;
                case 6: key = "MOUSESCROLLV"; break;
                case 7: key = "MOUSESCROLLH"; break;
                case 8: key = "MOUSESCROLLB"; break;
                default: break;
            }
            textKeyboard.Text = key;
            textCustom.Text = key;
        }


        //
        // Keyboard mapping key down
        //
        private void TextKeyboard_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            System.Windows.Forms.KeysConverter keysConverter = new System.Windows.Forms.KeysConverter();
            CultureInfo cultureInfo = new CultureInfo("en-US");
            int keyCode;
            List<string> keys = new List<string>();
            HashSet<string> keyAdded = new HashSet<string>();
            foreach (Key value in Enum.GetValues(typeof(Key)))
            {
                Key key = (Key)value;
                if (key > Key.None && Keyboard.IsKeyDown(key))
                {
                    keyCode = KeyInterop.VirtualKeyFromKey(key);


                    string keyName = keysConverter.ConvertToString(null, cultureInfo, keyCode).ToUpper();
                    switch (key)
                    {
                        case Key.LeftAlt: keyName = "LALT"; break;
                        case Key.RightAlt: keyName = "RALT"; break;
                        case Key.LeftCtrl: keyName = "LCTRL"; break;
                        case Key.RightCtrl: keyName = "RCTRL"; break;
                        case Key.LeftShift: keyName = "LSHIFT"; break;
                        case Key.RightShift: keyName = "RSHIFT"; break;
                        default: break;
                    }
                    if (!keyAdded.Contains(keyName))
                    {
                        keys.Add(keyName);
                        keyAdded.Add(keyName);
                    }
                }
            }


            keys.Sort(
                (a, b) =>
                {
                    int value = 0;
                    if (a.Contains("CTRL")) value--;
                    if (a.Contains("ALT")) value--;
                    if (a.Contains("SHIFT")) value--;
                    if (b.Contains("CTRL")) value++;
                    if (b.Contains("ALT")) value++;
                    if (b.Contains("SHIFT")) value++;
                    return value;
                }
            );

            // Set textboxes
            string keyText = string.Join("+", keys.ToArray());
            textKeyboard.Text = keyText;
            textCustom.Text = keyText;

            comboBoxMouse.SelectedIndex = 0;

            if (e != null)
                e.Handled = true;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {

        }

        //
        // Set
        //
        private void ButtonSet_Click(object sender, RoutedEventArgs e)
        {
            if (button != null)
            {
                Result = textCustom.Text;
                DialogResult = true;
                Close();
            }
        }


        //
        // Clear
        //
        private void ButtonClear_Click(object sender, RoutedEventArgs e)
        {
            if (button != null)
            {
                Result = "";
                DialogResult = true;
                Close();
            }
        }

        //
        // Cancel
        //
        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }


        //
        // Mouse combobox enter press -> set
        //
        private void ComboBoxMouse_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                ButtonSet_Click(sender, null);
            }
        }
    }
}
