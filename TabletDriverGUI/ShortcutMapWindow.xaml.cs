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
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for ShortcutMapWindow.xaml
    /// </summary>
    public partial class ShortcutMapWindow : Window
    {
        public List<ModifierKeys>   ModifierKey;
        public Key                  PressedKey;

        public ShortcutMapWindow()
        {
            ModifierKey = new List<ModifierKeys>();
            InitializeComponent();
        }

        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            ModifierKey.Clear();

            e.Handled = true;

            // Fetch the actual shortcut key.
            Key key = (e.Key == Key.System ? e.SystemKey : e.Key);

            PressedKey = key;

            // Ignore modifier keys.
            if (key == Key.LeftShift || key == Key.RightShift
                || key == Key.LeftCtrl || key == Key.RightCtrl
                || key == Key.LeftAlt || key == Key.RightAlt
                || key == Key.LWin || key == Key.RWin)
            {
                return;
            }

            // Build the shortcut key name.
            StringBuilder shortcutText = new StringBuilder();
            if ((Keyboard.Modifiers & ModifierKeys.Control) != 0)
            {
                ModifierKey.Add(ModifierKeys.Control);
                shortcutText.Append("Ctrl+");
            }
            if ((Keyboard.Modifiers & ModifierKeys.Shift) != 0)
            {
                ModifierKey.Add(ModifierKeys.Shift);
                shortcutText.Append("Shift+");
            }
            if ((Keyboard.Modifiers & ModifierKeys.Alt) != 0)
            {
                ModifierKey.Add(ModifierKeys.Alt);
                shortcutText.Append("Alt+");
            }
            shortcutText.Append(key.ToString());

            ShortcutResult.Content = shortcutText.ToString();
        }

        private void ButtonSet_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
