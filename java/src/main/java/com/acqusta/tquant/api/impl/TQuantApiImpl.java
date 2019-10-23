package com.acqusta.tquant.api.impl;

public class TQuantApiImpl {

    public static void setParams(String key, String value)
    {
        TQuantApiJni.setParams(key, value);
    }
}
