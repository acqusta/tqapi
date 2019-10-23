package com.acqusta.tquant.stralet;

import com.acqusta.tquant.api.DataApi;
import com.acqusta.tquant.api.TradeApi;

import java.time.LocalDateTime;
import java.util.HashMap;

public interface StraletContext {
    /**
     *
     * @return
     */
    Logger getLogger();

    /**
     *
     * @return
     */
    HashMap<String, Object> getProps();

    /**
     *
     * @return
     */
    int getTradingDay();

    /**
     *
     * @return
     */
    FinDateTime getCurTime();

    /**
     *
     * @return
     */
    LocalDateTime getCurDateTime();

    /**
     *
     * @param evt
     * @param data
     */
    void postEvent(String evt, long data);

    /**
     *
     * @param id
     * @param delay
     * @param data
     */
    void setTimer(long id, long delay, long data);

    /**
     *
     * @param id
     */
    void killTimer(long id);

    /**
     *
     * @return
     */
    DataApi getDataApi();

    /**
     *
     * @return
     */
    TradeApi getTradeApi();

    /**
     *
     * @return
     */
    String getMode();

    /**
     *
     */
    void stop();
}
