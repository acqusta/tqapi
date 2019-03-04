package com.acqusta.tquant.stralet;

import com.acqusta.tquant.stralet.impl.StraletJni;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.ObjectMapper;

public class RealTime {

    //@JsonIgnoreProperties(ignoreUnknown = true)
    public class Config {
        @JsonProperty("dapi_addr")
        public String dapiAddr;

        @JsonProperty("tapi_addr")
        public String tapiAddr;
    }

    static public void run(Config cfg, StraletCreator straletCreator) throws Exception {
        ObjectMapper mapper = new ObjectMapper();
        String txt = mapper.writeValueAsString(cfg);
        StraletJni.runRealTime(txt, straletCreator);
    }
}
