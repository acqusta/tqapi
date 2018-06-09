using Newtonsoft.Json;
using System.Collections.Generic;

namespace TQuant.Stralet
{
    public class BackTest
    {
        public delegate Stralet StraletCreator();

        public class Config
        {
            public class Holding
            {
                public string code;
                public string side;
                public long   size;
                public double cost_price;
            }

            public class AccountConfig
            {
                public string    account_id;
                public double    init_balance;
                public Holding[] init_holdings;
            }

            public string dapi_addr;
            public string data_level;
            public int    begin_date;
            public int    end_date;
            public string result_dir;

            public AccountConfig[] accounts;

            public Dictionary<string, object> properties = new Dictionary<string, object>();
        }

        static public void Run(Config cfg, StraletCreator creatae_stralet)
        {
            var json = JsonConvert.SerializeObject(cfg);

            Stralet stralet = null;
            Impl.TqsDll.StraletCreator my_creatae_stralet = () =>
            {
                // FIXME
                stralet = creatae_stralet();
                return stralet._Handle;
            };

            Impl.TqsDll.tqs_bt_run(json, my_creatae_stralet);

            stralet = null;
        }
    }
}