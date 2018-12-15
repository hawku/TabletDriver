using System;
using System.Linq;
using System.Collections;
using System.IO;
using System.Xml;
using System.Xml.Serialization;

namespace TabletDriverGUI
{
    [XmlRootAttribute("Configuration", IsNullable = true)]
    public class Configuration
    {
        public int ConfigVersion;
        public Area TabletArea;
        public Area TabletFullArea;
        public bool ForceAspectRatio;
        public double Rotation;
        public bool Invert;
        public bool ForceFullArea;
        public OutputModes OutputMode;
        public enum OutputModes
        {
            Absolute = 0,
            Relative = 1,
            Digitizer = 2
        }

        public Area ScreenArea;

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
        public double AntiSmoothingShape;
        public double AntiSmoothingCompensation;
        public bool AntiSmoothingOnlyWhenHover;

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
        public double PressureDeadzone;

        public double ScrollSensitivity;
        public double ScrollAcceleration;

        [XmlArray("CustomCommands")]
        [XmlArrayItem("Command")]
        public string[] CustomCommands;

        public int WindowWidth;
        public int WindowHeight;

        public bool AutomaticRestart;
        public bool RunAtStartup;

        public string DriverPath;
        public string DriverArguments;

        public bool DebuggingEnabled;
        public bool DeveloperMode;


        public Configuration()
        {
            ConfigVersion = 1;

            // Screen Map
            ScreenArea = new Area(0, 0, 0, 0);

            // Tablet area
            TabletArea = new Area(80, 45, 40, 22.5);
            TabletFullArea = new Area(100, 50, 50, 25);
            ForceFullArea = true;
            OutputMode = 0;
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
            ScrollSensitivity = 1.0;
            ScrollAcceleration = 1.0;

            SmoothingEnabled = false;
            SmoothingLatency = 0;
            SmoothingInterval = 4;
            SmoothingOnlyWhenButtons = false;

            NoiseFilterEnabled = false;
            NoiseFilterBuffer = 10;
            NoiseFilterThreshold = 0.5;

            AntiSmoothingEnabled = false;
            AntiSmoothingShape = 0.5;
            AntiSmoothingCompensation = 20.0;

            CustomCommands = new string[] { "" };

            WindowWidth = 700;
            WindowHeight = 710;

            AutomaticRestart = true;
            RunAtStartup = false;

            DriverPath = "bin/TabletDriverService.exe";
            DriverArguments = "config/init.cfg";

            DebuggingEnabled = false;
            DeveloperMode = false;
        }


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
