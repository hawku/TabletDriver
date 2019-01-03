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

        public class ButtonBinding
        {
            public string Key;
            public string Name;
            public bool Visible;

            public ButtonBinding(string name) : this("", name)
            {
            }
            public ButtonBinding(string key, string name)
            {
                Key = key;
                Name = name;
                Visible = true;
            }
            public override string ToString()
            {
                return Name;
            }
        }

        public OrderedDictionary mouseBindings;
        public OrderedDictionary multimediaBindings;

        public WindowButtonMapping()
        {
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            Owner = Application.Current.MainWindow;

            InitializeComponent();
            Result = "";


            mouseBindings = new OrderedDictionary()
            {
                { "", new ButtonBinding("None") },
                { "MOUSE1", new ButtonBinding("Mouse 1 (Left / Tip)") },
                { "MOUSE2", new ButtonBinding("Mouse 2 (Right / Barrel)") },
                { "MOUSE3", new ButtonBinding("Mouse 3 (Middle / Eraser)") },
                { "MOUSE4", new ButtonBinding("Mouse 4 (Back)") },
                { "MOUSE5", new ButtonBinding("Mouse 5 (Forward)") },
                { "MOUSESCROLLV", new ButtonBinding("Scroll Up/Down") },
                { "MOUSESCROLLH", new ButtonBinding("Scroll Left/Right") },
                { "MOUSESCROLLB", new ButtonBinding("Scroll Both") },
                { "SCROLLUP", new ButtonBinding("Scroll One Up") },
                { "SCROLLDOWN", new ButtonBinding("Scroll One Down") },
                { "SCROLLLEFT", new ButtonBinding("Scroll One Left") },
                { "SCROLLRIGHT", new ButtonBinding("Scroll One Right") },
            };

            multimediaBindings = new OrderedDictionary()
            {
                { "", new ButtonBinding("None") },
                { "VOLUMEUP", new ButtonBinding("Volume Up") },
                { "VOLUMEUP0.5", new ButtonBinding("Volume Up 0.5%") },
                { "VOLUMEUP5.0", new ButtonBinding("Volume Up 5.0%") },
                { "VOLUMEDOWN", new ButtonBinding("Volume Down") },
                { "VOLUMEDOWN0.5", new ButtonBinding("Volume Down 0.5%") },
                { "VOLUMEDOWN5.0", new ButtonBinding("Volume Down 5.0%") },
                { "VOLUMEMUTE", new ButtonBinding("Volume Mute") },
                { "VOLUMECONTROL", new ButtonBinding("Volume Control Up/Down") },
                { "BALANCECONTROL", new ButtonBinding("Balance Control Left/Right") },
                { "MEDIANEXT", new ButtonBinding("Media Next Track") },
                { "MEDIAPREV", new ButtonBinding("Media Previous Track") },
                { "MEDIASTOP", new ButtonBinding("Media Stop") },
                { "MEDIAPLAY", new ButtonBinding("Media Play/Pause") },
            };

            UpdateBindings();

        }

        public WindowButtonMapping(Button button, bool isPenButton) : this()
        {
            this.button = button;

            // Tablet buttons don't need tip, barrel and eraser info
            if (!isPenButton)
            {
                ((ButtonBinding)mouseBindings["MOUSE1"]).Name = "Mouse 1 (Left)";
                ((ButtonBinding)mouseBindings["MOUSE2"]).Name = "Mouse 2 (Right)";
                ((ButtonBinding)mouseBindings["MOUSE3"]).Name = "Mouse 3 (Middle)";
            }
            UpdateBindings();

            CheckKeyValue();
        }


        //
        // Update bindings and comboboxes
        //
        private void UpdateBindings()
        {

            // Mouse
            comboBoxMouse.Items.Clear();
            foreach (DictionaryEntry entry in mouseBindings)
            {
                ButtonBinding binding = (ButtonBinding)entry.Value;
                binding.Key = (string)entry.Key;
                if (binding.Visible)
                {
                    comboBoxMouse.Items.Add(binding);
                }
            }

            // Multimedia
            comboBoxMultimedia.Items.Clear();
            foreach (DictionaryEntry entry in multimediaBindings)
            {
                ButtonBinding binding = (ButtonBinding)entry.Value;
                binding.Key = (string)entry.Key;
                if (binding.Visible)
                {
                    comboBoxMultimedia.Items.Add(binding);
                }
            }

        }


        //
        // Check key value
        //
        public void CheckKeyValue()
        {
            string keys = button.Content.ToString().ToUpper().Trim();
            comboBoxMouse.SelectedIndex = 0;
            comboBoxMultimedia.SelectedIndex = 0;

            // Mouse
            if (mouseBindings.Contains(keys))
            {

                foreach (var item in comboBoxMouse.Items)
                {
                    ButtonBinding binding = (ButtonBinding)item;
                    if (binding.Key == keys)
                    {
                        comboBoxMouse.SelectedItem = item;
                        comboBoxMouse.Focus();
                    }
                }
            }

            // Multimedia
            else if (multimediaBindings.Contains(keys))
            {
                comboBoxMultimedia.SelectedIndex = 0;
                foreach (ButtonBinding item in comboBoxMultimedia.Items)
                {
                    //ButtonBinding binding = (ButtonBinding)item;
                    if (item.Key == keys)
                    {
                        comboBoxMultimedia.SelectedItem = item;
                        comboBoxMultimedia.Focus();
                    }
                }
            }

            // Keyboard
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
            if (comboBoxMouse.SelectedIndex >= 0 && comboBoxMouse.SelectedItem != null)
            {
                ButtonBinding binding = (ButtonBinding)comboBoxMouse.SelectedItem;
                if (comboBoxMouse.SelectedIndex > 0)
                    comboBoxMultimedia.SelectedIndex = 0;
                textKeyboard.Text = binding.Key;
                textCustom.Text = binding.Key;
            }
        }

        //
        // Multimedia key changed
        //
        private void ComboBoxMultimedia_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!IsLoaded) return;
            if (comboBoxMultimedia.SelectedIndex >= 0 && comboBoxMultimedia.SelectedItem is ButtonBinding)
            {
                ButtonBinding binding = (ButtonBinding)comboBoxMultimedia.SelectedItem;
                if (comboBoxMultimedia.SelectedIndex > 0)
                    comboBoxMouse.SelectedIndex = 0;
                textKeyboard.Text = binding.Key;
                textCustom.Text = binding.Key;
            }
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
