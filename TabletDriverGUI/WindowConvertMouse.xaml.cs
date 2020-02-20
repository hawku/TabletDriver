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
    /// <summary>
    /// Interaction logic for WindowConvertMouse.xaml
    /// </summary>
    public partial class WindowConvertMouse : Window
    {
        public double Dpi;

        public WindowConvertMouse()
        {
            InitializeComponent();
        }

        private void Window_Activated(object sender, EventArgs e)
        {
            FocusManager.SetFocusedElement(this, DpiText);
        }

        private void Ok_Click(object sender, RoutedEventArgs e)
        {
            bool valid = double.TryParse(DpiText.Text, out Dpi);
            if (valid)
            {
                DialogResult = true;
                Close();
            }
            else
            {
                MessageBox.Show("The entered value is not a number.");
            }
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }
    }
}
