using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for ButtonMapping.xaml
    /// </summary>
    public partial class WindowButtonMapping : Window
    {
        // WPF Button
        Button button;
        public string Result;

        public class MouseAction
        {
            public string Action;
            public string Name;
            public bool Visible;

            public MouseAction(string name) : this("", name)
            {                
            }
            public MouseAction(string action, string name)
            {
                Action = action;
                Name = name;
                Visible = true;
            }
            public override string ToString()
            {
                return Name;
            }
        }

        public OrderedDictionary mouseActions;

        public WindowButtonMapping()
        {
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            Owner = Application.Current.MainWindow;

            InitializeComponent();
            Result = "";


            mouseActions = new OrderedDictionary()
            {
                { "", new MouseAction("None") },
                { "MOUSE1", new MouseAction("Mouse 1 (Left / Tip)") },
                { "MOUSE2", new MouseAction("Mouse 2 (Right / Barrel)") },
                { "MOUSE3", new MouseAction("Mouse 3 (Middle / Eraser)") },
                { "MOUSE4", new MouseAction("Mouse 4 (Back)") },
                { "MOUSE5", new MouseAction("Mouse 5 (Forward)") },
                { "MOUSESCROLLV", new MouseAction("Scroll Up/Down") },
                { "MOUSESCROLLH", new MouseAction("Scroll Left/Right") },
                { "MOUSESCROLLB", new MouseAction("Scroll Both") },
                { "SCROLLUP", new MouseAction("Scroll One Up") },
                { "SCROLLDOWN", new MouseAction("Scroll One Down") },
                { "SCROLLLEFT", new MouseAction("Scroll One Left") },
                { "SCROLLRIGHT", new MouseAction("Scroll One Right") }
            };

            UpdateMouseActions();

        }

        public WindowButtonMapping(Button button, bool isPenButton) : this()
        {
            this.button = button;

            // Tablet buttons don't need tip, barrel and eraser info
            if (!isPenButton)
            {
                ((MouseAction)mouseActions["MOUSE1"]).Name = "Mouse 1 (Left)";
                ((MouseAction)mouseActions["MOUSE2"]).Name = "Mouse 2 (Right)";
                ((MouseAction)mouseActions["MOUSE3"]).Name = "Mouse 3 (Middle)";

                // Disable mouse scroll
                ((MouseAction)mouseActions["MOUSESCROLLV"]).Visible = false;
                ((MouseAction)mouseActions["MOUSESCROLLH"]).Visible = false;
                ((MouseAction)mouseActions["MOUSESCROLLB"]).Visible = false;
            }
            UpdateMouseActions();

            CheckKeyValue();
        }


        //
        // Update mouse actions and combobox
        //
        private void UpdateMouseActions()
        {
            comboBoxMouse.Items.Clear();
            foreach (DictionaryEntry entry in mouseActions)
            {
                MouseAction mouseAction = (MouseAction)entry.Value;
                mouseAction.Action = (string)entry.Key;
                if (mouseAction.Visible)
                {
                    comboBoxMouse.Items.Add(mouseAction);
                }
            }

        }


        //
        // Check key value
        //
        public void CheckKeyValue()
        {
            string keys = button.Content.ToString().ToUpper().Trim();

            if (mouseActions.Contains(keys))
            {
                foreach(var item in comboBoxMouse.Items)
                {
                    MouseAction mouseAction = (MouseAction)item;
                    if(mouseAction.Action == keys)
                    {
                        comboBoxMouse.SelectedItem = item;
                        comboBoxMouse.Focus();
                    }
                }
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

            // Combobox item item to MouseAction
            if(comboBoxMouse.SelectedIndex >= 0 && comboBoxMouse.SelectedItem != null)
            {
                MouseAction mouseAction = (MouseAction)comboBoxMouse.SelectedItem;
                key = mouseAction.Action;
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
        // Mouse combobox or custom keys enter press -> set
        //
        private void OnEnterKeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                ButtonSet_Click(sender, null);
            }
        }
    }
}
