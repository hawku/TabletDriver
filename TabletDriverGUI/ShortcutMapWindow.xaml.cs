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
using WindowsInput.Native;
using WindowsInput;
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
        [DllImport("User32.Dll", EntryPoint = "PostMessageA")]
        private static extern bool PostMessage(IntPtr hWnd, uint msg, int wParam, int lParam);

        public Key PressedKey;

        public ShortcutMapWindow()
        {
            InitializeComponent();
        }

        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            PressedKey = e.Key;

            e.Handled = true;

            // Fetch the actual shortcut key.
            Key key = (e.Key == Key.System ? e.SystemKey : e.Key);

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
                shortcutText.Append("Ctrl+");
            }
            if ((Keyboard.Modifiers & ModifierKeys.Shift) != 0)
            {
                shortcutText.Append("Shift+");
            }
            if ((Keyboard.Modifiers & ModifierKeys.Alt) != 0)
            {
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
            //Process[] processes = Process.GetProcesses();
            //foreach (Process proc in processes)
            //{
            //    const uint WM_KEYDOWN = 0x0100;
            //    PostMessage(proc.MainWindowHandle, WM_KEYDOWN, 0xBE, 0);
            //    PostMessage(proc.MainWindowHandle, WM_KEYDOWN, 0x63, 0);
                //SetForegroundWindow(proc.MainWindowHandle);
                //System.Windows.Forms.SendKeys.SendWait("(.3)");
            //}

            //InputSimulator inputSimulator = new InputSimulator();
            //inputSimulator.Keyboard.ModifiedKeyStroke(VirtualKeyCode.MENU, VirtualKeyCode.F4);
            //System.Windows.Forms.SendKeys.SendWait("(%{F4})");
        }
    }
}
