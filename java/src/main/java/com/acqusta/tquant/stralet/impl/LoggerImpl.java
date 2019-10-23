package com.acqusta.tquant.stralet.impl;

import com.acqusta.tquant.stralet.Logger;

import java.util.Formatter;

public class LoggerImpl implements Logger{

    private StraletContxtImpl ctx;

    public LoggerImpl(StraletContxtImpl context) {
        this.ctx = context;
    }

    @Override
    public void info(String format, Object... args) {
        String txt = new Formatter().format(format, args).toString();
        this.ctx.log(StraletContxtImpl.LogSeverity.INFO, txt);
    }

    @Override
    public void warn(String format, Object... args) {
        String txt = new Formatter().format(format, args).toString();
        this.ctx.log(StraletContxtImpl.LogSeverity.WARNING, txt);
    }

    @Override
    public void error(String format, Object... args) {
        String txt = new Formatter().format(format, args).toString();
        this.ctx.log(StraletContxtImpl.LogSeverity.ERROR, txt);
    }

    @Override
    public void fatal(String format, Object... args) {
        String txt = new Formatter().format(format, args).toString();
        this.ctx.log(StraletContxtImpl.LogSeverity.ERROR, txt);
        Exception e = new Exception();
        e.printStackTrace();
        System.exit(-1);
    }
}
