using Newtonsoft.Json;

namespace TQuant
{
    namespace Stralet
    {
        public class BackTestConfig
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
                public string account_id;
                public double init_balance;
                public Holding[] init_holdings;
            }

            public string   dapi_addr;
            public string   data_level;
            public int      begin_date;
            public int      end_date;
            public string   result_dir;
            public AccountConfig[] accounts;
        }

        public delegate Stralet CreateStralet();
        public delegate IFsmStralet CreateFSMStralet();

        public class BackTest
        {
            static public void Run(BackTestConfig cfg, CreateStralet creatae_stralet)
            {
                var json = JsonConvert.SerializeObject(cfg);

                Stralet stralet = null;
                Impl.TqsDll.BTRunCreateStralet bt_creatae_stralet = () =>
                {
                    // FIXME
                    stralet = creatae_stralet();
                    return stralet._Handle;
                };

                Impl.TqsDll.tqs_bt_run(json, bt_creatae_stralet);

                stralet = null;
            }

            //static public void RunFSM(TQuant.Stralet.BackTestConfig cfg, CreateFSMStralet creatae_stralet)
            //{
            //    TQuant.Stralet.BackTest.Run(cfg, () => { return creatae_stralet().Stralet; });
            //}

        }
    }
}