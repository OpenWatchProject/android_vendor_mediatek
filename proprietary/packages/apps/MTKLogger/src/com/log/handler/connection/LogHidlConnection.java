package com.log.handler.connection;

import android.os.HwBinder;
import android.os.RemoteException;

import com.log.handler.LogHandlerUtils;

import java.util.NoSuchElementException;

import vendor.mediatek.hardware.log.V1_0.ILog;
import vendor.mediatek.hardware.log.V1_0.ILogCallback;

/**
 * Class which will communicate with native layer. Send command to native by local socket, then
 * monitor response code, and feed that back to user
 */
public class LogHidlConnection extends AbstractLogConnection {
    private static final String TAG = LogHandlerUtils.TAG + "/LogHidlConnection";

    private ILog mLogHIDLService;

    /**
     * @param serverName
     *            String
     */
    public LogHidlConnection(String serverName) {
        super(serverName);
    }

    @Override
    public boolean connect() {
        try {
            LogHandlerUtils.logi(TAG, "LogHIDLConnection serverName = " + mServerName);
            mLogHIDLService = ILog.getService(mServerName);
            mLogHIDLService.setCallback(mLogCallback);
            mLogHIDLService.linkToDeath(mHidlDeathRecipient, 0);
            LogHandlerUtils.logi(TAG, "mLogHIDLService.setCallback() done!");
        } catch (RemoteException e) {
            e.printStackTrace();
            disConnect();
            return false;
        } catch (NoSuchElementException e) {
            e.printStackTrace();
            disConnect();
            return false;
        }
        return true;
    }

    @Override
    public boolean isConnection() {
        return mLogHIDLService != null;
    }

    @Override
    protected boolean sendDataToServer(String data) {
        LogHandlerUtils.logd(TAG,
                "sendDataToServer() mServerName = " + mServerName + ", data = " + data);
        boolean sendSuccess = false;
        try {
            Thread.sleep(50);
            sendSuccess = mLogHIDLService.sendToServer(data);
        } catch (RemoteException e) {
            LogHandlerUtils.loge(TAG, "RemoteException while sending command to native.", e);
            disConnect();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        LogHandlerUtils.logd(TAG, "sendToServer done! sendSuccess = " + sendSuccess);
        return sendSuccess;
    }

    @Override
    public void disConnect() {
        mLogHIDLService = null;
        super.disConnect();
    }

    HidlDeathRecipient mHidlDeathRecipient = new HidlDeathRecipient();

    /**
     * @author MTK81255
     *
     */
    class HidlDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            LogHandlerUtils.logi(TAG, "serviceDied! cookie = " + cookie);
            disConnect();
        }
    }

    private ILogCallback mLogCallback = new ILogCallback.Stub() {
        @Override
        public boolean callbackToClient(String data) throws RemoteException {
            LogHandlerUtils.logw(TAG, "callbackToClient data = " + data);
            setResponseFromServer(data);
            return true;
        }
    };

}
