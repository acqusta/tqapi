using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using TQuant.Api.Impl;

namespace TQuant
{
    namespace Api
    {
        public class TQuantApi
        {
            public static TradeApi CreateTradeApi(string addr)
            {
                var h = TqapiDll.tapi_create(addr);
                return h != IntPtr.Zero ? new TradeApiImpl(h, true) : null;
            }

            public static DataApi CreateDataApi(string addr)
            {
                var h = TqapiDll.dapi_create(addr);
                return h != IntPtr.Zero ? new DataApiImpl(h, true) : null;
            }
        }
    }
}
