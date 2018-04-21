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
            IntPtr handle;
            TradeApi tapi;
            Dictionary<string, DataApiImpl> dapi_map = new Dictionary<string, DataApiImpl>();

            TQuantApi(IntPtr h)
            {
                this.handle = h;
            }

            /**
            * 取数据接口
            *
            * @return
            */
            public TradeApi GetTradeApi()
            {
                if (tapi != null) return tapi;

                var h = TqapiDll.tqapi_get_trade_api(this.handle);
                if (h != null)
                    tapi = new TradeApiImpl(this, h);

                return tapi;
            }

            /**
            *  取交易接口
            *
            * @return
            */
            public DataApi GetDataApi(string source = "")
            {
                if (dapi_map.ContainsKey(source))
                    return dapi_map[source];

                var h = TqapiDll.tqapi_get_data_api(this.handle, source);
                if (h != null)
                {
                    var dapi = new DataApiImpl(this, h);
                    dapi_map[source] = dapi;
                    return dapi;
                }
                else
                {
                    return null;
                }
            }

            public static TQuantApi Create(string addr)
            {
                IntPtr h = TqapiDll.tqapi_create(addr);
                return h != null ? new TQuantApi(h) : null;
            }
        }
    }
}
