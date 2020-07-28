/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ims;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.Build;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
//import com.android.internal.telephony.gsm.GSMPhone;

import com.mediatek.ims.MtkImsConstants;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.InterruptedIOException;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicLong;
import java.util.ArrayList;
import vendor.mediatek.hardware.imsa.V1_0.IImsa;
import vendor.mediatek.hardware.imsa.V1_0.IImsaIndication;

import static com.mediatek.ims.VaConstants.*;

public class ImsAdapter extends Handler {

    private final static int MSG_IMSA_RETRY_CONNECT_SOCKET  = 1;
    private final static int MSG_IMSA_RETRY_IMS_ENABLE      = 2;
    private final static int MSG_IMSA_EVENT_PROXY_DEAD      = 3;
    private final static int IMSA_RETRY_SOCKET_TIME         = 500;
    private static final String MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    protected static final int IIMSA_GET_SERVICE_DELAY_MILLIS = 1 * 1000;

    public static class VaEvent {
        public static final int DEFAULT_MAX_DATA_LENGTH = 40960;

        private int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        private int request_id;
        private int data_len;
        private int read_offset;
        private byte data[];
        private int event_max_data_len = DEFAULT_MAX_DATA_LENGTH;

        /**
         * The VaEvent constructor with specified phone Id.
         *
         * @param phoneId the phone Id of the event
         * @param rid the request Id of the event
         */
        public VaEvent(int phoneId, int rid) {
            this(phoneId, rid, DEFAULT_MAX_DATA_LENGTH);
        }

        /**
         * The VaEvent constructor with specified phone Id.
         *
         * @param phoneId the phone Id of the event
         * @param rid the request Id of the event
         * @param length the max data length of the event
         */
        public VaEvent(int phoneId, int rid, int length) {
            mPhoneId = phoneId;
            request_id = rid;
            event_max_data_len = length;
            data = new byte[event_max_data_len];
            data_len = 0;
            read_offset = 0;
        }

        public int putInt(int value) {
            if (data_len > event_max_data_len - 4) {
                return -1;
            }

            synchronized (this) {
                for (int i = 0 ; i < 4 ; ++i) {
                    data[data_len] = (byte) ((value >> (8 * i)) & 0xFF);
                    data_len++;
                }
            }
            return 0;
        }

        public int putShort(int value) {
            if (data_len > event_max_data_len - 2) {
                return -1;
            }

            synchronized (this) {
                for (int i = 0 ; i < 2 ; ++i) {
                    data[data_len] = (byte) ((value >> (8 * i)) & 0xFF);
                    data_len++;
                }
            }

            return 0;
        }

        public int putByte(int value) {
            if (data_len > event_max_data_len - 1) {
                return -1;
            }

            synchronized (this) {
                data[data_len] = (byte) (value & 0xFF);
                data_len++;
            }

            return 0;
        }

        public int putString(String str, int len) {
            if (data_len > event_max_data_len - len) {
                return -1;
            }

            synchronized (this) {
                byte s[] = str.getBytes();
                if (len < str.length()) {
                    System.arraycopy(s, 0, data, data_len, len);
                    data_len += len;
                } else {
                    int remain = len - str.length();
                    System.arraycopy(s, 0, data, data_len, str.length());
                    data_len += str.length();
                    for (int i = 0 ; i < remain ; i++) {
                        data[data_len] = 0;
                        data_len++;
                    }
                }
            }

            return 0;
        }

        public int putBytes(byte [] value) {
            int len = value.length;

            if (len > event_max_data_len) {
                return -1;
            }

            synchronized (this) {
                System.arraycopy(value, 0, data, data_len, len);
                data_len += len;
            }

            return 0;
        }

        public int putDouble(double value) {
            if (data_len > event_max_data_len - 8) {
                return -1;
            }

            ByteBuffer buf = ByteBuffer.allocate(8);
            buf.putDouble(value);

            synchronized (this) {
                data[data_len++] = buf.get(7);
                data[data_len++] = buf.get(6);
                data[data_len++] = buf.get(5);
                data[data_len++] = buf.get(4);

                data[data_len++] = buf.get(3);
                data[data_len++] = buf.get(2);
                data[data_len++] = buf.get(1);
                data[data_len++] = buf.get(0);
            }
            return 0;
        }

        public int putLong(long value) {
            if (data_len > event_max_data_len - 8) {
                return -1;
            }

            ByteBuffer buf = ByteBuffer.allocate(8);
            buf.putLong(value);

            synchronized (this) {
                data[data_len++] = buf.get(7);
                data[data_len++] = buf.get(6);
                data[data_len++] = buf.get(5);
                data[data_len++] = buf.get(4);

                data[data_len++] = buf.get(3);
                data[data_len++] = buf.get(2);
                data[data_len++] = buf.get(1);
                data[data_len++] = buf.get(0);
            }
            return 0;
        }

        public byte [] getData() {
            return data;
        }

        public int getDataLen() {
            return data_len;
        }

        public int getRequestID() {
            return request_id;
        }

        public int getPhoneId() {
            return mPhoneId;
        }

        public int getInt() {
            int ret = 0;
            synchronized (this) {
                ret = ((data[read_offset + 3] & 0xff) << 24 | (data[read_offset + 2] & 0xff) << 16 | (data[read_offset + 1] & 0xff) << 8 | (data[read_offset] & 0xff));
                read_offset += 4;
            }
            return ret;
        }

        public int getShort() {
            int ret = 0;
            synchronized (this) {
                ret =  ((data[read_offset + 1] & 0xff) << 8 | (data[read_offset] & 0xff));
                read_offset += 2;
            }
            return ret;
        }

        // Notice: getByte is to get int8 type from VA, not get one byte.
        public int getByte() {
            int ret = 0;
            synchronized (this) {
                ret = (data[read_offset] & 0xff);
                read_offset += 1;
            }
            return ret;
        }

        public byte[] getBytes(int length) {
            if (length > data_len - read_offset) {
                return null;
            }

            byte[] ret = new byte[length];

            synchronized (this) {
                for (int i = 0 ; i < length ; i++) {
                    ret[i] = data[read_offset];
                    read_offset++;
                }
                return ret;
            }
        }

        public String getString(int len) {
            byte buf [] = new byte[len];

            synchronized (this) {
                System.arraycopy(data, read_offset, buf, 0, len);
                read_offset += len;
            }

            return (new String(buf)).trim();
        }

        public double getDouble() {
            byte[] buf = new byte[8];

            buf[0] = data[read_offset + 7];
            buf[1] = data[read_offset + 6];
            buf[2] = data[read_offset + 5];
            buf[3] = data[read_offset + 4];

            buf[4] = data[read_offset + 3];
            buf[5] = data[read_offset + 2];
            buf[6] = data[read_offset + 1];
            buf[7] = data[read_offset];

            read_offset += 8;
            return ByteBuffer.wrap(buf).getDouble();
        }
    }

    final class ImsaProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            Log.d("@M_" + TAG, "IMSA HIDL serviceDied");
            // todo: temp hack to send delayed message so that rild is back up by then
            mInstance.sendMessageDelayed(
                    mInstance.obtainMessage(MSG_IMSA_EVENT_PROXY_DEAD, mImsaProxyCookie.incrementAndGet()),
                    IIMSA_GET_SERVICE_DELAY_MILLIS);
        }
    }

    public class ImsaIndication extends IImsaIndication.Stub {
        private byte[] arrayListTobyte(ArrayList<Byte> data, int length) {
            byte[] byteList = new byte[length];
            for(int i = 0; i < length; i++)
            {
                byteList[i] = data.get(i);
            }
            Log.d("@M_" + TAG, "IMSA HIDL : arrayListTobyte, byteList = " + byteList);
            return byteList;
        }

        public void readEvent(ArrayList<Byte> data, int request_id, int length) {
            Log.d("@M_" + TAG, "readEvent : data = " + data + ", request_id = " + request_id + ", length = " + length);
            int requestId;
            int phoneId;
            byte buf [];
            VaEvent event;

            buf = arrayListTobyte(data, length);
            requestId = request_id;

            if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1)  > 1) {
                phoneId = (requestId & 0xFF000000) >> 24;
                Log.d("@M_" + TAG, "DUAL IMS Phone ID:" + phoneId);
            } else {
                phoneId = Util.getDefaultVoltePhoneId();
            }
            // remove Phone ID field in Request ID
            requestId = requestId & 0x00FFFFFF;

            event = new VaEvent(phoneId, requestId, length);
            event.putBytes(buf);
            // Confirm if event is null or not
            if (event != null) {
                Message msg = new Message();
                msg.obj = event;
                mImsEventDispatcher.sendMessage(msg);
            }
        }
    }

    public class VaSocketIO extends Thread {
        private byte buf[];

        private int mTyp = -1;
        private int mId  = -1;
        private LocalSocket mSocket = null;
        private OutputStream mOut = null;
        private DataInputStream mDin = null;
        //IMSA HIDL Proxy
        private volatile IImsa mImsaProxy = null;
        private int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;

        public VaSocketIO(IImsa imsa_proxy) {
            // TODO: buffer size confirm
            buf = new byte[8];
            Log.d("@M_" + TAG, "VaSocketIO(): Enter");
            mImsaProxy = imsa_proxy;
        }

        public void updateImsaProxy(IImsa imsa_proxy) {
            Log.d("@M_" + TAG, "VaSocketIO(): updateImsaProxy");
            mImsaProxy = imsa_proxy;
        }

        public void clearImsaProxy() {
            Log.d("@M_" + TAG, "VaSocketIO(): clearImsaProxy");
            mImsaProxy = null;
        }

        public void run() {
            Log.d("@M_" + TAG, "VaSocketIO(): Run");
        }

        private ArrayList<Byte> byteToArrayList(int length, byte [] value) {
            ArrayList<Byte> myarraylist = new ArrayList<Byte>();
            Log.d("@M_" + TAG, " byteToArrayList, value.length = " + value.length + ", value = " + value + ", length = " + length);
            for(int i = 0; i < length; i++)
            {
                myarraylist.add(value[i]);
            }
            return myarraylist;
        }

        public int writeEvent(VaEvent event) {
            Log.d("@M_" + TAG, "writeEvent Enter, phoneId = " + event.getPhoneId());
            int ret = -1;
            try {
                synchronized (this) {
                    if (mImsaProxy != null) {
                        if (event.getPhoneId() == SubscriptionManager.INVALID_PHONE_INDEX) {
                            Log.d("@M_" + TAG,
                                    "writeEvent event phoneId mismatch, event skipped. (event requestId="
                                            + event.getRequestID()
                                            + ", phoneId=" + event.getPhoneId()
                                            + ", socket phoneId=" + mPhoneId
                                            + ")");
                        } else {
                            int requestId = event.getRequestID();
                            dumpEvent(event);

                            if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1)  > 1) {
                                int phoneId = event.getPhoneId();
                                requestId = requestId | (phoneId << 24);
                                Log.d("@M_" + TAG, "DUAL IMS request ID:" + requestId);
                            }
                            mImsaProxy.writeEvent(requestId, event.getDataLen(), byteToArrayList(event.getDataLen(), event.getData()));
                            ret = 0;
                        }
                    } else {
                        Log.d("@M_" + TAG, "mImsaProxy is null, HIDL is not setup");
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
                return -1;
            }

            return ret;
        }

        private void dumpEvent(VaEvent event) {
            Log.d("@M_" + TAG, "dumpEvent: phone_id:" + event.getPhoneId()
                    + ",request_id:" + requestIdToString(event.getRequestID())
                    + ",data_len:" + event.getDataLen()
                    + ",event:" + event.getData());
        }
    }

    /* ImsAdapter class */
    private static final String SOCKET_NAME1 = "volte_imsm";

    private static final String TAG = "[ImsAdapter]";
    private Context mContext;
    //private GSMPhone mPhone;
    private VaSocketIO mIO;
    private static ImsEventDispatcher mImsEventDispatcher;
    //IMSA HIDL
    private volatile IImsa mImsaProxy = null;
    private final ImsaProxyDeathRecipient mImsaProxyDeathRecipient = new ImsaProxyDeathRecipient();
    private final AtomicLong mImsaProxyCookie = new AtomicLong(0);
    private ImsaIndication mImsaIndication = new ImsaIndication();;

    private static ImsAdapter mInstance;
    private static int mNumOfPhones = 0;
    private static boolean[] misImsAdapterEnabled;
    private static boolean mImsServiceUp = false;

    private boolean IS_USER_BUILD = "user".equals(Build.TYPE);
    private boolean IS_USERDEBUG_BUILD = "userdebug".equals(Build.TYPE);
    private boolean IS_ENG_BUILD = "eng".equals(Build.TYPE);

    private Object ImsEnabledThreadLock = new Object();

    public ImsAdapter(Context context) {

        mContext = context;

        if (mInstance == null) {
            mInstance = this;
        }

        Log.d("@M_" + TAG, "ImsAdapter(): ImsAdapter Enter");
        // new the mIO object to communicate with the va
        mIO = new VaSocketIO(getImsaProxy());
        mImsEventDispatcher = new ImsEventDispatcher(mContext, mIO);

        mNumOfPhones = TelephonyManager.getDefault().getPhoneCount();

        misImsAdapterEnabled = new boolean[mNumOfPhones];

        for (int i = 0; i < mNumOfPhones; i++) {
            misImsAdapterEnabled[i] = false;
        }

        mIO.start();
    }

    public IImsa getImsaProxy() {
        if (mImsaProxy != null) {
            return mImsaProxy;
        }
        try {
            mImsaProxy = IImsa.getService("imsa");
            Log.d("@M_" + TAG, "IImsa.getService");
            if (mImsaProxy != null) {
                mImsaProxy.linkToDeath(mImsaProxyDeathRecipient,
                        mImsaProxyCookie.incrementAndGet());
                Log.d("@M_" + TAG, "setResponseFunctions");
                mImsaProxy.setResponseFunctions(mImsaIndication);
            } else {
                Log.d("@M_" + TAG, "getImsaProxy: mImsaProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mImsaProxy = null;
            Log.d("@M_" + TAG, "ImsaProxy getService/setResponseFunctions: " + e);
        }
        if (mImsaProxy == null) {
            // if service is not up, treat it like death notification to try to get service again
            this.sendMessageDelayed(
                    this.obtainMessage(MSG_IMSA_EVENT_PROXY_DEAD,
                            mImsaProxyCookie.incrementAndGet()),
                    IIMSA_GET_SERVICE_DELAY_MILLIS);
        }
        return mImsaProxy;
    }

    private void resetProxyAndRequestList() {
        mImsaProxy = null;
        mIO.clearImsaProxy(); // reset mIO Imsa Proxy to null
        // increment the cookie so that death notification can be ignored
        mImsaProxyCookie.incrementAndGet();

        // todo: need to get service right away so setResponseFunctions() can be called for
        // unsolicited indications. getService() is not a blocking call, so it doesn't help to call
        // it here. Current hack is to call getService() on death notification after a delay.
    }

    public static ImsAdapter getInstance() {
        return mInstance;
    }

    public void enableImsAdapter(int phoneId) {
        Log.d("@M_" + TAG,
                "enableImsAdapter: misImsAdapterEnabled[phoneId]=" + misImsAdapterEnabled[phoneId] +
                ", phoneId = " + phoneId);

        synchronized (ImsEnabledThreadLock) {
            if (!misImsAdapterEnabled[phoneId]) {
                if (getImsaProxy() != null) {
                    Log.d("@M_" + TAG, "enalbeImsAdapter(): mImsaProxy success");

                    // start domain event dispatcher to recieve broadcast
                    VaEvent event = new VaEvent(phoneId, MSG_ID_IMS_ENABLE_IND);

                    Message msg = new Message();
                    msg.obj = event;
                    mImsEventDispatcher.sendMessage(msg);

                    misImsAdapterEnabled[phoneId] = true;
                    enableImsStack(phoneId);
                } else {
                    Log.d("@M_" + TAG, "enableImsAdapter(): mImsaProxy create fail");
                }
            }
        }
    }

    public boolean getImsAdapterEnable() {
        boolean isEnabled = false;

        for (int i = 0; i < misImsAdapterEnabled.length; i++) {
            if (misImsAdapterEnabled[i] == true) {
                isEnabled = true;
                break;
            }
        }

        return isEnabled;
    }

    public void disableImsAdapter(int phoneId, boolean isNormalDisable) {

        Log.d("@M_" + TAG,
                "disableImsAdapter(): misImsAdapterEnabled[phoneId]=" + misImsAdapterEnabled[phoneId] +
                ", isNormalDisable = " + isNormalDisable +
                ", phoneId = " + phoneId);

        synchronized (ImsEnabledThreadLock) {
            if(misImsAdapterEnabled[phoneId]) {
                misImsAdapterEnabled[phoneId] = false;

                disableImsStack(phoneId);

                VaEvent event = new VaEvent(phoneId, MSG_ID_IMS_DISABLE_IND);

                Message msg = new Message();
                msg.obj = event;
                mImsEventDispatcher.sendMessage(msg);
            }
        }
    }

    public void disableImsAdapterForAllPhone(boolean isNormalDisable) {

        if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) == 1){
            disableImsAdapter(Util.getDefaultVoltePhoneId(), isNormalDisable);
        } else {
            for (int i = 0; i < mNumOfPhones; i++) {
                disableImsAdapter(i, isNormalDisable);
            }
        }
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_IMSA_RETRY_IMS_ENABLE:
                Log.d("@M_" + TAG, "RETRY_IMS_ENABLE, phoneId:" + (int)msg.obj);
                enableImsAdapter((int)msg.obj);
                break;
            case MSG_IMSA_EVENT_PROXY_DEAD:
                Log.d("@M_" + TAG, "MSG_IMSA_EVENT_PROXY_DEAD");
                if ((long) msg.obj == mImsaProxyCookie.get()) {
                    resetProxyAndRequestList();
                    // todo: imsa should be back up since message was sent with a delay. this is
                    // a hack.
                    getImsaProxy();
                    if (mImsaProxy != null) {
                        // this API only be triggered when HIDL service died
                        mIO.updateImsaProxy(mImsaProxy);
                    }
                }
                break;

            default:
                Log.d("@M_" + TAG, "handleMessage receive unsupported message: " + msg.what);
                break;
        }
    }

    // for AP side UT, set event and call ImsAdapter.sendTestEvent(event)
    public void sendTestEvent(VaEvent event) {
        // Sample Code:
        // new the event object for Test Event
        // VaEvent event = new VaEvent(MSG_ID_IMSA_IMCB_TEST_A);
        // event.putInt(2);
        // event.putInt(3);
        mImsEventDispatcher.dispatchCallback(event);
    }

    public void ImsServiceUp() {
        mImsServiceUp = true;
        Log.d("@M_" + TAG, "ImsServiceUp, start to ACTION_IMS_SERVICE_UP intent");
    }

    public boolean getImsServiceUp() {
        return mImsServiceUp;
    }

    /**
     * This is a utility class for ImsAdapter related work.
     */
    public static class Util {

        /**
         * To get current the default Volte Phone Id.
         * Only for single 4G DSDS project, and it should always align to the 4G phone Id.
         *
         * @return current default Volte Phone Id. (align to 4G phone Id)
         */
        public static int getDefaultVoltePhoneId() {
            int phoneId =
                    SystemProperties.getInt(MtkImsConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
            if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
                phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            }
            return phoneId;
        }
    }

    private void enableImsStack(int phoneId) {

        // Send IMS Enable to IMSM
        VaEvent event = new VaEvent(phoneId, MSG_ID_IMS_ENABLE_IND);
        mIO.writeEvent(event);

        return;
    }

    private void disableImsStack(int phoneId) {

        // Send IMS Disable to IMSM
        VaEvent event = new VaEvent(phoneId, MSG_ID_IMS_DISABLE_IND);
        mIO.writeEvent(event);

        return;
    }

    public static String requestIdToString(int requestId) {
        switch (requestId) {
            case MSG_ID_WRAP_IMSPA_IMSM_INFORMATION_IND:
                return "MSG_ID_WRAP_IMSPA_IMSM_INFORMATION_IND";
            case MSG_ID_WRAP_IMSM_IMSPA_INFORMATION_REQ:
                return "MSG_ID_WRAP_IMSM_IMSPA_INFORMATION_REQ";
            case MSG_ID_IMS_ENABLE_IND: return "MSG_ID_IMS_ENABLE_IND";
            case MSG_ID_IMS_DISABLE_IND: return "MSG_ID_IMS_DISABLE_IND";

            /* Data */
            case MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
                return "MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ";
            case MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_ACK_RESP:
                return "MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_ACK_RESP";
            case MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_REJ_RESP:
                return "MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_REJ_RESP";
            case MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ:
                return "MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ";
            case MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_ACK_RESP:
                return "MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_ACK_RESP";
            case MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_REJ_RESP:
                return "MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_REJ_RESP";
            case MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_IND:
                return "MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_IND";

            /* others */
            case MSG_ID_NOTIFY_XUI_IND: return "MSG_ID_NOTIFY_XUI_IND";
            case MSG_ID_NOTIFY_SS_PROGRESS_INDICATION:
                return "MSG_ID_NOTIFY_SS_PROGRESS_INDICATION";
            case MSG_ID_REQUEST_PCSCF_DISCOVERY: return "MSG_ID_REQUEST_PCSCF_DISCOVERY";
            case MSG_ID_RESPONSE_PCSCF_DISCOVERY: return "MSG_ID_RESPONSE_PCSCF_DISCOVERY";
            case MSG_ID_REJECT_PCSCF_DISCOVERY: return "MSG_ID_REJECT_PCSCF_DISCOVERY";

            default:
                return Integer.toString(requestId);
        }
    }
}

