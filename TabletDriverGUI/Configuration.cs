using System;
using System.IO;
using System.Windows;
using System.Xml;
using System.Xml.Serialization;

namespace TabletDriverGUI
{
    [XmlRootAttribute("Configuration", IsNullable = true)]
    public class Configuration
    {
        public int ConfigVersion;

        public string TabletName;

        public Area ScreenArea;
        [XmlArray("ScreenAreas")]
        [XmlArrayItem("ScreenArea")]
        public Area[] ScreenAreas;
        public Area SelectedScreenArea;

        public Area TabletArea;
        [XmlArray("TabletAreas")]
        [XmlArrayItem("TabletArea")]
        public Area[] TabletAreas;
        public Area SelectedTabletArea;

        public Area TabletFullArea;
        public bool ForceAspectRatio;
        public double Rotation;
        public bool Invert;
        public OutputPositioning Positioning;
        public OutputModes Mode;

        public enum OutputPositioning
        {
            Absolute = 0,
            Relative = 1
        }
        public enum OutputModes
        {
            Standard = 0,
            WindowsInk = 1,
            Compatibility = 2
        }

        // Smoothing filter
        public bool SmoothingEnabled;
        public double SmoothingLatency;
        public int SmoothingInterval;
        public bool SmoothingOnlyWhenButtons;

        // Noise filter
        public bool NoiseFilterEnabled;
        public int NoiseFilterBuffer;
        public double NoiseFilterThreshold;

        // Anti-smoothing filter
        public bool AntiSmoothingEnabled;
        public class AntiSmoothingSetting
        {
            public bool Enabled;
            public double Velocity;
            public double Shape;
            public double Compensation;
            public AntiSmoothingSetting()
            {
                Enabled = false;
                Velocity = 0;
                Shape = 0.5;
                Compensation = 0;
            }
        };
        [XmlArray("AntiSmoothingSettingsList")]
        [XmlArrayItem("AntiSmoothingSettings")]
        public AntiSmoothingSetting[] AntiSmoothingSettings;
        public bool AntiSmoothingOnlyWhenHover;
        public double AntiSmoothingDragMultiplier;

        public Area DesktopSize;
        public bool AutomaticDesktopSize;

        [XmlArray("ButtonMap")]
        [XmlArrayItem("Button")]
        public string[] ButtonMap;
        public bool DisableButtons;

        [XmlArray("TabletButtonMap")]
        [XmlArrayItem("Button")]
        public string[] TabletButtonMap;
        public bool DisableTabletButtons;

        public double PressureSensitivity;
        public double PressureDeadzoneLow;
        public double PressureDeadzoneHigh;

        public double ScrollSensitivity;
        public double ScrollAcceleration;
        public bool ScrollStopCursor;
        public bool ScrollDrag;

        [XmlArray("CustomCommands")]
        [XmlArrayItem("Command")]
        public string[] CustomCommands;

        public int WindowWidth;
        public int WindowHeight;

        public class TabletViewSettings
        {
            public string BackgroundColor;
            public string InfoColor;
            public string InputColor;
            public string OutputColor;
            public string LatencyColor;
            public string DrawColor;
            public int InputTrailLength;
            public int OutputTrailLength;
            public int DrawLength;
            public string Font;
            public double FontSize;
            public Point OffsetText;
            public Point OffsetPressure;
            public bool FadeInOut;
            public bool Borderless;
            public TabletViewSettings()
            {
                BackgroundColor = "#FFFFFF";
                InfoColor = "#000000";
                InputColor = "#33AA33";
                OutputColor = "#AA3333";
                LatencyColor = "#3333AA";
                DrawColor = "#000000";
                InputTrailLength = 30;
                OutputTrailLength = 30;
                DrawLength = 0;
                Font = "Segoe UI";
                FontSize = 25;
                OffsetPressure = new Point(0, 0);
                OffsetText = new Point(0, 0);
                FadeInOut = false;
                Borderless = false;
            }
        };
        public TabletViewSettings TabletView;

        public bool AutomaticRestart;
        public bool RunAtStartup;

        public string DriverPath;
        public string DriverArguments;

        public bool DebuggingEnabled;
        public bool DeveloperMode;

        public class Preset
        {
            public string Name;
            public Action<Configuration> Action;
            public Preset(string name, Action<Configuration> action)
            {
                Name = name;
                Action = action;
            }
            public override string ToString()
            {
                return Name;
            }
        }


        public Configuration()
        {
            ConfigVersion = 2;

            // Screen Map
            ScreenArea = null;
            ScreenAreas = new Area[3];
            for (int i = 0; i < ScreenAreas.Length; i++)
            {
                ScreenAreas[i] = new Area(0, 0, 0, 0)
                {
                    IsEnabled = false
                };
            }
            ScreenAreas[0].IsEnabled = true;
            ScreenAreas[1] = new Area(1000, 500, 500, 250);

            // Tablet area
            TabletArea = null;
            TabletAreas = new Area[ScreenAreas.Length];
            for (int i = 0; i < GetAreaCount(); i++)
                TabletAreas[i] = new Area(100, 56, 50, 28);
            TabletFullArea = new Area(100, 50, 50, 25);
            Mode = OutputModes.Standard;
            ForceAspectRatio = true;
            Rotation = 0;

            DesktopSize = new Area(0, 0, 0, 0);
            AutomaticDesktopSize = true;

            ButtonMap = new string[] { "MOUSE1", "MOUSE2", "MOUSE3" };
            DisableButtons = false;

            TabletButtonMap = new string[16];
            for (int i = 0; i < 16; i++) TabletButtonMap[i] = "";
            DisableTabletButtons = false;

            PressureSensitivity = 0;
            ScrollSensitivity = 0.5;
            ScrollAcceleration = 1.0;
            ScrollStopCursor = false;

            SmoothingEnabled = false;
            SmoothingLatency = 0;
            SmoothingInterval = 4;
            SmoothingOnlyWhenButtons = false;

            NoiseFilterEnabled = false;
            NoiseFilterBuffer = 10;
            NoiseFilterThreshold = 0.5;

            AntiSmoothingEnabled = false;
            AntiSmoothingSettings = new AntiSmoothingSetting[5];
            for (int i = 0; i < AntiSmoothingSettings.Length; i++)
                AntiSmoothingSettings[i] = new AntiSmoothingSetting();
            AntiSmoothingDragMultiplier = 1.0;
            AntiSmoothingOnlyWhenHover = false;

            CustomCommands = new string[] { "" };

            WindowWidth = 700;
            WindowHeight = 700;

            TabletView = new TabletViewSettings();

            AutomaticRestart = true;
            RunAtStartup = false;

            DriverPath = "bin/TabletDriverService.exe";
            DriverArguments = "config/init.cfg";

            DebuggingEnabled = false;
            DeveloperMode = false;
        }


        //
        // Get number of areas
        //
        public int GetAreaCount()
        {
            if (ScreenAreas.Length <= TabletAreas.Length)
                return ScreenAreas.Length;
            return TabletAreas.Length;
        }


        //
        // Get maximum number of areas
        //
        public int GetMaxAreaCount()
        {
            return 5;
        }


        //
        // Get number of enabled areas
        //
        public int GetEnabledAreaCount()
        {
            int count = 0;
            foreach (Area area in ScreenAreas)
            {
                if (area.IsEnabled) count++;
            }
            return count;
        }


        //
        // Clear anti-smoothing filter settings
        //
        public void ClearAntiSmoothingSettings()
        {
            for (int i = 0; i < AntiSmoothingSettings.Length; i++)
            {
                AntiSmoothingSettings[i].Enabled = false;
                AntiSmoothingSettings[i].Velocity = 0;
                AntiSmoothingSettings[i].Shape = 0.5;
                AntiSmoothingSettings[i].Compensation = 0;
            }
        }


        //
        // Set anti-smoothing filter settings
        //
        public void SetAntiSmoothingSetting(int index, bool enabled, double velocity, double shape, double compensation)
        {
            AntiSmoothingSettings[index].Enabled = enabled;
            AntiSmoothingSettings[index].Velocity = velocity;
            AntiSmoothingSettings[index].Shape = shape;
            AntiSmoothingSettings[index].Compensation = compensation;
        }


        //
        // Write configuration to a XML file
        //
        public void Write(string filename)
        {
            var fileWriter = new StreamWriter(filename);

            XmlSerializer serializer = new XmlSerializer(typeof(Configuration));
            XmlWriterSettings xmlWriterSettings = new XmlWriterSettings() { Indent = true };
            XmlWriter writer = XmlWriter.Create(fileWriter, xmlWriterSettings);
            try
            {
                serializer.Serialize(writer, this);
            }
            catch (Exception)
            {
                fileWriter.Close();
                throw;
            }
            fileWriter.Close();
        }

        //
        // Create configuration from a XML file
        //
        public static Configuration CreateFromFile(string filename)
        {
            Configuration config = null;
            var serializer = new XmlSerializer(typeof(Configuration));
            var settings = new XmlWriterSettings() { Indent = true };
            var reader = XmlReader.Create(filename);

            try
            {
                config = (Configuration)serializer.Deserialize(reader);
            }
            catch (Exception)
            {
                reader.Close();
                throw;
            }
            reader.Close();
            return config;
        }

    }


}
