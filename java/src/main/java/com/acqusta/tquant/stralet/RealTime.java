package com.acqusta.tquant.stralet;

import com.acqusta.tquant.stralet.impl.StraletJni;
import com.fasterxml.jackson.databind.ObjectMapper;

public class RealTime {

    static public class Config {
        public String dapi_addr;
        public String tapi_addr;
    }

    static public void run(Config cfg, StraletCreator straletCreator) throws Exception {
        ObjectMapper mapper = new ObjectMapper();
        String txt = mapper.writeValueAsString(cfg);
        StraletJni.runRealTime(txt, straletCreator);
    }
}
