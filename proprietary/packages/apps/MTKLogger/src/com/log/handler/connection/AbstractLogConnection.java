package com.log.handler.connection;

import com.log.handler.LogHandlerUtils;

import java.util.HashMap;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;

/**
 * @author MTK81255
 *
 */
public abstract class AbstractLogConnection extends Observable implements ILogConnection {
    private static final String TAG = LogHandlerUtils.TAG + "/AbstractLogConnection";

    protected String mServerName = "";
    private Map<String, String> mCommandAndResponsMap = new HashMap<String, String>();

    /**
     * @param serverName
     *            String
     */
    public AbstractLogConnection(String serverName) {
        mServerName = serverName;
    }

    @Override
    public abstract boolean connect();

    @Override
    public abstract boolean isConnection();

    @Override
    public synchronized boolean sendToServer(String data) {
        LogHandlerUtils.logi(TAG,
                "sendToServer() mServerName = " + mServerName + " data = " + data);
        mCommandAndResponsMap.put(data, "");
        if (!isConnection()) {
            boolean connectSuccess = connect();
            if (!connectSuccess) {
                LogHandlerUtils.logw(TAG, "Service is not connect & re-connect failed!");
                return false;
            }
        }
        return sendDataToServer(data);
    }

    protected abstract boolean sendDataToServer(String data);

    protected synchronized void setResponseFromServer(String serverData) {
        LogHandlerUtils.logi(TAG, "setResponseFromServer() mServerName = " + mServerName
                + " serverData = " + serverData);
        String commandData = serverData;
        for (String keyCommand : mCommandAndResponsMap.keySet()) {
            if (serverData.startsWith(keyCommand + ",")) {
                commandData = keyCommand;
                break;
            }
        }
        mCommandAndResponsMap.put(commandData, serverData);
        // Notify observer the server had new response.
        setChanged();
        notifyObservers(serverData);
    }

    @Override
    public synchronized String getResponseFromServer(String sendData) {
        return mCommandAndResponsMap.get(sendData);
    }

    @Override
    public void addServerObserver(Observer observer) {
        addObserver(observer);
    }

    @Override
    public void deleteServerObserver(Observer observer) {
        deleteObserver(observer);
    }

    @Override
    public synchronized void disConnect() {
        mCommandAndResponsMap.clear();
    }

}
