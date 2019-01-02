using System.Windows;
using System.Windows.Controls;

namespace TabletDriverGUI
{
    /// <summary>
    /// Interaction logic for WindowAreaEditor.xaml
    /// </summary>
    public partial class WindowAreaEditor : Window
    {
        private Area screenArea, tabletArea;
        private Configuration config;
        private bool isLoadingSettings;

        public WindowAreaEditor(Configuration configuration, Area ScreenArea, Area TabletArea)
        {
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            Owner = Application.Current.MainWindow;

            InitializeComponent();
            screenArea = ScreenArea;
            tabletArea = TabletArea;
            config = configuration;
            isLoadingSettings = false;
            LoadValues();
        }

        //
        // Save click
        //
        private void ButtonSave_Click(object sender, RoutedEventArgs e)
        {
            StoreValues();
            DialogResult = true;
            Close();
        }

        //
        // Cancel click
        //
        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }


        //
        // Load values from configuration
        //
        private void LoadValues()
        {
            isLoadingSettings = true;

            // Screen area
            textScreenAreaWidth.Text = Utils.GetNumberString(screenArea.Width);
            textScreenAreaHeight.Text = Utils.GetNumberString(screenArea.Height);
            textScreenAreaX.Text = Utils.GetNumberString(screenArea.X - screenArea.Width / 2.0);
            textScreenAreaY.Text = Utils.GetNumberString(screenArea.Y - screenArea.Height / 2.0);

            // Tablet area
            textTabletAreaWidth.Text = Utils.GetNumberString(tabletArea.Width);
            textTabletAreaHeight.Text = Utils.GetNumberString(tabletArea.Height);
            textTabletAreaX.Text = Utils.GetNumberString(tabletArea.X);
            textTabletAreaY.Text = Utils.GetNumberString(tabletArea.Y);

            if(config.ForceAspectRatio)
            {
                textTabletAreaHeight.IsEnabled = false;
            } else
            {
                textTabletAreaHeight.IsEnabled = true;
            }


            isLoadingSettings = false;


        }

        //
        // Store values
        //
        private void StoreValues()
        {

            // Screen area
            if (Utils.ParseNumber(textScreenAreaWidth.Text, out double value))
                screenArea.Width = value;
            if (Utils.ParseNumber(textScreenAreaHeight.Text, out value))
                screenArea.Height = value;
            if (Utils.ParseNumber(textScreenAreaX.Text, out value))
                screenArea.X = value + screenArea.Width / 2.0;
            if (Utils.ParseNumber(textScreenAreaY.Text, out value))
                screenArea.Y = value + screenArea.Height / 2.0;

            // Tablet area
            if (Utils.ParseNumber(textTabletAreaWidth.Text, out value))
                tabletArea.Width = value;
            if (Utils.ParseNumber(textTabletAreaHeight.Text, out value))
                tabletArea.Height = value;
            if (Utils.ParseNumber(textTabletAreaX.Text, out value))
                tabletArea.X = value;
            if (Utils.ParseNumber(textTabletAreaY.Text, out value))
                tabletArea.Y = value;

        }

        //
        // TextBox key up
        //
        private void OnTextBoxKeyUp(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if(e.Key == System.Windows.Input.Key.Enter)
            {
                ButtonSave_Click(sender, null);
            }
        }

        //
        // TextBox changed
        //
        private void OnTextBoxChanged(object sender, TextChangedEventArgs e)
        {
            if (!IsLoaded || isLoadingSettings) return;

            if (config.ForceAspectRatio)
            {
                double screenWidth, screenHeight;
                double tabletWidth, tabletHeight;
                double aspectRatio;

                screenWidth = screenArea.Width;
                screenHeight = screenArea.Height;
                tabletWidth = tabletArea.Width;
                tabletHeight = tabletArea.Height;

                if (Utils.ParseNumber(textScreenAreaWidth.Text, out double value))
                    screenWidth = value;
                if (Utils.ParseNumber(textScreenAreaHeight.Text, out value))
                    screenHeight = value;

                if (Utils.ParseNumber(textTabletAreaWidth.Text, out value))
                    tabletWidth = value;
                if (Utils.ParseNumber(textTabletAreaHeight.Text, out value))
                    tabletHeight = value;

                aspectRatio = screenWidth / screenHeight;
                tabletHeight = tabletWidth / aspectRatio;
                textTabletAreaHeight.Text = Utils.GetNumberString(tabletHeight);
            }
        }


    }
}
