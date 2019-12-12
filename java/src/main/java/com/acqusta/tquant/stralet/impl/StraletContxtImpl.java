package com.acqusta.tquant.stralet.impl;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.api.TradeApi;
import com.acqusta.tquant.api.impl.DataApiImpl;
import com.acqusta.tquant.api.impl.TradeApiImpl;
import com.acqusta.tquant.stralet.FinDateTime;
import com.acqusta.tquant.stralet.Logger;
import com.acqusta.tquant.stralet.StraletContext;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.time.LocalDateTime;
import java.util.Dictionary;
import java.util.Enumeration;
import java.util.HashMap;

public class StraletContxtImpl implements StraletContext {

    private long handle;

    public enum LogSeverity
    {
        INFO,
        WARNING,
        ERROR,
        FATAL
    }

    private LoggerImpl logger;
    private DataApiImpl dapi;
    private TradeApiImpl tapi;
    private String mode;
    private HashMap<String, Object> properties;

    public StraletContxtImpl(long handle) throws Exception {
        this.handle = handle;
        this.logger = new LoggerImpl(this);

        this.tapi = new TradeApiImpl(StraletContextJni.getTradeApi(this.handle));
        this.dapi = new DataApiImpl(StraletContextJni.getDataApi(this.handle));

        this.mode = StraletContextJni.getMode(this.handle);

        String txt = StraletContextJni.getProperties(this.handle);
        ObjectMapper mapper = new ObjectMapper();

        this.properties = mapper.readValue(txt, HashMap.class);
        if (this.properties == null)
            this.properties = new HashMap<>();
    }

    @Override
    public Logger getLogger() {
        return logger;
    }

    @Override
    public HashMap<String, Object> getProps() {
        return this.properties;
    }

    @Override
    public int getTradingDay() {
        return StraletContextJni.getTradingDay(this.handle);
    }

    @Override
    public FinDateTime getCurTime() {
        long dt = StraletContextJni.getCurTime(this.handle);
        return new FinDateTime( (int)(dt / 1000000000), (int)(dt % 1000000000));
    }

    @Override
    public LocalDateTime getCurDateTime() {
        return getCurTime().toLocalDateTime();
    }

    @Override
    public void postEvent(String evt, long data) {
        StraletContextJni.postEvent(this.handle, evt, data);
    }

    @Override
    public void setTimer(long id, long delay, long data) {
        StraletContextJni.setTimer(this.handle, id, delay, data);
    }

    @Override
    public void killTimer(long id) {
        StraletContextJni.killTimer(this.handle, id);
    }

    @Override
    public DataApi getDataApi() {
        return this.dapi;
    }

    @Override
    public TradeApi getTradeApi() {
        return this.tapi;
    }

    @Override
    public String getMode() {
        return this.mode;
    }

    @Override
    public void stop() {
        StraletContextJni.stop(this.handle);

    }

    void log(LogSeverity severity, String str) {
        int l = 0;
        switch (severity) {
            case INFO:      l = 0; break;
            case WARNING:   l = 1; break;
            case ERROR:     l = 2; break;
            case FATAL:     l = 3; break;
        }

        StraletContextJni.log(this.handle, l, str);
    }

}
