using TQuant.Api;

namespace TestUI
{
    class GlobalData
    {
        static TQuantApi tqapi;

        public static void Init()
        {
            tqapi = TQuantApi.Create("tcp://127.0.0.1:10001");
        }

        public static DataApi GetDataApi()
        {
            return tqapi.GetDataApi();
        }

        public static TradeApi GetTradeApi()
        {
            return tqapi.GetTradeApi();
        }
    }
}
