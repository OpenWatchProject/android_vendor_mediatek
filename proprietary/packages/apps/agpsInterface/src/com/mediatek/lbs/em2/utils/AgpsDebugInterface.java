package com.mediatek.lbs.em2.utils;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.util.Calendar;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import static com.mediatek.lbs.em2.utils.DataCoder2.getInt;
import static com.mediatek.lbs.em2.utils.DataCoder2.getString;
import static com.mediatek.lbs.em2.utils.DataCoder2.getBinary;

import vendor.mediatek.hardware.lbs.V1_0.ILbs;
import vendor.mediatek.hardware.lbs.V1_0.ILbsCallback;
import android.os.RemoteException;
import android.os.HwBinder;
import java.util.ArrayList;
import java.util.Arrays;
import com.mediatek.lbs.em2.utils.DataCoder2.DataCoderBuffer;

public class AgpsDebugInterface {

    protected static final int DEBUG_MGR_MESSAGE = 0;
    protected static final int DEBUG_MGR_STATE = 1;

    protected static final int DEBUG_MGR_MESSAGE_TOAST = (1 << 0);
    protected static final int DEBUG_MGR_MESSAGE_VIEW = (1 << 1);
    protected static final int DEBUG_MGR_MESSAGE_DIALOG = (1 << 2);
    protected static final int DEBUG_MGR_MESSAGE_VZW_DEBUG_MSG = (1 << 3);
    protected static final int DEBUG_MGR_MESSAGE_TOAST_VIEW = DEBUG_MGR_MESSAGE_TOAST
            | DEBUG_MGR_MESSAGE_VIEW;

    protected final static String SOCKET_ADDRESS = "agpsd3";

    protected DataCoderBuffer in = new DataCoderBuffer(8 * 1024);
    protected MessageHandler messageHandler;

    public AgpsDebugInterface(AgpsDebugListener listener) throws IOException {
        if (listener == null) {
            throw new RuntimeException("listener cannot be null");
        }
        messageHandler = new MessageHandler(listener);
        connect();
    }


    //HIDL start
    public static ArrayList<Byte> convertByteArrayToArrayList(byte[] data, int size) {
        if (data == null) {
            return null;
        }
        int max = (size < data.length)? size : data.length;
        ArrayList<Byte> ret = new ArrayList<Byte>();
        for (int i = 0; i < max; i++) {
            ret.add(data[i]);
        }
        return ret;
    }

    public static void covertArrayListToByteArray(ArrayList<Byte> in, byte[] out) {
        for(int i = 0; i < in.size() && i < out.length; i++) {
            out[i] = in.get(i);
        }
    }

    ILbs mLbsHidlClient;

    LbsHidlCallback mLbsHidlCallback = new LbsHidlCallback();
    class LbsHidlCallback extends ILbsCallback.Stub {
        public boolean callbackToClient(ArrayList<Byte> data) {
            log("callbackToClient() size=" + data.size());
            //TODO callback to user
            covertArrayListToByteArray(data, in.mBuff);
            in.clear();
            while(in.mOffset < data.size()) {
                try {
                    int type = getInt(in);
                    int data1 = getInt(in);
                    int data2 = getInt(in);
                    int data3 = getInt(in);
                    String msg1 = getString(in);
                    String msg2 = getString(in);
                    byte[] bin = getBinary(in);
                    sendMessage2Handler(type, data1, data2, data3, msg1, msg2, bin);
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }
            return true;
        }
    };

    LbsHidlDeathRecipient mLLbsHidlDeathRecipient = new LbsHidlDeathRecipient();
    class LbsHidlDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            log("serviceDied");
            mLbsHidlClient = null;
            boolean ret = false;
            while(!ret) {
                msleep(500);
                ret = doHidl("AgpsDebugInterface");
            }
        }
    }

    private boolean doHidl(String name) {
        try {
            if(mLbsHidlClient == null) {
                mLbsHidlClient = ILbs.getService(name);
                mLbsHidlClient.linkToDeath(mLLbsHidlDeathRecipient, 0);
                mLbsHidlClient.setCallback(mLbsHidlCallback);
            }
            return true;
        } catch (RemoteException | RuntimeException e) {
            e.printStackTrace();
        }
        return false;
    }
    //HIDL end

    public void removeListener() {
        write(0xff);
    }

    protected static class MessageHandler extends Handler {
        protected AgpsDebugListener listener;

        public MessageHandler(AgpsDebugListener listener) {
            this.listener = listener;
        }

        @Override
        public void handleMessage(Message msg) {
            Bundle b = msg.getData();

            int type = b.getInt("type");
            int data1 = b.getInt("data1");
            int data2 = b.getInt("data2");
            int data3 = b.getInt("data3");
            String msg1 = b.getString("msg1");
            String msg2 = b.getString("msg2");
            byte[] bin = b.getByteArray("bin");

            switch (type) {
            case DEBUG_MGR_MESSAGE: {
                if ((data1 & DEBUG_MGR_MESSAGE_TOAST) == DEBUG_MGR_MESSAGE_TOAST) {
                    listener.onToastMessageReceived(msg2);
                }
                if ((data1 & DEBUG_MGR_MESSAGE_VIEW) == DEBUG_MGR_MESSAGE_VIEW) {
                    listener.onViewMessageReceived(data2, msg2);
                }
                if ((data1 & DEBUG_MGR_MESSAGE_DIALOG) == DEBUG_MGR_MESSAGE_DIALOG) {
                    listener.onDialogMessageReceived(msg1, msg2);
                }
                if ((data1 & DEBUG_MGR_MESSAGE_VZW_DEBUG_MSG) == DEBUG_MGR_MESSAGE_VZW_DEBUG_MSG) {
                    listener.onVzwDebugMessageReceived(msg2);
                }
                break;
            }
            case DEBUG_MGR_STATE: {
                log("type=" + type + " data1=" + data1 + " data2=" + data2
                        + " data3=" + data3 + " msg1=" + msg1 + " msg2=" + msg2
                        + " bin=" + bin);
                if (bin != null) {
                    log("bin.len=" + bin.length);
                }
                break;
            }
            default: {
                break;
            }
            }
        }
    }

    protected void sendMessage2Handler(int type, int data1, int data2,
            int data3, String msg1, String msg2, byte[] bin) {
        Bundle b = new Bundle();
        b.putInt("type", type);
        b.putInt("data1", data1);
        b.putInt("data2", data2);
        b.putInt("data3", data3);
        b.putString("msg1", msg1);
        b.putString("msg2", msg2);
        b.putByteArray("bin", bin);
        Message msg = Message.obtain();
        msg.what = 0;
        msg.setData(b);
        messageHandler.sendMessage(msg);
    }

    protected boolean retryConnectToAgpsd() {
        // wait 20 seconds
        for (int i = 0; i < 100; i++) {
            msleep(200);
            try {
                log("i=" + i + " debug connecting..");
                connect();
                log("i=" + i + " debug connecting success");
                return true;
            } catch (IOException e1) {
                log("i=" + i + " debug connecting failure");
                continue;
            }
        }
        return false;
    }

    protected void channelCloseAction(int type, String message) {
        logw("debug port is closed reason=[" + message + "]");
        sendMessage2Handler(DEBUG_MGR_MESSAGE, type, 0xff0000, 0, null, message, null);
    }

    protected void write(int data) {
        //HIDL write
        try {
            ArrayList<Byte> list = new ArrayList<Byte>();
            list.add((byte) 0xff);
            mLbsHidlClient.sendToServer(list);
        } catch (RemoteException | RuntimeException e) {
            e.printStackTrace();
        }
    }

    protected void connect() throws IOException {
        doHidl("AgpsDebugInterface");
    }

    protected void close() {
    }

    // ============================================================
    protected static void msleep(int milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    protected static void logw(Object msg) {
        Log.w("AgpsDebugInterface", "[agps] WARNING: " + msg);
    }

    protected static void log(Object msg) {
        Log.d("AgpsDebugInterface", "" + msg);
    }

    protected static void loge(Object msg) {
        Log.d("AgpsDebugInterface", "[agps] ERR:" + msg);
    }

    protected static String getTimeString() {
        String str = "";
        Calendar c = Calendar.getInstance();
        str += String.format("%d/%02d/%02d [%02d:%02d:%02d.%03d] ",
                c.get(Calendar.YEAR), (c.get(Calendar.MONTH) + 1),
                c.get(Calendar.DAY_OF_MONTH), c.get(Calendar.HOUR_OF_DAY),
                c.get(Calendar.MINUTE), c.get(Calendar.SECOND),
                c.get(Calendar.MILLISECOND));
        return str;
    }
}
