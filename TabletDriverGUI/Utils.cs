using System.Globalization;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;

namespace TabletDriverGUI
{
    public class Utils
    {
        public static CultureInfo cultureInfo = null;


        //
        // Check and create culture info
        //
        public static void CheckCultureInfo()
        {
            if (cultureInfo == null)
            {
                cultureInfo = new CultureInfo("en-US");
                cultureInfo.NumberFormat.PerMilleSymbol = "";

                cultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                cultureInfo.NumberFormat.NumberGroupSeparator = "";

                cultureInfo.NumberFormat.PercentDecimalSeparator = ".";
                cultureInfo.NumberFormat.PercentGroupSeparator = "";

                cultureInfo.NumberFormat.CurrencyDecimalSeparator = ".";
                cultureInfo.NumberFormat.CurrencyGroupSeparator = "";
            }
        }

        //
        // String to Number
        //
        public static bool ParseNumber(string str, out double val)
        {
            CheckCultureInfo();
            val = 0;

            // Replace commas with dots
            str = str.Replace(',', '.');

            // Parse
            if (double.TryParse(str, NumberStyles.AllowDecimalPoint | NumberStyles.AllowLeadingSign, cultureInfo.NumberFormat, out double tmp))
            {
                val = tmp;
                return true;
            }
            return false;
        }

        //
        // Number to String
        //
        public static string GetNumberString(double val)
        {
            CheckCultureInfo();
            return GetNumberString(val, "0.##");
        }
        public static string GetNumberString(double val, string format)
        {
            CheckCultureInfo();
            return val.ToString(format, cultureInfo.NumberFormat);
        }




        //
        // RGB to hex string
        //
        public static string RGBToHex(byte r, byte g, byte b)
        {
            return string.Format("{0:X2}{1:X2}{2:X2}", r, g, b); ;
        }

        //
        // RGB to color hex string
        //
        public static string RGBToHexColor(byte r, byte g, byte b)
        {
            return "#" + RGBToHex(r, g, b);
        }

    }
}
