using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TabletDriverGUI
{
    class TabletNotRecognizedException : Exception
    {
        public TabletNotRecognizedException(string message) : base(message)
        {

        }
    }
}
