/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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


package com.mediatek.ims.ril;

import static com.android.internal.telephony.RILConstants.RADIO_NOT_AVAILABLE;
import static com.android.internal.telephony.RILConstants.RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_ANSWER;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_CONFERENCE;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_DIAL;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_DTMF;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_DTMF_START;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_DTMF_STOP;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_EXPLICIT_CALL_TRANSFER;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_GET_ACTIVITY_INFO;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_HANGUP;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_LAST_CALL_FAIL_CAUSE;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SET_MUTE;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SHUTDOWN;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;
import static com.android.internal.telephony.RILConstants.RIL_RESPONSE_ACKNOWLEDGEMENT;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_QUERY_FACILITY_LOCK;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SET_FACILITY_LOCK;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_CHANGE_BARRING_PASSWORD;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SET_CALL_FORWARD;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_QUERY_CALL_FORWARD_STATUS;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_QUERY_CALL_WAITING;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SET_CALL_WAITING;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_GET_CLIR;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SET_CLIR;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_QUERY_CLIP;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SEPARATE_CONNECTION;
import static com.android.internal.telephony.RILConstants.RIL_UNSOL_SUPP_SVC_NOTIFICATION;


// SMS-START
import android.hardware.radio.V1_0.CdmaSmsAck;
import android.hardware.radio.V1_0.CdmaSmsMessage;
import android.hardware.radio.V1_0.ImsSmsMessage;
import android.hardware.radio.V1_0.GsmSmsMessage;
import com.android.internal.telephony.nano.TelephonyProto.SmsSession;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.SmsConstants;
// SMS-END
import android.hardware.radio.V1_0.IRadio;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.TimeZone;
import java.util.Vector;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import java.util.NoSuchElementException;

import vendor.mediatek.hardware.mtkradioex.V1_0.ConferenceDial;
import vendor.mediatek.hardware.mtkradioex.V1_0.CallForwardInfoEx;
import vendor.mediatek.hardware.mtkradioex.V1_6.IMtkRadioEx;
import vendor.mediatek.hardware.mtkradioex.V1_0.VendorSetting;

import android.content.Context;
import android.hardware.radio.V1_0.Dial;
import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioIndicationType;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.RadioResponseType;
import android.hardware.radio.V1_0.UusInfo;
import android.net.ConnectivityManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.WorkSource;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ModemActivityInfo;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.TelephonyHistogram;
import android.telephony.ims.ImsReasonInfo;
import android.util.SparseArray;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.ClientWakelockTracker;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.UUSInfo;
import com.android.internal.telephony.metrics.TelephonyMetrics;

import com.mediatek.ims.ImsServiceCallTracker;
import com.mediatek.ims.ImsCallInfo;
import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.internal.telephony.MtkCallForwardInfo;

// for External component
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ExtensionPluginFactory;
import com.mediatek.ims.plugin.impl.ImsSelfActivatorBase;
import com.mediatek.ims.plugin.impl.ImsCallPluginBase;

import android.telephony.TelephonyManager;

/**
 * {@hide}
 */
class RILRequest {
    static final String LOG_TAG = "IMS-RilRequest";

    //***** Class Variables
    static Random sRandom = new Random();
    static AtomicInteger sNextSerial = new AtomicInteger(0);
    private static Object sPoolSync = new Object();
    private static RILRequest sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 4;

    //***** Instance Variables
    int mSerial;
    int mRequest;
    Message mResult;
    RILRequest mNext;
    int mWakeLockType;
    WorkSource mWorkSource;
    String mClientId;
    // time in ms when RIL request was made
    long mStartTimeMs;

    /**
     * Retrieves a new RILRequest instance from the pool.
     *
     * @param request RIL_REQUEST_*
     * @param result sent when operation completes
     * @return a RILRequest instance from the pool.
     */
    static RILRequest obtain(int request, Message result) {
        RILRequest rr = null;

        synchronized(sPoolSync) {
            if (sPool != null) {
                rr = sPool;
                sPool = rr.mNext;
                rr.mNext = null;
                sPoolSize--;
            }
        }

        if (rr == null) {
            rr = new RILRequest();
        }

        rr.mSerial = sNextSerial.getAndIncrement();

        rr.mRequest = request;
        rr.mResult = result;


        rr.mWakeLockType = RIL.INVALID_WAKELOCK;
        rr.mWorkSource = null;
        rr.mStartTimeMs = SystemClock.elapsedRealtime();
        if (result != null && result.getTarget() == null) {
            throw new NullPointerException("Message target must not be null");
        }
        return rr;
    }
    /**
     * Retrieves a new RILRequest instance from the pool and sets the clientId
     *
     * @param request RIL_REQUEST_*
     * @param result sent when operation completes
     * @param workSource WorkSource to track the client
     * @return a RILRequest instance from the pool.
     */
    static RILRequest obtain(int request, Message result, WorkSource workSource) {
        RILRequest rr = null;

        rr = obtain(request, result);
        if(workSource != null) {
            rr.mWorkSource = workSource;
            rr.mClientId = String.valueOf(workSource.get(0)) + ":" + workSource.getName(0);
        } else {
            Rlog.e(LOG_TAG, "null workSource " + request);
        }

        return rr;
    }

    /**
     * Returns a RILRequest instance to the pool.
     *
     * Note: This should only be called once per use.
     */
    void release() {
        synchronized (sPoolSync) {
            if (sPoolSize < MAX_POOL_SIZE) {
                mNext = sPool;
                sPool = this;
                sPoolSize++;
                mResult = null;
                if(mWakeLockType != RIL.INVALID_WAKELOCK) {
                    if(mWakeLockType == RIL.FOR_WAKELOCK) {
                        Rlog.e(LOG_TAG, "RILRequest releasing with held wake lock: "
                                + serialString());
                    }
                }
            }
        }
    }

    private RILRequest() {
    }

    static void
    resetSerial() {
        // use a random so that on recovery we probably don't mix old requests
        // with new.
        sNextSerial.set(sRandom.nextInt());
    }

    String
    serialString() {
        //Cheesy way to do %04d
        StringBuilder sb = new StringBuilder(8);
        String sn;

        long adjustedSerial = (((long)mSerial) - Integer.MIN_VALUE)%10000;

        sn = Long.toString(adjustedSerial);

        //sb.append("J[");
        sb.append('[');
        for (int i = 0, s = sn.length() ; i < 4 - s; i++) {
            sb.append('0');
        }

        sb.append(sn);
        sb.append(']');
        return sb.toString();
    }

    void
    onError(int error, Object ret) {
        CommandException ex;

        ex = CommandException.fromRilErrno(error);

        if (ImsRILAdapter.IMS_RILA_LOGD) Rlog.d(LOG_TAG, serialString() + "< "
            + ImsRILAdapter.requestToString(mRequest)
            + " error: " + ex + " ret=" + ImsRILAdapter.retToString(mRequest, ret));

        if (mResult != null) {
            AsyncResult.forMessage(mResult, ret, ex);
            mResult.sendToTarget();
        }
    }
}



/**
 * IMS RIL Adapter implementation.
 *
 * {@hide}
 */
public final class ImsRILAdapter extends ImsBaseCommands implements ImsCommandsInterface {

    static final String IMSRIL_LOG_TAG = "IMS_RILA";
    // Have a separate wakelock instance for Ack
    static final String RILJ_ACK_WAKELOCK_NAME = "IMSRIL_ACK_WL";
    static final boolean IMSRIL_LOGD = true;
    static final boolean IMSRIL_LOGV = false; // STOPSHIP if true
    static final int RIL_HISTOGRAM_BUCKET_COUNT = 5;

    /**
     * Wake lock timeout should be longer than the longest timeout in
     * the vendor ril.
     */
    private static final int DEFAULT_WAKE_LOCK_TIMEOUT_MS = 60000;

    // Wake lock default timeout associated with ack
    private static final int DEFAULT_ACK_WAKE_LOCK_TIMEOUT_MS = 200;

    // Variables used to differentiate ack messages from request while calling clearWakeLock()
    public static final int INVALID_WAKELOCK = -1;
    public static final int FOR_WAKELOCK = 0;
    public static final int FOR_ACK_WAKELOCK = 1;
    private final ClientWakelockTracker mClientWakelockTracker = new ClientWakelockTracker();
    //***** Instance Variables
    Context mContext;

    final WakeLock mWakeLock;           // Wake lock associated with request/response
    final WakeLock mAckWakeLock;        // Wake lock associated with ack sent
    final int mWakeLockTimeout;
    final int mAckWakeLockTimeout;      // Timeout associated with ack sent
    // The number of wakelock requests currently active.  Don't release the lock
    // until dec'd to 0
    int mWakeLockCount;

    // Variables used to identify releasing of WL on wakelock timeouts
    volatile int mWlSequenceNum = 0;
    volatile int mAckWlSequenceNum = 0;
    SparseArray<RILRequest> mRequestList = new SparseArray<RILRequest>();
    static SparseArray<TelephonyHistogram> mRilTimeHistograms = new
            SparseArray<TelephonyHistogram>();

    Object[]     mLastNITZTimeInfo;

    // When we are testing emergency calls
    AtomicBoolean mTestingEmergencyCall = new AtomicBoolean(false);

    final Integer mPhoneId;

    /* default work source which will blame phone process */
    private WorkSource mRILDefaultWorkSource;

    /* Worksource containing all applications causing wakelock to be held */
    private WorkSource mActiveWakelockWorkSource;

    /** Telephony metrics instance for logging metrics event */
    private TelephonyMetrics mMetrics = TelephonyMetrics.getInstance();

    /**
     * Property to override DEFAULT_WAKE_LOCK_TIMEOUT
     */
    static final String PROPERTY_WAKE_LOCK_TIMEOUT = "ro.ril.wake_lock_timeout";
    // DTMF Queue
    private DtmfQueueHandler mDtmfReqQueue = new DtmfQueueHandler();

    // WWOP Extension APIs
    private OpImsCommandsInterface mOpCI;

    boolean mIsMobileNetworkSupported;
    RadioResponseImpl mRadioResponse;
    RadioIndicationImpl mRadioIndication;
    ImsRadioResponse mImsRadioResponse;
    ImsRadioIndication mImsRadioIndication;
    volatile IRadio mRadioProxy = null;
    volatile IMtkRadioEx mMtkRadioProxy = null;
    final AtomicLong mRadioProxyCookie = new AtomicLong(0);
    final AtomicLong mMtkRadioProxyCookie = new AtomicLong(0);
    final RadioProxyDeathRecipient mRadioProxyDeathRecipient;
    final MtkRadioProxyDeathRecipient mMtkRadioProxyDeathRecipient;
    final RilHandler mRilHandler;


    //***** Events
    static final int EVENT_SEND                      = 1;
    static final int EVENT_WAKE_LOCK_TIMEOUT         = 2;
    static final int EVENT_ACK_WAKE_LOCK_TIMEOUT     = 4;
    static final int EVENT_BLOCKING_RESPONSE_TIMEOUT = 5;
    static final int EVENT_RADIO_PROXY_DEAD          = 6;
    static final int EVENT_MTK_RADIO_PROXY_DEAD      = 7;
    static final int EVENT_TRIGGER_TO_FIRE_PENDING_URC = 8;

    //***** Constants
    // IMS MTK
    static final String [] MTK_IMS_HIDL_SERVICE_NAME =
        {"imsSlot1", "imsSlot2", "imsSlot3","imsSlot4"};
    // IMS AOSP
    static final String [] IMS_HIDL_SERVICE_NAME =
        {"imsAospSlot1", "imsAospSlot2", "imsAospSlot3","imsAospSlot4"};

    static final int IRADIO_GET_SERVICE_DELAY_MILLIS = 4 * 1000;
    static final boolean IMS_RILA_LOGD = true;

    /**
     * Get Telephony RIL Timing Histograms
     * @return
     */
    public static List<TelephonyHistogram> getTelephonyRILTimingHistograms() {
        List<TelephonyHistogram> list;
        synchronized (mRilTimeHistograms) {
            list = new ArrayList<TelephonyHistogram>(mRilTimeHistograms.size());
            for (int i = 0; i < mRilTimeHistograms.size(); i++) {
                TelephonyHistogram entry = new TelephonyHistogram(mRilTimeHistograms.valueAt(i));
                list.add(entry);
            }
        }
        return list;
    }

    // DTML Queue for 'request will be ignored when duplicated sending'
    class DtmfQueueHandler {
        public class DtmfQueueRR {
            public RILRequest rr;
            public Object[] params;
            public DtmfQueueRR(RILRequest rr, Object[] params) {
                this.rr = rr;
                this.params = params;
            }
        }
        public DtmfQueueHandler() {
            mDtmfStatus = DTMF_STATUS_STOP;
        }
        public void start() {
            mDtmfStatus = DTMF_STATUS_START;
        }
        public void stop() {
            mDtmfStatus = DTMF_STATUS_STOP;
        }
        public boolean isStart() {
            return (mDtmfStatus == DTMF_STATUS_START);
        }
        public void add(DtmfQueueRR o) {
            mDtmfQueue.addElement(o);
        }
        public void remove(DtmfQueueRR o) {
            mDtmfQueue.remove(o);
        }
        public void remove(int idx) {
            mDtmfQueue.removeElementAt(idx);
        }
        public DtmfQueueRR get() {
            return (DtmfQueueRR) mDtmfQueue.get(0);
        }
        public int size() {
            return mDtmfQueue.size();
        }
        public void setPendingRequest(DtmfQueueRR r) {
            mPendingCHLDRequest = r;
        }
        public DtmfQueueRR getPendingRequest() {
            return mPendingCHLDRequest;
        }
        public void setSendChldRequest() {
            mIsSendChldRequest = true;
        }
        public void resetSendChldRequest() {
            mIsSendChldRequest = false;
        }
        public boolean hasSendChldRequest() {
            riljLog("mIsSendChldRequest = " + mIsSendChldRequest);
            return mIsSendChldRequest;
        }
        public final int MAXIMUM_DTMF_REQUEST = 32;
        private final boolean DTMF_STATUS_START = true;
        private final boolean DTMF_STATUS_STOP = false;
        private boolean mDtmfStatus = DTMF_STATUS_STOP;
        private Vector<DtmfQueueRR> mDtmfQueue = new Vector<DtmfQueueRR>(MAXIMUM_DTMF_REQUEST);
        private DtmfQueueRR mPendingCHLDRequest = null;
        private boolean mIsSendChldRequest = false;
        public DtmfQueueHandler.DtmfQueueRR buildDtmfQueueRR(RILRequest rr, Object[] param) {
            if (rr == null) {
                return null;
            }
            if (IMS_RILA_LOGD) {
                riljLog("DtmfQueueHandler.buildDtmfQueueRR build ([" + rr.mSerial + "] reqId="
                        + rr.mRequest + ")");
            }
            return new DtmfQueueHandler.DtmfQueueRR(rr, param);
        }
    }

    class RilHandler extends Handler {
        //***** Handler implementation
        @Override public void
        handleMessage(Message msg) {
            RILRequest rr;

            switch (msg.what) {
                case EVENT_WAKE_LOCK_TIMEOUT:
                    // Haven't heard back from the last request.  Assume we're
                    // not getting a response and  release the wake lock.

                    // The timer of WAKE_LOCK_TIMEOUT is reset with each
                    // new send request. So when WAKE_LOCK_TIMEOUT occurs
                    // all requests in mRequestList already waited at
                    // least DEFAULT_WAKE_LOCK_TIMEOUT_MS but no response.
                    //
                    // Note: Keep mRequestList so that delayed response
                    // can still be handled when response finally comes.

                    synchronized (mRequestList) {
                        if (msg.arg1 == mWlSequenceNum && clearWakeLock(FOR_WAKELOCK)) {
                            if (IMSRIL_LOGD) {
                                int count = mRequestList.size();
                                Rlog.d(IMSRIL_LOG_TAG, "WAKE_LOCK_TIMEOUT " +
                                        " mRequestList=" + count);
                                for (int i = 0; i < count; i++) {
                                    rr = mRequestList.valueAt(i);
                                    Rlog.d(IMSRIL_LOG_TAG, i + ": [" + rr.mSerial + "] "
                                            + requestToString(rr.mRequest));
                                }
                            }
                        }
                    }
                    break;

                case EVENT_ACK_WAKE_LOCK_TIMEOUT:
                    if (msg.arg1 == mAckWlSequenceNum && clearWakeLock(FOR_ACK_WAKELOCK)) {
                        if (IMSRIL_LOGV) {
                            Rlog.d(IMSRIL_LOG_TAG, "ACK_WAKE_LOCK_TIMEOUT");
                        }
                    }
                    break;

                case EVENT_BLOCKING_RESPONSE_TIMEOUT:
                    int serial = msg.arg1;
                    rr = findAndRemoveRequestFromList(serial);
                    // If the request has already been processed, do nothing
                    if(rr == null) {
                        break;
                    }

                    // build a response if expected
                    if (rr.mResult != null) {
                        Object timeoutResponse = getResponseForTimedOutRILRequest(rr);
                        AsyncResult.forMessage( rr.mResult, timeoutResponse, null);
                        rr.mResult.sendToTarget();
                        mMetrics.writeOnRilTimeoutResponse(mPhoneId, rr.mSerial, rr.mRequest);
                    }

                    decrementWakeLock(rr);
                    rr.release();
                    break;

                case EVENT_RADIO_PROXY_DEAD:
                    riljLog("handleMessage: EVENT_RADIO_PROXY_DEAD cookie = " + msg.obj +
                            " mRadioProxyCookie = " + mRadioProxyCookie.get());
                    if ((Long) msg.obj == mRadioProxyCookie.get()) {
                        resetProxyAndRequestList();

                        // todo: rild should be back up since message was sent with a delay. this is
                        // a hack.
                        getRadioProxy(null);
                    }
                    break;
                case EVENT_MTK_RADIO_PROXY_DEAD:
                    riljLog("handleMessage: EVENT_MTK_RADIO_PROXY_DEAD cookie = " + msg.obj +
                            " mMtkRadioProxyCookie = " + mMtkRadioProxyCookie.get());
                    if ((Long) msg.obj == mMtkRadioProxyCookie.get()) {
                        resetMtkProxyAndRequestList();

                        // todo: rild should be back up since message was sent with a delay. this is
                        // a hack.
                        IMtkRadioEx mtkProxy = getMtkRadioProxy(null);
                        if (mtkProxy != null) {
                            // Trigger to fire pending URC
                            notifyImsServiceReady();
                        } else {
                            mRilHandler.sendMessage(
                                    mRilHandler.obtainMessage(EVENT_TRIGGER_TO_FIRE_PENDING_URC));
                        }
                    }
                    break;
                case EVENT_TRIGGER_TO_FIRE_PENDING_URC:
                    IMtkRadioEx mtkProxy = getMtkRadioProxy(null);
                    if (mtkProxy != null) {
                        // Trigger to fire pending URC
                        riljLog("Trigger to fire pending URC " + mPhoneId);
                        notifyImsServiceReady();
                    } else {
                        // Delay 300ms and trigger again
                        mRilHandler.sendMessageDelayed(
                                    mRilHandler.obtainMessage(EVENT_TRIGGER_TO_FIRE_PENDING_URC),
                                    300);
                    }
                    break;
            }
        }
    }

    /**
     * In order to prevent calls to Telephony from waiting indefinitely
     * low-latency blocking calls will eventually time out. In the event of
     * a timeout, this function generates a response that is returned to the
     * higher layers to unblock the call. This is in lieu of a meaningful
     * response.
     * @param rr The RIL Request that has timed out.
     * @return A default object, such as the one generated by a normal response
     * that is returned to the higher layers.
     **/
    private static Object getResponseForTimedOutRILRequest(RILRequest rr) {
        if (rr == null ) return null;

        Object timeoutResponse = null;
        switch(rr.mRequest) {
            case RIL_REQUEST_GET_ACTIVITY_INFO:
                timeoutResponse = new ModemActivityInfo(
                        0, 0, 0, new int [ModemActivityInfo.TX_POWER_LEVELS], 0, 0);
                break;
        };
        return timeoutResponse;
    }

    final class RadioProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            riljLog("serviceDied");
            // todo: temp hack to send delayed message so that rild is back up by then
            //mRilHandler.sendMessage(mRilHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie));
            mRilHandler.sendMessage(mRilHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie));
        }
    }

    final class MtkRadioProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            riljLog("MtkRadioProxyDeathRecipient, serviceDied");
            // todo: temp hack to send delayed message so that rild is back up by then
            //mRilHandler.sendMessage(mRilHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie));
            mRilHandler.sendMessage(mRilHandler.obtainMessage(EVENT_MTK_RADIO_PROXY_DEAD, cookie));
        }
    }

    private void resetProxyAndRequestList() {
        riljLogi("resetProxyAndRequestList");
        mRadioProxy = null;

        // increment the cookie so that death notification can be ignored
        mRadioProxyCookie.incrementAndGet();

        setRadioState(TelephonyManager.RADIO_POWER_UNAVAILABLE);

        RILRequest.resetSerial();
        // Clear request list on close
        clearRequestList(RADIO_NOT_AVAILABLE, false);

        // todo: need to get service right away so setResponseFunctions() can be called for
        // unsolicited indications. getService() is not a blocking call, so it doesn't help to call
        // it here. Current hack is to call getService() on death notification after a delay.
    }

    private IRadio getRadioProxy(Message result) {
        if (!mIsMobileNetworkSupported) {
            if (IMSRIL_LOGV) riljLog("getRadioProxy: Not calling getService(): wifi-only");
            return null;
        }

        if (mRadioProxy != null) {
            return mRadioProxy;
        }

        try {
            try {
                mRadioProxy = android.hardware.radio.V1_4.IRadio.getService(
                        IMS_HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId], false);
            } catch (NoSuchElementException e) {
            }

            if (mRadioProxy == null) {
                try {
                    mRadioProxy = android.hardware.radio.V1_3.IRadio.getService(
                            IMS_HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId], false);
                } catch (NoSuchElementException e) {
                }
            }

            if (mRadioProxy == null) {
                try {
                    mRadioProxy = android.hardware.radio.V1_2.IRadio.getService(
                            IMS_HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId], false);
                } catch (NoSuchElementException e) {
                }
            }

            if (mRadioProxy == null) {
                try {
                    mRadioProxy = android.hardware.radio.V1_1.IRadio.getService(
                            IMS_HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId], false);
                } catch (NoSuchElementException e) {
                }
            }

            if (mRadioProxy == null) {
                try {
                    mRadioProxy = android.hardware.radio.V1_0.IRadio.getService(
                            IMS_HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId], false);
                } catch (NoSuchElementException e) {
                }
            }
            riljLogi("getRadioProxy: " + mRadioProxy);
            if (mRadioProxy != null) {
                mRadioProxy.linkToDeath(mRadioProxyDeathRecipient,
                        mRadioProxyCookie.incrementAndGet());
                mRadioProxy.setResponseFunctions(mRadioResponse, mRadioIndication);
                riljLogi("setResponseFunctionsIms");
                // DTMF Handling
                if (mDtmfReqQueue != null) {
                    synchronized (mDtmfReqQueue) {
                        int i;
                        if (IMS_RILA_LOGD) riljLog("queue size  " + mDtmfReqQueue.size());
                        for (i = mDtmfReqQueue.size() - 1; i >= 0; i--) {
                            mDtmfReqQueue.remove(i);
                        }
                        if (IMS_RILA_LOGD) riljLog("queue size  after " + mDtmfReqQueue.size());
                        if (mDtmfReqQueue.getPendingRequest() != null) {
                            riljLog("reset pending switch request");

                            DtmfQueueHandler.DtmfQueueRR pendingDqrr = mDtmfReqQueue
                                    .getPendingRequest();
                            RILRequest pendingRequest = pendingDqrr.rr;
                            if (pendingRequest.mResult != null) {
                                AsyncResult.forMessage(pendingRequest.mResult, null, null);
                                pendingRequest.mResult.sendToTarget();
                            }

                            mDtmfReqQueue.resetSendChldRequest();
                            mDtmfReqQueue.setPendingRequest(null);
                        }
                    }
                }
            } else {
                riljLoge("getRadioProxy: mRadioProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mRadioProxy = null;
            riljLoge("RadioProxy getService/setResponseFunctions: " + e);
        }

        if (mRadioProxy == null) {
            if (result != null) {
                AsyncResult.forMessage(result, null,
                        CommandException.fromRilErrno(RADIO_NOT_AVAILABLE));
                result.sendToTarget();
            }
        }

        return mRadioProxy;
    }

    //***** Constructors

    public ImsRILAdapter(Context context, int instanceId) {
        super(context, instanceId);
        if (IMS_RILA_LOGD) {
            riljLogi("Ims-RIL: init phone = " + instanceId);
        }

        mContext = context;
        mPhoneId = instanceId;

        ConnectivityManager cm = (ConnectivityManager)context.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        mIsMobileNetworkSupported = cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE);

        mRadioResponse = new RadioResponseImpl(this, instanceId);
        mRadioIndication = new RadioIndicationImpl(this, instanceId);
        mImsRadioResponse = new ImsRadioResponse(this, instanceId);
        mImsRadioIndication = new ImsRadioIndication(this, instanceId);

        mRilHandler = new RilHandler();
        mRadioProxyDeathRecipient = new RadioProxyDeathRecipient();
        mMtkRadioProxyDeathRecipient = new MtkRadioProxyDeathRecipient();

        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, IMSRIL_LOG_TAG);
        mWakeLock.setReferenceCounted(false);
        mAckWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, RILJ_ACK_WAKELOCK_NAME);
        mAckWakeLock.setReferenceCounted(false);
        mWakeLockTimeout = SystemProperties.getInt(TelephonyProperties.PROPERTY_WAKE_LOCK_TIMEOUT,
                DEFAULT_WAKE_LOCK_TIMEOUT_MS);
        mAckWakeLockTimeout = SystemProperties.getInt(
                TelephonyProperties.PROPERTY_WAKE_LOCK_TIMEOUT, DEFAULT_ACK_WAKE_LOCK_TIMEOUT_MS);
        mWakeLockCount = 0;
        mRILDefaultWorkSource = new WorkSource(context.getApplicationInfo().uid,
                context.getPackageName());

        // set radio callback; needed to set RadioIndication callback (should be done after
        // wakelock stuff is initialized above as callbacks are received on separate binder threads)
        IRadio proxy = getRadioProxy(null);
        if (IMS_RILA_LOGD) {
            riljLogi("Ims-RIL: proxy = " + (proxy == null));
        }

        IMtkRadioEx mtkProxy = getMtkRadioProxy(null);
        if (IMS_RILA_LOGD) {
            riljLog("Mtk-Ims-RIL: proxy = " + (mtkProxy == null));
        }
        // Create WWOP Command Interfaces
        mOpCI = OpImsRILUtil.makeCommandInterface(context, instanceId);
    }

    private void addRequest(RILRequest rr) {
        acquireWakeLock(rr, FOR_WAKELOCK);
        synchronized (mRequestList) {
            rr.mStartTimeMs = SystemClock.elapsedRealtime();
            mRequestList.append(rr.mSerial, rr);
        }
    }

    private RILRequest obtainRequest(int request, Message result, WorkSource workSource) {
        RILRequest rr = RILRequest.obtain(request, result, workSource);
        addRequest(rr);
        return rr;
    }

    private void handleRadioProxyExceptionForRR(RILRequest rr, String caller, Exception e) {
        riljLoge(caller + ": " + e);
        resetProxyAndRequestList();
    }

    private void handleMtkRadioProxyExceptionForRR(RILRequest rr,
            String caller, Exception e) {
        riljLoge(caller + ": " + e);
        resetMtkProxyAndRequestList();
    }

    /**
     * Convert Null String to Empty String
     * AOSP Implementation
     * @param string
     * @return
     */
    protected String convertNullToEmptyString(String string) {
        return string != null ? string : "";
    }

    /// Public APIs /////////////////////////////////////////////////////////////////////
    /**
     * Get IMS WWOP Command Interface
     * @return
     */
    @Override
    public OpImsCommandsInterface getOpCommandsInterface() {
        return mOpCI;
    }

    /**
     * Set Volume Mute
     * @param enableMute Enable Mute functionalities
     * @param result
     */
    @Override
    public void setMute(boolean enableMute, Message result) {

        IRadio radioProxy = getRadioProxy(null);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_MUTE, null,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " mute = "
                        + enableMute);
            }

            try {
                radioProxy.setMute(rr.mSerial, enableMute);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setMute", e);
            }
        }
    }

    /**
    * Dial VoLTE or ViLTE call.
    *
    * @param callee dialing number string
    * @param clirMode to present the dialing number or not
    * @param isEmergency indicate is emergency call or not
    * @param isVideoCall indicate is belong to vilte call or volte call
    * @param result command result
    *
    */

    @Override
    public void start(String callee, ImsCallProfile callProfile, int clirMode, boolean isEmergency,
            boolean isVideoCall, Message result) {

        if (isVideoCall) {
            vtDial(callee, clirMode, null, result);
        } else if (isEmergency) {
            emergencyDial(callee, callProfile, clirMode, null, result);
        } else {
            dial(callee, clirMode, result);
        }
    }

    /**
    * Dial VvoLTE or ViLTE conference call
    *
    * @param participants participants dialing number string
    * @param clirMode to present the dialing number or not
    * @param isVideoCall indicate is belong to vilte call or volte call
    * @param result command result
    *
    */

    @Override
    public void startConference(String[] participants, int clirMode, boolean isVideoCall,
                                Message result) {

        conferenceDial(participants, clirMode, isVideoCall, result);
    }

    /**
     * Accept a call
     */
    @Override
    public void accept(Message response) {

        IRadio radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_ANSWER, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }
            try {
                radioProxy.acceptCall(rr.mSerial);
                mMetrics.writeRilAnswer(mPhoneId, rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "acceptCall", e);
            }
        }
    }

    /**
     * Accept video call.
     *
     * @param videoMode indicate to the accept video call as video, audio, video_rx, or video_tx.
     * @param callId indicate which call we want to accept.
     * @param response Response Object
     *
     */
    @Override
    public void acceptVideoCall(int videoMode, int callId, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_VIDEO_CALL_ACCEPT, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " videoMode = "
                        + videoMode + " callId = " + callId);
            }
            try {
                radioProxy.videoCallAccept(rr.mSerial, videoMode, callId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "acceptCall", e);
            }
        }
    }

    /**
     * Approve ECC redial or not
     *
     * @param approve 0: disapprove, 1: approve
     * @param callId Call ID
     * @param response Response Object
     *
     */
    @Override
    public void approveEccRedial(int approve, int callId, Message response) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_ECC_REDIAL_APPROVE, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " approve = "
                        + approve + " callId = " + callId);
            }
            try {
                radioProxy.eccRedialApprove(rr.mSerial, approve, callId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "approveEccRedial", e);
            }
        }
    }

    /**
     * Hangup call
     * @param callId Call Id
     * @param response Response Object
     */
    @Override
    public void hangup(int callId, Message response) {

        IRadio radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_HANGUP, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " callId = " + callId);
            }
            try {
                radioProxy.hangup(rr.mSerial, callId);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "hangup", e);
            }
        }
    }

    /**
     * hangupWithReason call
     * @param callId Call Id
     * @param response Response Object
     */
    @Override
    public void hangup(int callId, int reason, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {

            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_HANGUP_WITH_REASON, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " callId = " + callId + "reason="
                        + reason);
            }

            try {
                 radioProxy.hangupWithReason(rr.mSerial, callId, reason);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "hangupWithReason", e);
            }
        }
    }

    /**
    * Explicit call transfer.
    * @param result command result
    */
    @Override
    public void explicitCallTransfer(Message response) {

        IRadio radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(
                    RIL_REQUEST_EXPLICIT_CALL_TRANSFER, response, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            handleChldRelatedRequest(rr, null);
        }
    }

    /**
    * Unattended call transfer.
    * @param result command response
    */
    @Override
    public void unattendedCallTransfer(String number, int type, Message response) {
        internalImsEct(number, type, response);
    }

    /**
     * To hold the call.
     * @param callId toIndicate which call session to hold.
     * @param result command result.
     */
    @Override
    public void hold(int callId, Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_HOLD_CALL, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + "callId = " + callId);
            }

            try {
                radioProxy.controlCall(rr.mSerial, ImsRILConstants.CONTROL_CALL_HOLD, callId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "holdCall", e);
            }
        }
    }

    /**
     * To resume the call.
     * @param callId toIndicate which call session to resume.
     * @param result command result.
     */
    @Override
    public void resume(int callId, Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_RESUME_CALL, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + "callId = " + callId);
            }

            try {
                radioProxy.controlCall(rr.mSerial, ImsRILConstants.CONTROL_CALL_RESUME, callId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "resumeCall", e);
            }
        }

    }

    /**
     * Conference Call
     * @param response Response Object
     */
    @Override
    public void conference(Message response) {

        IRadio radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_CONFERENCE, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            handleChldRelatedRequest(rr, null);
        }
    }

    /**
     * Send DTMF Key
     * @param c should be one of 0-9, '*' or '#'. Other values will be
     * @param result is present the command is OK or fail
     */
    @Override
    public void sendDtmf(char c, Message result) {

        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_DTMF, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                // Do not log function arg for privacy
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.sendDtmf(rr.mSerial, c + "");
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "sendDtmf", e);
            }
        }
    }

    /**
     * Start to paly a DTMF tone on the active call. Ignored if there is no active call
     * or there is a playing DTMF tone.
     * @param c should be one of 0-9, '*' or '#'. Other values will be
     * @param result is present the command is OK or fail
     */
    @Override
    public void startDtmf(char c, Message result) {

        synchronized (mDtmfReqQueue) {
            if (!mDtmfReqQueue.hasSendChldRequest()
                    && mDtmfReqQueue.size() < mDtmfReqQueue.MAXIMUM_DTMF_REQUEST) {
                if (!mDtmfReqQueue.isStart()) {
                    IRadio radioProxy = getRadioProxy(result);
                    if (radioProxy != null) {
                        RILRequest rr = obtainRequest(RIL_REQUEST_DTMF_START, result,
                                mRILDefaultWorkSource);
                        mDtmfReqQueue.start();
                        Object[] param = { c };
                        DtmfQueueHandler.DtmfQueueRR dqrr = mDtmfReqQueue.buildDtmfQueueRR(rr,
                                param);
                        mDtmfReqQueue.add(dqrr);
                        if (mDtmfReqQueue.size() == 1) {
                            riljLog("send start dtmf");
                            // Do not log function arg for privacy
                            if (IMS_RILA_LOGD)
                                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                            sendDtmfQueueRR(dqrr);
                        }
                    }
                } else {
                    riljLog("DTMF status conflict, want to start DTMF when status is "
                            + mDtmfReqQueue.isStart());
                }
            }
        }
    }

    /**
     * Stop the playing DTMF tone. Ignored if there is no playing DTMF
     * DTMF request will be ignored when the count of requests reaches 32
     * tone or no active call.
     * @param result is present the command is OK or fail
     */
    @Override
    public void stopDtmf(Message result) {

        synchronized (mDtmfReqQueue) {
            if (!mDtmfReqQueue.hasSendChldRequest()
                    && mDtmfReqQueue.size() < mDtmfReqQueue.MAXIMUM_DTMF_REQUEST) {
                if (mDtmfReqQueue.isStart()) {
                    IRadio radioProxy = getRadioProxy(result);
                    if (radioProxy != null) {
                        RILRequest rr = obtainRequest(RIL_REQUEST_DTMF_STOP, result,
                                mRILDefaultWorkSource);
                        mDtmfReqQueue.stop();
                        Object[] param = null;
                        DtmfQueueHandler.DtmfQueueRR dqrr = mDtmfReqQueue.buildDtmfQueueRR(rr,
                                param);
                        mDtmfReqQueue.add(dqrr);
                        if (mDtmfReqQueue.size() == 1) {
                            riljLog("send stop dtmf");
                            if (IMS_RILA_LOGD)
                                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                            sendDtmfQueueRR(dqrr);
                        }
                    }
                } else {
                    riljLog("DTMF status conflict, want to start DTMF when status is "
                            + mDtmfReqQueue.isStart());
                }
            }
        }
    }

    /**
     * Set Call Indication
     * @param mode
     * @param callId
     * @param seqNum
     * @param response
     */
    @Override
    public void setCallIndication(int mode, int callId, int seqNum, int cause, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_CALL_INDICATION, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " mode = " + mode
                        + " callId = " + callId + " seqNum = " + seqNum + " cause = " + cause);
            }

            try {
                radioProxy.setCallIndication(rr.mSerial, mode, callId, seqNum, cause);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setCallIndication", e);
            }
        }
    }

    /**
     * De-register IMS with no cause <br/>
     * Cause Id will be set to 1
     * @param response response object
     */
    @Override
    public void deregisterIms(Message response) {

        // De-register reason is always 1, if no cause available
        deregisterImsWithCause(1, response);
    }

    // For VOLTE SS with cause.
    /**
     * De-register IMS with no cause <br/>
     * For VOLTE SS with cause
     * @param response response object
     */
    @Override
    public void deregisterImsWithCause(int cause, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_DEREG_NOTIFICATION , response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.imsDeregNotification(rr.mSerial, cause);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "imsDeregNotification", e);
            }
        }
    }

    /**
     * Turn On/OFF IMS Features <br/>
     * After MD 93 Architecture
     * @param params Configuration Parameters
     * @param result
     */
    @Override
    public void setImsCfg(int[] params, Message response) {

        boolean volteEnable = (params[0] == 1) ? true : false;
        boolean vilteEnable = (params[1] == 1) ? true : false;
        boolean vowifiEnable = (params[2] == 1) ? true : false;
        boolean viwifiEnable = (params[3] == 1) ? true : false;
        boolean smsEnable = (params[4] == 1) ? true : false;
        boolean eimsEnable = (params[5] == 1) ? true : false;

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMSCFG, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " volteEnable = " + params[0]
                        + " vilteEnable = " + params[1]
                        + " vowifiEnable = " + params[2]
                        + " viwifiEnable = " + params[3]
                        + " smsEnable = " + params[4]
                        + " eimsEnable = " + params[5]);
            }

            try {
                radioProxy.setImscfg(rr.mSerial, volteEnable,
                                                 vilteEnable,
                                                 vowifiEnable,
                                                 viwifiEnable,
                                                 smsEnable,
                                                 eimsEnable);
                if (ImsCommonUtil.supportMdAutoSetupIms()) {
                    findAndRemoveRequestFromList(rr.mSerial);
                    riljLog(rr.serialString()
                            + "<  " + requestToString(rr.mRequest) + " sent and removed");
                }

            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsCfg", e);
            }
        }
    }

    /**
     * Update multiple Ims config to modem.
     * @param keys as multiple Ims config keys.
     * @param values as multiple Ims config values.
     * @param phoneId as which phone to config
     */
    @Override
    public void setModemImsCfg(String keys, String values, int type, Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_MD_IMSCFG, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " keys = " + keys
                        + " values = " + values
                        + " type = " + type);
            }

            try {
                radioProxy.setModemImsCfg(rr.mSerial, keys, values, type);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "sendModemImsCfg", e);
            }
        }
    }

    /**
     * Turn On IMS
     * @param result
     */
    @Override
    public void turnOnIms(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = ON");
            }

            try {
                radioProxy.setImsEnable(rr.mSerial, true);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsEnable", e);
            }
        }
    }

    /**
     * Turn off IMS
     * @param result
     */
    @Override
    public void turnOffIms(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.setImsEnable(rr.mSerial, false);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsEnable", e);
            }
        }

    }
    /**
     * Turn on VoLTE
     * @param result
     */
    @Override
    public void turnOnVolte(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_VOLTE_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = ON");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_VOLTE_ENABLE,
                        Integer.toString(1));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn off VoLTE
     * @param result
     */
    @Override
    public void turnOffVolte(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_VOLTE_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_VOLTE_ENABLE,
                        Integer.toString(0));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn on Wi-Fi calling
     * @param result
     */
    @Override
    public void turnOnWfc(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_WFC_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = ON");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_WFC_ENABLE,
                        Integer.toString(1));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn off Wi-Fi calling
     * @param result
     */
    @Override
    public void turnOffWfc(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_WFC_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_WFC_ENABLE,
                        Integer.toString(0));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn on ViLTE
     * @param result
     */
    @Override
    public void turnOnVilte(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_VILTE_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = ON");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_VILTE_ENABLE,
                        Integer.toString(1));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn off ViLTE
     * @param result
     */
    @Override
    public void turnOffVilte(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_VILTE_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_VILTE_ENABLE,
                        Integer.toString(0));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn on Video on Wi-Fi
     * @param result
     */
    @Override
    public void turnOnViwifi(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_VIWIFI_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = ON");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_VIWIFI_ENABLE,
                        Integer.toString(1));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn off Video on Wi-Fi
     * @param result
     */
    @Override
    public void turnOffViwifi(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_VIWIFI_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_VIWIFI_ENABLE,
                        Integer.toString(0));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn on Rcs Ua
     * @param result
     */
    @Override
    public void turnOnRcsUa(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_RCS_UA_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = ON");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_RCS_UA_ENABLE,
                        Integer.toString(1));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn off Rcs ua
     * @param result
     */
    @Override
    public void turnOffRcsUa(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_RCS_UA_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.setVendorSetting(rr.mSerial, VendorSetting.VENDOR_SETTING_RCS_UA_ENABLE,
                        Integer.toString(0));
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVendorSetting", e);
            }
        }
    }

    /**
     * Turn on IMS video
     * @param result
     */
    @Override
    public void turnOnImsVideo(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_VIDEO_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = ON");
            }

            try {
                radioProxy.setImsVideoEnable(rr.mSerial, true);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsVideoEnable", e);
            }
        }
    }

    /**
     * Turn off IMS video
     * @param result
     */
    @Override
    public void turnOffImsVideo(Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_VIDEO_ENABLE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.setImsVideoEnable(rr.mSerial, false);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsVideoEnable", e);
            }
        }
    }

    /**
     * Get Provision Value
     * @param provisionStr Provision Key
     * @param result Response Data Parcel
     */
    @Override
    public void getProvisionValue(String provisionStr, Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_GET_PROVISION_VALUE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " provisionStr = "
                        + provisionStr);
            }
            try {
                radioProxy.getProvisionValue(rr.mSerial, provisionStr);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getProvisionValue", e);
            }
        }
    }

    /**
     * Set Provision Value
     * @param provisionStr Provision String
     * @param provisionValue Provision Value
     * @param result Response Data Parcel
     */
    @Override
    public void setProvisionValue(String provisionStr,
            String provisionValue, Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_PROVISION_VALUE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " provisionStr = "
                        + provisionStr + " provisionValue" + provisionValue);
            }
            try {
                radioProxy.setProvisionValue(rr.mSerial, provisionStr, provisionValue);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setProvisionValue", e);
            }
        }
    }

     /* Set IMS Config Feature Value
     *
     * @param featureId
     * @param network
     * @param value
     * @param isLast, is last command of set multiple feature values.
     * @param result
     */
    public void setImsCfgFeatureValue(int featureId, int network, int value, int isLast,
                                      Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_CONFIG_SET_FEATURE, result,
                    mRILDefaultWorkSource);

            try {
                radioProxy.setImsCfgFeatureValue(rr.mSerial, featureId, network, value,
                        isLast);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsCfgFeatureValue", e);
            }
        }
    }

    /**
     * Get IMS Config Feature Value
     *
     * @param featureId
     * @param network
     * @param result
     */
    public void getImsCfgFeatureValue(int featureId, int network, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_FEATURE, result,
                    mRILDefaultWorkSource);

            try {
                radioProxy.getImsCfgFeatureValue(rr.mSerial, featureId, network);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getImsCfgFeatureValue", e);
            }
        }
    }

    /**
     * Set IMS Config Provision Value
     *
     * @param configId
     * @param value
     * @param result
     */
    public void setImsCfgProvisionValue(int configId, String value, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_CONFIG_SET_PROVISION, result,
                    mRILDefaultWorkSource);

            try {
                radioProxy.setImsCfgProvisionValue(rr.mSerial, configId, value);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsCfgProvisionValue", e);
            }
        }
    }

    /**
     * Get IMS Config Provision Value
     *
     * @param configId
     * @param result
     */
    public void getImsCfgProvisionValue(int configId, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_PROVISION, result,
                    mRILDefaultWorkSource);

            try {
                radioProxy.getImsCfgProvisionValue(rr.mSerial, configId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getImsCfgProvisionValue", e);
            }
        }
    }

    /**
     * Get IMS Config Resource Capability Value
     *
     * @param featureId
     * @param result
     */
    public void getImsCfgResourceCapValue(int featureId, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP, result,
                    mRILDefaultWorkSource);

            try {
                radioProxy.getImsCfgResourceCapValue(rr.mSerial, featureId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getImsCfgResourceCapValue", e);
            }
        }
    }

    /**
    * Add a new member to VoLTE conference call according to the parameter - address.
    *
    * @param confCallId IMS(VoLTE) conference call id
    * @param participant The address(phone number or SIP URI)
    * @param response Command response.
    *
    */
    @Override
    public void inviteParticipants(int confCallId, String participant, Message response) {
        int participantCallId =
                ImsServiceCallTracker.getInstance(mPhoneId).getParticipantCallId(participant);


        internalAddConferenceMember(confCallId, participant,
                                    participantCallId, response);
    }

    /**
    * Remove a member from VoLTE conference call according to the parameter - address.
    *
    * @param confCallId IMS(VoLTE) conference call id.
    * @param participant The address(phone number or SIP URI).
    * @param response Command response.
    *
    */
    @Override
    public void removeParticipants(int confCallId, String participant, Message response) {
        int participantCallId =
                ImsServiceCallTracker.getInstance(mPhoneId).getParticipantCallId(participant);

        internalRemoveConferenceMember(confCallId, participant,
                                       participantCallId, response);
    }


    /**
    * Add a new member to VoLTE conference call according to the parameter - callId.
    * [ALPS02475154] invite participants using call id. @{
    * @param confCallId IMS(VoLTE) conference call id
    * @param callInfo The added participant call info
    * @param response Command response.
    */
    @Override
    public void inviteParticipantsByCallId(int confCallId, ImsCallInfo callInfo,
            Message response) {
        if (callInfo == null) {
            Rlog.d(IMSRIL_LOG_TAG, "Invite participants failed, call info is null");
            return;
        }
        String callId = callInfo.mCallId;

        int id = -1;
        try {
            id = Integer.parseInt(callId);
        } catch (NumberFormatException e) {
            Rlog.d(IMSRIL_LOG_TAG, "Invite participants failed: id is not integer: "
                                   + callId);
            return;
        }

        internalAddConferenceMember(confCallId, callInfo.mCallNum, id, response);
    }

    /**
    * To get last call fail cause.
    *
    * @param response Command response.
    */
    @Override
    public void getLastCallFailCause(Message result) {

        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_LAST_CALL_FAIL_CAUSE, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.getLastCallFailCause(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "getLastCallFailCause", e);
            }
        }
    }

    /**
    * To release all calls.
    * @param response Command response.
    */
    @Override
    public void hangupAllCall(Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_HANGUP_ALL, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.hangupAll(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "hangupAll", e);
            }
        }
    }

    /*
    * To swap calls.
    * @param response Response object
    */
    @Override
    public void swap(Message response) {

        IRadio radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(
                    RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            handleChldRelatedRequest(rr, null);
        }
    }

    /**
     * Send WFC Profile Info
     * @param wfcPreference
     * @param response
     */
    @Override
    public void sendWfcProfileInfo(int wfcPreference, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_WFC_PROFILE, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " wfcPreference = "
                        + wfcPreference);
            }

            try {
                radioProxy.setWfcProfile(rr.mSerial, wfcPreference);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setWfcProfile", e);
            }
        }
    }


    /**
     * Dial
     * @param address
     * @param clirMode
     * @param phoneId
     * @param result
     */
    @Override
    public void dial (String address, int clirMode, Message result) {
        dial(address, clirMode, null, result);
    }

    /**
     * Dial <br/>
     * AOSP Implementation
     * @param address Phone Address
     * @param clirMode CLIR Mode
     * @param USSInfo Uss Information
     * @param response Response object
     */
    @Override
    public void dial(String address, int clirMode, UUSInfo uusInfo,
            Message response) {

        if (!PhoneNumberUtils.isUriNumber(address)) {
            IRadio radioProxy = getRadioProxy(response);
            if (radioProxy != null) {
                RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_DIAL, response,
                        mRILDefaultWorkSource);
                Dial dialInfo = new Dial();
                dialInfo.address = convertNullToEmptyString(address);
                dialInfo.clir = clirMode;
                if (uusInfo != null) {
                    UusInfo info = new UusInfo();
                    info.uusType = uusInfo.getType();
                    info.uusDcs = uusInfo.getDcs();
                    info.uusData = new String(uusInfo.getUserData());
                    dialInfo.uusInfo.add(info);
                }
                if (IMS_RILA_LOGD) {
                    // Do not log function arg for privacy
                    riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                }
                try {
                    radioProxy.dial(rr.mSerial, dialInfo);
                } catch (RemoteException | RuntimeException e) {
                    handleRadioProxyExceptionForRR(rr, "dial", e);
                }
            }
        }
        else {
            IMtkRadioEx radioProxy = getMtkRadioProxy(response);
            if (radioProxy != null) {
                RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_DIAL_WITH_SIP_URI, response,
                        mRILDefaultWorkSource);

                if (IMS_RILA_LOGD) {
                    // Do not log function arg for privacy
                    riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                }
                try {
                    radioProxy.dialWithSipUri(rr.mSerial, address);
                } catch (RemoteException | RuntimeException e) {
                    handleMtkRadioProxyExceptionForRR(rr, "dialWithSipUri", e);
                }
            }
        }
    }

    private void emergencyDial(String address, ImsCallProfile callprofile,
                               int clirMode, UUSInfo uusInfo, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        // IRadio V1.4
        android.hardware.radio.V1_4.IRadio radioProxy14 =
                (android.hardware.radio.V1_4.IRadio) radioProxy;
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_EMERGENCY_DIAL, result,
                    mRILDefaultWorkSource);
            Dial dialInfo = new Dial();
            dialInfo.address = convertNullToEmptyString(address);
            dialInfo.clir = clirMode;
            if (uusInfo != null) {
                UusInfo info = new UusInfo();
                info.uusType = uusInfo.getType();
                info.uusDcs = uusInfo.getDcs();
                info.uusData = new String(uusInfo.getUserData());
                dialInfo.uusInfo.add(info);
            }

            if (IMS_RILA_LOGD) {
                // Do not log function arg for privacy
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }


            try {
                radioProxy14.emergencyDial(rr.mSerial, dialInfo,
                        callprofile.getEmergencyServiceCategories(),
                        (ArrayList) callprofile.getEmergencyUrns(),
                        callprofile.getEmergencyCallRouting(), false, false);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "emergencyDial", e);
            }
        }
    }

    /**
     * Dial conference call </br>
     * IMS VoLTE conference dial feature start
     * @param participants participants' dailing number.
     * @param clirMode indication to present the dialing number or not.
     * @param isVideoCall indicate the call is belong to video call or voice call.
     * @param result the command result.
     */

    @Override
    public void conferenceDial(String[] participants, int clirMode, boolean isVideoCall,
            Message result) {
        if(participants == null) {
            riljLoge("Participants MUST NOT be null in conferenceDial");
            return;
        }

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_CONFERENCE_DIAL, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " clirMode = "
                        + clirMode + " isVideoCall = " + isVideoCall);
            }

            ConferenceDial dialInfo = new ConferenceDial();
            dialInfo.clir = clirMode;
            dialInfo.isVideoCall = isVideoCall;
            for (String dialNumber : participants) {
                dialInfo.dialNumbers.add(dialNumber);
                if (IMS_RILA_LOGD) {
                    riljLog("conferenceDial: dialNumber " +
                            ImsServiceCallTracker.sensitiveEncode(dialNumber));
                }
            }

            try {
                radioProxy.conferenceDial(rr.mSerial, dialInfo);
            } catch (RemoteException | RuntimeException e) {
                Rlog.w(IMSRIL_LOG_TAG, "conferenceDial failed");
                handleMtkRadioProxyExceptionForRR(rr, "conferenceDial", e);
            }
        }
    }

    /**
     * MTK Proprietary API
     * For 3G VT only
     * @param address Address
     * @param clirMode CLIR mode
     * @param uusInfo USS Information
     * @param phoneId phoneId
     * @param result Response object
     */
    @Override
    public void vtDial(String address, int clirMode, UUSInfo uusInfo,
            Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            if (!PhoneNumberUtils.isUriNumber(address)) {
                RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_VT_DIAL, response,
                        mRILDefaultWorkSource);
                Dial dialInfo = new Dial();
                dialInfo.address = convertNullToEmptyString(address);
                dialInfo.clir = clirMode;
                if (uusInfo != null) {
                    UusInfo info = new UusInfo();
                    info.uusType = uusInfo.getType();
                    info.uusDcs = uusInfo.getDcs();
                    info.uusData = new String(uusInfo.getUserData());
                    dialInfo.uusInfo.add(info);
                }
                if (IMS_RILA_LOGD) {
                    // Do not log function arg for privacy
                    riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                }
                try {
                    radioProxy.vtDial(rr.mSerial, dialInfo);
                } catch (RemoteException | RuntimeException e) {
                    handleMtkRadioProxyExceptionForRR(rr, "vtDial", e);
                }
            }
            else {
                RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_VT_DIAL_WITH_SIP_URI, response,
                        mRILDefaultWorkSource);

                if (IMS_RILA_LOGD) {
                    // Do not log function arg for privacy
                    riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
                }
                try {
                    radioProxy.vtDialWithSipUri(rr.mSerial, address);
                } catch (RemoteException | RuntimeException e) {
                    handleMtkRadioProxyExceptionForRR(rr, "vtDialWithSipUri", e);
                }
            }
        }
    }

    /**
     * MTK Proprietary API
     * Send/cancel USSI RIL request
     * @param ussiString USSI
     * @param response Response object
     */
    @Override
    public void sendUSSI(String ussiString, Message response) {

        if (SystemProperties.get("persist.vendor.ims.ussi.ap").equals("1")) {
            if (IMS_RILA_LOGD) {
                riljLog("Wrap sendUSSI, ussiString = " + ussiString);
            }

            response.sendToTarget();
            return;
        }

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SEND_USSI, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " ussiString = " + ussiString);
            }

            try {
                radioProxy.sendUssi(rr.mSerial, ussiString);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "sendUssi", e);
            }
        }
    }

    /**
     * MTK Proprietary API
     * Cancel the pending USSI RIL request
     * @param result Response object
     */
    @Override
    public void cancelPendingUssi(Message response) {

        if (SystemProperties.get("persist.vendor.ims.ussi.ap").equals("1")) {
            if (IMS_RILA_LOGD) {
                riljLog("Wrap cancelPendingUssi");
            }

            response.sendToTarget();
            return;
        }

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_CANCEL_USSI, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.cancelUssi(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "cancelUssi", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void queryFacilityLock(String facility, String password, int serviceClass,
                                  Message result) {
        queryFacilityLockForApp(facility, password, serviceClass, "A0000000871002", result);
    }

    /**
     * AOSP Method
     */
    @Override
    public void queryFacilityLockForApp(String facility, String password, int serviceClass,
                                        String appId, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_QUERY_FACILITY_LOCK, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " facility = " + facility + " serviceClass = " + serviceClass
                        + " appId = " + appId);
            }
            try {
                radioProxy.getFacilityLockForApp(rr.mSerial,
                        convertNullToEmptyString(facility),
                        convertNullToEmptyString(password),
                        serviceClass,
                        convertNullToEmptyString(appId));
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "getFacilityLockForApp", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void setFacilityLock(String facility, boolean lockState, String password,
                                int serviceClass, Message result) {
        setFacilityLockForApp(facility, lockState, password, serviceClass, "A0000000871002", result);
    }

    /**
     * AOSP Method
     */
    @Override
    public void setFacilityLockForApp(String facility, boolean lockState, String password,
                                      int serviceClass, String appId, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_FACILITY_LOCK, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " facility = " + facility + " lockstate = " + lockState
                        + " serviceClass = " + serviceClass + " appId = " + appId);
            }
            try {
                radioProxy.setFacilityLockForApp(rr.mSerial,
                        convertNullToEmptyString(facility),
                        lockState,
                        convertNullToEmptyString(password),
                        serviceClass,
                        convertNullToEmptyString(appId));
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setFacilityLockForApp", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void setCallForward(int action, int cfReason, int serviceClass,
                   String number, int timeSeconds, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_CALL_FORWARD, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " action = " + action + " cfReason = " + cfReason + " serviceClass = "
                        + serviceClass + " timeSeconds = " + timeSeconds);
            }
            android.hardware.radio.V1_0.CallForwardInfo cfInfo =
                    new android.hardware.radio.V1_0.CallForwardInfo();
            cfInfo.status = action;
            cfInfo.reason = cfReason;
            cfInfo.serviceClass = serviceClass;
            cfInfo.toa = PhoneNumberUtils.toaFromString(number);
            cfInfo.number = convertNullToEmptyString(number);
            cfInfo.timeSeconds = timeSeconds;
            try {
                radioProxy.setCallForward(rr.mSerial, cfInfo);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setCallForward", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void queryCallForwardStatus(int cfReason, int serviceClass,
                           String number, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " cfreason = " + cfReason + " serviceClass = " + serviceClass);
            }
            android.hardware.radio.V1_0.CallForwardInfo cfInfo =
                    new android.hardware.radio.V1_0.CallForwardInfo();
            cfInfo.reason = cfReason;
            cfInfo.serviceClass = serviceClass;
            cfInfo.toa = PhoneNumberUtils.toaFromString(number);
            cfInfo.number = convertNullToEmptyString(number);
            cfInfo.timeSeconds = 0;
            try {
                radioProxy.getCallForwardStatus(rr.mSerial, cfInfo);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "queryCallForwardStatus", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void queryCallWaiting(int serviceClass, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_QUERY_CALL_WAITING, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " serviceClass = " + serviceClass);
            }
            try {
                radioProxy.getCallWaiting(rr.mSerial, serviceClass);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "queryCallWaiting", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void setCallWaiting(boolean enable, int serviceClass, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_CALL_WAITING, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " enable = " + enable + " serviceClass = " + serviceClass);
            }
            try {
                radioProxy.setCallWaiting(rr.mSerial, enable, serviceClass);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setCallWaiting", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void getCLIR(Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_GET_CLIR, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            try {
                radioProxy.getClir(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "getCLIR", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void setCLIR(int clirMode, Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_CLIR, result, mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " clirMode = " + clirMode);
            }
            try {
                radioProxy.setClir(rr.mSerial, clirMode);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setCLIR", e);
            }
        }
    }

    /**
     * AOSP Method
     */
    @Override
    public void queryCLIP(Message result) {
        IRadio radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_QUERY_CLIP, result,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            try {
                radioProxy.getClip(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "queryCLIP", e);
            }
        }
    }

    /**
     * MTK Method
     * Set CLIP (Calling Line Identification Presentation)
     */
    @Override
    public void setCLIP(int clipEnable, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_CLIP, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " clipEnable = " + clipEnable);
            }

            try {
                radioProxy.setClip(rr.mSerial, clipEnable);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setCLIP", e);
            }
        }
    }

    /**
     * MTK Method
     * Query COLR (Connected Line Identification Restriction)
     */
    @Override
    public void getCOLR(Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_GET_COLR, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.getColr(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getCOLR", e);
            }
        }
    }

    /**
     * MTK Method
     * Set colr
     */
    @Override
    public void setCOLR(int colrEnable, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_COLR, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " colrEnable = " + colrEnable);
            }

            try {
                radioProxy.setColr(rr.mSerial, colrEnable);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setCOLR", e);
            }
        }
    }

    /**
     * MTK Method
     * Query COLP (Connected Line Identification Presentation)
     */
    @Override
    public void getCOLP(Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_GET_COLP, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.getColp(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getCOLP", e);
            }
        }
    }

    /**
     * MTK Method
     * Set colp
     */
    @Override
    public void setCOLP(int colpEnable, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_COLP, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " colpEnable = " + colpEnable);
            }

            try {
                radioProxy.setColp(rr.mSerial, colpEnable);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setCOLP", e);
            }
        }
    }

    /**
     * MTK Method
     */
    @Override
    public void queryCallForwardInTimeSlotStatus(int cfReason, int serviceClass, Message result) {
        String number = "";
        String timeSlotBegin = "";
        String timeSlotEnd = "";

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT,
                    result, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " cfreason = " + cfReason + " serviceClass = " + serviceClass);
            }

            CallForwardInfoEx cfInfoEx =
                    new CallForwardInfoEx();
            cfInfoEx.reason = cfReason;
            cfInfoEx.serviceClass = serviceClass;
            cfInfoEx.toa = PhoneNumberUtils.toaFromString(number);
            cfInfoEx.number = convertNullToEmptyString(number);
            cfInfoEx.timeSeconds = 0;
            cfInfoEx.timeSlotBegin = convertNullToEmptyString(timeSlotBegin);
            cfInfoEx.timeSlotEnd = convertNullToEmptyString(timeSlotEnd);

            try {
                radioProxy.queryCallForwardInTimeSlotStatus(rr.mSerial, cfInfoEx);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "queryCallForwardInTimeSlotStatus", e);
            }
        }
    }

    /**
     * MTK Method
     */
    @Override
    public void setCallForwardInTimeSlot(int action, int cfReason, int serviceClass,
                   String number, int timeSeconds, long[] timeSlot, Message result) {
        String timeSlotBegin = "";
        String timeSlotEnd = "";

        // convertToSeverTime
        if (timeSlot != null && timeSlot.length == 2) {
            for (int i = 0; i < timeSlot.length; i++) {
                Date date = new Date(timeSlot[i]);
                SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm");
                dateFormat.setTimeZone(TimeZone.getTimeZone("GMT+8"));

                if (i == 0) {
                    timeSlotBegin = dateFormat.format(date);
                } else {
                    timeSlotEnd = dateFormat.format(date);
                }
            }
        }

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT,
                    result, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " action = " + action + " cfReason = " + cfReason + " serviceClass = "
                        + serviceClass + " timeSeconds = " + timeSeconds
                        + "timeSlot = [" + timeSlotBegin + ":" + timeSlotEnd + "]");
            }

            CallForwardInfoEx cfInfoEx =
                    new CallForwardInfoEx();
            cfInfoEx.status = action;
            cfInfoEx.reason = cfReason;
            cfInfoEx.serviceClass = serviceClass;
            cfInfoEx.toa = PhoneNumberUtils.toaFromString(number);
            cfInfoEx.number = convertNullToEmptyString(number);
            cfInfoEx.timeSeconds = timeSeconds;
            cfInfoEx.timeSlotBegin = convertNullToEmptyString(timeSlotBegin);
            cfInfoEx.timeSlotEnd = convertNullToEmptyString(timeSlotEnd);

            try {
                radioProxy.setCallForwardInTimeSlot(rr.mSerial, cfInfoEx);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setCallForwardInTimeSlot", e);

            }
        }
    }

    /**
     * run Gba Authentication
     * @param nafFqdn NAF FQDN String
     * @param nafSecureProtocolId nafSecureProtocolId Value
     * @param forceRun boolean
     * @param netId Integer
     * @param phoneId Integer
     * @param result Response Data Parcel
     */
    @Override
    public void runGbaAuthentication(String nafFqdn, String nafSecureProtocolId,
            boolean forceRun, int netId, Message result) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_RUN_GBA, result, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " nafFqdn = " + nafFqdn
                        + " nafSecureProtocolId = " + nafSecureProtocolId
                        + " forceRun = " + forceRun
                        + " netId = " + netId);
            }

            try {
                radioProxy.runGbaAuthentication(rr.mSerial, nafFqdn, nafSecureProtocolId,
                        forceRun, netId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "runGbaAuthentication", e);
            }
        }

    }

    /**
     * getXcapStatus
     * @param result Response Data Parcel
     */
    @Override
    public void getXcapStatus(Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_GET_XCAP_STATUS, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.getXcapStatus(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getXcapStatus", e);
            }
        }
    }

    /**
     * resetSuppServ
     * @param result Response Data Parcel
     */
    @Override
    public void resetSuppServ(Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_RESET_SUPP_SERV, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.resetSuppServ(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "getXcapStatus", e);
            }
        }
    }

    /**
     * resetSuppServ
     * @param result Response Data Parcel
     * @param userAgent Customized XCAP user agent string
     */
    @Override
    public void setupXcapUserAgentString(String userAgent, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SETUP_XCAP_USER_AGENT_STRING, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " userAgent = " + userAgent);
            }

            try {
                radioProxy.setupXcapUserAgentString(rr.mSerial, userAgent);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setupXcapUserAgentString", e);
            }
        }
    }
    /* MTK SS Feature : End */

    /**
     * AOSP Method
     * Request Exit ECBM (Emergency Callback Mode)
     * @param response
     */
    public void requestExitEmergencyCallbackMode(Message response) {

        IRadio radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE,
                    response, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.exitEmergencyCallbackMode(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "exitEmergencyCallbackMode", e);
            }
        }
    }

    /**
     * MTK Proprietary API
     * Force Hang up
     * @param callId Call ID
     * @param result response object
     */
    @Override
    public void forceHangup(int callId, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_FORCE_RELEASE_CALL, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " callId = " + callId);
            }

            try {
                radioProxy.forceReleaseCall(rr.mSerial, callId);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "forceHangup", e);
            }
        }
    }

    /**
     * Send bearer activation done to MD
     * @param aid Bearer Id
     * @param action of bearer's operation
     * @param status Result of bearer's operation
     * @param response response object
     */
    @Override
    public void responseBearerStateConfirm(int aid, int action, int status, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_BEARER_STATE_CONFIRM,
                    response, mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " aid = " + aid + " action =" + action + " status =" + status);
            }

            try {
                radioProxy.imsBearerStateConfirm(rr.mSerial, aid, action, status);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "imsBearerStateConfirm", e);
            }
        }
    }

    /**
     * config bearer notification.
     * @param aid
     * @param status
     * @param response
     */
    @Override
    public void setImsBearerNotification(int enable, Message response) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION,
                    response, mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " enable = " + enable);
            }

            try {
                radioProxy.setImsBearerNotification(rr.mSerial, enable);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsBearerNotification", e);
            }
        }
    }

    /**
     * Pull Call
     * @param target Taget
     * @param isVideoCall Is Video call
     * @param response Response Object
     */
    @Override
    public void pullCall(String target, boolean isVideoCall, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_PULL_CALL, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " target = " + isVideoCall + " isVideoCall = "
                        + isVideoCall);
            }

            try {
                radioProxy.pullCall(rr.mSerial, target, isVideoCall);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "pullCall", e);
            }
        }
    }

    /**
     * Set IMS Report
     * @param target Taget
     * @param isVideoCall Is Video call
     * @param response Response Object
     */
    @Override
    public void setImsRegistrationReport(Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(null);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_REGISTRATION_REPORT,
                    response, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.setImsRegistrationReport(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsRegistrationReport", e);
            }
        }
    }

    /**
     * Set IMS RTP Info
     * @param pdnId PDN Id
     * @param networkId Network Id
     * @param timer Timer
     * @param response Response Object
     */
    @Override
    public void setImsRtpInfo(int pdnId, int networkId, int timer, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_RTP_REPORT, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " PDN id = " + pdnId + " network Id = " + networkId
                        + " Timer = " + timer);
            }

            try {
                radioProxy.setImsRtpReport(rr.mSerial, pdnId, networkId, timer);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsRtpReport", e);
            }
        }
    }

    /**
     * Set Voice Call Preference
     * @param vdp
     * @param response
     */
    @Override
    public void setVoiceDomainPreference(int vdp, Message response) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest) + " vdp = " + vdp);
            }

            try {
                radioProxy.setVoiceDomainPreference(rr.mSerial, vdp);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setVoiceDomainPreference", e);
            }
        }
    }

    //***** Private Methods

    /**
     * Internal method for adding conference method <br/>
     * PS call only
     * Add conference member
     * @param confCallId
     * @param address
     * @param callIdToAdd
     * @param result
     */
    private void internalAddConferenceMember(int confCallId, String address,
            int callIdToAdd, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER,
                    response, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " confCallId = "
                        + confCallId + " address = " + ImsServiceCallTracker.sensitiveEncode(address)
                        + " callIdToAdd =" + callIdToAdd);
            }

            try {
                radioProxy.controlImsConferenceCallMember(rr.mSerial,
                        ImsRILConstants.CONTROL_MEMBER_ADD, confCallId, address, callIdToAdd);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "addImsConferenceCallMember", e);
            }
        }
    }

    /**
     * Internal method for removing conference member
     * @param confCallId
     * @param address
     * @param callIdToRemove
     * @param result
     */
    private void internalRemoveConferenceMember(int confCallId, String address,
            int callIdToRemove, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER,
                    response, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " confCallId = "
                        + confCallId + " address = " + ImsServiceCallTracker.sensitiveEncode(address)
                        + " callIdToRemove =" + callIdToRemove);
            }

            try {
                radioProxy.controlImsConferenceCallMember(rr.mSerial,
                        ImsRILConstants.CONTROL_MEMBER_REMOVE, confCallId, address, callIdToRemove);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "removeImsConferenceCallMember", e);
            }
        }
    }

    /**
     * Internal method for IMS ECT command
     * @param number
     * @param type
     * @param response
     */
    private void internalImsEct(String number, int type, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(
                    ImsRILConstants.RIL_REQUEST_IMS_ECT, response, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.imsEctCommand(rr.mSerial, number, type);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "imsEctCommand", e);
            }
        }
    }

    /**
     * DTMF request special handling
     * To protect modem status we need to avoid two case :
     * 1. DTMF start -> CHLD request -> DTMF stop
     * 2. CHLD request -> DTMF request
     * @param rr RIL Request Object
     * @param params Parameters
     */
    private void handleChldRelatedRequest(RILRequest rr, Object[] params) {
        synchronized (mDtmfReqQueue) {
            int queueSize = mDtmfReqQueue.size();
            int i, j;
            if (queueSize > 0) {
                DtmfQueueHandler.DtmfQueueRR dqrr2 = mDtmfReqQueue.get();
                RILRequest rr2 = dqrr2.rr;
                if (rr2.mRequest == RIL_REQUEST_DTMF_START) {
                    // need to send the STOP command
                    if (IMS_RILA_LOGD) riljLog("DTMF queue isn't 0, first request is START, "
                            + "send stop dtmf and pending switch");
                    if (queueSize > 1) {
                        j = 2;
                    } else {
                        // need to create a new STOP command
                        j = 1;
                    }
                    if (IMS_RILA_LOGD) riljLog("queue size  " + mDtmfReqQueue.size());
                    for (i = queueSize - 1; i >= j; i--) {
                        mDtmfReqQueue.remove(i);
                    }
                    if (IMS_RILA_LOGD) riljLog("queue size  after " + mDtmfReqQueue.size());
                    if (mDtmfReqQueue.size() == 1) { // only start command
                                                     // , we need to add stop command
                        if (IMS_RILA_LOGD) riljLog("add dummy stop dtmf request");
                        RILRequest rr3 = obtainRequest(RIL_REQUEST_DTMF_STOP, null,
                                mRILDefaultWorkSource);

                        Object[] myParam = { rr3.mSerial };
                        DtmfQueueHandler.DtmfQueueRR dqrr3 = mDtmfReqQueue.buildDtmfQueueRR(
                                rr3, myParam);
                        mDtmfReqQueue.stop();
                        mDtmfReqQueue.add(dqrr3);
                    }
                }
                else {
                    // first request is STOP, just remove it and send switch
                    if (IMS_RILA_LOGD)
                        riljLog("DTMF queue isn't 0, first request is STOP, penging switch");
                    j = 1;
                    for (i = queueSize - 1; i >= j; i--) {
                        mDtmfReqQueue.remove(i);
                    }
                }

                // Need to check if there is pending request before calling setPendingRequest.
                // If there is pending request and exist message. we must send it to target.
                if (mDtmfReqQueue.getPendingRequest() != null){
                    DtmfQueueHandler.DtmfQueueRR pendingDqrr = mDtmfReqQueue.getPendingRequest();
                    RILRequest pendingRequest = pendingDqrr.rr;
                    if (pendingRequest.mResult != null) {
                        AsyncResult.forMessage(pendingRequest.mResult, null, null);
                        pendingRequest.mResult.sendToTarget();
                    }
                }

                DtmfQueueHandler.DtmfQueueRR dqrr = mDtmfReqQueue.buildDtmfQueueRR(rr, params);
                mDtmfReqQueue.setPendingRequest(dqrr);
            } else {
                if (IMS_RILA_LOGD) riljLog("DTMF queue is 0, send switch Immediately");
                mDtmfReqQueue.setSendChldRequest();
                DtmfQueueHandler.DtmfQueueRR dqrr = mDtmfReqQueue.buildDtmfQueueRR(rr, params);
                sendDtmfQueueRR(dqrr);
            }
        }
    }
    /**
     * Send DTMF Queue
     * @param dqrr
     */
    private void sendDtmfQueueRR(DtmfQueueHandler.DtmfQueueRR dqrr) {
        RILRequest rr = dqrr.rr;
        IRadio radioProxy = getRadioProxy(rr.mResult);
        if (radioProxy == null) {
            riljLoge("get RadioProxy null. ([" + rr.serialString() + "] request: "
                    + requestToString(rr.mRequest) + ")");
            return;
        }
        if (IMS_RILA_LOGD) {
            riljLog(rr.serialString() + "> " + requestToString(rr.mRequest) + " (by DtmfQueueRR)");
        }
        try {
            Object[] params = null;
            switch (rr.mRequest) {
            case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
                radioProxy.switchWaitingOrHoldingAndActive(rr.mSerial);
                break;
            case RIL_REQUEST_CONFERENCE:
                radioProxy.conference(rr.mSerial);
                break;
            case RIL_REQUEST_SEPARATE_CONNECTION:
                params = dqrr.params;
                if (params.length != 1) {
                    riljLoge("request " + requestToString(rr.mRequest) + " params error. ("
                            + Arrays.toString(params) + ")");
                } else {
                    int gsmIndex = (int) params[0];
                    radioProxy.separateConnection(rr.mSerial, gsmIndex);
                }
                break;
            case RIL_REQUEST_EXPLICIT_CALL_TRANSFER:
                radioProxy.explicitCallTransfer(rr.mSerial);
                break;
            case RIL_REQUEST_DTMF_START:
                params = dqrr.params;
                if (params.length != 1) {
                    riljLoge("request " + requestToString(rr.mRequest) + " params error. ("
                            + params.toString() + ")");
                } else {
                    char c = (char)params[0];
                    radioProxy.startDtmf(rr.mSerial, c + "");
                }
                break;
            case RIL_REQUEST_DTMF_STOP:
                radioProxy.stopDtmf(rr.mSerial);
                break;
            default:
                riljLoge("get RadioProxy null. ([" + rr.serialString() + "] request: "
                        + requestToString(rr.mRequest) + ")");
            }
        } catch (RemoteException | RuntimeException e) {
            handleRadioProxyExceptionForRR(rr,
                    "DtmfQueueRR(" + requestToString(rr.mRequest) + ")", e);
        }
    }
    /**
     * DTMF request special handling
     * @param serial
     */
    void handleDtmfQueueNext(int serial) {
        // DTMF request will be ignored when the count of requests reaches 32
        if (IMS_RILA_LOGD) {
            riljLog("handleDtmfQueueNext (serial = " + serial);
        }
        synchronized (mDtmfReqQueue) {
            DtmfQueueHandler.DtmfQueueRR dqrr = null;
            for (int i = 0; i< mDtmfReqQueue.mDtmfQueue.size(); i++) {
                DtmfQueueHandler.DtmfQueueRR adqrr =
                        (DtmfQueueHandler.DtmfQueueRR) mDtmfReqQueue.mDtmfQueue.get(i);
                if (adqrr != null && adqrr.rr.mSerial == serial) {
                    dqrr = adqrr;
                    break;
                }
            }
            if (dqrr == null) {
                riljLoge("cannot find serial " + serial + " from mDtmfQueue. (size = "
                        + mDtmfReqQueue.size() + ")");
            } else {
                mDtmfReqQueue.remove(dqrr);
                if (IMS_RILA_LOGD) {
                    riljLog("remove first item in dtmf queue done. (size = " + mDtmfReqQueue.size()
                            + ")");
                }
            }
            if (mDtmfReqQueue.size() > 0) {
                DtmfQueueHandler.DtmfQueueRR dqrr2 = mDtmfReqQueue.get();
                RILRequest rr2 = dqrr2.rr;
                if (IMS_RILA_LOGD) {
                    riljLog(rr2.serialString() + "> " + requestToString(rr2.mRequest));
                }
                sendDtmfQueueRR(dqrr2);
            } else {
                if (mDtmfReqQueue.getPendingRequest() != null) {
                    riljLog("send pending switch request");
                    DtmfQueueHandler.DtmfQueueRR pendingReq = mDtmfReqQueue.getPendingRequest();
                    sendDtmfQueueRR(pendingReq);
                    mDtmfReqQueue.setSendChldRequest();
                    mDtmfReqQueue.setPendingRequest(null);
                }
            }
        }
    }

    //***** Package Methods

    /**
     * This is a helper function to be called when a RadioIndication callback is called.
     * It takes care of acquiring wakelock and sending ack if needed.
     * @param indicationType RadioIndicationType received
     */
    void processIndication(int indicationType) {
        if (indicationType == RadioIndicationType.UNSOLICITED_ACK_EXP) {
            sendAck();
            //if (IMSRIL_LOGD) riljLog("Unsol response received; Sending ack to ril.cpp");
        } else {
            // ack is not expected to be sent back. Nothing is required to be done here.
        }
    }

    /**
         * This is a helper function to be called when a RadioIndication callback is called.
         * It takes care of acquiring wakelock and sending ack if needed.
         * @param indicationType RadioIndicationType received
         */
    void processMtkIndication(int indicationType) {
        if (indicationType == RadioIndicationType.UNSOLICITED_ACK_EXP) {
            sendMtkAck();
            //if (IMSRIL_LOGD) riljLog("Unsol response received; Sending ack to ril.cpp");
        } else {
            // ack is not expected to be sent back. Nothing is required to be done here.
        }
    }

    void processRequestAck(int serial) {
        RILRequest rr;
        synchronized (mRequestList) {
            rr = mRequestList.get(serial);
        }
        if (rr == null) {
            Rlog.w(ImsRILAdapter.IMSRIL_LOG_TAG,
                    "processRequestAck: Unexpected solicited ack response! " +
                    "serial: " + serial);
        } else {
            decrementWakeLock(rr);
            if (IMSRIL_LOGD) {
                riljLog(rr.serialString() + " Ack < " + requestToString(rr.mRequest));
            }
        }
    }

    /**
     * This is a helper function to be called when a RadioResponse callback is called.
     * It takes care of acks, wakelocks, and finds and returns RILRequest corresponding to the
     * response if one is found.
     * @param responseInfo RadioResponseInfo received in response callback
     * @return RILRequest corresponding to the response
     */
    RILRequest processResponse(RadioResponseInfo responseInfo, boolean isProprietary) {
        int serial = responseInfo.serial;
        int error = responseInfo.error;
        int type = responseInfo.type;

        RILRequest rr = null;

        if (type == RadioResponseType.SOLICITED_ACK) {
            synchronized (mRequestList) {
                rr = mRequestList.get(serial);
            }
            if (rr == null) {
                Rlog.w(IMSRIL_LOG_TAG, "Unexpected solicited ack response! sn: " + serial);
            } else {
                decrementWakeLock(rr);
                if (IMS_RILA_LOGD) {
                    riljLog(rr.serialString() + " Ack < " + requestToString(rr.mRequest));
                }
            }
            return rr;
        }

        rr = findAndRemoveRequestFromList(serial);
        if (rr == null) {
            Rlog.e(ImsRILAdapter.IMSRIL_LOG_TAG, "processResponse: Unexpected response! serial: " + serial
                    + " error: " + error);
            return null;
        }

        // Time logging for RIL command and storing it in TelephonyHistogram.
        addToRilHistogram(rr);

        if (type == RadioResponseType.SOLICITED_ACK_EXP) {
            if (isProprietary) {
                sendMtkAck();
            } else {
                sendAck();
            }
            if (ImsRILAdapter.IMS_RILA_LOGD) {
                riljLog("Response received for " + rr.serialString() + " "
                        + requestToString(rr.mRequest) + " Sending ack to ril.cpp");
            }
        } else {
            // ack sent for SOLICITED_ACK_EXP above; nothing to do for SOLICITED response
        }

        // Here and below fake RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, see b/7255789.
        // This is needed otherwise we don't automatically transition to the main lock
        // screen when the pin or puk is entered incorrectly.
        switch (rr.mRequest) {
            case RIL_REQUEST_SHUTDOWN:
                setRadioState(TelephonyManager.RADIO_POWER_UNAVAILABLE);
                break;
        }

        if (error != RadioError.NONE) {
            // Do in Error Condition
        } else {
            // Do in Normal Condition
        }
        return rr;
    }

    /**
     * This is a helper function to be called at the end of all RadioResponse callbacks.
     * It takes care of sending error response, logging, decrementing wakelock if needed, and
     * releases the request from memory pool.
     * @param rr RILRequest for which response callback was called
     * @param responseInfo RadioResponseInfo received in the callback
     * @param ret object to be returned to request sender
     */
    void processResponseDone(RILRequest rr, RadioResponseInfo responseInfo, Object ret) {
        if (responseInfo.error == 0) {
            if (IMS_RILA_LOGD) {
                if (rr.mRequest != ImsRILConstants.RIL_REQUEST_IMS_CONFIG_SET_FEATURE &&
                        rr.mRequest != ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_FEATURE &&
                        rr.mRequest != ImsRILConstants.RIL_REQUEST_IMS_CONFIG_SET_PROVISION &&
                        rr.mRequest != ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_PROVISION &&
                        rr.mRequest != ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP) {
                    riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                            + " " + retToString(rr.mRequest, ret));
                }
            }
        } else {
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                        + " error " + responseInfo.error);
            }
            rr.onError(responseInfo.error, ret);
        }
        mMetrics.writeOnRilSolicitedResponse(mPhoneId, rr.mSerial, responseInfo.error,
                rr.mRequest, ret);

        if (rr.mRequest == RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE ||
               rr.mRequest == RIL_REQUEST_CONFERENCE ||
               rr.mRequest == RIL_REQUEST_SEPARATE_CONNECTION ||
               rr.mRequest == RIL_REQUEST_EXPLICIT_CALL_TRANSFER) {
           riljLog("clear mIsSendChldRequest");
           mDtmfReqQueue.resetSendChldRequest();
        }

        if (rr != null) {
            if (responseInfo.type == RadioResponseType.SOLICITED) {
                decrementWakeLock(rr);
            }
            rr.release();
        }
    }


    /**
     * Function to send ack and acquire related wakelock
     */
    private void sendAck() {
        // TODO: Remove rr and clean up acquireWakelock for response and ack
        RILRequest rr = RILRequest.obtain(RIL_RESPONSE_ACKNOWLEDGEMENT, null,
                mRILDefaultWorkSource);
        acquireWakeLock(rr, RIL.FOR_ACK_WAKELOCK);
        IRadio radioProxy = getRadioProxy(null);
        if (radioProxy != null) {
            try {
                radioProxy.responseAcknowledgement();
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "sendAck", e);
                riljLoge("sendAck: " + e);
            }
        } else {
            Rlog.e(IMSRIL_LOG_TAG, "Error trying to send ack, radioProxy = null");
        }
        rr.release();
    }

    /**
     * Function to send ack and acquire related wakelock
     */
    private void sendMtkAck() {
        // TODO: Remove rr and clean up acquireWakelock for response and ack
        RILRequest rr = RILRequest.obtain(RIL_RESPONSE_ACKNOWLEDGEMENT, null,
                mRILDefaultWorkSource);
        acquireWakeLock(rr, RIL.FOR_ACK_WAKELOCK);
        IMtkRadioEx RadioProxy = getMtkRadioProxy(null);
        if (RadioProxy != null) {
            try {
                RadioProxy.responseAcknowledgementMtk();
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "sendMtkAck", e);
                riljLoge("sendMtkAck: " + e);
            }
        } else {
            Rlog.e(IMSRIL_LOG_TAG, "Error trying to send MTK ack, RadioProxy = null");
        }
        rr.release();
    }

    private WorkSource getDeafultWorkSourceIfInvalid(WorkSource workSource) {
        if (workSource == null) {
            workSource = mRILDefaultWorkSource;
        }

        return workSource;
    }

    private String getWorkSourceClientId(WorkSource workSource) {
        if (workSource != null) {
            return String.valueOf(workSource.get(0)) + ":" + workSource.getName(0);
        }

        return null;
    }

    // SMS-START
    private GsmSmsMessage constructGsmSendSmsRilRequest(String smscPdu, String pdu) {
        GsmSmsMessage msg = new GsmSmsMessage();
        msg.smscPdu = smscPdu == null ? "" : smscPdu;
        msg.pdu = pdu == null ? "" : pdu;
        return msg;
    }

    protected void constructCdmaSendSmsRilRequest(CdmaSmsMessage msg, byte[] pdu) {
        int addrNbrOfDigits;
        int subaddrNbrOfDigits;
        int bearerDataLength;
        ByteArrayInputStream bais = new ByteArrayInputStream(pdu);
        DataInputStream dis = new DataInputStream(bais);

        try {
            msg.teleserviceId = dis.readInt(); // teleServiceId
            msg.isServicePresent = (byte) dis.readInt() == 1 ? true : false; // servicePresent
            msg.serviceCategory = dis.readInt(); // serviceCategory
            msg.address.digitMode = dis.read();  // address digit mode
            msg.address.numberMode = dis.read(); // address number mode
            msg.address.numberType = dis.read(); // address number type
            msg.address.numberPlan = dis.read(); // address number plan
            addrNbrOfDigits = (byte) dis.read();
            for (int i = 0; i < addrNbrOfDigits; i++) {
                msg.address.digits.add(dis.readByte()); // address_orig_bytes[i]
            }
            msg.subAddress.subaddressType = dis.read(); //subaddressType
            msg.subAddress.odd = (byte) dis.read() == 1 ? true : false; //subaddr odd
            subaddrNbrOfDigits = (byte) dis.read();
            for (int i = 0; i < subaddrNbrOfDigits; i++) {
                msg.subAddress.digits.add(dis.readByte()); //subaddr_orig_bytes[i]
            }

            bearerDataLength = dis.read();
            for (int i = 0; i < bearerDataLength; i++) {
                msg.bearerData.add(dis.readByte()); //bearerData[i]
            }
        } catch (IOException ex) {
            if (IMS_RILA_LOGD) {
                riljLog("sendSmsCdma: conversion from input stream to object failed: "
                        + ex);
            }
        }
    }

    /**
     * send Sms
     */
    @Override
    public void sendSms(int token, int messageRef, String format, String smsc, boolean isRetry,
            byte[] pdu, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_IMS_SEND_SMS_EX, response,
                    mRILDefaultWorkSource);
            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }
            ImsSmsMessage msg = new ImsSmsMessage();
            msg.retry = isRetry;
            msg.messageRef = messageRef;

            if (SmsConstants.FORMAT_3GPP.equals(format)) {
                msg.tech = RILConstants.GSM_PHONE;
                GsmSmsMessage gsmMsg = constructGsmSendSmsRilRequest(smsc,
                        IccUtils.bytesToHexString(pdu));
                msg.gsmMessage.add(gsmMsg);
            } else if (SmsConstants.FORMAT_3GPP2.equals(format)) {
                msg.tech = RILConstants.CDMA_PHONE;
                CdmaSmsMessage cdmaMsg = new CdmaSmsMessage();
                constructCdmaSendSmsRilRequest(cdmaMsg, pdu);
                msg.cdmaMessage.add(cdmaMsg);
            } else {
                riljLog(rr.serialString() + "> " + "SMS format Error");
                return;
            }
            try {
                radioProxy.sendImsSmsEx(rr.mSerial, msg);
                mMetrics.writeRilSendSms(mPhoneId, rr.mSerial, SmsSession.Event.Tech.SMS_IMS,
                        (SmsConstants.FORMAT_3GPP.equals(format))?
                        SmsSession.Event.Format.SMS_FORMAT_3GPP :
                        SmsSession.Event.Format.SMS_FORMAT_3GPP2);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "sendImsGsmSms", e);
            }
        }
    }

    @Override
    public void acknowledgeLastIncomingGsmSms(boolean success, int cause, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SMS_ACKNOWLEDGE_EX, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " success = " + success + " cause = " + cause);
            }

            try {
                radioProxy.acknowledgeLastIncomingGsmSmsEx(rr.mSerial, success, cause);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "acknowledgeLastIncomingGsmSms", e);
            }
        }
    }

    @Override
    public void acknowledgeLastIncomingCdmaSmsEx(boolean success, int cause, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX, result,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " success = " + success + " cause = " + cause);
            }

            CdmaSmsAck msg = new CdmaSmsAck();
            msg.errorClass = success ? 0 : 1;
            msg.smsCauseCode = cause;

            try {
                radioProxy.acknowledgeLastIncomingCdmaSmsEx(rr.mSerial, msg);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "acknowledgeLastIncomingCdmaSms", e);
            }
        }
    }
    // SMS-END

    // Client API start
    /**
     * Set SIP header
     * @param total
     * @param index
     * @param headerCount
     * @param headerValuePair
     * @param response
     */
    @Override
    public void setSipHeader(int total, int index, int headerCount,
            String headerValuePair, Message response) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_SIP_HEADER,
                    response, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            ArrayList<String> arrList = new ArrayList<>();
            arrList.add(Integer.toString(total));
            arrList.add(Integer.toString(index));
            arrList.add(Integer.toString(headerCount));
            arrList.add(headerValuePair);

            try {
                radioProxy.setSipHeader(rr.mSerial, arrList);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setSipHeader", e);
            }
        }
    }

    /**
     * Enable SIP header report
     * @param callId
     * @param headerType
     * @param response
     */
    @Override
    public void setSipHeaderReport(String callId, String headerType, Message response) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SIP_HEADER_REPORT, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " callId = " + callId + " headerType = " + headerType);
            }

            ArrayList<String> arrList = new ArrayList<>();
            arrList.add(callId);
            arrList.add(headerType);

            try {
                radioProxy.setSipHeaderReport(rr.mSerial, arrList);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setSipHeaderReport", e);
            }
        }
    }

    /**
     * Set IMS call mode
     * @param mode
     * @param response
     */
    @Override
    public void setImsCallMode(int mode, Message response) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_IMS_CALL_MODE, response,
                    mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " mode = " + mode);
            }

            try {
                radioProxy.setImsCallMode(rr.mSerial, mode);
            } catch (RemoteException | RuntimeException e) {
                handleMtkRadioProxyExceptionForRR(rr, "setImsCallMode", e);
            }
        }
    }
    // Client API End

    /**
     * Holds a PARTIAL_WAKE_LOCK whenever
     * a) There is outstanding RIL request sent to RIL deamon and no replied
     * b) There is a request pending to be sent out.
     *
     * There is a WAKE_LOCK_TIMEOUT to release the lock, though it shouldn't
     * happen often.
     */

    private void acquireWakeLock(RILRequest rr, int wakeLockType) {
        synchronized (rr) {
            if (rr.mWakeLockType != INVALID_WAKELOCK) {
                Rlog.d(IMSRIL_LOG_TAG, "Failed to aquire wakelock for " + rr.serialString());
                return;
            }

            switch(wakeLockType) {
                case FOR_WAKELOCK:
                    synchronized (mWakeLock) {
                        mWakeLock.acquire();
                        mWakeLockCount++;
                        mWlSequenceNum++;

                        String clientId = getWorkSourceClientId(rr.mWorkSource);
                        if (!mClientWakelockTracker.isClientActive(clientId)) {
                            if (mActiveWakelockWorkSource != null) {
                                mActiveWakelockWorkSource.add(rr.mWorkSource);
                            } else {
                                mActiveWakelockWorkSource = rr.mWorkSource;
                            }
                            mWakeLock.setWorkSource(mActiveWakelockWorkSource);
                        }

                        mClientWakelockTracker.startTracking(rr.mClientId,
                                rr.mRequest, rr.mSerial, mWakeLockCount);

                        Message msg = mRilHandler.obtainMessage(EVENT_WAKE_LOCK_TIMEOUT);
                        msg.arg1 = mWlSequenceNum;
                        mRilHandler.sendMessageDelayed(msg, mWakeLockTimeout);
                    }
                    break;
                case FOR_ACK_WAKELOCK:
                    synchronized (mAckWakeLock) {
                        mAckWakeLock.acquire();
                        mAckWlSequenceNum++;

                        Message msg = mRilHandler.obtainMessage(EVENT_ACK_WAKE_LOCK_TIMEOUT);
                        msg.arg1 = mAckWlSequenceNum;
                        mRilHandler.sendMessageDelayed(msg, mAckWakeLockTimeout);
                    }
                    break;
                default: //WTF
                    Rlog.w(IMSRIL_LOG_TAG, "Acquiring Invalid Wakelock type " + wakeLockType);
                    return;
            }
            rr.mWakeLockType = wakeLockType;
        }
    }

    private void decrementWakeLock(RILRequest rr) {
        synchronized (rr) {
            switch(rr.mWakeLockType) {
                case FOR_WAKELOCK:
                    synchronized (mWakeLock) {
                        mClientWakelockTracker.stopTracking(rr.mClientId,
                                rr.mRequest, rr.mSerial,
                                (mWakeLockCount > 1) ? mWakeLockCount - 1 : 0);
                        String clientId = getWorkSourceClientId(rr.mWorkSource);;
                        if (!mClientWakelockTracker.isClientActive(clientId)
                                && (mActiveWakelockWorkSource != null)) {
                            mActiveWakelockWorkSource.remove(rr.mWorkSource);
                            if (mActiveWakelockWorkSource.size() == 0) {
                                mActiveWakelockWorkSource = null;
                            }
                            mWakeLock.setWorkSource(mActiveWakelockWorkSource);
                        }

                        if (mWakeLockCount > 1) {
                            mWakeLockCount--;
                        } else {
                            mWakeLockCount = 0;
                            mWakeLock.release();
                        }
                    }
                    break;
                case FOR_ACK_WAKELOCK:
                    //We do not decrement the ACK wakelock
                    break;
                case INVALID_WAKELOCK:
                    break;
                default:
                    Rlog.w(IMSRIL_LOG_TAG, "Decrementing Invalid Wakelock type " + rr.mWakeLockType);
            }
            rr.mWakeLockType = INVALID_WAKELOCK;
        }
    }

    private boolean clearWakeLock(int wakeLockType) {
        if (wakeLockType == FOR_WAKELOCK) {
            synchronized (mWakeLock) {
                if (mWakeLockCount == 0 && !mWakeLock.isHeld()) return false;
                Rlog.d(IMSRIL_LOG_TAG, "NOTE: mWakeLockCount is " + mWakeLockCount
                        + "at time of clearing");
                mWakeLockCount = 0;
                mWakeLock.release();
                mClientWakelockTracker.stopTrackingAll();
                mActiveWakelockWorkSource = null;
                return true;
            }
        } else {
            synchronized (mAckWakeLock) {
                if (!mAckWakeLock.isHeld()) return false;
                mAckWakeLock.release();
                return true;
            }
        }
    }

    /**
     * Release each request in mRequestList then clear the list
     * @param error is the RIL_Errno sent back
     * @param loggable true means to print all requests in mRequestList
     */
    private void clearRequestList(int error, boolean loggable) {
        RILRequest rr;
        synchronized (mRequestList) {
            int count = mRequestList.size();
            if (IMSRIL_LOGD && loggable) {
                Rlog.d(IMSRIL_LOG_TAG, "clearRequestList " + " mWakeLockCount="
                        + mWakeLockCount + " mRequestList=" + count);
            }

            for (int i = 0; i < count; i++) {
                rr = mRequestList.valueAt(i);
                if (IMSRIL_LOGD && loggable) {
                    Rlog.d(IMSRIL_LOG_TAG, i + ": [" + rr.mSerial + "] "
                            + requestToString(rr.mRequest));
                }
                rr.onError(error, null);
                decrementWakeLock(rr);
                rr.release();
            }
            mRequestList.clear();
        }
    }

    private RILRequest findAndRemoveRequestFromList(int serial) {
        RILRequest rr = null;
        synchronized (mRequestList) {
            rr = mRequestList.get(serial);
            if (rr != null) {
                mRequestList.remove(serial);
            }
        }

        return rr;
    }

    private void addToRilHistogram(RILRequest rr) {
        long endTime = SystemClock.elapsedRealtime();
        int totalTime = (int) (endTime - rr.mStartTimeMs);

        synchronized (mRilTimeHistograms) {
            TelephonyHistogram entry = mRilTimeHistograms.get(rr.mRequest);
            if (entry == null) {
                // We would have total #RIL_HISTOGRAM_BUCKET_COUNT range buckets for RIL commands
                entry = new TelephonyHistogram(TelephonyHistogram.TELEPHONY_CATEGORY_RIL,
                        rr.mRequest, RIL_HISTOGRAM_BUCKET_COUNT);
                mRilTimeHistograms.put(rr.mRequest, entry);
            }
            entry.addTimeTaken(totalTime);
        }
    }

    /**
     * Request RTT Modify Response
     * RIL_REQUEST_RTT_MODIFY_REQUST_RESPONSE
     * @param callId Call id
     * @param result Result
     * @param response Response object
     */
    @Override
    public void setRttModifyRequestResponse(int callId, int result, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_RTT_MODIFY_REQUST_RESPONSE, response,
                    mRILDefaultWorkSource);

            if (IMSRIL_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " callId = " + callId + " result = " + result);
            }

            try {
                radioProxy.rttModifyRequestResponse(rr.mSerial, callId, result);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "rttModifyRequestResponse", e);
            }
        }
    }

    /**
     * Send RTT Modify Request
     * RIL_REQUEST_SEND_RTT_MODIFY_REQUEST
     * @param callId Call id
     * @param newMode New mode
     * @param response Response object
     */
    @Override
    public void sendRttModifyRequest(int callId, int newMode, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SEND_RTT_MODIFY_REQUEST, response,
                    mRILDefaultWorkSource);

            if (IMSRIL_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " callId = " + callId + " newMode = " + newMode);
            }

            try {
                radioProxy.sendRttModifyRequest(rr.mSerial, callId, newMode);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "sendRttModifyRequest", e);
            }
        }

    }

    /**
     * Send RTT Text
     * RIL_REQUEST_SEND_RTT_TEXT
     * @param callId Call id
     * @param length Length of text
     * @param text Text
     * @param response Response object
     */
    @Override
    public void sendRttText(int callId, String text, int length, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SEND_RTT_TEXT, response,
                    mRILDefaultWorkSource);

            if (IMSRIL_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " callId = " + callId
                        + " text = " + text
                        + " length = " + length);
            }

            try {
                radioProxy.sendRttText(rr.mSerial, callId, length, text);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "sendRttText", e);
            }
        }
    }

    /**
     * Set RTT Mode
     * RIL_REQUEST_SET_RTT_MODE
     * @param mode Mode
     * @param response Response object
     */
    @Override
    public void setRttMode(int mode, Message response) {

        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_RTT_MODE, response,
                    mRILDefaultWorkSource);

            if (IMSRIL_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " mode = " + mode);
            }

            try {
                radioProxy.setRttMode(rr.mSerial, mode);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setRttMode", e);
            }
        }
    }




    static String responseToString(int response) {
        switch(response) {
            case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED: return "RADIO_STATE_CHANGED";
            case ImsRILConstants.RIL_UNSOL_CALL_INFO_INDICATION: return "CALL_INFO_INDICATION";
            case ImsRILConstants.RIL_UNSOL_INCOMING_CALL_INDICATION:
                return "INCOMING_CALL_INDICATION";
            case ImsRILConstants.RIL_UNSOL_CIPHER_INDICATION: return "CIPHER_INDICATION";
            case ImsRILConstants.RIL_UNSOL_ECONF_RESULT_INDICATION : return "ECONF_RESULT_INDICATION";
            case ImsRILConstants.RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR:
                return "SIP_CALL_PROGRESS_INDICATOR";
            case ImsRILConstants.RIL_UNSOL_CALLMOD_CHANGE_INDICATOR:
                return "CALLMOD_CHANGE_INDICATOR";
            case ImsRILConstants.RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR:
                return "VIDEO_CAPABILITY_INDICATOR";
            case ImsRILConstants.RIL_UNSOL_ON_USSI: return "ON_USSI";
            case ImsRILConstants.RIL_UNSOL_GET_PROVISION_DONE: return "GET_PROVISION_DONE";
            case ImsRILConstants.RIL_UNSOL_IMS_RTP_INFO: return "UNSOL_IMS_RTP_INFO";
            case ImsRILConstants.RIL_UNSOL_ON_XUI: return "ON_XUI";
            case ImsRILConstants.RIL_UNSOL_ON_VOLTE_SUBSCRIPTION:
                return "ImsRILConstants.RIL_UNSOL_ON_VOLTE_SUBSCRIPTION";
            case RIL_UNSOL_SUPP_SVC_NOTIFICATION:
                return "RIL_UNSOL_SUPP_SVC_NOTIFICATION";
            case ImsRILConstants.RIL_UNSOL_IMS_EVENT_PACKAGE_INDICATION:
                return "IMS_EVENT_PACKAGE_INDICATION";
            case ImsRILConstants.RIL_UNSOL_IMS_REGISTRATION_INFO:
                return "IMS_REGISTRATION_INFO";
            case ImsRILConstants.RIL_UNSOL_SPEECH_CODEC_INFO : return "SPEECH_CODEC_INFO";
            case ImsRILConstants.RIL_UNSOL_IMS_ENABLE_DONE: return "IMS_ENABLE_DONE";
            case ImsRILConstants.RIL_UNSOL_IMS_DISABLE_DONE: return "IMS_DISABLE_DONE";
            case ImsRILConstants.RIL_UNSOL_IMS_ENABLE_START: return "IMS_ENABLE_START";
            case ImsRILConstants.RIL_UNSOL_IMS_DISABLE_START: return "IMS_DISABLE_START";
            case ImsRILConstants.RIL_UNSOL_IMS_DEREG_DONE: return "IMS_DEREG_DONE";
            case ImsRILConstants.RIL_UNSOL_ECT_INDICATION: return "ECT_INDICATION";
            case ImsRILConstants.RIL_UNSOL_VOLTE_SETTING: return "VOLTE_SETTING";
            case ImsRILConstants.RIL_UNSOL_IMS_BEARER_STATE_NOTIFY: return "IMS_BEARER_STATE_NOTIFY";
            case ImsRILConstants.RIL_UNSOL_IMS_BEARER_INIT: return "RIL_UNSOL_IMS_BEARER_INIT";
            case ImsRILConstants.RIL_UNSOL_IMS_DATA_INFO_NOTIFY: return "RIL_UNSOL_IMS_DATA_INFO_NOTIFY";
            case ImsRILConstants.RIL_UNSOL_IMS_MULTIIMS_COUNT: return "IMS_MULTIIMS_COUNT";
            case ImsRILConstants.RIL_UNSOL_IMS_CONFIG_DYNAMIC_IMS_SWITCH_COMPLETE:
                return "DYNAMIC_IMS_SWITCH_COMPLETE";
            case ImsRILConstants.RIL_UNSOL_IMS_CONFIG_CONFIG_CHANGED:
                return "IMS_CONFIG_CHANGED";
            case ImsRILConstants.RIL_UNSOL_IMS_CONFIG_FEATURE_CHANGED:
                return "IMS_FEATURE_CHANGED";
            case ImsRILConstants.RIL_UNSOL_IMS_CONFIG_CONFIG_LOADED:
                return "IMS_CONFIG_LOADED";
            case ImsRILConstants.RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX:
                return "RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX";
            case ImsRILConstants.RIL_UNSOL_RESPONSE_NEW_SMS_EX:
                return "RIL_UNSOL_RESPONSE_NEW_SMS_EX";
            case ImsRILConstants.RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_EX:
                return "UNSOL_CDMA_NEW_SMS_EX";
            case ImsRILConstants.RIL_UNSOL_NO_EMERGENCY_CALLBACK_MODE:
                return "RIL_UNSOL_NO_EMERGENCY_CALLBACK_MODE";
            case ImsRILConstants.RIL_UNSOL_IMS_CONFERENCE_INFO_INDICATION:
                return "RIL_UNSOL_IMS_CONFERENCE_INFO_INDICATION";
            case ImsRILConstants.RIL_UNSOL_LTE_MESSAGE_WAITING_INDICATION:
                return "RIL_UNSOL_LTE_MESSAGE_WAITING_INDICATION";
            case ImsRILConstants.RIL_UNSOL_IMS_DIALOG_INDICATION:
                return "RIL_UNSOL_IMS_DIALOG_INDICATION";
            case ImsRILConstants.RIL_UNSOL_RTT_MODIFY_RESPONSE:
                return "RIL_UNSOL_RTT_MODIFY_RESPONSE";
            case ImsRILConstants.RIL_UNSOL_RTT_TEXT_RECEIVE:
                return "RIL_UNSOL_RTT_TEXT_RECEIVE";
            case ImsRILConstants.RIL_UNSOL_RTT_CAPABILITY_INDICATION:
                return "RIL_UNSOL_RTT_CAPABILITY_INDICATION";
            case ImsRILConstants.RIL_UNSOL_RTT_MODIFY_REQUEST_RECEIVE:
                return "RIL_UNSOL_RTT_MODIFY_REQUEST_RECEIVE";
            case ImsRILConstants.RIL_UNSOL_AUDIO_INDICATION:
                return "RIL_UNSOL_AUDIO_INDICATION";
            case ImsRILConstants.RIL_UNSOL_VOPS_INDICATION:
                return "RIL_UNSOL_VOPS_INDICATION";
            case ImsRILConstants.RIL_UNSOL_CALL_ADDITIONAL_INFO:
                return "RIL_UNSOL_CALL_ADDITIONAL_INFO";
            case ImsRILConstants.RIL_UNSOL_SIP_HEADER:
                return "RIL_UNSOL_SIP_HEADER";
            default: return "<unknown response>" + String.valueOf(response);
        }
    }

    static String requestToString(int request) {
        switch(request) {
            case RIL_REQUEST_ANSWER: return "ANSWER";
            case RIL_REQUEST_CONFERENCE: return "CONFERENCE";
            case RIL_REQUEST_DIAL: return "DIAL";
            case RIL_REQUEST_DTMF: return "DTMF";
            case RIL_REQUEST_DTMF_START: return "DTMF_START";
            case RIL_REQUEST_DTMF_STOP: return "DTMF_STOP";
            case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
                return "EXIT_EMERGENCY_CALLBACK_MODE";
            case RIL_REQUEST_EXPLICIT_CALL_TRANSFER: return "EXPLICIT_CALL_TRANSFER";
            case RIL_REQUEST_GET_ACTIVITY_INFO: return "GET_ACTIVITY_INFO";
            case RIL_REQUEST_HANGUP: return "HANGUP";
            case ImsRILConstants.RIL_REQUEST_HANGUP_WITH_REASON: return "HANGUP_WITH_REASON";
            case ImsRILConstants.RIL_REQUEST_HANGUP_ALL: return "HANGUP_ALL";
            case RIL_REQUEST_LAST_CALL_FAIL_CAUSE: return "LAST_CALL_FAIL_CAUSE";
            case RIL_REQUEST_SET_MUTE: return "SET_MUTE";
            case RIL_REQUEST_SHUTDOWN: return "SHUTDOWN";
            case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
                return "SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
            case ImsRILConstants.RIL_REQUEST_SET_IMS_ENABLE: return "SET_IMS_ENABLE";
            case ImsRILConstants.RIL_REQUEST_SET_VOLTE_ENABLE: return "SET_VOLTE_ENABLE";
            case ImsRILConstants.RIL_REQUEST_SET_WFC_ENABLE: return "SET_WFC_ENABLE";
            case ImsRILConstants.RIL_REQUEST_SET_VILTE_ENABLE: return "SET_VILTE_ENABLE";
            case ImsRILConstants.RIL_REQUEST_SET_VIWIFI_ENABLE: return "SET_VIWIFI_ENABLE";
            case ImsRILConstants.RIL_REQUEST_SET_IMS_VIDEO_ENABLE: return "SET_IMS_VIDEO_ENABLE";
            case ImsRILConstants.RIL_REQUEST_VIDEO_CALL_ACCEPT: return "VIDEO_CALL_ACCEPT";
            case ImsRILConstants.RIL_REQUEST_ECC_REDIAL_APPROVE: return "ECC_REDIAL_APPROVE";
            case ImsRILConstants.RIL_REQUEST_SET_IMSCFG: return "SET_IMSCFG";
            case ImsRILConstants.RIL_REQUEST_GET_PROVISION_VALUE: return "GET_PROVISION_VALUE";
            case ImsRILConstants.RIL_REQUEST_SET_PROVISION_VALUE: return "SET_PROVISION_VALUE";
            case ImsRILConstants.RIL_REQUEST_IMS_CONFIG_SET_FEATURE:
                return "IMS_CONFIG_SET_FEATURE";
            case ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_FEATURE:
                return "IMS_CONFIG_GET_FEATURE";
            case ImsRILConstants.RIL_REQUEST_IMS_CONFIG_SET_PROVISION:
                return "IMS_CONFIG_SET_PROVISION";
            case ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_PROVISION:
                return "IMS_CONFIG_GET_PROVISION";
            case ImsRILConstants.RIL_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP:
                return "IMS_CONFIG_GET_RESOURCE_CAP";
            case ImsRILConstants.RIL_REQUEST_IMS_BEARER_STATE_CONFIRM:
                return "IMS_BEARER_STATE_CONFIRM";
            case ImsRILConstants.RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION:
                return "RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION";
            case ImsRILConstants.RIL_REQUEST_IMS_DEREG_NOTIFICATION: return "IMS_DEREG_NOTIFICATION";
            case ImsRILConstants.RIL_REQUEST_IMS_ECT: return "IMS_ECT";
            case ImsRILConstants.RIL_REQUEST_HOLD_CALL: return "HOLD_CALL";
            case ImsRILConstants.RIL_REQUEST_RESUME_CALL: return "RESUME_CALL";
            case ImsRILConstants.RIL_REQUEST_SET_ECC_LIST: return "SET_ECC_LIST";
            case ImsRILConstants.RIL_REQUEST_VT_DIAL_WITH_SIP_URI: return "VT_DIAL_WITH_SIP_URI";
            case ImsRILConstants.RIL_REQUEST_SEND_USSI: return "SEND_USSI";
            case ImsRILConstants.RIL_REQUEST_CANCEL_USSI: return "CANCEL_USSI";
            case ImsRILConstants.RIL_REQUEST_SET_WFC_PROFILE: return "SET_WFC_PROFILE";
            case ImsRILConstants.RIL_REQUEST_SET_IMS_REGISTRATION_REPORT:
                return "SET_IMS_REGISTRATION_REPORT";
            case ImsRILConstants.RIL_REQUEST_PULL_CALL: return "PULL_CALL";
            case ImsRILConstants.RIL_REQUEST_IMS_DIAL: return "IMS_DIAL";
            case ImsRILConstants.RIL_REQUEST_IMS_VT_DIAL: return "IMS_VT_DIAL";
            case ImsRILConstants.RIL_REQUEST_IMS_EMERGENCY_DIAL: return "IMS_EMERGENCY_DIAL";
            case ImsRILConstants.RIL_REQUEST_SET_IMS_RTP_REPORT: return "ImsRILConstants.RIL_REQUEST_SET_IMS_RTP_REPORT";
            case ImsRILConstants.RIL_REQUEST_CONFERENCE_DIAL: return "RIL_REQUEST_CONFERENCE_DIAL";
            case ImsRILConstants.RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER:
                return "RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER";
            case ImsRILConstants.RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER:
                return "RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER";
            case ImsRILConstants.RIL_REQUEST_SET_CALL_INDICATION:
                return "RIL_REQUEST_SET_CALL_INDICATION";
            case ImsRILConstants.RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE:
                return "RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE";
            case ImsRILConstants.RIL_REQUEST_SET_MD_IMSCFG: return "ImsRILConstants.RIL_REQUEST_SET_MD_IMSCFG";
            // SMS-START
            case ImsRILConstants.RIL_REQUEST_IMS_SEND_SMS_EX: return "RIL_REQUEST_IMS_SEND_SMS_EX";
            case ImsRILConstants.RIL_REQUEST_SMS_ACKNOWLEDGE_EX: return "RIL_REQUEST_SMS_ACKNOWLEDGE_EX";
            case ImsRILConstants.RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX: return "CDMA_SMS_ACKNOWLEDGE_EX";
            // SMS-END
            //MTK-START SS
            case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS: return "QUERY_CALL_FORWARD_STATUS";
            case RIL_REQUEST_SET_CALL_FORWARD: return "SET_CALL_FORWARD";
            case RIL_REQUEST_GET_CLIR: return "GET_CLIR";
            case RIL_REQUEST_SET_CLIR: return "SET_CLIR";
            case RIL_REQUEST_QUERY_CALL_WAITING: return "QUERY_CALL_WAITING";
            case RIL_REQUEST_SET_CALL_WAITING: return "SET_CALL_WAITING";
            case RIL_REQUEST_QUERY_FACILITY_LOCK: return "QUERY_FACILITY_LOCK";
            case RIL_REQUEST_SET_FACILITY_LOCK: return "SET_FACILITY_LOCK";
            case RIL_REQUEST_CHANGE_BARRING_PASSWORD: return "CHANGE_BARRING_PASSWORD";
            case RIL_REQUEST_QUERY_CLIP: return "QUERY_CLIP";
            case ImsRILConstants.RIL_REQUEST_SET_CLIP: return "SET_CLIP";
            case ImsRILConstants.RIL_REQUEST_GET_COLP: return "GET_COLP";
            case ImsRILConstants.RIL_REQUEST_SET_COLP: return "SET_COLP";
            case ImsRILConstants.RIL_REQUEST_GET_COLR: return "GET_COLR";
            case ImsRILConstants.RIL_REQUEST_SET_COLR: return "SET_COLR";
            case ImsRILConstants.RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT:
                return "QUERY_CALL_FORWARD_IN_TIME_SLOT";
            case ImsRILConstants.RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT: return "SET_CALL_FORWARD_IN_TIME_SLOT";
            case ImsRILConstants.RIL_REQUEST_RUN_GBA: return "RIL_REQUEST_RUN_GBA";
            case ImsRILConstants.RIL_REQUEST_GET_XCAP_STATUS: return "RIL_REQUEST_GET_XCAP_STATUS";
            case ImsRILConstants.RIL_REQUEST_RESET_SUPP_SERV: return "RIL_REQUEST_RESET_SUPP_SERV";
            case ImsRILConstants.RIL_REQUEST_SETUP_XCAP_USER_AGENT_STRING: return "SETUP_XCAP_USER_AGENT_STRING";
            //MTK-END SS
            case ImsRILConstants.RIL_REQUEST_SET_RCS_UA_ENABLE: return "RIL_REQUEST_SET_RCS_UA_ENABLE";
            //MTK-START END
            case ImsRILConstants.RIL_REQUEST_SET_RTT_MODE:
                return "RIL_REQUEST_SET_RTT_MODE";
            case ImsRILConstants.RIL_REQUEST_SEND_RTT_MODIFY_REQUEST:
                return "RIL_REQUEST_SEND_RTT_MODIFY_REQUEST";
            case ImsRILConstants.RIL_REQUEST_SEND_RTT_TEXT:
                return "RIL_REQUEST_SEND_RTT_TEXT";
            case ImsRILConstants.RIL_REQUEST_RTT_MODIFY_REQUST_RESPONSE:
                return "RIL_REQUEST_RTT_MODIFY_REQUST_RESPONSE";
            case ImsRILConstants.RIL_REQUEST_QUERY_VOPS_STATUS: return "RIL_REQUEST_QUERY_VOPS_STATUS";
            //Client API Start
            case ImsRILConstants.RIL_REQUEST_SET_SIP_HEADER:
                return "RIL_REQUEST_SET_SIP_HEADER";
            case ImsRILConstants.RIL_REQUEST_SIP_HEADER_REPORT:
                return "RIL_REQUEST_SIP_HEADER_REPORT";
            case ImsRILConstants.RIL_REQUEST_SET_IMS_CALL_MODE:
                return "RIL_REQUEST_SET_IMS_CALL_MODE";
            case ImsRILConstants.RIL_REQUEST_QUERY_SSAC_STATUS:
                return "RIL_REQUEST_QUERY_SSAC_STATUS";
            case ImsRILConstants.RIL_REQUEST_TOGGLE_RTT_AUDIO_INDICATION:
                return "RIL_REQUEST_TOGGLE_RTT_AUDIO_INDICATION";
            //Client API End
            case ImsRILConstants.RIL_REQUEST_SET_CALL_ADDITIONAL_INFO:
                return "RIL_REQUEST_SET_CALL_ADDITIONAL_INFO";
            default: return "<unknown request>: " + String.valueOf(request);
        }
    }

    static String retToString(int req, Object ret) {
        if (ret == null) return "";

        StringBuilder sb;
        String s;
        int length;
        if (ret instanceof int[]) {
            int[] intArray = (int[]) ret;
            length = intArray.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(intArray[i++]);
                while (i < length) {
                    sb.append(", ").append(intArray[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else if (ret instanceof String[]) {
            String[] strings = (String[]) ret;
            length = strings.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(strings[i++]);
                while (i < length) {
                    sb.append(", ").append(strings[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else {
            s = ret.toString();
        }
        return s;
    }

    void riljLogi(String msg) {
        Rlog.i(IMSRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""));
    }

    void riljLog(String msg) {
        Rlog.d(IMSRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""));
    }

    void riljLoge(String msg) {
        Rlog.e(IMSRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""));
    }

    void riljLoge(String msg, Exception e) {
        Rlog.e(IMSRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""), e);
    }

    void riljLogv(String msg) {
        Rlog.v(IMSRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""));
    }

    void unsljLog(int response) {
        riljLog("[UNSL]< " + responseToString(response));
    }

    void unsljLogMore(int response, String more) {
        riljLog("[UNSL]< " + responseToString(response) + " " + more);
    }

    void unsljLogRet(int response, Object ret) {
        riljLog("[UNSL]< " + responseToString(response) + " " + retToString(response, ret));
    }

    void unsljLogvRet(int response, Object ret) {
        riljLogv("[UNSL]< " + responseToString(response) + " " + retToString(response, ret));
    }

    /// Deprecated Methods Here for Backward Compatible ///////////////////////

    @Override
    @Deprecated
    public void accept() {
        accept(null);
    }

    @Override
    @Deprecated
    public void getProvisionValue(int phoneid, String provisionStr,
            Message response) {
        getProvisionValue(provisionStr, response);
    }

    @Override
    @Deprecated
    public void setProvisionValue(int phoneid, String provisionStr,
            String provisionValue, Message response) {
        setProvisionValue(provisionStr, provisionValue, response);
    }

    @Override
    @Deprecated
    public void sendWfcProfileInfo(int wfcPreference, int phoneid, Message response) {
        sendWfcProfileInfo(wfcPreference, response);
    }

    @Override
    @Deprecated
    public void responseBearerStateConfirm(int phoneid, int aid, int action, int status) {
        responseBearerStateConfirm(aid, action, status, null);
    }

    @Override
    @Deprecated
    public void setImsBearerNotification(int phoneid, int enable) {
        setImsBearerNotification(enable, null);
    }

    @Override
    @Deprecated
    public void startConference(String[] participants, int clirMode,
            boolean isVideoCall, int phoneid, Message response) {
        startConference(participants, clirMode, isVideoCall, response);
    }

    @Override
    @Deprecated
    public void getLastCallFailCause(int phoneid, Message response) {
        getLastCallFailCause(response);
    }

    @Override
    @Deprecated
    public void setImsCfg(int[] params, int phoneid, Message response) {
        setImsCfg(params, response);
    }

    @Override
    @Deprecated
    public void setImsRegistrationReport(int phoneid, Message response) {
        setImsRegistrationReport(response);
    }

    @Override
    @Deprecated
    public void turnOffIms(int phoneid, Message response) {
        turnOffIms(response);
    }

    @Override
    @Deprecated
    public void turnOnIms(int phoneid, Message response) {
        turnOnIms(response);
    }

    @Override
    @Deprecated
    public void reject(int callId) {
        hangup(callId, null);
    }

    @Deprecated
    public void reject(int callId, int reason) {
        riljLog(" reject with reason: " + reason);
        hangup(callId, reason, null);
    }

    @Override
    @Deprecated
    public void terminate(int callId) {
        hangup(callId, null);
    }

    @Deprecated
    public void terminate(int callId, int reason) {
        riljLog("terminate with reason: " + reason);
        hangup(callId, reason, null);;
    }

    @Override
    @Deprecated
    public void merge(Message response) {
        conference(response);
    }

    @Override
    @Deprecated
    public void forceHangup(int callId) {
        forceHangup(callId, null);
    }

    @Deprecated
    public void forceHangup(int callId, int reason) {
        riljLog("forceHangup with reason: " + reason);
        forceHangup(callId, null); //TODO: send reason further.
    }

    @Override
    @Deprecated
    public void acceptVideoCall(int videoMode, int callId) {
        acceptVideoCall(videoMode, callId, null);
    }

    @Override
    @Deprecated
    public void setCallIndication(int mode, int callId, int seqNum, int cause) {
        setCallIndication(mode, callId, seqNum, cause, null);
    }

    private void resetMtkProxyAndRequestList() {
        mMtkRadioProxy = null;

        // increment the cookie so that death notification can be ignored
        mMtkRadioProxyCookie.incrementAndGet();

        setRadioState(TelephonyManager.RADIO_POWER_UNAVAILABLE);

        RILRequest.resetSerial();
        // Clear request list on close
        clearRequestList(RADIO_NOT_AVAILABLE, false);

        // todo: need to get service right away so setResponseFunctions() can be called for
        // unsolicited indications. getService() is not a blocking call, so it doesn't help to call
        // it here. Current hack is to call getService() on death notification after a delay.
    }

    private IMtkRadioEx getMtkRadioProxy(Message result) {
        if (!mIsMobileNetworkSupported) {
            if (IMSRIL_LOGV) riljLog("getRadioProxy: Not calling getService(): wifi-only");
            return null;
        }

        if (mMtkRadioProxy != null) {
            return mMtkRadioProxy;
        }

        try {
            mMtkRadioProxy = IMtkRadioEx.getService(
                    MTK_IMS_HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId], false);
            if (mMtkRadioProxy != null) {
                mMtkRadioProxy.linkToDeath(mMtkRadioProxyDeathRecipient,
                        mMtkRadioProxyCookie.incrementAndGet());

                mMtkRadioProxy.setResponseFunctionsIms(mImsRadioResponse, mImsRadioIndication);
            } else {
                riljLoge("getMtkRadioProxy: mMtkRadioProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mRadioProxy = null;
            riljLoge("getMtkRadioProxy getService/setResponseFunctions: " + e);
        }

        if (mMtkRadioProxy == null) {
            if (result != null) {
                AsyncResult.forMessage(result, null,
                        CommandException.fromRilErrno(RADIO_NOT_AVAILABLE));
                result.sendToTarget();
            }
        }

        return mMtkRadioProxy;
    }

    public void queryVopsStatus(Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_QUERY_VOPS_STATUS, result,
                    mRILDefaultWorkSource);

            try {
                radioProxy.queryVopsStatus(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "queryVopsStatus", e);
            }
        }
    }

    public void notifyImsServiceReady() {
        IMtkRadioEx radioProxy = getMtkRadioProxy(null);
        if (radioProxy != null) {
            try {
                radioProxy.notifyImsServiceReady();
            } catch (RemoteException | RuntimeException e) {
                riljLoge("notifyImsServiceReady error: " + e);
            }
        }
    }

    public void querySsacStatus(Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_QUERY_SSAC_STATUS, result,
                    mRILDefaultWorkSource);

            try {
                radioProxy.querySsacStatus(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "queryVopsStatus", e);
            }
        }
    }

    public void toggleRttAudioIndication(int callId, int enable, Message response) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_TOGGLE_RTT_AUDIO_INDICATION,
                    response, mRILDefaultWorkSource);
            if (IMSRIL_LOGD) {
                riljLog(rr.serialString() + ">  " + requestToString(rr.mRequest)
                        + " callId = " + callId + " enable = " + enable);
            }

            try {
                radioProxy.toggleRttAudioIndication(rr.mSerial, callId, enable);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "toggleRttAudioIndication", e);
            }
        }
    }

    public void setCallAdditionalInfo(ArrayList<String> info, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(ImsRILConstants.RIL_REQUEST_SET_CALL_ADDITIONAL_INFO,
                    result, mRILDefaultWorkSource);

            if (IMS_RILA_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.setCallAdditionalInfo(rr.mSerial, info);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setCallAdditionalInfo", e);
            }
        }
    }
}

