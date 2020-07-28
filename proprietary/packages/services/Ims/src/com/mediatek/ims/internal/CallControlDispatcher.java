package com.mediatek.ims.internal;

import android.content.Context;
import android.content.Intent;
import android.telephony.Rlog;
import android.text.TextUtils;
import android.os.Build;
import android.os.SystemProperties;
import android.support.v4.content.LocalBroadcastManager;

import com.mediatek.ims.ImsAdapter.VaSocketIO;
import com.mediatek.ims.ImsAdapter.VaEvent;
import com.mediatek.ims.ImsConstants;
import com.mediatek.ims.ImsEventDispatcher;
import com.mediatek.ims.ImsServiceCallTracker;
import static com.mediatek.ims.VaConstants.*;



public class CallControlDispatcher implements ImsEventDispatcher.VaEventDispatcher {

    private Context mContext;
    private VaSocketIO mSocket;
    private static final String TAG = "[CallControlDispatcher]";

    // Sensitive log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private static final int IMC_PROGRESS_NOTIFY_CONFERENCE = 257;
    private static final int IMC_PROGRESS_NOTIFY_MWI = 258;
    private static final int IMC_PROGRESS_NOTIFY_DIALOG = 256;

    public CallControlDispatcher(Context context, VaSocketIO IO) {
        mContext = context;
        mSocket = IO;
    }

    public void enableRequest(int phoneId) {
        Rlog.d(TAG, "enableRequest()");
    }

    public void disableRequest(int phoneId) {
        Rlog.d(TAG, "disableRequest()");
    }

    public void vaEventCallback(VaEvent event) {
        try {
            int requestId;
            int len;
            int callId;
            int phoneId;
            int serviceId;
            String data;
            byte[] byteData;

            requestId = event.getRequestID();
            len = event.getDataLen();
            callId = event.getInt();
            phoneId = event.getPhoneId();
            serviceId = event.getInt();
            byteData = event.getBytes(4000);
            data  = new String(byteData);
            len = getDataLength(byteData, 4000);

            if (!SENLOG || TELDBG) {
                Rlog.d(TAG, "requestId = " + requestId + ", length = " + len +
                        ", callId = " + callId + ", phoneId = " + phoneId +
                        ", serviceId = " + serviceId + ", data = " +
                        ImsServiceCallTracker.sensitiveEncode(data.substring(0, len)));
            } else {
                Rlog.d(TAG, "requestId = " + requestId + ", length = " + len +
                        ", callId = " + callId + ", phoneId = " + phoneId +
                        ", serviceId = " + serviceId + ", data = [hidden]");
            }

            Intent intent;
            switch (serviceId) {
                case IMC_PROGRESS_NOTIFY_MWI:
                    /* Send to APP directly */
                    intent = new Intent(ImsConstants.ACTION_LTE_MESSAGE_WAITING_INDICATION);
                    intent.putExtra(ImsConstants.EXTRA_LTE_MWI_BODY, data);
                    intent.putExtra(ImsConstants.EXTRA_PHONE_ID, phoneId);
                    mContext.sendBroadcast(intent,
                            ImsConstants.PERMISSION_READ_LTE_MESSAGE_WAITING_INDICATION);
                    Rlog.d(TAG, "Message Waiting Message is sent.");
                    break;
                case IMC_PROGRESS_NOTIFY_CONFERENCE:
                    /* Send to ImsConferenceHandler */
                    intent = new Intent(ImsConstants.ACTION_IMS_CONFERENCE_CALL_INDICATION);
                    intent.putExtra(ImsConstants.EXTRA_MESSAGE_CONTENT, data.substring(0, len));
                    intent.putExtra(ImsConstants.EXTRA_CALL_ID, callId);
                    intent.putExtra(ImsConstants.EXTRA_PHONE_ID, phoneId);
                    LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
                    Rlog.d(TAG, "Conference call XML message is sent.");
                    break;
                case IMC_PROGRESS_NOTIFY_DIALOG:
                    intent = new Intent(ImsConstants.ACTION_IMS_DIALOG_EVENT_PACKAGE);
                    intent.putExtra(ImsConstants.EXTRA_DEP_CONTENT, data.substring(0, len));
                    intent.putExtra(ImsConstants.EXTRA_CALL_ID, callId);
                    intent.putExtra(ImsConstants.EXTRA_PHONE_ID, phoneId);
                    LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
                    Rlog.d(TAG, "Dialog event package intent is sent.");
                    break;
                default:
                    Rlog.d(TAG, "Unkonwn serviceId: " + serviceId);
                    break;
            }


        } catch (Exception e) {
            e.printStackTrace();
        } /* Enf of try */
    }

    /* Caculate the data length except 0x0 */
    private int getDataLength(byte[] data, int originLen) {
       int i;
       for (i = 0; i < originLen; i++) {
          if (data[i] == 0x0) {
             return i;
          }
       }

       return i;
    }
}
