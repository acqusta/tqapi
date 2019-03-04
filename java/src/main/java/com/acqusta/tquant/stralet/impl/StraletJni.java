package com.acqusta.tquant.stralet.impl;

import com.acqusta.tquant.api.impl.TQuantApiJni;
import com.acqusta.tquant.stralet.Stralet;
import com.acqusta.tquant.stralet.StraletCreator;

public class StraletJni {
    static {
        TQuantApiJni.init();
    }

    public static native long create(Stralet stralet);
    public static native void destry(long h);

    public static native void runBacktest(String cfg, StraletCreator straletCreator);
    public static native void runRealTime(String cfg, StraletCreator straletCreator);
}
