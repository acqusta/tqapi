using Newtonsoft.Json;

namespace TQuant.Stralet
{

    public class RealTime
    {
        public delegate Stralet CreateStralet();

        public class Config
        {
            public string tqapi_addr;
        }

        static public void Run(Config cfg, CreateStralet creatae_stralet)
        {
            var json = JsonConvert.SerializeObject(cfg);

            Stralet stralet = null;
            Impl.TqsDll.StraletCreator my_creatae_stralet = () =>
            {
                // FIXME
                stralet = creatae_stralet();
                return stralet._Handle;
            };

            Impl.TqsDll.tqs_rt_run(json, my_creatae_stralet);

            stralet = null;
        }

    }
}