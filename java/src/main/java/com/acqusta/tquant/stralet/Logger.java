package com.acqusta.tquant.stralet;

public interface Logger {
    void info(String format, Object...args);
    void warn(String format, Object...args);
    void error(String format, Object...args);
    void fatal(String format, Object...args);
}
