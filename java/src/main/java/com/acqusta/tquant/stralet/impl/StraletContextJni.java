package com.acqusta.tquant.stralet.impl;

import com.acqusta.tquant.api.impl.TQuantApiJni;

class StraletContextJni {
    static {
        TQuantApiJni.init();
    }

    static native int getTradingDay(long handle);

    static native long getCurTime(long handle);

    static native void postEvent(long handle, String evt, long data);

    static native void setTimer(long handle, long id, long delay, long data);

    static native void killTimer(long handle, long id);

    static native long getDataApi(long handle);

    static native long getTradeApi(long handle);

    static native void log(long handle, int severity, String str);

    static native String getProperties(long h);

    static native String getMode(long h);

    static native void stop();

}
