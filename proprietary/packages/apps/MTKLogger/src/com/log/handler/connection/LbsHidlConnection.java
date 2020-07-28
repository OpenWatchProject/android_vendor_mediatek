package com.log.handler.connection;

import android.os.HwBinder;
import android.os.RemoteException;

import com.log.handler.LogHandlerUtils;

import java.util.ArrayList;
import java.util.NoSuchElementException;

import vendor.mediatek.hardware.lbs.V1_0.ILbs;
import vendor.mediatek.hardware.lbs.V1_0.ILbsCallback;

/**
 * Class which will communicate with native layer. Send command to native by local socket, then
 * monitor response code, and feed that back to user
 */
public class LbsHidlConnection extends AbstractLogConnection {
    private static final String TAG = LogHandlerUtils.TAG + "/LbsHidlConnection";

    private ILbs mLbsHIDLService;
    private ILbs mLbsHIDLCallback;

    /**
     * @param serverName
     *            String
     */
    public LbsHidlConnection(String serverName) {
        super(serverName);
    }

    @Override
    public boolean connect() {
        try {
            LogHandlerUtils.logi(TAG, "LbsHidlConnection serverName = " + mServerName);
            mLbsHIDLService = ILbs.getService(mServerName); // mtk_mtklogger2mnld
            // mLbsHIDLService.setCallback(mLbsCallback);
            mLbsHIDLService.linkToDeath(mHidlDeathRecipient, 0);

            mLbsHIDLCallback = ILbs.getService("mtk_mnld2mtklogger"); // mtk_mnld2mtklogger
            mLbsHIDLCallback.setCallback(mLbsCallback);
            mLbsHIDLCallback.linkToDeath(mHidlDeathRecipient, 0);
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
        return mLbsHIDLService != null && mLbsHIDLCallback != null;
    }

    @Override
    protected boolean sendDataToServer(String data) {
        LogHandlerUtils.logd(TAG,
                "sendDataToServer() mServerName = " + mServerName + ", data = " + data);
        boolean sendSuccess = false;
        try {
            Thread.sleep(50);
            sendSuccess = mLbsHIDLService.sendToServer(covertStringToArrayList(data));
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
        mLbsHIDLService = null;
        mLbsHIDLCallback = null;
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

    private ILbsCallback mLbsCallback = new ILbsCallback.Stub() {
        @Override
        public boolean callbackToClient(ArrayList<Byte> data) throws RemoteException {
            String dataStr = covertArrayListToString(data);
            LogHandlerUtils.logw(TAG, "callbackToClient data = " + dataStr);
            setResponseFromServer(dataStr);
            return true;
        }
    };

    private String covertArrayListToString(ArrayList<Byte> dataList) {
        byte[] bytes = new byte[dataList.size()];
        for (int i = 0; i < dataList.size(); i++) {
            bytes[i] = dataList.get(i);
        }
        return new String(bytes);
    }

    private ArrayList<Byte> covertStringToArrayList(String dataStr) {
        byte[] bytes = dataStr.getBytes();
        ArrayList<Byte> dataList = new ArrayList<Byte>();
        for (byte bytee : bytes) {
            dataList.add(bytee);
        }
        return dataList;
    }

}
