package com.mediatek.ims.internal;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Build;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.util.Log;
import android.text.TextUtils;

import com.android.ims.ImsManager;

import com.mediatek.ims.ImsAdapter.VaEvent;
import com.mediatek.ims.ImsAdapter.VaSocketIO;
import com.mediatek.ims.ImsConstants;
import com.mediatek.ims.ImsEventDispatcher;
import com.mediatek.ims.ImsService;
import com.mediatek.ims.ImsUtImpl;
import com.mediatek.ims.MtkImsService;
import com.mediatek.ims.VaConstants;

import java.io.UnsupportedEncodingException;

/**
 * IMS Simserv dispatcher class.
 * To handle IMS event for Simserv from ImsEventDispatcher
 *
 */
public class ImsSimservsDispatcher implements ImsEventDispatcher.VaEventDispatcher {
    private static final String TAG = "ImsSimservsDispatcher";
    private static final boolean DUMP_TRANSACTION = true;
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");

    private static final int IMC_MAX_XUI_LEN = 512;

    private static ImsSimservsDispatcher sInstance;

    private Context mContext;
    private VaSocketIO mSocket;

    private Phone mPhone;

    private Handler mHandler;
    private Thread mHandlerThread = new Thread() {
        @Override
        public void run() {
            Looper.prepare();
            mHandler = new Handler() { // create handler here
                @Override
                public void handleMessage(Message msg) {
                    if (msg.obj instanceof VaEvent) {
                        VaEvent event = (VaEvent) msg.obj;
                        log("ImsSimservsDispatcher receives request [" + msg.what + ", "
                                + event.getDataLen() + "]");
                        switch (msg.what) {
                            case VaConstants.MSG_ID_NOTIFY_XUI_IND:
                                handleXuiUpdate(event);
                                break;
                            default:
                                log("ImsSimservsDispatcher receives unhandled message [" + msg.what
                                        + "]");
                        }
                    }
                }
            };
            Looper.loop();
        }
    };

    /**
     * Constructor.
     *
     * @param context   Context
     * @param io        VaSocketIO
     *
     */
    public ImsSimservsDispatcher(Context context, VaSocketIO io) {
        mContext = context;
        mSocket = io;
        sInstance = this;
        mHandlerThread.start();
    }

    public static ImsSimservsDispatcher getInstance() {
        return sInstance;
    }

    /**
     * Enable request.
     *
     */
    public void enableRequest(int phoneId) {
    }

    /**
     * Disable Request.
     *
     */
    public void disableRequest(int phoneId) {
    }

    /**
     * Event callback for ImsEventDispatcher.
     *
     * @param event     VaEvent
     *
     */
    public void vaEventCallback(VaEvent event) {
        // relay to main thread to keep receiver and callback handler is working
        // under the same thread
        mHandler.sendMessage(mHandler.obtainMessage(event.getRequestID(), event));
    }

    public void setSocket(VaSocketIO socket) {
        // this method is used for testing
        // we could set a dummy socket used to verify the response
        mSocket = socket;
    }

    private void sendVaEvent(VaEvent event) {
        log("ImsSimservsDispatcher send event [" + event.getRequestID() + ", " + event.getDataLen()
                + "]");
        mSocket.writeEvent(event);
    }

    private void handleXuiUpdate(VaEvent event) {
        /*
         * typedef struct {
         * imcf_uint32 xui_len;
         * imcf_uint8 xui[IMC_MAX_XUI_LEN];
         * } imsa_imcb_xui_ind_struct;
         * imcf_uint32 ==unsigned
         * int IMX_MAX_XUI_LEN == 512
         */
        int length = event.getInt();
        byte[] byteArray = event.getBytes(length);
        String xui = null;
        int phoneId = event.getPhoneId();

        if (byteArray == null) {
            log("handleXuiUpdate event.getBytes() = null");
            return;
        }

        try {
            xui = new String(byteArray, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }

        log("handleXuiUpdate xui=" + ImsUtImpl.encryptString(xui));

        ImsXuiManager xuim = ImsXuiManager.getInstance();
        xuim.setXui(phoneId, xui);

        ImsService imsService = ImsService.getInstance(mContext);
        if (imsService != null) {
            imsService.updateSelfIdentity(phoneId);
        }
    }

    private static void log(String text) {
        Log.d("@M_" + TAG, "[ims] ImsSimservsDispatcher " + text);
    }
}
