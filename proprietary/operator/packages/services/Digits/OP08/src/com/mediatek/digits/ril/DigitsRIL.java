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


package com.mediatek.digits.ril;

import static com.android.internal.telephony.RILConstants.RADIO_NOT_AVAILABLE;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_GET_ACTIVITY_INFO;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_SHUTDOWN;
import static com.android.internal.telephony.RILConstants.RIL_RESPONSE_ACKNOWLEDGEMENT;

import static com.mediatek.opcommon.telephony.MtkRILConstantsOp.RIL_REQUEST_SET_DIGITS_LINE;
import static com.mediatek.opcommon.telephony.MtkRILConstantsOp.RIL_REQUEST_SET_TRN;

import static com.mediatek.opcommon.telephony.MtkRILConstantsOp.RIL_UNSOL_DIGITS_LINE_INDICATION;
import static com.mediatek.opcommon.telephony.MtkRILConstantsOp.RIL_UNSOL_GET_TRN_INDICATION;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import vendor.mediatek.hardware.radio_op.V2_0.IRadioOp;
import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioIndicationType;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.RadioResponseType;

import android.content.Context;
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
import android.telephony.ModemActivityInfo;
import android.telephony.Rlog;
import android.telephony.TelephonyHistogram;
import android.util.SparseArray;

import com.android.internal.telephony.ClientWakelockTracker;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.metrics.TelephonyMetrics;

/**
 * {@hide}
 */
class RILRequest {
    static final String LOG_TAG = "DigitsRIL-RilRequest";

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

        if (DigitsRIL.DIGITS_RILA_LOGD) Rlog.d(LOG_TAG, serialString() + "< "
            + DigitsRIL.requestToString(mRequest)
            + " error: " + ex + " ret=" + DigitsRIL.retToString(mRequest, ret));

        if (mResult != null) {
            AsyncResult.forMessage(mResult, ret, ex);
            mResult.sendToTarget();
        }
    }
}



/**
 * Digits RIL Adapter implementation.
 *
 * {@hide}
 */
public final class DigitsRIL extends DigitsBaseCommands implements DigitsCommandsInterface {

    static final String DigitsRIL_LOG_TAG = "DigitsRIL";
    // Have a separate wakelock instance for Ack
    static final String RILJ_ACK_WAKELOCK_NAME = "DigitsRIL_ACK_WL";
    static final boolean DigitsRIL_LOGD = true;
    static final boolean DigitsRIL_LOGV = false; // STOPSHIP if true
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

    boolean mIsMobileNetworkSupported;
    DigitsRadioResponse mRadioResponse;
    DigitsRadioIndication mRadioIndication;
    volatile IRadioOp mRadioProxy = null;

    final AtomicLong mRadioProxyCookie = new AtomicLong(0);
    final RadioProxyDeathRecipient mRadioProxyDeathRecipient;
    final RilHandler mRilHandler;

    //***** Events
    static final int EVENT_SEND                      = 1;
    static final int EVENT_WAKE_LOCK_TIMEOUT         = 2;
    static final int EVENT_ACK_WAKE_LOCK_TIMEOUT     = 4;
    static final int EVENT_BLOCKING_RESPONSE_TIMEOUT = 5;
    static final int EVENT_RADIO_PROXY_DEAD          = 6;

    //***** Constants
    static final String [] IMS_HIDL_SERVICE_NAME =
        {"OpImsRILd1", "OpImsRILd2", "OpImsRILd3","OpImsRILd4"};

    static final int IRADIO_GET_SERVICE_DELAY_MILLIS = 4 * 1000;
    static final boolean DIGITS_RILA_LOGD = true;

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
                            if (DigitsRIL_LOGD) {
                                int count = mRequestList.size();
                                Rlog.d(DigitsRIL_LOG_TAG, "WAKE_LOCK_TIMEOUT " +
                                        " mRequestList=" + count);
                                for (int i = 0; i < count; i++) {
                                    rr = mRequestList.valueAt(i);
                                    Rlog.d(DigitsRIL_LOG_TAG, i + ": [" + rr.mSerial + "] "
                                            + requestToString(rr.mRequest));
                                }
                            }
                        }
                    }
                    break;

                case EVENT_ACK_WAKE_LOCK_TIMEOUT:
                    if (msg.arg1 == mAckWlSequenceNum && clearWakeLock(FOR_ACK_WAKELOCK)) {
                        if (DigitsRIL_LOGV) {
                            Rlog.d(DigitsRIL_LOG_TAG, "ACK_WAKE_LOCK_TIMEOUT");
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
            mRilHandler.sendMessageDelayed(
                    mRilHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie),
                    IRADIO_GET_SERVICE_DELAY_MILLIS);
        }
    }

    private void resetProxyAndRequestList() {
        mRadioProxy = null;

        // increment the cookie so that death notification can be ignored
        mRadioProxyCookie.incrementAndGet();

        // setRadioState(RadioState.RADIO_UNAVAILABLE);

        RILRequest.resetSerial();
        // Clear request list on close
        clearRequestList(RADIO_NOT_AVAILABLE, false);

        // todo: need to get service right away so setResponseFunctions() can be called for
        // unsolicited indications. getService() is not a blocking call, so it doesn't help to call
        // it here. Current hack is to call getService() on death notification after a delay.
    }

    private IRadioOp getRadioProxy(Message result) {
        if (!mIsMobileNetworkSupported) {
            if (DigitsRIL_LOGV) riljLog("getRadioProxy: Not calling getService(): wifi-only");
            return null;
        }

        if (mRadioProxy != null) {
            return mRadioProxy;
        }

        try {
            mRadioProxy = IRadioOp.getService(
                          IMS_HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId]);

            if (mRadioProxy != null) {
                mRadioProxy.linkToDeath(mRadioProxyDeathRecipient,
                        mRadioProxyCookie.incrementAndGet());

                mRadioProxy.setResponseFunctionsDigits(mRadioResponse, mRadioIndication);

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

            // if service is not up, treat it like death notification to try to get service again
            mRilHandler.sendMessageDelayed(
                    mRilHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD,
                            mRadioProxyCookie.incrementAndGet()),
                    IRADIO_GET_SERVICE_DELAY_MILLIS);
        }

        return mRadioProxy;
    }

    //***** Constructors

    public DigitsRIL(Context context, int instanceId) {
        super(context, instanceId);

        mContext = context;
        mPhoneId = instanceId;

        ConnectivityManager cm = (ConnectivityManager)context.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        mIsMobileNetworkSupported = cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE);

        mRadioResponse = new DigitsRadioResponse(this, instanceId);
        mRadioIndication = new DigitsRadioIndication(this, instanceId);

        mRilHandler = new RilHandler();
        mRadioProxyDeathRecipient = new RadioProxyDeathRecipient();
        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, DigitsRIL_LOG_TAG);
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
        IRadioOp proxy = getRadioProxy(null);
        if (DIGITS_RILA_LOGD) {
            riljLog("DigitsRIL: proxy = " + (proxy == null));
        }
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

        // service most likely died, handle exception like death notification to try to get service
        // again
        mRilHandler.sendMessageDelayed(
                mRilHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD,
                        mRadioProxyCookie.incrementAndGet()),
                IRADIO_GET_SERVICE_DELAY_MILLIS);
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

    /// Public APIs for IRadioOP V1.1 ///////////////////////////////////////////////////

    /**
     * Request XXX
     * Example of Ril Action
     * @param callId Call id
     * @param result Result
     * @param response Response object
     */
    /*******************************************************************************
    @Override
    public void rilAction(Message response) {

        IRadioOp radioProxy = getRadioProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_XXX_XXX, result,
                    mRILDefaultWorkSource);

            if (DIGITS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " switch = OFF");
            }

            try {
                radioProxy.xxxx(rr.mSerial, false);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "xxxxx", e);
            }
        }
    }
    ******************************************************************************/

    /// Public APIs for IRadioOP V2.1 ///////////////////////////////////////////////////

    /**
     * Set Digits Line
     * RIL_REQUEST_SET_DIGITS_LINE
     *
     * @param accountId Accound id
     * @param digitsSerial Serial number
     * @param isLogout Is logout
     * @param hasNext If has next AT command
     * @param isNative If it's a native line
     * @param msisdn Msisdn
     * @param sit Service instance token (SIT)
     * @param response Response object
     */
    @Override
    public void setDigitsLine(int accountId, int digitsSerial, boolean isLogout, boolean hasNext,
        boolean isNative, String msisdn, String sit, Message response) {

        IRadioOp radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_DIGITS_LINE, response,
                    mRILDefaultWorkSource);

            if (DIGITS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest)
                        + " accountId = " + accountId + " digitsSerial = " + digitsSerial
                        + " isLogout = " + isLogout + " hasNext = " + hasNext
                        + " isNative = " + isNative + " msisdn = " + msisdn + "sit = " + sit);
            }

            try {
                radioProxy.setDigitsLine(rr.mSerial, accountId, digitsSerial, isLogout,
                    hasNext, isNative, msisdn, sit);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setDigitsLineResponse", e);
            }
        }
    }

    /**
     * Set Trn
     * RIL_REQUEST_SET_TRN
     *
     * @param fromMsisdn
     * @param toMsisdn
     * @param trn
     */
    @Override
    public void setTrn(String fromMsisdn, String toMsisdn, String trn, Message response) {

        IRadioOp radioProxy = getRadioProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_TRN, response,
                    mRILDefaultWorkSource);

            if (DIGITS_RILA_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " fromMsisdn = " + fromMsisdn +
                        " toMsisdn = " + toMsisdn + " trn = " + trn );
            }

            try {
                radioProxy.setTrn(rr.mSerial, fromMsisdn, toMsisdn, trn);
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "setTrnResponse", e);
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
            if (DigitsRIL_LOGD) riljLog("Unsol response received; Sending ack to ril.cpp");
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
            Rlog.w(DigitsRIL.DigitsRIL_LOG_TAG,
                    "processRequestAck: Unexpected solicited ack response! " +
                    "serial: " + serial);
        } else {
            decrementWakeLock(rr);
            if (DigitsRIL_LOGD) {
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
    RILRequest processResponse(RadioResponseInfo responseInfo) {
        int serial = responseInfo.serial;
        int error = responseInfo.error;
        int type = responseInfo.type;

        RILRequest rr = null;

        if (type == RadioResponseType.SOLICITED_ACK) {
            synchronized (mRequestList) {
                rr = mRequestList.get(serial);
            }
            if (rr == null) {
                Rlog.w(DigitsRIL_LOG_TAG, "Unexpected solicited ack response! sn: " + serial);
            } else {
                decrementWakeLock(rr);
                if (DIGITS_RILA_LOGD) {
                    riljLog(rr.serialString() + " Ack < " + requestToString(rr.mRequest));
                }
            }
            return rr;
        }

        rr = findAndRemoveRequestFromList(serial);
        if (rr == null) {
            Rlog.e(DigitsRIL.DigitsRIL_LOG_TAG, "processResponse: Unexpected response! serial: " + serial
                    + " error: " + error);
            return null;
        }

        // Time logging for RIL command and storing it in TelephonyHistogram.
        addToRilHistogram(rr);

        if (type == RadioResponseType.SOLICITED_ACK_EXP) {
            sendAck();
            if (DIGITS_RILA_LOGD) {
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
            if (DIGITS_RILA_LOGD) {
                riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                        + " " + retToString(rr.mRequest, ret));
            }
        } else {
            if (DIGITS_RILA_LOGD) {
                riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                        + " error " + responseInfo.error);
            }
            rr.onError(responseInfo.error, ret);
        }
        mMetrics.writeOnRilSolicitedResponse(mPhoneId, rr.mSerial, responseInfo.error,
                rr.mRequest, ret);
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
        IRadioOp radioProxy = getRadioProxy(null);
        if (radioProxy != null) {
            try {
                radioProxy.responseAcknowledgement();
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "sendAck", e);
                riljLoge("sendAck: " + e);
            }
        } else {
            Rlog.e(DigitsRIL_LOG_TAG, "Error trying to send ack, radioProxy = null");
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
                Rlog.d(DigitsRIL_LOG_TAG, "Failed to aquire wakelock for " + rr.serialString());
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
                    Rlog.w(DigitsRIL_LOG_TAG, "Acquiring Invalid Wakelock type " + wakeLockType);
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
                    Rlog.w(DigitsRIL_LOG_TAG, "Decrementing Invalid Wakelock type " + rr.mWakeLockType);
            }
            rr.mWakeLockType = INVALID_WAKELOCK;
        }
    }

    private boolean clearWakeLock(int wakeLockType) {
        if (wakeLockType == FOR_WAKELOCK) {
            synchronized (mWakeLock) {
                if (mWakeLockCount == 0 && !mWakeLock.isHeld()) return false;
                Rlog.d(DigitsRIL_LOG_TAG, "NOTE: mWakeLockCount is " + mWakeLockCount
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
            if (DigitsRIL_LOGD && loggable) {
                Rlog.d(DigitsRIL_LOG_TAG, "clearRequestList " + " mWakeLockCount="
                        + mWakeLockCount + " mRequestList=" + count);
            }

            for (int i = 0; i < count; i++) {
                rr = mRequestList.valueAt(i);
                if (DigitsRIL_LOGD && loggable) {
                    Rlog.d(DigitsRIL_LOG_TAG, i + ": [" + rr.mSerial + "] "
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

    static String responseToString(int request)
    {
        switch(request) {
            case RIL_UNSOL_DIGITS_LINE_INDICATION:
                return "RIL_UNSOL_DIGITS_LINE_INDICATION";
            case RIL_UNSOL_GET_TRN_INDICATION:
                return "RIL_UNSOL_GET_TRN_INDICATION";
            default: return "<unknown response>";
        }
    }

    static String requestToString(int request) {
        switch(request) {
            case RIL_REQUEST_SET_DIGITS_LINE:
                return "RIL_REQUEST_SET_DIGITS_LINE";
            case RIL_REQUEST_SET_TRN:
                return "RIL_REQUEST_SET_TRN";
            default: return "<unknown request>";
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

    void riljLog(String msg) {
        Rlog.d(DigitsRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""));
    }

    void riljLoge(String msg) {
        Rlog.e(DigitsRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""));
    }

    void riljLoge(String msg, Exception e) {
        Rlog.e(DigitsRIL_LOG_TAG, msg
                + (mPhoneId != null ? (" [SUB" + mPhoneId + "]") : ""), e);
    }

    void riljLogv(String msg) {
        Rlog.v(DigitsRIL_LOG_TAG, msg
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

    /// Mediatek APIs /////////////////////////////////////////////////////////


    /// Deprecated Methods Here for Backward Compatible ///////////////////////


}

