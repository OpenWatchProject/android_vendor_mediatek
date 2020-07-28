package com.log.handler.connection;

import java.util.Observer;

/**
 * @author MTK81255
 *
 */
public interface ILogConnection {
    /**
     * @return boolean
     */
    boolean connect();

    /**
     * @return boolean
     */
    boolean isConnection();

    /**
     * @param data
     *            String
     * @return boolean
     */
    boolean sendToServer(String data);

    /**
     * @param sendData
     *            String
     * @return String
     */
    String getResponseFromServer(String sendData);

    /**
     * @param observer
     *            Observer
     */
    void addServerObserver(Observer observer);

    /**
     * @param observer
     *            Observer
     */
    void deleteServerObserver(Observer observer);

    /**
     *
     */
    void disConnect();
}
