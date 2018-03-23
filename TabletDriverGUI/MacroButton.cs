using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace TabletDriverGUI
{
    //[XmlType("MacroButton")]
    public class MacroButton
    {
        public int Index;
        [XmlArray("MacroKeys")]
        [XmlArrayItem("Key")]
        public int[] MacroKeys;

        public MacroButton() { }

        public MacroButton(int index, int[] macroKeys)
        {
            Index = index;
            MacroKeys = macroKeys;
        }
    }
}
