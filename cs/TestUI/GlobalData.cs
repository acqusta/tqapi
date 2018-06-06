using TQuant.Api;

namespace TestUI
{
    class GlobalData
    {
        static DataApi dapi;
        static TradeApi tapi;

        public static void Init()
        {
            dapi = TQuantApi.CreateDataApi("ipc://tqc_10001");
            tapi = TQuantApi.CreateTradeApi("ipc://tqc_10001");
        }

        public static DataApi GetDataApi()
        {
            return dapi;
        }

        public static TradeApi GetTradeApi()
        {
            return tapi;
        }
    }
}
