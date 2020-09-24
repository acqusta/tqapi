//using Newtonsoft.Json;
using System.Text.Json;

namespace TQuant.Stralet
{

    public class RealTime
    {
        public delegate Stralet CreateStralet();

        public class Config
        {
            public string dapi_addr;
            public string tapi_addr;
        }

        static public void Run(Config cfg, CreateStralet creatae_stralet)
        {
            //var json = JsonConvert.SerializeObject(cfg);
            var json = JsonSerializer.Serialize(cfg);

            StraletWrap wrap = null;
            Impl.TqsDll.StraletCreator my_creatae_stralet = () =>
            {
                // FIXME
                var stralet = creatae_stralet();
                wrap = new StraletWrap(stralet);
                return wrap.handle;
            };

            Impl.TqsDll.tqs_rt_run(json, my_creatae_stralet);

            wrap = null;
        }
    }
}