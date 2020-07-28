/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.ims.legacy.ss;

import android.app.ActivityManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;

import android.net.ConnectivityManager;
import android.net.Network;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.os.PowerManager.WakeLock;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;

import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;

import java.net.Authenticator;
import java.net.PasswordAuthentication;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.mediatek.gba.GbaHttpUrlCredential;
import com.mediatek.ims.OperatorUtils.OPID;
import com.mediatek.internal.telephony.MtkCallForwardInfo;
import com.mediatek.internal.telephony.MtkSuppServHelper;
import com.mediatek.simservs.client.SimServs;
import com.mediatek.simservs.client.CommunicationDiversion;
import com.mediatek.simservs.client.CommunicationWaiting;
import com.mediatek.simservs.client.IncomingCommunicationBarring;
import com.mediatek.simservs.client.SimservType;
import com.mediatek.simservs.client.OriginatingIdentityPresentation;
import com.mediatek.simservs.client.OriginatingIdentityPresentationRestriction;
import com.mediatek.simservs.client.TerminatingIdentityPresentation;
import com.mediatek.simservs.client.TerminatingIdentityPresentationRestriction;

import com.mediatek.ims.ril.ImsRILConstants;
import com.mediatek.ims.OperatorUtils;
import com.mediatek.ims.SuppSrvConfig;


import com.mediatek.simservs.client.OutgoingCommunicationBarring;
import com.mediatek.simservs.client.policy.Actions;
import com.mediatek.simservs.client.policy.Conditions;
import com.mediatek.simservs.client.policy.ForwardTo;
import com.mediatek.simservs.client.policy.Rule;
import com.mediatek.simservs.client.policy.RuleSet;
import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.Map;
import java.util.TimeZone;

/**
 * {@hide}
 */
class MMTelSSRequest {
    static final String LOG_TAG = "MMTelSSReq";

    //***** Class Variables
    static int sNextSerial = 0;
    static Object sSerialMonitor = new Object();
    private static Object sPoolSync = new Object();
    private static MMTelSSRequest sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 4;

    //***** Instance Variables
    int mSerial;
    int mRequest;
    Message mResult;
    Parcel mp;
    //For complex SS Operation: It can be used to carry the Rule or RuleSet object for
    //MMTelSSTransmitter to parse & compare with remote XCAP server's data
    //Add by mtk01411 2013-0911
    Object requestParm;
    MMTelSSRequest mNext;

    /**
     * Retrieves a new MMTelSSRequest instance from the pool.
     *
     * @param request MMTELSS_REQ_*
     * @param result sent when operation completes
     * @return a MMTelSSRequest instance from the pool.
     */
    static MMTelSSRequest obtain(int request, Message result) {
        MMTelSSRequest rr = null;

        synchronized (sPoolSync) {
            if (sPool != null) {
                rr = sPool;
                sPool = rr.mNext;
                rr.mNext = null;
                sPoolSize--;
            }
        }

        if (rr == null) {
            rr = new MMTelSSRequest();
        }

        synchronized (sSerialMonitor) {
            rr.mSerial = sNextSerial++;
        }
        rr.mRequest = request;
        rr.mResult = result;
        rr.mp = Parcel.obtain();

        if (result != null && result.getTarget() == null) {
            throw new NullPointerException("Message target must not be null");
        }

        // first elements in any MMTelSSRequest Parcel (Before returning the rr, it already
        // fills two elements into the Parcel)
        rr.mp.writeInt(request);
        rr.mp.writeInt(rr.mSerial);

        return rr;
    }

    /**
     * Returns a MMTelSSRequest instance to the pool.
     *
     * Note: This should only be called once per use.
     */
    void release() {
        synchronized (sPoolSync) {
            if (sPoolSize < MAX_POOL_SIZE) {
                this.mNext = sPool;
                sPool = this;
                sPoolSize++;
                mResult = null;
            }
        }
    }

    private MMTelSSRequest() {
    }

    static void
    resetSerial() {
        synchronized (sSerialMonitor) {
            sNextSerial = 0;
        }
    }

    String
    serialString() {
        //Cheesy way to do %04d
        StringBuilder sb = new StringBuilder(8);
        String sn;

        sn = Integer.toString(mSerial);

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

        //[TBD] It should modify as XCAP Errno & Exception by mtk01411
        ex = CommandException.fromRilErrno(error);

        if (MMTelSSTransport.DBG) Rlog.d(LOG_TAG, serialString() + "< "
                + MMTelSSTransport.requestToString(mRequest)
                + " error: " + ex);

        if (mResult != null) {
            AsyncResult.forMessage(mResult, ret, ex);
            mResult.sendToTarget();
        }

        if (mp != null) {
            mp.recycle();
            mp = null;
        }
    }
}


/**
 * Implementation for MMTel SS Transport
 *
 * {@hide}
 *
 */
public final class MMTelSSTransport {
    private static final String LOG_TAG = "MMTelSS";
    static final boolean DBG = true;
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");

    // Singleton instance
    private static final MMTelSSTransport INSTANCE = new MMTelSSTransport();
    private PowerManager pm = null;
    HandlerThread mSenderThread;
    MMTelSSTransmitter mSender;
    Context mContext = null;
    private XcapMobileDataNetworkManager mXcapMobileDataNetworkManager = null;
    private Network mNetwork = null;

    //***** MMTelSSRequest
    static final int MMTELSS_REQ_SET_CLIR               = 1;
    static final int MMTELSS_REQ_GET_CLIR               = 2;
    static final int MMTELSS_REQ_GET_CLIP               = 3;
    static final int MMTELSS_REQ_GET_COLP               = 4;
    static final int MMTELSS_REQ_GET_COLR               = 5;
    static final int MMTELSS_REQ_SET_CB                 = 6;
    static final int MMTELSS_REQ_GET_CB                 = 7;
    static final int MMTELSS_REQ_SET_CF                 = 8;
    static final int MMTELSS_REQ_GET_CF                 = 9;
    static final int MMTELSS_REQ_SET_CW                 = 10;
    static final int MMTELSS_REQ_GET_CW                 = 11;
    //[SET OIP/SET TIP/SET TIR are not supported by 2/3G SS feature set]
    static final int MMTELSS_REQ_SET_CLIP               = 12;
    static final int MMTELSS_REQ_SET_COLP               = 13;
    static final int MMTELSS_REQ_SET_COLR               = 14;
    /// For OP01 UT @{
    static final int MMTELSS_REQ_SET_CF_TIME_SLOT       = 15;
    static final int MMTELSS_REQ_GET_CF_TIME_SLOT       = 16;
    /// @}

    //***** Events
    static final int EVENT_SEND                 = 1;
    static final int EVENT_WAKE_LOCK_TIMEOUT    = 2;
    //static final int EVENT_RADIO_OFF_OR_NOT_AVAILABLE = 3;
    //static final int EVENT_RADIO_AVAILABLE = 4;
    //static final int EVENT_RADIO_ON = 5;

    //[TBD] Need to sync the maximun number of bytes with SimServs's capability
    static final int MMTELSS_MAX_COMMAND_BYTES = (8 * 1024);

    private static final int DEFAULT_WAKE_LOCK_TIMEOUT = 5000;

    WakeLock mWakeLock;
    int mWakeLockTimeout;
    // The number of requests pending to be sent out, it increases before calling
    // EVENT_SEND and decreases while handling EVENT_SEND. It gets cleared while
    // WAKE_LOCK_TIMEOUT occurs.
    int mRequestMessagesPending = 0;
    // The number of requests sent out but waiting for response. It increases while
    // sending request and decreases while handling response. It should match
    // mRequestList.size() unless there are requests no replied while
    // WAKE_LOCK_TIMEOUT occurs.
    int mRequestMessagesWaiting;

    //I'd rather this be LinkedList or something
    ArrayList<MMTelSSRequest> mRequestsList = new ArrayList<MMTelSSRequest>();

    IntentFilter mFilter = null;

    private static SimServs mSimservs = SimServs.getInstance();

    //Following Constants definition must be same with EngineerMode/ims/ImsActivity.java
    private final static String PROP_SS_MODE = "persist.vendor.radio.ss.mode";
    private final static String MODE_SS_XCAP = "Prefer XCAP";
    private final static String MODE_SS_CS = "Prefer CS";
    private final static String PROP_SS_DISABLE_METHOD = "persist.vendor.radio.ss.xrdm";
    private final static String PROP_SS_CFNUM = "persist.vendor.radio.xcap.cfn";

    private final int CACHE_IDX_CF = 0;
    private final int CACHE_IDX_OCB = 1;
    private final int CACHE_IDX_ICB = 2;
    private final int CACHE_IDX_CW = 3;
    private final int CACHE_IDX_OIP = 4;
    private final int CACHE_IDX_OIR = 5;
    private final int CACHE_IDX_TIP = 6;
    private final int CACHE_IDX_TIR = 7;
    private final int CACHE_IDX_TOTAL = 8;
    private SimservType[] mCacheSimserv = new SimservType[CACHE_IDX_TOTAL];
    private int[] mCachePhoneId = new int[CACHE_IDX_TOTAL];
    private long[] mLastQueried = new long[CACHE_IDX_TOTAL];
    private final Object mWaitLock = new Object();

    private static SuppSrvConfig mSSConfig = null;

    private static int[] CF_REASON = {
            CommandsInterface.CF_REASON_BUSY,
            CommandsInterface.CF_REASON_NO_REPLY,
            CommandsInterface.CF_REASON_NOT_REACHABLE,
            ImsRILConstants.CF_REASON_NOT_REGISTERED,
            CommandsInterface.CF_REASON_UNCONDITIONAL};

    private static String[] CB_FACILITY = {
            CommandsInterface.CB_FACILITY_BAOC,
            CommandsInterface.CB_FACILITY_BAOIC,
            CommandsInterface.CB_FACILITY_BAOICxH,
            CommandsInterface.CB_FACILITY_BAIC,
            CommandsInterface.CB_FACILITY_BAICr };

    private static final int CB_NONE = 0;
    private static final int CB_OCB = 1;
    private static final int CB_ICB = 2;
    private static final int CB_MO = 3;
    private static final int CB_MT = 4;
    private static final int CB_ALL = 5;

    private static final int IDENTITY_CLIP = 0;
    private static final int IDENTITY_CLIR = 1;
    private static final int IDENTITY_COLP = 2;
    private static final int IDENTITY_COLR = 3;

    private static final int CLIR_PROVISION = 1;        // permanently provisioned
    private static final int CLIR_NOT_PROVISION = 2;    // not provisioned
    private static final int CLIR_TEMP_DISALLOW = 3;    // temporary presentation disallowed
    private static final int CLIR_TEMP_ALLOW = 4;       // temporary presentation allowed

    private static final int MATCHED_MEDIA_NO_MATCHED = 0;
    private static final int MATCHED_MEDIA_AUDIO = (1 << 0);
    private static final int MATCHED_MEDIA_VIDEO = (1 << 1);

    private static final int MODIFIED_SERVICE_AUDIO = (1 << 0);
    private static final int MODIFIED_SERVICE_VIDEO = (1 << 1);
    private static final int INVALID_PHONE_ID = -1;

    private static final int WAIT_GBA_TIME_OUT = 1000;

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Rlog.d(LOG_TAG, "onReceive:" + intent.getAction());
            if (intent.getAction().equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED) ||
                    intent.getAction().equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                onReceiveSimStateChangedIntent(intent);
            }
        }
    };

    private ServiceConnection mGbaConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Rlog.d(LOG_TAG, "GbaService onServiceConnected");
            synchronized (mWaitLock) {
                mWaitLock.notifyAll();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Rlog.d(LOG_TAG, "GbaService onServiceFailed");
        }
    };

    public MMTelSSTransport() {
        //pm = (PowerManager)mContext.getSystemService(Context.POWER_SERVICE);
        //mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, LOG_TAG);
        //mWakeLock.setReferenceCounted(false);
        //mWakeLockTimeout = SystemProperties.getInt(TelephonyProperties.PROPERTY_WAKE_LOCK_TIMEOUT,
        //        DEFAULT_WAKE_LOCK_TIMEOUT);

        mSenderThread = new HandlerThread("MMTelSSTransmitter");
        mSenderThread.start();
        Looper looper = mSenderThread.getLooper();
        mSender = new MMTelSSTransmitter(looper);

        // init cache
        for (int i=0; i<CACHE_IDX_TOTAL; i++) {
            mCacheSimserv[i] = null;
            mCachePhoneId[i] = -1;
            mLastQueried[i] = 0;
        }
    }

    public static MMTelSSTransport getInstance() {
        return INSTANCE;
    }

    public void registerUtService(Context context) {
        mContext = context;
        if (mWakeLock == null) {
            pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, LOG_TAG);
            mWakeLock.setReferenceCounted(false);
            mWakeLockTimeout = SystemProperties.getInt(
                    TelephonyProperties.PROPERTY_WAKE_LOCK_TIMEOUT, DEFAULT_WAKE_LOCK_TIMEOUT);
        }

        // Manages XCAP mobile data network connectivity related stuff
        if (mXcapMobileDataNetworkManager == null) {
            mXcapMobileDataNetworkManager = new XcapMobileDataNetworkManager(mContext, mSenderThread.getLooper());
        }

        Rlog.d(LOG_TAG, "registerReceiver");
        mFilter = new IntentFilter();
        mFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mContext.registerReceiver(mIntentReceiver, mFilter);
    }

    private void requestXcapNetwork(int phoneId) {
        Rlog.d(LOG_TAG, "requestXcapNetwork(): phoneId = " + phoneId
                + ", mXcapMobileDataNetworkManager = " + mXcapMobileDataNetworkManager);
        mNetwork = null;
        if (mXcapMobileDataNetworkManager != null) {
            mNetwork = mXcapMobileDataNetworkManager.acquireNetwork(phoneId);

            ConnectivityManager connMgr =
                    (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (connMgr != null) {
                Rlog.d(LOG_TAG, "Bind process to xcap network");
                connMgr.bindProcessToNetwork(mNetwork);
            }
        }
    }

    private void startGbaService(Context context) {
        Rlog.d(LOG_TAG, "start gba service");
        synchronized (mWaitLock) {
            ComponentName gbaService =
                new ComponentName("com.mediatek.gba", "com.mediatek.gba.GbaService");
            Intent gbaIntent = new Intent();
            gbaIntent.setComponent(gbaService);
            mContext.bindService(gbaIntent, mGbaConnection, Context.BIND_AUTO_CREATE);
            Rlog.d(LOG_TAG, "Is gba service running = " + isGbaServiceRunning(context));
            try {
                mWaitLock.wait(WAIT_GBA_TIME_OUT);
            } catch (InterruptedException e) {
                Rlog.d(LOG_TAG, "wait request interrupted");
            }
        }
    }

    private boolean isGbaServiceRunning(Context context) {
        boolean isRunning = false;
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> runningServices = am.getRunningServices(Integer.MAX_VALUE);

        for (int i = 0; i < runningServices.size(); i++) {
            ActivityManager.RunningServiceInfo info = runningServices.get(i);
            if (info.service.getClassName().equals("com.mediatek.gba.GbaService")) {
                Rlog.d(LOG_TAG, "Gba service is running");
                isRunning = true;
                break;
            }
        }
        return isRunning;
    }

    private boolean updateNetworkInitSimServ(int phoneId) {
        Rlog.d(LOG_TAG, "updateNetworkInitSimServ:" + phoneId);

        if (mSSConfig.isNotSupportXcap()) {
            Rlog.d(LOG_TAG, "Use CS instead of XCAP.");
            return false;
        }

        mSimservs = MMTelSSUtils.initSimserv(mContext, phoneId);

        String xcapRoot = MMTelSSUtils.getXcapRootUri(phoneId, mContext);
        if (xcapRoot == null || xcapRoot.isEmpty()) {
            Rlog.d(LOG_TAG, "XcapRoot is empty");
            return false;
        }

        if (!isGbaServiceRunning(mContext)) {
            startGbaService(mContext);
        }

        // request network first, then pass the network to authentication
        requestXcapNetwork(phoneId);
        initAuthentication(xcapRoot, phoneId, mNetwork);

        /**
         * If we request XCAP network failed and we aren't in default data sub id,
         * we should do CSFB directly.
         */
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        int defaultDataPhoneId = SubscriptionManager.getPhoneId(subId);
        Rlog.d(LOG_TAG, "defaultDataPhoneId = " + defaultDataPhoneId + ", phoneId = " + phoneId);
        if (mNetwork == null && defaultDataPhoneId != phoneId) {
            Rlog.e(LOG_TAG, "mNetwork is null and" +
                   "default data doesn't match the current SIM card, request XCAP failed.");
            return false;
        }
        return true;
    }

    public void initAuthentication(String xcapRoot, int phoneId, Network network) {
        String digestId = mSSConfig.getDigestId();
        String digestPwd = mSSConfig.getDigestPwd();

        Rlog.d(LOG_TAG, "initAuthentication: xcapRoot=" + xcapRoot + ", phoneId=" + phoneId
                + ", network=" + network + ", digestId=" + digestId + ", digestPwd=" + digestPwd);

        if (!TextUtils.isEmpty(digestId) || !TextUtils.isEmpty(digestPwd)) {
            Authenticator.setDefault(new Authenticator() {
                protected PasswordAuthentication getPasswordAuthentication() {
                    String digestId = mSSConfig.getDigestId();
                    String digestPwd = mSSConfig.getDigestPwd();
                    if(TextUtils.isEmpty(digestId)) {
                        digestId = MMTelSSUtils.getXui(phoneId, mContext);
                    }
                    Rlog.d(LOG_TAG, "getPasswordAuthentication: digestId=" + digestId
                            + ", digestPwd=" + digestPwd);
                    return new PasswordAuthentication(digestId, digestPwd.toCharArray());
                }
            });
            System.setProperty("http.digest.support", "true");
        }
    }

    private void onReceiveSimStateChangedIntent(Intent intent) {
        String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
        int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, INVALID_PHONE_ID);
        if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simStatus) ||
                intent.getAction().equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
            Rlog.d(LOG_TAG, "onReceiveSimStateChangedIntent: simStatus=" + simStatus +
                   "phoneId=" + phoneId);
            // clear cache
            for (int i = 0; i < CACHE_IDX_TOTAL; i++) {
                mCacheSimserv[i] = null;
                mCachePhoneId[i] = -1;
                mLastQueried[i] = 0;
            }

            // clear xcap root, xui, in SimServes
            mSimservs.resetParameters();
        }
    }

    class MMTelSSTransmitter extends Handler implements Runnable {
        public MMTelSSTransmitter(Looper looper) {
            super(looper);
        }

        // Only allocated once
        byte[] dataLength = new byte[4];

        //***** Runnable implementation
        public void
        run() {
            //setup if needed
        }
        public String getMediaType(int serviceClass) {
            if ((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                return "audio";
            } else if ((serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                return "video";
            }
            return "";
        }

        public Exception reportXcapException(XcapException xcapException) {
            Exception exception = null;
            if (xcapException.isConnectionError()) {
                Rlog.d(LOG_TAG, "reportXcapException: isConnectionError()");
                exception = new UnknownHostException();
            } else if (mSSConfig.isHttpErrToUnknownHostErr()
                            && (xcapException.getHttpErrorCode() != 0)) {
                Rlog.d(LOG_TAG, "reportXcapException: HttpErrCode="
                        + xcapException.getHttpErrorCode());
                exception = new UnknownHostException();
            } else {
                exception = xcapException;
            }
            return exception;
        }

        public String identityToString(int identity) {
            switch (identity) {
                case IDENTITY_CLIP:
                    return "CLIP";
                case IDENTITY_CLIR:
                    return "CLIR";
                case IDENTITY_COLP:
                    return "COLP";
                case IDENTITY_COLR:
                    return "COLR";
                default:
                    return "ERR";
            }
        }

        public int identityToCacheId(int identity) {
            switch (identity) {
                case IDENTITY_CLIP:
                    return CACHE_IDX_OIP;
                case IDENTITY_CLIR:
                    return CACHE_IDX_OIR;
                case IDENTITY_COLP:
                    return CACHE_IDX_TIP;
                case IDENTITY_COLR:
                    return CACHE_IDX_TIR;
                default:
                    return CACHE_IDX_TOTAL;
            }
        }

        public void handleGetIdentity(MMTelSSRequest rr, int identity) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int phoneId = rr.mp.readInt();

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", identity=" + identityToString(identity)
                    + ", phoneId=" + phoneId);

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(rr.mResult);
                return;
            }

            Exception exceptionReport = null;
            int[] response = new int[2];

            Rlog.d(LOG_TAG, "handleGetIdentity() " + requestToString(reqNo)
                    + ", identity=" + identityToString(identity)
                    + ", phoneId=" + phoneId);

            try {
                if (identity == IDENTITY_CLIP) {
                    response[0] = 0;
                    OriginatingIdentityPresentation oip
                            = (OriginatingIdentityPresentation)getCache(
                                    identityToCacheId(identity), phoneId);
                    response[0] = (oip.isActive() ? 1 : 0);
                    Rlog.d(LOG_TAG, "handleGetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", enable=" + response[0]);
                } else if (identity == IDENTITY_CLIR) {
                    response[0] = CommandsInterface.CLIR_DEFAULT;
                    response[1] = CLIR_NOT_PROVISION;
                    OriginatingIdentityPresentationRestriction oir
                            = (OriginatingIdentityPresentationRestriction)getCache(
                                    identityToCacheId(identity), phoneId);
                    boolean restricted = oir.isDefaultPresentationRestricted();
                    if (restricted) {
                        response[0] = CommandsInterface.CLIR_INVOCATION;
                        response[1] = CLIR_TEMP_DISALLOW;
                    } else {
                        response[0] = CommandsInterface.CLIR_SUPPRESSION;
                        response[1] = CLIR_TEMP_ALLOW;
                    }
                    Rlog.d(LOG_TAG, "handleGetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", restricted=" + restricted);
                } else if (identity == IDENTITY_COLP) {
                    response[0] = 0;    // 0:diable, 1:enable
                    response[1] = 0;    // 0:not provision, 1:provision
                    TerminatingIdentityPresentation tip
                            = (TerminatingIdentityPresentation)getCache(
                                    identityToCacheId(identity), phoneId);
                    response[0] = response[1] = (tip.isActive() ? 1 : 0);
                    Rlog.d(LOG_TAG, "handleGetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", enable=" + response[0]);
                } else if (identity == IDENTITY_COLR) {
                    response[0] = 0;
                    TerminatingIdentityPresentationRestriction tir
                            = (TerminatingIdentityPresentationRestriction)getCache(
                                    identityToCacheId(identity), phoneId);
                    response[0] = (tir.isActive() ? 1 : 0);
                    Rlog.d(LOG_TAG, "handleGetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", enable=" + response[0]);
                }
            } catch (UnknownHostException unknownHostException) {
                unknownHostException.printStackTrace();
                exceptionReport = unknownHostException;
            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
            } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            if (exceptionReport != null) {
                clearCache(identityToCacheId(identity));
            }

            if (rr.mResult != null) {
                AsyncResult.forMessage(rr.mResult, response, exceptionReport);
                rr.mResult.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }

        public void handleSetIdentity(MMTelSSRequest rr, int identity) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int mode = rr.mp.readInt();
            int phoneId = rr.mp.readInt();

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", identity=" + identityToString(identity)
                    + ", mode=" + mode
                    + ", phoneId=" + phoneId);

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(rr.mResult);
                return;
            }

            Exception exceptionReport = null;

            Rlog.d(LOG_TAG, "handleSetIdentity() " + requestToString(reqNo)
                    + ", identity=" + identityToString(identity)
                    + ", mode=" + mode
                    + ", phoneId=" + phoneId);

            try {
                if (identity == IDENTITY_CLIP) {
                    OriginatingIdentityPresentation oip
                            = (OriginatingIdentityPresentation)getCache(
                                    identityToCacheId(identity), phoneId);
                    boolean enable = (mode == 1 ? true : false);
                    Rlog.d(LOG_TAG, "handleSetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", enable=" + enable);
                    oip.setActive(enable);
                } else if (identity == IDENTITY_CLIR) {
                    OriginatingIdentityPresentationRestriction oir
                            = (OriginatingIdentityPresentationRestriction)getCache(
                                    identityToCacheId(identity), phoneId);
                    boolean enable = (mode == CommandsInterface.CLIR_INVOCATION ?
                                true : false);
                    boolean putWhole = mSSConfig.isPutWholeCLIR();
                    Rlog.d(LOG_TAG, "handleSetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", enable=" + enable
                            + ", putWhole=" + putWhole);
                    if (putWhole || !oir.isContainDefaultBehaviour()) {
                        oir.setDefaultPresentationRestricted(enable, enable,
                                OriginatingIdentityPresentationRestriction.NODE_ROOT_FULL_CHILD,
                                true);
                    } else {
                        oir.setDefaultPresentationRestricted(enable);
                    }
                } else if (identity == IDENTITY_COLP) {
                    TerminatingIdentityPresentation tip
                            = (TerminatingIdentityPresentation)getCache(
                                    identityToCacheId(identity), phoneId);
                    boolean enable = (mode == 1 ? true : false);
                    Rlog.d(LOG_TAG, "handleSetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", enable=" + enable);
                    tip.setActive(enable);
                } else if (identity == IDENTITY_COLR) {
                    TerminatingIdentityPresentationRestriction tir
                            = (TerminatingIdentityPresentationRestriction)getCache(
                                    identityToCacheId(identity), phoneId);
                    boolean enable = (mode == CommandsInterface.CLIR_INVOCATION ?
                                true : false);
                    boolean putWhole = mSSConfig.isPutWholeCLIR();
                    Rlog.d(LOG_TAG, "handleSetIdentity() " + requestToString(reqNo)
                            + ", identity=" + identityToString(identity)
                            + ", enable=" + enable
                            + ", putWhole=" + putWhole);
                    if (putWhole || !tir.isContainDefaultBehaviour()) {
                        tir.setDefaultPresentationRestricted(enable, enable,
                                TerminatingIdentityPresentationRestriction.NODE_ROOT_FULL_CHILD,
                                true);
                    } else {
                        tir.setDefaultPresentationRestricted(enable);
                    }
                }
            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
            } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            clearCache(identityToCacheId(identity));

            if (rr.mResult != null) {
                AsyncResult.forMessage(rr.mResult, null, exceptionReport);
                rr.mResult.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }


        public void handleGetCW(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int serviceClass = rr.mp.readInt();
            int phoneId = rr.mp.readInt();

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", serviceClass=" + serviceClass
                    + ", phoneId=" + phoneId);

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(rr.mResult);
                return;
            }

            Exception exceptionReport = null;
            int[] response = new int[2];
            serviceClass = convertServiceClass(serviceClass);

            Rlog.d(LOG_TAG, "handleGetCW() " + requestToString(reqNo)
                    + ", serviceClass=" + serviceClassToString(serviceClass)
                    + ", phoneId=" + phoneId);

            try {
                CommunicationWaiting cw = (CommunicationWaiting)getCache(CACHE_IDX_CW, phoneId);
                response[0] = (cw.isActive() ? 1 : 0);
                Rlog.d(LOG_TAG, "handleGetCW(): isActive = " + response[0]);
            } catch (UnknownHostException unknownHostException) {
                exceptionReport = unknownHostException;
            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
            } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            if (exceptionReport != null) {
                clearCache(CACHE_IDX_CW);
            }

            if (response[0] == 1) {
                if (serviceClass == CommandsInterface.SERVICE_CLASS_NONE) {
                    response[1] |= CommandsInterface.SERVICE_CLASS_VOICE;
                    response[1] |= ImsRILConstants.SERVICE_CLASS_VIDEO;
                } else {
                    response[1] |= serviceClass;
                    if (serviceClass == ImsRILConstants.SERVICE_CLASS_VIDEO) {
                        response[1] |= CommandsInterface.SERVICE_CLASS_VOICE;
                    }
                }
                Rlog.d(LOG_TAG, "handleGetCW(): service=" + serviceClassToString(response[1]));
            }


            // OP156 CW via XCAP is not ready, need CSFB
            if (exceptionReport != null && OperatorUtils.isMatched(OPID.OP156, phoneId)) {
                Rlog.d(LOG_TAG, "handleGetCW() OP156 not support, just CSFB");
                exceptionReport = new UnknownHostException();
            }
            if (rr.mResult != null) {
                AsyncResult.forMessage(rr.mResult, response, exceptionReport);
                rr.mResult.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }

        public String getCBFacility(int cbType, Conditions cond) {
            String r = null;
            if (cbType == CB_OCB) {
                if (cond == null) {
                    r = CommandsInterface.CB_FACILITY_BAOC;
                } else if (cond.comprehendInternational()) {
                    r = CommandsInterface.CB_FACILITY_BAOIC;
                } else if (cond.comprehendInternationalExHc()) {
                    r = CommandsInterface.CB_FACILITY_BAOICxH;
                } else {
                    r = CommandsInterface.CB_FACILITY_BAOC;
                }
            } else if (cbType == CB_ICB) {
                if (cond == null) {
                    r = CommandsInterface.CB_FACILITY_BAIC;
                } else if (cond.comprehendRoaming()) {
                    r = CommandsInterface.CB_FACILITY_BAICr;
                } else {
                    r = CommandsInterface.CB_FACILITY_BAIC;
                }
            } else {
                r = "ERR";
            }
            Rlog.d(LOG_TAG, "getCBFacility() " + r
                    + ": cbType=" + cbTypeToString(cbType)
                    + (cond == null ? ", cond=null" :
                    ", OCB: international=" + cond.comprehendInternational()
                    + ",internationalExHc=" + cond.comprehendInternationalExHc()
                    + " | ICB roaming=" + cond.comprehendRoaming()));

            return r;
        }

        public int getCB(SimservType cb, String facility, int serviceClass, int phoneId) {
            Rlog.d(LOG_TAG, "getCB() facility=" + facility
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", phoneId=" + phoneId
                    + ", cb=" + cb);
            int result = 0;
            RuleSet ruleSet = null;
            List<Rule> ruleList = null;
            int cbType = getCBType(facility);
            if (cbType == CB_OCB) {
                ruleSet = ((OutgoingCommunicationBarring)cb).getRuleSet();
                ruleList = ruleSet.getRules();
            } else if (cbType == CB_ICB) {
                ruleSet = ((IncomingCommunicationBarring)cb).getRuleSet();
                ruleList = ruleSet.getRules();
            } else {
                Rlog.d(LOG_TAG, "getCB() not support facility=" + facility
                        + ", cbType=" + cbTypeToString(cbType));
            }
            if (ruleList != null) {
                for (Rule rule : ruleList) {
                    Conditions cond = rule.getConditions();
                    Actions act = rule.getActions();
                    // Search in the exist rules to match the CB operation
                    // Not only "faility" but also "media" need match
                    // Usually there is only one rule to match,
                    // but still may found many rules which are the same CB type
                    if (getCBFacility(cbType, cond).equals(facility)
                            && isRuleMatchServiceClass(rule, serviceClass)
                                    != MATCHED_MEDIA_NO_MATCHED) {
                        // get enable status
                        boolean enable = false;
                        if ((act != null && act.isAllow() == false)
                                && (cond == null
                                    || (cond.comprehendRuleDeactivated() == false))) {
                            result |= serviceClass;
                            enable = true;
                        }
                        Rlog.d(LOG_TAG, "getCB() found rule: " + rule.mId
                                + ", facility=" + facility
                                + ", service=" + serviceClassToString(serviceClass)
                                + ", status=" + (enable ? "Enable" : "Disable")
                                + ", result=" + result);
                    }
                }
            }
            return result;
        }

        public int getCBType(String facility) {
            if (facility.equals(CommandsInterface.CB_FACILITY_BAOC)
                    || facility.equals(CommandsInterface.CB_FACILITY_BAOIC)
                    || facility.equals(CommandsInterface.CB_FACILITY_BAOICxH)) {
                return CB_OCB;
            } else if(facility.equals(CommandsInterface.CB_FACILITY_BAIC)
                    || facility.equals(CommandsInterface.CB_FACILITY_BAICr)) {
                return CB_ICB;
            } else if(facility.equals(CommandsInterface.CB_FACILITY_BA_MO)) {
                return CB_MO;
            } else if(facility.equals(CommandsInterface.CB_FACILITY_BA_MT)) {
                return CB_MT;
            } else if(facility.equals(CommandsInterface.CB_FACILITY_BA_ALL)) {
                return CB_ALL;
            } else {
                return CB_NONE;
            }
        }

        public String cbTypeToString(int cbType) {
            switch (cbType) {
                case CB_OCB:
                    return "OCB";
                case CB_ICB:
                    return "ICB";
                case CB_MO:
                    return "CB_MO";
                case CB_MT:
                    return "CB_MT";
                case CB_ALL:
                    return "CB_ALL";
                default:
                    return "ERR";
            }
        }

        public void handleGetCB(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            String facility = rr.mp.readString();
            int serviceClass = rr.mp.readInt();
            int phoneId = rr.mp.readInt();

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", facility=" + facility
                    + ", serviceClass=" + serviceClass
                    + ", phoneId=" + phoneId);

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(rr.mResult);
                return;
            }

            Exception exceptionReport = null;
            serviceClass = convertServiceClass(serviceClass);
            int[] response = new int[]{0};
            int cbType = getCBType(facility);
            int cacheIdx = (cbType == CB_OCB ? CACHE_IDX_OCB :
                        (cbType == CB_ICB ? CACHE_IDX_ICB : CACHE_IDX_TOTAL));

            Rlog.d(LOG_TAG, "handleGetCB() " + requestToString(reqNo)
                    + ", facility=" + facility
                    + ", cbType=" + cbTypeToString(cbType)
                    + ", serviceClass=" + serviceClassToString(serviceClass)
                    + ", phoneId=" + phoneId);

            try {
                if (serviceClass == CommandsInterface.SERVICE_CLASS_NONE) {
                    response[0] |= getCB(getCache(cacheIdx, phoneId), facility,
                            CommandsInterface.SERVICE_CLASS_VOICE, phoneId);
                    response[0] |= getCB(getCache(cacheIdx, phoneId), facility,
                            ImsRILConstants.SERVICE_CLASS_VIDEO, phoneId);
                } else {
                    response[0] |= getCB(getCache(cacheIdx, phoneId), facility,
                            serviceClass, phoneId);
                }
            } catch (UnknownHostException unknownHostException) {
                exceptionReport = unknownHostException;
            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
            } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            if (exceptionReport != null) {
                clearCache(cacheIdx);
            }

            if (rr.mResult != null) {
                AsyncResult.forMessage(rr.mResult, response, exceptionReport);
                rr.mResult.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }

        public List<MtkCallForwardInfo> getCFInfo(CommunicationDiversion cd, int action, int reason,
                int serviceClass) {
            Rlog.d(LOG_TAG, "getCFInfo() reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass));
            boolean found = false;
            List<MtkCallForwardInfo> result = new ArrayList<MtkCallForwardInfo>();
            RuleSet ruleSet = cd.getRuleSet();
            List<Rule> ruleList = ruleSet.getRules();
            long[] timeSlot = null;
            if (ruleList != null) {
                for (Rule rule : ruleList) {
                    // Search in the exist rules to match the CF operation
                    // Not only "reason" but also "media" need match
                    // Usually there is only one rule to match,
                    // but still may found many rules which are the same CF type
                    if (getCFType(rule.getConditions()) == reason
                            && isRuleMatchServiceClass(rule, serviceClass)
                                    != MATCHED_MEDIA_NO_MATCHED) {
                        found = true;

                        // get enable status
                        int enable = 0;
                        if (rule.getConditions().comprehendRuleDeactivated() == false) {
                            enable = 1;
                        }
                        // get number
                        String number = null;
                        if (rule.getActions().getFowardTo() != null) {
                            number = convertUriToNumber(
                                    rule.getActions().getFowardTo().getTarget());
                        }
                        // get time slot
                        if (enable == 1) {
                            timeSlot = convertToLocalTime(rule.getConditions().comprehendTime());
                        }
                        MtkCallForwardInfo cfInfo = new MtkCallForwardInfo();
                        cfInfo.status = enable;
                        cfInfo.reason = reason;
                        cfInfo.serviceClass = serviceClass;
                        cfInfo.toa = 0;
                        cfInfo.number = number;
                        cfInfo.timeSeconds = cd.getNoReplyTimer();
                        cfInfo.timeSlot = timeSlot;
                        result.add(cfInfo);
                        Rlog.d(LOG_TAG, "getCFInfo() found rule: " + rule.mId
                                + ", reason=" + reasonCFToString(reason)
                                + ", service=" + serviceClassToString(serviceClass)
                                + ", status=" + (enable == 1 ? "Enable" : "Disable")
                                + ", number=" + MtkSuppServHelper.encryptString(number)
                                + ", time=" + cfInfo.timeSeconds
                                + ", timeSlot=" + timeSlot);
                    }
                }
            }
            // If not found any matched rule, add a default "Disable" record
            if (found == false) {
                MtkCallForwardInfo cfInfo = new MtkCallForwardInfo();
                cfInfo.status = 0;
                cfInfo.reason = reason;
                cfInfo.serviceClass = serviceClass;
                cfInfo.toa = 0;
                cfInfo.number = null;
                cfInfo.timeSeconds = cd.getNoReplyTimer();
                cfInfo.timeSlot = null;
                result.add(cfInfo);
                Rlog.d(LOG_TAG, "getCFInfo() not found rule"
                        + ", reason=" + reasonCFToString(reason)
                        + ", service=" + serviceClassToString(serviceClass));
            }
            return result;
        }

        public List<MtkCallForwardInfo> getCFInfoList(CommunicationDiversion cd, int action,
                int reason, int serviceClass) {
            Rlog.d(LOG_TAG, "getCFInfoList() reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass));
            List<MtkCallForwardInfo> result = new ArrayList<MtkCallForwardInfo>();

            if (serviceClass == CommandsInterface.SERVICE_CLASS_NONE) {
                result.addAll(getCFInfo(cd, action, reason, CommandsInterface.SERVICE_CLASS_VOICE));
                result.addAll(getCFInfo(cd, action, reason, ImsRILConstants.SERVICE_CLASS_VIDEO));
            } else {
                result.addAll(getCFInfo(cd, action, reason, serviceClass));
            }
            return result;
        }

        public Object convertCFInfo(boolean isGetTimeSlot, MtkCallForwardInfo[] infos) {
            if (infos == null) {
                infos = new MtkCallForwardInfo[0];
            }
            if (isGetTimeSlot) {
                return infos;
            }
            CallForwardInfo[] result = new CallForwardInfo[infos.length];
            for (int i=0; i<infos.length; i++) {
                result[i] = new CallForwardInfo();
                result[i].status = infos[i].status;
                result[i].reason = infos[i].reason;
                result[i].serviceClass = infos[i].serviceClass;
                result[i].toa = 0;
                result[i].number = infos[i].number;
                result[i].timeSeconds = infos[i].timeSeconds;
            }
            return result;
        }

        public String cacheIdxToString(int idx) {
            switch (idx) {
                case CACHE_IDX_CF:
                    return "CF";
                case CACHE_IDX_OCB:
                    return "OCB";
                case CACHE_IDX_ICB:
                    return "ICB";
                case CACHE_IDX_CW:
                    return "CW";
                case CACHE_IDX_OIP:
                    return "OIP";
                case CACHE_IDX_OIR:
                    return "OIR";
                case CACHE_IDX_TIP:
                    return "TIP";
                case CACHE_IDX_TIR:
                    return "TIR";
                default:
                    return "ERR: " + String.valueOf(idx);
            }
        }

        public void updateNetwork(int idx) {
            switch (idx) {
                case CACHE_IDX_CF:
                    ((CommunicationDiversion)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
                case CACHE_IDX_OCB:
                    ((OutgoingCommunicationBarring)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
                case CACHE_IDX_ICB:
                    ((IncomingCommunicationBarring)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
                case CACHE_IDX_CW:
                    ((CommunicationWaiting)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
                case CACHE_IDX_OIP:
                    ((OriginatingIdentityPresentation)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
                case CACHE_IDX_OIR:
                    ((OriginatingIdentityPresentationRestriction)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
                case CACHE_IDX_TIP:
                    ((TerminatingIdentityPresentation)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
                case CACHE_IDX_TIR:
                    ((TerminatingIdentityPresentationRestriction)mCacheSimserv[idx]).setNetwork(mNetwork);
                    break;
            }
        }

        public SimservType getCache(int idx, int phoneId) throws Exception {
            boolean usingCache = true;
            long curTime = System.currentTimeMillis();
            Rlog.d(LOG_TAG, "getCache(): " + cacheIdxToString(idx)
                    + " phoneId=" + phoneId
                    + ", cachePhoneId=" + mCachePhoneId[idx]
                    + ", curTime=" + curTime
                    + ", lastQuery=" + mLastQueried[idx]
                    + ", mCacheSimserv=" + mCacheSimserv[idx]);

            Long cacheValidTime = mSSConfig.getCacheValidTime();

            if (phoneId != mCachePhoneId[idx] || mCacheSimserv[idx] == null
                    || (curTime - mLastQueried[idx]) > cacheValidTime) {
                usingCache = false;
                switch (idx) {
                    case CACHE_IDX_CF:
                        mCacheSimserv[idx] =
                                mSimservs.getCommunicationDiversion(true, mNetwork);
                        break;
                    case CACHE_IDX_OCB:
                        mCacheSimserv[idx] =
                                mSimservs.getOutgoingCommunicationBarring(true, mNetwork);
                        break;
                    case CACHE_IDX_ICB:
                        mCacheSimserv[idx] =
                                mSimservs.getIncomingCommunicationBarring(true, mNetwork);
                        break;
                    case CACHE_IDX_CW:
                        mCacheSimserv[idx] =
                                mSimservs.getCommunicationWaiting(true, mNetwork);
                        break;
                    case CACHE_IDX_OIP:
                        mCacheSimserv[idx] =
                                mSimservs.getOriginatingIdentityPresentation(true, mNetwork);
                        break;
                    case CACHE_IDX_OIR:
                        mCacheSimserv[idx] =
                                mSimservs.getOriginatingIdentityPresentationRestriction(true, mNetwork);
                        break;
                    case CACHE_IDX_TIP:
                        mCacheSimserv[idx] =
                                mSimservs.getTerminatingIdentityPresentation(true, mNetwork);
                        break;
                    case CACHE_IDX_TIR:
                        mCacheSimserv[idx] =
                                mSimservs.getTerminatingIdentityPresentationRestriction(true, mNetwork);
                        break;
                }
                mCachePhoneId[idx] = phoneId;
                mLastQueried[idx] = curTime;
                Rlog.d(LOG_TAG, "getCache(): new Cache phoneId=" + phoneId
                        + ", curTime=" + curTime
                        + ", mCacheSimserv=" + mCacheSimserv[idx]);
            }
            if (usingCache) {
                updateNetwork(idx);
            }
            return mCacheSimserv[idx];
        }

        public void clearCache(int idx) {
            Rlog.d(LOG_TAG, "Clear [" + cacheIdxToString(idx) + "] cache");
            mCacheSimserv[idx] = null;
            mCachePhoneId[idx] = -1;
            mLastQueried[idx] = 0;
        }

        public void removeDuplicateCF(ArrayList<MtkCallForwardInfo> cfInfoList) {
            for (int i=0; i<5; i++) {
                int reason = CF_REASON[i];
                MtkCallForwardInfo firstCfInfo = null;
                Iterator<MtkCallForwardInfo> iterator = cfInfoList.iterator();
                while (iterator.hasNext()) {
                    MtkCallForwardInfo cfInfo = iterator.next();
                    if (firstCfInfo == null && cfInfo.reason == reason) {
                        firstCfInfo = cfInfo;
                        Rlog.d(LOG_TAG, "firstCfInfo() reason=" + cfInfo.reason
                                + ", service=" + serviceClassToString(cfInfo.serviceClass)
                                + ", number=" + MtkSuppServHelper.encryptString(cfInfo.number));
                    } else if (firstCfInfo != null && firstCfInfo.reason == cfInfo.reason
                            && firstCfInfo.serviceClass == cfInfo.serviceClass) {
                        iterator.remove();
                        Rlog.d(LOG_TAG, "removeDuplicateCF() reason=" + cfInfo.reason
                                + ", service=" + serviceClassToString(cfInfo.serviceClass)
                                + ", number=" + MtkSuppServHelper.encryptString(cfInfo.number));
                    } else if (firstCfInfo != null && firstCfInfo.serviceClass != cfInfo.serviceClass) {
                        firstCfInfo = cfInfo;
                        Rlog.d(LOG_TAG, "reassign cf info, reason=" + cfInfo.reason
                                + ", service=" + serviceClassToString(cfInfo.serviceClass)
                                + ", number=" + MtkSuppServHelper.encryptString(cfInfo.number));
                    }
                }
            }
        }

        public void handleGetCF(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int action = rr.mp.readInt();
            int reason = rr.mp.readInt();
            int serviceClass = rr.mp.readInt();
            String number = rr.mp.readString();
            int phoneId = rr.mp.readInt();
            Message msg = rr.mResult;

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", action=" + action
                    + ", reason=" + reason
                    + ", serviceClass=" + serviceClass
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", phoneId=" + phoneId
                    + ", msg=" + msg);

            handleGetCF(reqNo, serialNo, action, reason, serviceClass, number, phoneId, false, msg);
        }

        public void handleGetCF(int reqNo, int serialNo, int action, int reason,
                int serviceClass, String number, int phoneId, boolean isGetTimeSlot, Message msg) {

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(msg);
                return;
            }

            Exception exceptionReport = null;
            MtkCallForwardInfo infos[] = null;
            serviceClass = convertServiceClass(serviceClass);

            Rlog.d(LOG_TAG, "handleGetCF() " + requestToString(reqNo)
                    + ", action=" + actionCFToString(action)
                    + ", reason=" + reasonCFToString(reason)
                    + ", serviceClass=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", phoneId=" + phoneId
                    + ", isGetTimeSlot=" + isGetTimeSlot);

            try {
                CommunicationDiversion cd = (CommunicationDiversion)getCache(CACHE_IDX_CF, phoneId);

                if (isEmptyCF(cd)) {
                    clearCache(CACHE_IDX_CF);
                    infos = new MtkCallForwardInfo[0];
                } else {
                    // Find out all cfInfo
                    ArrayList<MtkCallForwardInfo> cfInfoList = new ArrayList<MtkCallForwardInfo>();
                    if (reason == CommandsInterface.CF_REASON_ALL_CONDITIONAL) {
                        // CFB/CFNRy/CFNRc/CFNL
                        for (int i=0; i<4; i++) {
                            reason = CF_REASON[i];
                            cfInfoList.addAll(getCFInfoList(cd, action, reason, serviceClass));
                        }
                    } else if (reason == CommandsInterface.CF_REASON_ALL) {
                        // CFB/CFNRy/CFNRc/CFNL/CFU
                        for (int i=0; i<5; i++) {
                            reason = CF_REASON[i];
                            cfInfoList.addAll(getCFInfoList(cd, action, reason, serviceClass));
                        }
                    } else {
                        cfInfoList.addAll(getCFInfoList(cd, action, reason, serviceClass));
                    }
                    removeDuplicateCF(cfInfoList);
                    infos = cfInfoList.toArray(new MtkCallForwardInfo[cfInfoList.size()]);
                }

            } catch (UnknownHostException unknownHostException) {
                exceptionReport = unknownHostException;
            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
            } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            if (exceptionReport != null) {
                clearCache(CACHE_IDX_CF);
            }

            if (msg != null) {
                AsyncResult.forMessage(msg, convertCFInfo(isGetTimeSlot, infos),
                        exceptionReport);
                msg.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }

        public void handleSetCW(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int enable = rr.mp.readInt();
            int serviceClass = rr.mp.readInt();
            int phoneId = rr.mp.readInt();

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", enable=" + enable
                    + ", serviceClass=" + serviceClass
                    + ", phoneId=" + phoneId);

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(rr.mResult);
                return;
            }

            Exception exceptionReport = null;

            Rlog.d(LOG_TAG, "handleSetCW() " + requestToString(reqNo)
                    + ", enable=" + enable
                    + ", serviceClass=" + serviceClassToString(serviceClass)
                    + ", phoneId=" + phoneId);

            try {
                CommunicationWaiting cw = (CommunicationWaiting)getCache(CACHE_IDX_CW, phoneId);
                cw.setActive((enable == 1 ? true : false));
            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
            } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            clearCache(CACHE_IDX_CW);

            // OP156 CW via XCAP is not ready, need CSFB
            if (exceptionReport != null && OperatorUtils.isMatched(OPID.OP156, phoneId)) {
                Rlog.d(LOG_TAG, "handleSetCW() OP156 not support, just CSFB");
                exceptionReport = new UnknownHostException();
            }

            if (rr.mResult != null) {
                AsyncResult.forMessage(rr.mResult, null, exceptionReport);
                rr.mResult.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }

        public String reasonCFToString(int reason) {
            if (reason == CommandsInterface.CF_REASON_UNCONDITIONAL) {
                return "CFU";
            } else if (reason == CommandsInterface.CF_REASON_BUSY) {
                return "CFB";
            } else if (reason == CommandsInterface.CF_REASON_NO_REPLY) {
                return "CFNRy";
            } else if (reason == CommandsInterface.CF_REASON_NOT_REACHABLE) {
                return "CFNRc";
            } else if (reason == ImsRILConstants.CF_REASON_NOT_REGISTERED) {
                return "CFNL";
            } else if (reason == CommandsInterface.CF_REASON_ALL) {
                return "CF All";
            } else if (reason == CommandsInterface.CF_REASON_ALL_CONDITIONAL) {
                return "CF All Conditional";
            } else {
                return "ERR: " + String.valueOf(reason);
            }
        }

        public String actionCFToString(int action) {
            if (action == CommandsInterface.CF_ACTION_DISABLE) {
                return "Disable";
            } else if (action == CommandsInterface.CF_ACTION_ENABLE) {
                return "Enable";
            } else if (action == 2) {
                return "UNUSED";
            } else if (action == CommandsInterface.CF_ACTION_REGISTRATION) {
                return "Registration";
            } else if (action == CommandsInterface.CF_ACTION_ERASURE) {
                return "Erasure";
            } else {
                return "ERR: " + String.valueOf(action);
            }
        }

        public String serviceClassToString(int service) {
            if (service == CommandsInterface.SERVICE_CLASS_NONE) {
                return "None";
            } else if (service == ImsRILConstants.SERVICE_CLASS_LINE2) {
                return "Line2";
            } else if (service == ImsRILConstants.SERVICE_CLASS_VIDEO) {
                return "Video";
            } else if (service == CommandsInterface.SERVICE_CLASS_VOICE) {
                return "Voice";
            } else if (service == CommandsInterface.SERVICE_CLASS_DATA) {
                return "Data";
            } else if (service == CommandsInterface.SERVICE_CLASS_FAX) {
                return "Fax";
            } else if (service == CommandsInterface.SERVICE_CLASS_SMS) {
                return "Sms";
            } else if (service == CommandsInterface.SERVICE_CLASS_DATA_SYNC) {
                return "Data sync";
            } else if (service == CommandsInterface.SERVICE_CLASS_DATA_ASYNC) {
                return "Data async";
            } else if (service == CommandsInterface.SERVICE_CLASS_PACKET) {
                return "Packet";
            } else if (service == CommandsInterface.SERVICE_CLASS_PAD) {
                return "Pad";
            } else if (service == CommandsInterface.SERVICE_CLASS_MAX) {
                return "Max";
            } else if (service == (CommandsInterface.SERVICE_CLASS_VOICE
                                    | ImsRILConstants.SERVICE_CLASS_VIDEO)) {
                return "Voice&Video";
            } else {
                return "ERR: " + String.valueOf(service);
            }
        }

        public String serviceClassToMediaString(int service) {
            if (service == CommandsInterface.SERVICE_CLASS_VOICE) {
                return "audio";
            } else if (service == ImsRILConstants.SERVICE_CLASS_VIDEO) {
                return "video";
            } else {
                return "ERR: " + String.valueOf(service);
            }
        }

        public String matchedMediaToString(int matchedMedia) {
            String r = "";
            if ((matchedMedia & MATCHED_MEDIA_AUDIO) != 0) {
                r += "audio ";
            }
            if ((matchedMedia & MATCHED_MEDIA_VIDEO) != 0) {
                r += "video ";
            }
            if (r.equals("")) {
                r = "no matched";
            }
            return r;
        }

        public String mediaTypeToString(int type) {
            if (type == SuppSrvConfig.MEDIA_TYPE_STANDARD) {
                return "Standard";
            } else if (type == SuppSrvConfig.MEDIA_TYPE_ONLY_AUDIO) {
                return "Only Audio";
            } else if (type == SuppSrvConfig.MEDIA_TYPE_SEPERATE) {
                return "Seperate";
            } else if (type == SuppSrvConfig.MEDIA_TYPE_VIDEO_WITH_AUDIO) {
                return "Video with Audio";
            } else {
                return "ERR: " + String.valueOf(type);
            }
        }

        public int getCFType(Conditions cond) {
            Rlog.d(LOG_TAG, "getCFType() "
                    + (cond == null ? "cond=null" :
                    "Busy=" + cond.comprehendBusy()
                    + ",NoAnswer=" + cond.comprehendNoAnswer()
                    + ",NotReachable=" + cond.comprehendNotReachable()
                    + ",NotRegistered=" + cond.comprehendNotRegistered()));
            if (cond == null) {
                return CommandsInterface.CF_REASON_UNCONDITIONAL;
            }
            if (cond.comprehendBusy()) {
                return CommandsInterface.CF_REASON_BUSY;
            } else if (cond.comprehendNoAnswer()) {
                return CommandsInterface.CF_REASON_NO_REPLY;
            } else if (cond.comprehendNotReachable()) {
                return CommandsInterface.CF_REASON_NOT_REACHABLE;
            } else if (cond.comprehendNotRegistered()) {
                return ImsRILConstants.CF_REASON_NOT_REGISTERED;
            } else {
                return CommandsInterface.CF_REASON_UNCONDITIONAL;
            }
        }

        public int isMediaMatchServiceClass(List<String> media, int serviceClass) {
            int r = MATCHED_MEDIA_NO_MATCHED;
            int mediaType = mSSConfig.getMediaTagType();
            String printMedia = "";
            for (String m : media) {
                printMedia = printMedia + m + " ";
            }
            boolean containAudio = media.contains(serviceClassToMediaString(
                    CommandsInterface.SERVICE_CLASS_VOICE));
            boolean containVideo = media.contains(serviceClassToMediaString(
                    ImsRILConstants.SERVICE_CLASS_VIDEO));

            // To be readable and clear logic, do not combine these if-else logic.
            if ((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                if (containAudio) {
                    if (containVideo) {
                        // (audio & video)
                        if (mediaType == SuppSrvConfig.MEDIA_TYPE_ONLY_AUDIO
                                || mediaType == SuppSrvConfig.MEDIA_TYPE_SEPERATE
                                || mediaType == SuppSrvConfig.MEDIA_TYPE_VIDEO_WITH_AUDIO) {
                            r = MATCHED_MEDIA_NO_MATCHED;
                        } else {
                            r = MATCHED_MEDIA_AUDIO;
                        }
                    } else {
                        // (audio)
                        r = MATCHED_MEDIA_AUDIO;
                    }
                } else {
                    if (containVideo) {
                        // (video)
                        r = MATCHED_MEDIA_NO_MATCHED;
                    } else {
                        // none media tag
                        r = MATCHED_MEDIA_AUDIO;
                    }
                }
            } else if ((serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                if (containVideo) {
                    if (containAudio) {
                        // (audio & video)
                        if (mediaType == SuppSrvConfig.MEDIA_TYPE_STANDARD
                                || mediaType == SuppSrvConfig.MEDIA_TYPE_VIDEO_WITH_AUDIO) {
                            r = MATCHED_MEDIA_VIDEO;
                        } else {
                            r = MATCHED_MEDIA_NO_MATCHED;
                        }
                    } else {
                        // (video)
                        if (mediaType == SuppSrvConfig.MEDIA_TYPE_STANDARD
                                || mediaType == SuppSrvConfig.MEDIA_TYPE_SEPERATE) {
                            r = MATCHED_MEDIA_VIDEO;
                        } else {
                            r = MATCHED_MEDIA_NO_MATCHED;
                        }
                    }
                } else {
                    if (containAudio) {
                        // (audio)
                        r = MATCHED_MEDIA_NO_MATCHED;
                    } else {
                        // none media tag
                        if (mediaType == SuppSrvConfig.MEDIA_TYPE_ONLY_AUDIO) {
                            r = MATCHED_MEDIA_NO_MATCHED;
                        } else {
                            r = MATCHED_MEDIA_VIDEO;
                        }
                    }
                }
            } else if (serviceClass == CommandsInterface.SERVICE_CLASS_NONE) {
                // Usually not go into this case, SERVICE_CLASS_NONE should be break down before
                Rlog.d(LOG_TAG, "isMediaMatchServiceClass: break down SERVICE_CLASS_NONE");
                r |= isMediaMatchServiceClass(media, CommandsInterface.SERVICE_CLASS_VOICE);
                r |= isMediaMatchServiceClass(media, ImsRILConstants.SERVICE_CLASS_VIDEO);
            }
            Rlog.d(LOG_TAG, "isMediaMatchServiceClass()=" + matchedMediaToString(r)
                    + ", mediaType=" + mediaTypeToString(mediaType)
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", media=" + printMedia);
            return r;
        }

        public int isRuleMatchServiceClass(Rule rule, int serviceClass) {
            boolean isSupportMediaTag = mSSConfig.isSupportMediaTag();
            int r = MATCHED_MEDIA_NO_MATCHED;
            if (isSupportMediaTag) {
                r = isMediaMatchServiceClass(rule.getConditions().getMedias(), serviceClass);
            } else {
                if (serviceClass == CommandsInterface.SERVICE_CLASS_NONE) {
                    r = (MATCHED_MEDIA_AUDIO | MATCHED_MEDIA_VIDEO);
                } else if ((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                    r = MATCHED_MEDIA_AUDIO;
                } else if ((serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                    r = MATCHED_MEDIA_VIDEO;
                }
            }
            Rlog.d(LOG_TAG, "isRuleMatchServiceClass()=" + matchedMediaToString(r)
                    + ", isSupportMediaTag=" + isSupportMediaTag
                    + ", service=" + serviceClassToString(serviceClass));
            return r;
        }

        public void setForwardTo(Actions action, String number) {
            if (number != null) {
                action.setFowardTo(number, true);
            }
            // Stop modify invalid action element
            //action.getFowardTo().setRevealIdentityToCaller(true);
            //action.getFowardTo().setRevealIdentityToTarget(true);
        }

        public void setMedia(Conditions cond, int serviceClass) {
            boolean isSupportMedia = mSSConfig.isSupportMediaTag();
            Rlog.d(LOG_TAG, "setMedia() isSupportMedia=" + isSupportMedia
                    + ", service=" + serviceClassToString(serviceClass));
            if (!isSupportMedia) {
                return;
            }
            if ((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                cond.addMedia(serviceClassToMediaString(CommandsInterface.SERVICE_CLASS_VOICE));
            }
            if ((serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                cond.addMedia(serviceClassToMediaString(ImsRILConstants.SERVICE_CLASS_VIDEO));
            }
        }

        public List<Rule> modifyMatchedCFRule(Rule rule, int reason,
                int action, int serviceClass, String number, int time, String timeSlot) {
            Rlog.d(LOG_TAG, "modifyMatchedCFRule() reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time + ", timeSlot=" + timeSlot);
            List<Rule> result = new ArrayList<Rule>();
            Conditions cond = rule.getConditions();
            Actions act = rule.getActions();

            if (action == CommandsInterface.CF_ACTION_ENABLE
                    || action == CommandsInterface.CF_ACTION_REGISTRATION) {
                // Enable the CF
                cond.removeRuleDeactivated();
                // Setup forward-to
                setForwardTo(act, number);
                // special case for NoReplyTimer in CFNRy element
                if (mSSConfig.isNoReplyTimeInsideCFAction() && time > 0
                        && reason == CommandsInterface.CF_REASON_NO_REPLY
                        && act.getNoReplyTimer() != -1) {
                    Rlog.d(LOG_TAG, "Set inside no-reply timer = " + time);
                    act.setNoReplyTimer(time);
                }
                // special case for need timeslot
                if (mSSConfig.isSupportTimeSlot() && timeSlot != null) {
                    cond.addTime(timeSlot);
                } else {
                    cond.addTime(null);
                }
            } else if (action == CommandsInterface.CF_ACTION_DISABLE
                    || action == CommandsInterface.CF_ACTION_ERASURE) {
                // Disable the CF
                cond.addRuleDeactivated();
                if (action == CommandsInterface.CF_ACTION_ERASURE) {
                    if (act.getFowardTo() != null) {
                        setForwardTo(act, "");
                    }
                }
            }
            result.add(rule);
            return result;
        }

        private int modifyCFRuleForSeperateMedia(List<Rule> result, RuleSet ruleSet, int reason,
                int action, int serviceClass, String number, int time, String timeSlot) {
            Rlog.d(LOG_TAG, "modifyCFRuleForSeperateMedia() reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time + ", timeSlot=" + timeSlot);
            List<Rule> ruleList = ruleSet.getRules();
            List<Integer> modifyRuleIdx = new ArrayList<Integer>();
            boolean findNotMatch = false;
            int r = 0;
            for (int i = 0; i < ruleList.size(); i++) {
                Rule rule = ruleList.get(i);
                if (getCFType(rule.getConditions()) == reason) {
                    int isMatchMedia = isRuleMatchServiceClass(rule, serviceClass);
                    if (isMatchMedia != MATCHED_MEDIA_NO_MATCHED) {
                        modifyRuleIdx.add(i);
                    } else {
                        findNotMatch = true;
                    }

                    // Record the modified rule for what media
                    if ((isMatchMedia & MATCHED_MEDIA_AUDIO) != 0) {
                        r |= MODIFIED_SERVICE_AUDIO;
                    }
                    if ((isMatchMedia & MATCHED_MEDIA_VIDEO) != 0) {
                        r |= MODIFIED_SERVICE_VIDEO;
                    }
                }
            }

            Rlog.d(LOG_TAG, "modifyRuleIdx size: " + modifyRuleIdx.size() +
                    ", findNotMatch: " + findNotMatch);

            if (modifyRuleIdx.size() == 0) {
                if (findNotMatch) {
                    if ((serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                        result.addAll(createCFRuleForService(ruleSet, reason, action,
                            ImsRILConstants.SERVICE_CLASS_VIDEO, number, time, timeSlot, "_VIDEO"));
                    } else if ((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                        result.addAll(createCFRuleForService(ruleSet, reason, action,
                            CommandsInterface.SERVICE_CLASS_VOICE, number, time, timeSlot, "_AUDIO"));
                    }
                }
            } else {
                // Only modify the rule.
                for (int i = 0; i < modifyRuleIdx.size(); i++) {
                    result.addAll(modifyMatchedCFRule(ruleList.get(modifyRuleIdx.get(i)),
                            reason, action, serviceClass, number, time, timeSlot));
                }
            }

            return r;
        }

        public int modifyCFRule(List<Rule> result, RuleSet ruleSet, Rule rule, int reason,
                int action, int serviceClass, String number, int time, String timeSlot) {
            Rlog.d(LOG_TAG, "modifyCFRule() reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time + ", timeSlot=" + timeSlot);
            int r = 0;
            int isMatchMedia = isRuleMatchServiceClass(rule, serviceClass);

            // If the media status is matched for ServiceClass, modify the rule
            if (isMatchMedia != MATCHED_MEDIA_NO_MATCHED) {
                result.addAll(modifyMatchedCFRule(rule, reason, action,
                    serviceClass, number, time, timeSlot));
            } else if (mSSConfig.getMediaTagType() == SuppSrvConfig.MEDIA_TYPE_SEPERATE
                    && (serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                //create new rule_postfix only for op18 video rule
                result.addAll(createCFRuleForService(ruleSet, reason, action,
                            ImsRILConstants.SERVICE_CLASS_VIDEO, number, time, timeSlot, "_VIDEO"));
            }

            // Record the modified rule for what media
            if ((isMatchMedia & MATCHED_MEDIA_AUDIO) != 0) {
                r |= MODIFIED_SERVICE_AUDIO;
            }
            if ((isMatchMedia & MATCHED_MEDIA_VIDEO) != 0) {
                r |= MODIFIED_SERVICE_VIDEO;
            }
            return r;
        }

        public String getRuleId(int reason) {
            Map idMap = mSSConfig.getRuleId();
            switch (reason) {
                case CommandsInterface.CF_REASON_UNCONDITIONAL:
                    return (String)idMap.get(SuppSrvConfig.RULEID_CFU);
                case CommandsInterface.CF_REASON_BUSY:
                    return (String)idMap.get(SuppSrvConfig.RULEID_CFB);
                case CommandsInterface.CF_REASON_NO_REPLY:
                    return (String)idMap.get(SuppSrvConfig.RULEID_CFNRy);
                case CommandsInterface.CF_REASON_NOT_REACHABLE:
                    return (String)idMap.get(SuppSrvConfig.RULEID_CFNRc);
                case ImsRILConstants.CF_REASON_NOT_REGISTERED:
                    return (String)idMap.get(SuppSrvConfig.RULEID_CFNL);
                default:
                    return "None";
            }
        }

        public List<Rule> createCFRuleForService(RuleSet ruleSet, int reason, int action,
                int serviceClass, String number, int time, String timeSlot, String ruleIdPostfix) {
            Rlog.d(LOG_TAG, "createCFRuleForService() reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time + ", timeSlot=" + timeSlot);
            List<Rule> result = new ArrayList<Rule>();
            Rule rule = ruleSet.createNewRule(getRuleId(reason) + ruleIdPostfix);
            Conditions cond = rule.createConditions();
            Actions act = rule.createActions();
            // Setup forward-to
            setForwardTo(act, number);
            // Setup conditions
            switch (reason) {
                case CommandsInterface.CF_REASON_UNCONDITIONAL:
                    break;
                case CommandsInterface.CF_REASON_BUSY:
                    cond.addBusy();
                    break;
                case CommandsInterface.CF_REASON_NO_REPLY:
                    cond.addNoAnswer();
                    // special case for NoReplyTimer in CFNry element
                    if (mSSConfig.isNoReplyTimeInsideCFAction() && time > 0) {
                        act.setNoReplyTimer(time);
                    }
                    break;
                case CommandsInterface.CF_REASON_NOT_REACHABLE:
                    cond.addNotReachable();
                    break;
                case ImsRILConstants.CF_REASON_NOT_REGISTERED:
                    cond.addNotRegistered();
                    break;
            }
            // Setup media
            setMedia(cond, serviceClass);
            // special case for need timeslot
            if (mSSConfig.isSupportTimeSlot() && timeSlot != null) {
                cond.addTime(timeSlot);
            }

            result.add(rule);
            return result;
        }

        public String modifiedServiceToString(int modifiedService) {
            String r = "";
            if ((modifiedService & MODIFIED_SERVICE_AUDIO) != 0) {
                r += "audio ";
            }
            if ((modifiedService & MODIFIED_SERVICE_VIDEO) != 0) {
                r += "video ";
            }
            if (r.equals("")) {
                r = "no modified";
            }
            return r;
        }

        public int[] serviceNeedToCreate(int modifiedService) {
            int[] r = new int[1];
            String print_r = "";
            int mediaType = mSSConfig.getMediaTagType();

            if (mediaType == SuppSrvConfig.MEDIA_TYPE_STANDARD) {
                if (modifiedService == 0) {
                    r[0] = (CommandsInterface.SERVICE_CLASS_VOICE
                            | ImsRILConstants.SERVICE_CLASS_VIDEO);
                } else if (modifiedService == MODIFIED_SERVICE_AUDIO) {
                    r[0] = ImsRILConstants.SERVICE_CLASS_VIDEO;
                } else if (modifiedService == MODIFIED_SERVICE_VIDEO) {
                    r[0] = CommandsInterface.SERVICE_CLASS_VOICE;
                } else {
                    r = new int[0];
                }
            } else if (mediaType == SuppSrvConfig.MEDIA_TYPE_ONLY_AUDIO) {
                if (modifiedService == 0) {
                    r[0] = CommandsInterface.SERVICE_CLASS_VOICE;
                } else {
                    r = new int[0];
                }
            } else if (mediaType == SuppSrvConfig.MEDIA_TYPE_SEPERATE) {
                if (modifiedService == 0) {
                    r = new int[2];
                    r[0] = CommandsInterface.SERVICE_CLASS_VOICE;
                    r[1] = ImsRILConstants.SERVICE_CLASS_VIDEO;
                } else if (modifiedService == MODIFIED_SERVICE_AUDIO) {
                    r[0] = ImsRILConstants.SERVICE_CLASS_VIDEO;
                } else if (modifiedService == MODIFIED_SERVICE_VIDEO) {
                    r[0] = CommandsInterface.SERVICE_CLASS_VOICE;
                } else {
                    r = new int[0];
                }
            } else if (mediaType == SuppSrvConfig.MEDIA_TYPE_VIDEO_WITH_AUDIO) {
                if (modifiedService == 0) {
                    r = new int[2];
                    r[0] = CommandsInterface.SERVICE_CLASS_VOICE;
                    r[1] = (CommandsInterface.SERVICE_CLASS_VOICE
                            | ImsRILConstants.SERVICE_CLASS_VIDEO);
                } else if (modifiedService == MODIFIED_SERVICE_AUDIO) {
                    r[0] = (CommandsInterface.SERVICE_CLASS_VOICE
                            | ImsRILConstants.SERVICE_CLASS_VIDEO);
                } else if (modifiedService == MODIFIED_SERVICE_VIDEO) {
                    r[0] = CommandsInterface.SERVICE_CLASS_VOICE;
                } else {
                    r = new int[0];
                }
            }
            for (int i=0; i<r.length; i++) {
                print_r += serviceClassToString(r[i]) + " ";
            }
            Rlog.d(LOG_TAG, "serviceNeedToCreate(): " + print_r
                    + ", mediaType=" + mediaTypeToString(mediaType)
                    + ", modifiedService=" + modifiedServiceToString(modifiedService));
            return r;
        }

        public List<Rule> createCFRule(int modifiedRuleService, RuleSet ruleSet, int reason,
                int action, int serviceClass, String number, int time, String timeSlot) {
            Rlog.d(LOG_TAG, "createCFRule()"
                    + " modifiedRuleService=" + modifiedServiceToString(modifiedRuleService)
                    + ", reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time + ", timeSlot=" + timeSlot);
            List<Rule> result = new ArrayList<Rule>();

            if ((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                result.addAll(createCFRuleForService(ruleSet, reason, action,
                        CommandsInterface.SERVICE_CLASS_VOICE, number, time, timeSlot,""));
            } else if ((serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                String ruleIdPostfix = "";
                if (mSSConfig.getMediaTagType() == SuppSrvConfig.MEDIA_TYPE_SEPERATE) {
                    ruleIdPostfix = "_VIDEO";
                }
                result.addAll(createCFRuleForService(ruleSet, reason, action,
                        ImsRILConstants.SERVICE_CLASS_VIDEO, number, time, timeSlot, ruleIdPostfix));
            } else if (serviceClass == CommandsInterface.SERVICE_CLASS_NONE) {
                // check modifiedRuleService and media type
                // 1. decide create one or two rule
                // 2. decide corresponding rule service
                int[] serviceNeedCreate = serviceNeedToCreate(modifiedRuleService);
                for (int i=0; i<serviceNeedCreate.length; i++) {
                    String ruleIdPostfix = "";
                    if (serviceNeedCreate[i] == ImsRILConstants.SERVICE_CLASS_VIDEO
                            && mSSConfig.getMediaTagType() == SuppSrvConfig.MEDIA_TYPE_SEPERATE) {
                        ruleIdPostfix = "_VIDEO";
                    }
                    result.addAll(createCFRuleForService(ruleSet, reason, action,
                            serviceNeedCreate[i], number, time, timeSlot, ruleIdPostfix));
                }
            }

            return result;
        }

        public List<Rule> getRuleForSetCF(CommunicationDiversion cd, int reason,
                int action, int serviceClass, String number, int time, String timeSlot) {
            Rlog.d(LOG_TAG, "getRuleForSetCF() reason=" + reasonCFToString(reason)
                    + ", action=" + actionCFToString(action)
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time + ", timeSlot=" + timeSlot);
            List<Rule> result = new ArrayList<Rule>();
            RuleSet ruleSet = cd.getRuleSet();
            boolean foundRule = false;
            int modifiedRuleService = 0;
            List<Rule> ruleList = ruleSet.getRules();
            if (ruleList != null) {
                if (mSSConfig.getMediaTagType() == SuppSrvConfig.MEDIA_TYPE_SEPERATE) {
                    List<Rule> modifiedRuleList = new ArrayList<Rule>();
                    modifiedRuleService |= modifyCFRuleForSeperateMedia(modifiedRuleList, ruleSet,
                            reason, action, serviceClass, number, time, timeSlot);
                    result.addAll(modifiedRuleList);
                    if (modifiedRuleList.size() > 0) {
                        foundRule = true;
                    }
                } else {
                    for (int i = 0; i < ruleList.size(); i++) {
                        Rule rule = ruleList.get(i);
                        // Search in the exist rules to match the CF operation
                        // Usually there is only one rule to match,
                        // but still may found many rules which are the same CF type
                        if (getCFType(rule.getConditions()) == reason) {
                            foundRule = true;
                            List<Rule> modifiedRuleList = new ArrayList<Rule>();
                            modifiedRuleService |= modifyCFRule(modifiedRuleList, ruleSet, rule, reason, action,
                                    serviceClass, number, time, timeSlot);
                            if (!rule.getActions().getFowardTo().mIsValidTargetNumber &&
                                !mSSConfig.isSupportPutNonUriNumber()) {
                                Rlog.d(LOG_TAG, "getRuleForSetCF() skip rule = " + rule.toXmlString());
                                continue;
                            }
                            result.addAll(modifiedRuleList);
                        }
                    }
                }
            }

            // Only in the situation of "enable CF" (don't care "disable CF") need create rule
            if (action == CommandsInterface.CF_ACTION_ENABLE
                    || action == CommandsInterface.CF_ACTION_REGISTRATION) {
                // Create rule when "not found" or "found but media not match"
                // or when SERVICE_CLASS_NONE but only modified AUDIO or VIDEO
                if (foundRule == false || modifiedRuleService == 0
                        || (serviceClass == CommandsInterface.SERVICE_CLASS_NONE
                                && (modifiedRuleService == MODIFIED_SERVICE_AUDIO
                                        || modifiedRuleService == MODIFIED_SERVICE_VIDEO))) {
                    result.addAll(createCFRule(modifiedRuleService, ruleSet, reason, action,
                            serviceClass, number, time, timeSlot));
                }
            }
            return result;
        }

        public boolean isEmptyCF(CommunicationDiversion cd) {
            boolean r = false;
            RuleSet ruleSet = cd.getRuleSet();
            List<Rule> ruleList = (ruleSet == null ? null : ruleSet.getRules());
            if (ruleSet == null || ruleList == null) {
                r = true;
            }
            Rlog.d(LOG_TAG, "isEmptyCF()=" + r);
            return r;
        }

        public String convertUriToNumber(String uri) {
            String r = uri;
            if (uri != null && (uri.startsWith("sip:") || uri.startsWith("sips:"))) {
                int offset = uri.indexOf(";");
                if (offset == -1) {
                    offset = uri.length();
                }
                r = uri.substring(uri.indexOf(":") + 1, offset);
                if (r.contains("@")) {
                    r = r.substring(r.indexOf(":") + 1, r.indexOf("@"));
                }
            } else if (uri != null && uri.startsWith("tel:")) {
                int offset = uri.indexOf(";");
                if (offset == -1) {
                    offset = uri.length();
                }
                r = uri.substring(uri.indexOf(":")+1, offset);
            }
            Rlog.d(LOG_TAG, "convertUriToNumber: " + ((!SENLOG) ? r : "[hidden]"));
            return r;
        }

        public boolean isAllRulesDeativated(List<Rule> ruleList) {
            for (Rule rule : ruleList) {
                if (rule.getConditions().comprehendRuleDeactivated() == false) {
                    return false;
                }
            }
            return true;
        }

        public String convertCFNumber(String number, int phoneId) {
            // target should be a SIP URI (IETF RFC 3261 [6]) or TEL URL (IETF RFC 3966 [7])
            // also refer to 3gpp 24.229 - 5.1.2A.1.5
            if (number != null) {
                boolean isContainPlus = number.contains("+");
                boolean isContainContext = number.contains("phone-context");
                boolean isURIFormat = number.startsWith("tel:") || number.startsWith("sip:")
                        || number.startsWith("sips:");
                boolean isPhoneContextNeed = false;

                String domain = MMTelSSUtils.getXui(phoneId, mContext);
                if (!isContainPlus && !isContainContext) {
                    int offset = domain.indexOf("@");
                    if (offset != -1) {
                        domain = domain.substring(offset + 1);
                        isPhoneContextNeed = true;
                    }
                    if (!TextUtils.isEmpty(mSSConfig.getPhoneContext())) {
                        domain = mSSConfig.getPhoneContext();
                        isPhoneContextNeed = true;
                    }
                    Rlog.d(LOG_TAG, "domain:" + domain);
                }

                if (isPhoneContextNeed) {
                    if (!isURIFormat) {
                        if (mSSConfig.isFwdNumUseSipUri()) {
                            number = "sip:" + number + ";phone-context=" + domain +
                                    "@" + domain + ";user=phone";
                        } else {
                            number = "tel:" + number + ";phone-context=" + domain;
                        }
                    } else {
                        number = number + ";phone-context=" + domain;
                    }
                } else {
                    if (!isURIFormat) {
                        number = "tel:" + number;
                    }
                }
            }

            String XcapCFNum = SystemProperties.get(PROP_SS_CFNUM, "");
            if (XcapCFNum.startsWith("sip:") || XcapCFNum.startsWith("sips:")
                    || XcapCFNum.startsWith("tel:")) {
                Rlog.d(LOG_TAG, "handleSetCF():get call forwarding num from EM setting:"
                        + XcapCFNum);
                String ss_mode = SystemProperties.get(PROP_SS_MODE, MODE_SS_XCAP);
                Rlog.d(LOG_TAG, "handleSetCF():ss_mode=" + ss_mode);
                if (MODE_SS_XCAP.equals(ss_mode)) {
                    number = XcapCFNum;
                }
            }
            return number;
        }

        public int convertServiceClass(int serviceClass) {
            int r = serviceClass;
            //Change the serviceClass (VIDEO + DATA_SYNC) to VIDEO directly
            if (serviceClass == (ImsRILConstants.SERVICE_CLASS_VIDEO
                    | CommandsInterface.SERVICE_CLASS_DATA_SYNC)) {
                r = ImsRILConstants.SERVICE_CLASS_VIDEO;
            }
            return r;
        }

        public void triggerCSFB(Message msg) {
            Rlog.d(LOG_TAG, "triggerCSFB msg=" + msg);
            if (msg != null) {
                AsyncResult.forMessage(msg, null, new UnknownHostException());
                msg.sendToTarget();
            }
        }

        public void handleSetCF(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int action = rr.mp.readInt();
            int reason = rr.mp.readInt();
            int serviceClass = rr.mp.readInt();
            String number = rr.mp.readString();
            int time = rr.mp.readInt();
            int phoneId = rr.mp.readInt();
            Message msg = rr.mResult;

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", action=" + action
                    + ", reason=" + reason
                    + ", serviceClass=" + serviceClass
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time
                    + ", phoneId=" + phoneId
                    + ", msg=" + msg);

            handleSetCF(reqNo, action, reason, serviceClass, number, time, null, phoneId, msg);
        }

        public void handleSetCF(int reqNo, int action, int reason, int serviceClass,
                String number, int time, String timeSlot, int phoneId, Message msg) {

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(msg);
                return;
            }

            Exception exceptionReport = null;
            number = convertCFNumber(number, phoneId);
            serviceClass = convertServiceClass(serviceClass);

            Rlog.d(LOG_TAG, "handleSetCF() " + requestToString(reqNo)
                    + ", action=" + actionCFToString(action)
                    + ", reason=" + reasonCFToString(reason)
                    + ", serviceClass=" + serviceClassToString(serviceClass)
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time
                    + ", timeSlot=" + timeSlot
                    + ", phoneId=" + phoneId);


            try {
                CommunicationDiversion cd = (CommunicationDiversion)getCache(CACHE_IDX_CF, phoneId);
                boolean isEmptyRules = isEmptyCF(cd);

                // Find out the rules which are ready to PUT
                List<Rule> resultList = new ArrayList<Rule>();
                int newReason = -1;
                boolean needAdd = true;

                if (reason == CommandsInterface.CF_REASON_ALL_CONDITIONAL) {
                    // CFB/CFNRy/CFNRc/CFNL
                    for (int i=0; i<4; i++) {
                        newReason = CF_REASON[i];
                        needAdd = !(mSSConfig.isNotSupportCFNotRegistered()
                                && newReason == ImsRILConstants.CF_REASON_NOT_REGISTERED);
                        Rlog.d(LOG_TAG, "reason == 4, needAdd = " + needAdd);
                        if (needAdd) {
                            resultList.addAll(getRuleForSetCF(cd, newReason, action,
                                    serviceClass, number, time, timeSlot));
                        }
                    }
                } else if (reason == CommandsInterface.CF_REASON_ALL) {
                    // CFB/CFNRy/CFNRc/CFNL/CFU
                    for (int i=0; i<5; i++) {
                        newReason = CF_REASON[i];
                        needAdd = !(mSSConfig.isNotSupportCFNotRegistered()
                                && newReason == ImsRILConstants.CF_REASON_NOT_REGISTERED);
                        Rlog.d(LOG_TAG, "reason == 5, needAdd = " + needAdd);
                        if (needAdd) {
                            resultList.addAll(getRuleForSetCF(cd, newReason, action,
                                    serviceClass, number, time, timeSlot));
                        }
                    }
                } else {
                    resultList.addAll(getRuleForSetCF(cd, reason, action,
                            serviceClass, number, time, timeSlot));
                    if (reason == CommandsInterface.CF_REASON_NOT_REACHABLE
                            && mSSConfig.isSetCFNRcWithCFNL()) {
                        newReason = ImsRILConstants.CF_REASON_NOT_REGISTERED;
                        resultList.addAll(getRuleForSetCF(cd, newReason, action,
                                serviceClass, number, time, timeSlot));
                    }
                }

                if (mSSConfig.isSupportPutCfRoot()
                        && (mSSConfig.isSaveWholeNode()
                        || reason == CommandsInterface.CF_REASON_ALL_CONDITIONAL
                        || reason == CommandsInterface.CF_REASON_ALL)) {

                    RuleSet ruleSet = cd.getRuleSet();
                    List<Rule> fulllist = ruleSet.getRules();

                    if (time >0
                            && (reason == CommandsInterface.CF_REASON_ALL_CONDITIONAL
                            || reason ==  CommandsInterface.CF_REASON_ALL
                            || reason == CommandsInterface.CF_REASON_NO_REPLY)) {
                        cd.setNoReplyTimer(time,false);
                    }

                    /*
                     * Currently TMO doesn't allow user to update active status in header.
                     * So we will always update active status to true in header.
                     * Once there is an operator requests to update active status to false when
                     * disable all CF rules then we will refine the solution.
                     */
                    //if (isAllRulesDeativated(fulllist)) {
                    //    cd.save(false);
                    //} else {
                        cd.save(true);
                    //}
                } else {
                    // Update the found rules to server
                    // If original CF is empty, update the RuleSet node.
                    // Otherwise update each rules.
                    if (isEmptyRules) {
                        cd.saveRuleSet();
                    } else {
                        for (Rule rule : resultList) {
                            Rlog.d(LOG_TAG, "handleSetCF(): rule=" + MtkSuppServHelper.encryptString(rule.toXmlString()));
                            cd.saveRule(rule);
                            // For CFNRy to update NoReplyTimer
                            if (getCFType(rule.getConditions()) ==
                                    CommandsInterface.CF_REASON_NO_REPLY
                                    && time > 0 && cd.getNoReplyTimer() > -1
                                    && !mSSConfig.isNoReplyTimeInsideCFAction()) {
                                Rlog.d(LOG_TAG, "Set outside no-reply timer = " + time);
                                cd.setNoReplyTimer(time,true);
                            }
                        }
                    }
                }

            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
           } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            clearCache(CACHE_IDX_CF);

            if (msg != null) {
                AsyncResult.forMessage(msg, null, exceptionReport);
                msg.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }

        public List<Rule> createCBRuleForService(RuleSet ruleSet, String facility, int serviceClass,
                int lockState) {
            Rlog.d(LOG_TAG, "createCBRuleForService() facility=" + facility
                    + ", lockState=" + (lockState == 1 ? "Enable" : "Disable")
                    + ", service=" + serviceClassToString(serviceClass));
            List<Rule> result = new ArrayList<Rule>();
            Rule rule = ruleSet.createNewRule(facility);
            Conditions cond = rule.createConditions();
            Actions act = rule.createActions();

            // Setup conditions
            if (facility.equals(CommandsInterface.CB_FACILITY_BAOIC)) {
                cond.addInternational();
            } else if (facility.equals(CommandsInterface.CB_FACILITY_BAOICxH)) {
                cond.addInternationalExHc();
            } else if (facility.equals(CommandsInterface.CB_FACILITY_BAICr)) {
                cond.addRoaming();
            }
            // Setup media
            setMedia(cond, serviceClass);
            // Enable CB
            act.setAllow(false);

            result.add(rule);
            return result;
        }

        public List<Rule> createCBRule(int modifiedRuleService, RuleSet ruleSet,
                String facility, int serviceClass, int lockState) {
            Rlog.d(LOG_TAG, "createCBRule() "
                    + " modifiedRuleService=" + modifiedServiceToString(modifiedRuleService)
                    + ", facility=" + facility
                    + ", lockState=" + (lockState == 1 ? "Enable" : "Disable")
                    + ", service=" + serviceClassToString(serviceClass));
            List<Rule> result = new ArrayList<Rule>();

            if ((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                result.addAll(createCBRuleForService(ruleSet, facility,
                        CommandsInterface.SERVICE_CLASS_VOICE, lockState));
            } else if ((serviceClass & ImsRILConstants.SERVICE_CLASS_VIDEO) != 0) {
                result.addAll(createCBRuleForService(ruleSet, facility,
                        ImsRILConstants.SERVICE_CLASS_VIDEO, lockState));
            } else if (serviceClass == CommandsInterface.SERVICE_CLASS_NONE) {
                // check modifiedRuleService and media type
                // 1. decide create one or two rule
                // 2. decide corresponding rule service
                int[] serviceNeedCreate = serviceNeedToCreate(modifiedRuleService);
                for (int i=0; i<serviceNeedCreate.length; i++) {
                    result.addAll(createCBRuleForService(ruleSet, facility, serviceNeedCreate[i],
                            lockState));
                }

            }

            return result;
        }

        public List<Rule> modifyMatchedCBRule(Rule rule, String facility, int serviceClass,
                int lockState) {
            Rlog.d(LOG_TAG, "modifyMatchedCBRule() facility=" + facility
                    + ", lockState=" + (lockState == 1 ? "Enable" : "Disable")
                    + ", service=" + serviceClassToString(serviceClass));
            List<Rule> result = new ArrayList<Rule>();
            Conditions cond = rule.getConditions();
            Actions act = rule.getActions();

            if (lockState == 1) {
                // Enable CB
                cond.removeRuleDeactivated();
                act.setAllow(false);
            } else {
                // Disable CB
                // TODO: 1. remove rule or 2. add <rule-deactivated> or 3. allow= true
                cond.addRuleDeactivated();
            }

            result.add(rule);
            return result;
        }

        public int modifyCBRule(List<Rule> result, Rule rule, String facility,
                int serviceClass, int lockState) {
            Rlog.d(LOG_TAG, "modifyCBRule() facility=" + facility
                    + ", lockState=" + (lockState == 1 ? "Enable" : "Disable")
                    + ", service=" + serviceClassToString(serviceClass));
            int r = 0;
            int isMatchMedia = isRuleMatchServiceClass(rule, serviceClass);

            // If the media status is matched for ServiceClass, modify the rule
            if (isMatchMedia != MATCHED_MEDIA_NO_MATCHED) {
                result.addAll(modifyMatchedCBRule(rule, facility, serviceClass, lockState));
            }

            // Record the modified rule for what media
            if ((isMatchMedia & MATCHED_MEDIA_AUDIO) != 0) {
                r |= MODIFIED_SERVICE_AUDIO;
            }
            if ((isMatchMedia & MATCHED_MEDIA_VIDEO) != 0) {
                r |= MODIFIED_SERVICE_VIDEO;
            }

            return r;
        }

        public List<Rule> getRuleForSetCB(SimservType cb, String facility, int serviceClass,
                int lockState, int phoneId) {
            Rlog.d(LOG_TAG, "getRuleForSetCB() facility=" + facility
                    + ", service=" + serviceClassToString(serviceClass)
                    + ", lockState=" + lockState + ", phoneId=" + phoneId);
            List<Rule> result = new ArrayList<Rule>();
            RuleSet ruleSet = null;
            if (getCBType(facility) == CB_OCB) {
                ruleSet = ((OutgoingCommunicationBarring)cb).getRuleSet();
            } else if (getCBType(facility) == CB_ICB) {
                ruleSet = ((IncomingCommunicationBarring)cb).getRuleSet();
            }
            boolean foundRule = false;
            int modifiedRuleService = 0;
            List<Rule> ruleList = (ruleSet == null ? null : ruleSet.getRules());
            if (ruleList != null) {
                for (Rule rule : ruleList) {
                    Conditions cond = rule.getConditions();
                    Actions act = rule.getActions();
                    // Search in the exist rules to match the CB operation
                    // Usually there is only one rule to match,
                    // but still may found many rules which are the same CB type
                    if (getCBFacility(getCBType(facility), cond).equals(facility)) {
                        foundRule = true;
                        List<Rule> modifiedRuleList = new ArrayList<Rule>();
                        modifiedRuleService |= modifyCBRule(modifiedRuleList, rule, facility,
                                serviceClass, lockState);
                        result.addAll(modifiedRuleList);
                    }
                }
            }

            // Only in the situation of "enable CB" (don't care "disable CB") need create rule
            if (lockState == 1) {
                // Create rule when "not found" or "found but media not match"
                // or when SERVICE_CLASS_NONE but only modified AUDIO or VIDEO
                if (foundRule == false || modifiedRuleService == 0
                        || (serviceClass == CommandsInterface.SERVICE_CLASS_NONE
                                && (modifiedRuleService == MODIFIED_SERVICE_AUDIO
                                        || modifiedRuleService == MODIFIED_SERVICE_VIDEO))) {
                    result.addAll(createCBRule(modifiedRuleService, ruleSet, facility, serviceClass,
                            lockState));
                }
            }
            return result;
        }

        public boolean isEmptyCB(int cbType, SimservType cb) {
            boolean r = false;
            RuleSet ruleSet = null;
            if (cbType == CB_OCB) {
                ruleSet = ((OutgoingCommunicationBarring)cb).getRuleSet();
            } else if (cbType == CB_ICB) {
                ruleSet = ((IncomingCommunicationBarring)cb).getRuleSet();
            }
            List<Rule> ruleList = (ruleSet == null ? null : ruleSet.getRules());
            if (ruleSet == null || ruleList == null) {
                r = true;
            }
            Rlog.d(LOG_TAG, "isEmptyCB()=" + r);
            return r;
        }

        public void handleSetCB(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            String facility = rr.mp.readString();
            int lockState = rr.mp.readInt();
            int serviceClass = rr.mp.readInt();
            int phoneId = rr.mp.readInt();

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", facility=" + facility
                    + ", serviceClass=" + serviceClass
                    + ", lockState=" + lockState
                    + ", phoneId=" + phoneId);

            if (!MMTelSSUtils.isPreferXcap(phoneId, mContext)
                    || !updateNetworkInitSimServ(phoneId)) {
                triggerCSFB(rr.mResult);
                return;
            }

            Exception exceptionReport = null;
            serviceClass = convertServiceClass(serviceClass);
            int cbType = getCBType(facility);

            Rlog.d(LOG_TAG, "handleSetCB() " + requestToString(reqNo)
                    + ", facility=" + facility
                    + ", cbType=" + cbTypeToString(cbType)
                    + ", serviceClass=" + serviceClassToString(serviceClass)
                    + ", lockState=" + lockState
                    + ", phoneId=" + phoneId);

            try {
                if (lockState != 0 && (cbType == CB_ALL || cbType == CB_MO || cbType == CB_MT)) {
                    throw new Exception();
                }

                SimservType ocb = null;
                SimservType icb = null;
                boolean isEmptyOcbRules = false;
                boolean isEmptyIcbRules = false;

                if (cbType == CB_ALL || cbType == CB_MO || cbType == CB_OCB) {
                    ocb = getCache(CACHE_IDX_OCB, phoneId);
                    isEmptyOcbRules = isEmptyCB(CB_OCB, ocb);
                }
                if (cbType == CB_ALL || cbType == CB_MT || cbType == CB_ICB) {
                    icb = getCache(CACHE_IDX_ICB, phoneId);
                    isEmptyIcbRules = isEmptyCB(CB_ICB, icb);
                }

                // Find out the rules which are ready to PUT
                List<Rule> resultOcbList = new ArrayList<Rule>();
                List<Rule> resultIcbList = new ArrayList<Rule>();
                if (cbType == CB_ALL) {
                    // AO/OI/OX/AI/IR
                    for (int i=0; i<5; i++) {
                        facility = CB_FACILITY[i];
                        if (i < 3) {
                            resultOcbList.addAll(getRuleForSetCB(ocb, facility, serviceClass,
                                    lockState, phoneId));
                        } else {
                            resultIcbList.addAll(getRuleForSetCB(icb, facility, serviceClass,
                                    lockState, phoneId));
                        }
                    }
                } else if (cbType == CB_MO) {
                    // AO/OI/OX
                    for (int i=0; i<3; i++) {
                        facility = CB_FACILITY[i];
                        resultOcbList.addAll(getRuleForSetCB(ocb, facility, serviceClass,
                                lockState, phoneId));
                    }
                } else if (cbType == CB_MT) {
                    // AI/IR
                    for (int i=3; i<5; i++) {
                        facility = CB_FACILITY[i];
                        resultIcbList.addAll(getRuleForSetCB(icb, facility, serviceClass,
                                lockState, phoneId));
                    }
                } else if (cbType == CB_OCB) {
                    resultOcbList.addAll(getRuleForSetCB(ocb, facility, serviceClass,
                            lockState, phoneId));
                } else if (cbType == CB_ICB) {
                    resultIcbList.addAll(getRuleForSetCB(icb, facility, serviceClass,
                            lockState, phoneId));
                }

                if (mSSConfig.isSaveWholeNode()) {
                    if ((cbType == CB_ALL || cbType == CB_MO || cbType == CB_OCB) &&
                            (ocb != null)) {

                        RuleSet ruleSet = ((OutgoingCommunicationBarring)ocb).getRuleSet();
                        List<Rule> fulllist = ruleSet.getRules();

                        if (isAllRulesDeativated(fulllist)) {
                            ((OutgoingCommunicationBarring)ocb).save(false);
                        } else {
                            ((OutgoingCommunicationBarring)ocb).save(true);
                        }
                    }

                    if ((cbType == CB_ALL || cbType == CB_MT || cbType == CB_ICB) &&
                            (icb != null)) {

                        RuleSet ruleSet = ((IncomingCommunicationBarring)icb).getRuleSet();
                        List<Rule> fulllist = ruleSet.getRules();

                        if (isAllRulesDeativated(fulllist)) {
                            ((IncomingCommunicationBarring)icb).save(false);
                        } else {
                            ((IncomingCommunicationBarring)icb).save(true);
                        }
                    }
                } else {
                    // Update the found rules to server
                    // If original CB is empty, update the RuleSet node.
                    // Otherwise update each rules.
                    if (isEmptyOcbRules) {
                        ((OutgoingCommunicationBarring)ocb).saveRuleSet();
                    } else {
                        for (Rule rule : resultOcbList) {
                            Rlog.d(LOG_TAG, "handleSetCB(): rule=" + rule.toXmlString());
                            ((OutgoingCommunicationBarring)ocb).saveRule(rule.mId);
                        }
                    }

                    if (isEmptyIcbRules) {
                        ((IncomingCommunicationBarring)icb).saveRuleSet();
                    } else {
                        for (Rule rule : resultIcbList) {
                            Rlog.d(LOG_TAG, "handleSetCB(): rule=" + rule.toXmlString());
                            ((IncomingCommunicationBarring)icb).saveRule(rule.mId);
                        }
                    }
                }
            } catch (XcapException xcapException) {
                xcapException.printStackTrace();
                exceptionReport = reportXcapException(xcapException);
            } catch (Exception e) {
                e.printStackTrace();
                exceptionReport = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
            }

            if (cbType == CB_OCB || cbType == CB_MO) {
                clearCache(CACHE_IDX_OCB);
            } else if (cbType == CB_ICB || cbType == CB_MT) {
                clearCache(CACHE_IDX_ICB);
            } else {
                clearCache(CACHE_IDX_OCB);
                clearCache(CACHE_IDX_ICB);
            }

            if (rr.mResult != null) {
                AsyncResult.forMessage(rr.mResult, null, exceptionReport);
                rr.mResult.sendToTarget();
            }

            if (mXcapMobileDataNetworkManager != null) {
                mXcapMobileDataNetworkManager.releaseNetwork();
            }
        }

        /// For OP01 UT @{
        public void handleGetCFInTimeSlot(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int action = rr.mp.readInt();
            int reason = rr.mp.readInt();
            int serviceClass = rr.mp.readInt();
            int phoneId = rr.mp.readInt();
            Message msg = rr.mResult;

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", action=" + action
                    + ", reason=" + reason
                    + ", serviceClass=" + serviceClass
                    + ", phoneId=" + phoneId
                    + ", msg=" + msg);

            handleGetCF(reqNo, serialNo, action, reason,
                    serviceClass, null, phoneId, true, msg);
        }

        public void handleSetCFInTimeSlot(MMTelSSRequest rr) {
            rr.mp.setDataPosition(0);
            int reqNo = rr.mp.readInt();
            int serialNo = rr.mp.readInt();
            int action = rr.mp.readInt();
            int reason = rr.mp.readInt();
            int serviceClass = rr.mp.readInt();
            String number = rr.mp.readString();
            int time = rr.mp.readInt();
            long[] timeSlot = new long[2];
            try {
                rr.mp.readLongArray(timeSlot);
            } catch (Exception e) {
                timeSlot = null;
            }
            String timeSlotString = convertToSeverTime(timeSlot);
            int phoneId = rr.mp.readInt();
            Message msg = rr.mResult;

            Rlog.d(LOG_TAG, "Read from parcel: " + requestToString(reqNo)
                    + ", action=" + action
                    + ", reason=" + reason
                    + ", serviceClass=" + serviceClass
                    + ", number=" + MtkSuppServHelper.encryptString(number)
                    + ", time=" + time
                    + ", timeSlot=" + timeSlotString
                    + ", phoneId=" + phoneId
                    + ", msg=" + msg);

            handleSetCF(reqNo, action, reason, serviceClass, number, time, timeSlotString, phoneId,
                    msg);
        }

        public long[] convertToLocalTime(String timeSlotString) {
            long[] timeSlot = null;
            if (timeSlotString != null) {
                String[] timeArray = timeSlotString.split(",", 2);
                if (timeArray.length == 2) {
                    timeSlot = new long[2];
                    for (int i = 0; i < 2; i++) {
                        SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm");
                        dateFormat.setTimeZone(TimeZone.getTimeZone("GMT+8"));
                        try {
                            Date date = dateFormat.parse(timeArray[i]);
                            timeSlot[i] = date.getTime();
                        } catch (ParseException e) {
                            e.printStackTrace();
                            return null;
                        }
                    }
                }
            }
            return timeSlot;
        }

        public String convertToSeverTime(long[] timeSlot) {
            String timeSlotString = null;
            if (timeSlot == null || timeSlot.length != 2) {
                return null;
            }
            for (int i = 0; i < timeSlot.length; i++) {
                Date date = new Date(timeSlot[i]);
                SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm");
                dateFormat.setTimeZone(TimeZone.getTimeZone("GMT+8"));
                if (i == 0) {
                    timeSlotString = dateFormat.format(date);
                } else {
                    timeSlotString += "," + dateFormat.format(date);
                }
            }
            return timeSlotString;
        }
        /// @}

        //***** Handler implementation
        @Override public void
        handleMessage(Message msg) {
            MMTelSSRequest rr = (MMTelSSRequest) (msg.obj);
            MMTelSSRequest req = null;

            switch (msg.what) {
                case EVENT_SEND:
                    /**
                     * mRequestMessagePending++ already happened for every
                     * EVENT_SEND, thus we must make sure
                     * mRequestMessagePending-- happens once and only once
                     */
                    boolean alreadySubtracted = false;
                    int reqNo = -1;
                    int serialNo = -1;

                    Rlog.d(LOG_TAG, "handleMessage(): EVENT_SEND:"
                            + "mRequestMessagesPending = " + mRequestMessagesPending
                            + ", mRequestsList.size() = " + mRequestsList.size());
                    try {
                        synchronized (mRequestsList) {
                            mRequestsList.add(rr);
                        }

                        mRequestMessagesPending--;
                        alreadySubtracted = true;
                        //MTK-END [mtk04070][111121][ALPS00093395]MTK modified


                        //[MMTelSS] Because it always gets response from simServs immediately,
                        // it must invoke findAndRemoveRequestFromList() here instead of RIL's
                        // invoking at proceeResponse()
                        findAndRemoveRequestFromList(rr.mSerial);

                        //Rlog.d(LOG_TAG, "Receive MMTelSS Request:" + requestToString(rr.mRequest)
                        // + ", parcel dataLen=" + data.length);
                        Rlog.d(LOG_TAG, "Receive MMTelSS Request:" + requestToString(rr.mRequest));

                        switch (rr.mRequest) {
                            case MMTELSS_REQ_GET_CLIP:
                                handleGetIdentity(rr, IDENTITY_CLIP);
                                break;
                            case MMTELSS_REQ_SET_CLIP:
                                handleSetIdentity(rr, IDENTITY_CLIP);
                                break;
                            case MMTELSS_REQ_GET_CLIR:
                                handleGetIdentity(rr, IDENTITY_CLIR);
                                break;
                            case MMTELSS_REQ_SET_CLIR:
                                handleSetIdentity(rr, IDENTITY_CLIR);
                                break;
                            case MMTELSS_REQ_GET_COLP:
                                handleGetIdentity(rr, IDENTITY_COLP);
                                break;
                            case MMTELSS_REQ_SET_COLP:
                                handleSetIdentity(rr, IDENTITY_COLP);
                                break;
                            case MMTELSS_REQ_GET_COLR:
                                handleGetIdentity(rr, IDENTITY_COLR);
                                break;
                            case MMTELSS_REQ_SET_COLR:
                                handleSetIdentity(rr, IDENTITY_COLR);
                                break;
                            case MMTELSS_REQ_SET_CW:
                                handleSetCW(rr);
                                break;
                            case MMTELSS_REQ_GET_CW:
                                handleGetCW(rr);
                                break;
                            case MMTELSS_REQ_SET_CB:
                                handleSetCB(rr);
                                break;
                            case MMTELSS_REQ_GET_CB:
                                handleGetCB(rr);
                                break;
                            case MMTELSS_REQ_SET_CF:
                                handleSetCF(rr);
                                break;
                            case MMTELSS_REQ_GET_CF:
                                handleGetCF(rr);
                                break;
                            /// For OP01 UT @{
                            case MMTELSS_REQ_SET_CF_TIME_SLOT:
                                handleSetCFInTimeSlot(rr);
                                break;
                            case MMTELSS_REQ_GET_CF_TIME_SLOT:
                                handleGetCFInTimeSlot(rr);
                                break;
                            /// @}
                            default:
                                Rlog.d(LOG_TAG, "Invalid MMTelSS Request:" + rr.mRequest);
                                throw new RuntimeException("Unrecognized MMTelSS Request: "
                                        + rr.mRequest);
                        }

                        //Rlog.v(LOG_TAG, "writing packet: " + data.length + " bytes");

                    } catch (RuntimeException exc) {
                        Rlog.e(LOG_TAG, "Uncaught exception ", exc);
                        req = findAndRemoveRequestFromList(rr.mSerial);
                        // make sure this request has not already been handled,
                        // eg, if RILReceiver cleared the list.
                        Rlog.d(LOG_TAG, "handleMessage(): RuntimeException:"
                                + "mRequestMessagesPending = " + mRequestMessagesPending
                                + ", mRequestsList.size() = " + mRequestsList.size());
                        if (req != null || !alreadySubtracted) {
                            rr.onError(RILConstants.GENERIC_FAILURE, null);
                            rr.release();
                        }
                    } finally {
                        // Note: We are "Done" only if there are no outstanding
                        // requests or replies. Thus this code path will only release
                        // the wake lock on errors.
                        releaseWakeLockIfDone();
                    }

                    //MTK-START [mtk04070][111121][ALPS00093395]MTK modified
                    if (!alreadySubtracted) {
                        Rlog.d(LOG_TAG, "handleMessage(): !alreadySubtracted:"
                                + "mRequestMessagesPending = " + mRequestMessagesPending
                                + ", mRequestsList.size() = " + mRequestsList.size());
                        mRequestMessagesPending--;
                    }
                    //MTK-END [mtk04070][111121][ALPS00093395]MTK modified

                    //Recycle the Parcel object back to the pool by mtk01411
                    if (rr.mp != null) {
                        rr.mp.recycle();
                        rr.mp = null;
                    }

                    if ((mRequestMessagesPending != 0) || (mRequestsList.size() != 0)) {
                        Rlog.d(LOG_TAG, "handleMessage(): ERROR wakeLock:"
                                + "mRequestMessagesPending = " + mRequestMessagesPending
                                + ", mRequestsList.size() = " + mRequestsList.size());
                    }
                    break;

                case EVENT_WAKE_LOCK_TIMEOUT:
                    // Haven't heard back from the last request.  Assume we're
                    // not getting a response and  release the wake lock.
                    // TODO should we clean up mRequestList and mRequestPending
                    synchronized (mWakeLock) {
                        if (mWakeLock.isHeld()) {
                            if (DBG) {
                                synchronized (mRequestsList) {
                                    int count = mRequestsList.size();
                                    Rlog.d(LOG_TAG, "WAKE_LOCK_TIMEOUT " +
                                            " mReqPending=" + mRequestMessagesPending +
                                            " mRequestList=" + count);

                                    for (int i = 0; i < count; i++) {
                                        rr = mRequestsList.get(i);
                                        Rlog.d(LOG_TAG, i + ": [" + rr.mSerial + "] " +
                                                requestToString(rr.mRequest));

                                    }
                                }
                            }
                            mWakeLock.release();
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    };

    /**
     * Set CLIR for the specific phoneId.
     * @param clirMode enable/disable CLIR
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    setCLIR(int clirMode, Message result, int phoneId) {
        //OriginatingIdentityPresentation oip =
        // SimServs.getOriginatingIdentityPresentation(xcapUri, TEST_USER, "password");
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_CLIR, result);
        //clirMode: 0-CommandsInterface.CLIR_DEFAULT , 1-CommandsInterface.CLIR_INVOCATION
        // (restrict CLI presentation), 2-CommandsInterface.CLIR_SUPPRESSION
        // (allow CLI presentation)
        rr.mp.writeInt(clirMode);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get CLIR mode for the specific phoneId.
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    getCLIR(Message result, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_GET_CLIR, result);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Set CLIP for the specific phoneId.
     * @param clipEnable enable/disable CLIP
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    setCLIP(int clipEnable, Message result, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_CLIP, result);
        rr.mp.writeInt(clipEnable);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get CLIP for the specific phoneId.
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    queryCLIP(Message result, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_GET_CLIP, result);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Set COLP for the specific phoneId.
     * @param colpEnable enable/disable COLP
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    setCOLP(int colpEnable, Message result, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_COLP, result);
        rr.mp.writeInt(colpEnable);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get COLP for the specific phoneId.
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    getCOLP(Message result, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr
        = MMTelSSRequest.obtain(MMTELSS_REQ_GET_COLP, result);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Set COLR for the specific phoneId.
     * @param colrMode enable/disable COLR
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    setCOLR(int colrMode, Message result, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_COLR, result);
        rr.mp.writeInt(colrMode);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get COLR for the specific phoneId.
     * @param result Message callback
     * @param phoneId the phone index
     */
    public void
    getCOLR(Message result, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr
        = MMTelSSRequest.obtain(MMTELSS_REQ_GET_COLR, result);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Set Call Waiting for the specific phoneId.
     * @param enable enable/disable Call Waiting
     * @param serviceClass service class for Call Waiting
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    setCallWaiting(boolean enable, int serviceClass, Message response, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_CW, response);
        rr.mp.writeInt((enable == true) ? 1 : 0);
        rr.mp.writeInt(serviceClass);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get Call Waiting mode for the specific phoneId.
     * @param serviceClass service class for Call Waiting
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    queryCallWaiting(int serviceClass, Message response, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_GET_CW, response);
        rr.mp.writeInt(serviceClass);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Set Call Barring for the specific phoneId.
     * @param facility Call Barring type
     * @param lockState enable/disable Call Barring
     * @param password password
     * @param serviceClass service class for Call Barring
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    setFacilityLock(String facility, boolean lockState, String password,
            int serviceClass, Message response, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_CB, response);
        rr.mp.writeString(facility);
        rr.mp.writeInt((lockState == true) ? 1 : 0);
        rr.mp.writeInt(serviceClass);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get Call Barring mode for the specific phoneId.
     * @param facility Call Barring type
     * @param password password
     * @param serviceClass service class for Call Barring
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    queryFacilityLock(String facility, String password, int serviceClass,
            Message response, int phoneId) {

        //[Example]Testing fake-result by mtk01411 2013-0904
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_GET_CB, response);
        rr.mp.writeString(facility);
        rr.mp.writeInt(serviceClass);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Set Call Forwarding for the specific phoneId.
     * @param action CommandsInterface Call Forwarding action
     * @param cfReason CommandsInterface Call Forwarding reason
     * @param serviceClass service class for Call Barring
     * @param number forwarded-to number
     * @param timeSeconds no-reply time
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    setCallForward(int action, int cfReason, int serviceClass,
            String number, int timeSeconds, Message response, int phoneId) {
        Rlog.d(LOG_TAG, "number: " + ((!SENLOG) ? number : "[hidden]"));
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_CF, response);
        rr.mp.writeInt(action);
        rr.mp.writeInt(cfReason);
        rr.mp.writeInt(serviceClass);
        rr.mp.writeString(number);
        rr.mp.writeInt(timeSeconds);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get Call Forwarding status for the specific phoneId.
     * @param cfReason CommandsInterface Call Forwarding reason
     * @param serviceClass service class for Call Barring
     * @param number forwarded-to number
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    queryCallForwardStatus(int cfReason, int serviceClass,
            String number, Message response, int phoneId) {
        //[Example]Testing fake-result by mtk01411 2013-0904
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_GET_CF, response);

        rr.mp.writeInt(2); // cfAction filed: 2 is for query action, not in used anyway
                           // (See ril.h RIL_CallForwardInfo: 2 = interrogate)
        rr.mp.writeInt(cfReason);
        rr.mp.writeInt(serviceClass);
        //Only through GsmMMICode:It will carry the dialNumber
        if (number != null) {
            rr.mp.writeString(number);
        } else {
            rr.mp.writeString("");
        }
        rr.mp.writeInt(phoneId);

        send(rr);

    }

    /// For OP01 UT @{
    /**
     * Set Call Forwarding with timeSolt for the specific phoneId.
     * @param action CommandsInterface Call Forwarding action
     * @param cfReason CommandsInterface Call Forwarding reason
     * @param serviceClass service class for Call Barring
     * @param number forwarded-to number
     * @param timeSeconds no-reply time
     * @param timeSlot time slot for CFU
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    setCallForwardInTimeSlot(int action, int cfReason, int serviceClass,
                String number, int timeSeconds, long[] timeSlot, Message response, int phoneId) {
        // target should be a SIP URI (IETF RFC 3261 [6]) or TEL URL (IETF RFC 3966 [7])
        if (number != null && !number.startsWith("sip:") && !number.startsWith("sips:")
                && !number.startsWith("tel:")) {
            number = "tel:" + number;
        }
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_SET_CF_TIME_SLOT, response);
        rr.mp.writeInt(action);
        rr.mp.writeInt(cfReason);
        rr.mp.writeInt(serviceClass);
        rr.mp.writeString(number);
        rr.mp.writeInt(timeSeconds);
        rr.mp.writeLongArray(timeSlot);
        rr.mp.writeInt(phoneId);
        send(rr);
    }

    /**
     * Get Call Forwarding with timeSolt for the specific phoneId.
     * @param cfReason CommandsInterface Call Forwarding reason
     * @param serviceClass service class for Call Barring
     * @param response Message callback
     * @param phoneId the phone index
     */
    public void
    queryCallForwardInTimeSlotStatus(int cfReason,
            int serviceClass, Message response, int phoneId) {
        mSSConfig = SuppSrvConfig.getInstance(mContext);
        MMTelSSRequest rr = MMTelSSRequest.obtain(MMTELSS_REQ_GET_CF_TIME_SLOT, response);
        rr.mp.writeInt(2); // cfAction filed: 2 is for query action, not in used anyway
                           // (See ril.h RIL_CallForwardInfo: 2 = interrogate)
        rr.mp.writeInt(cfReason);
        rr.mp.writeInt(serviceClass);
        rr.mp.writeInt(phoneId);
        send(rr);
    }
    /// @}

    private void
    acquireWakeLock() {
        Rlog.d(LOG_TAG, "=>wakeLock() "
                + "mRequestMessagesPending = " + mRequestMessagesPending
                + ", mRequestsList.size() = " + mRequestsList.size());
        synchronized (mWakeLock) {
            mWakeLock.acquire();
            mRequestMessagesPending++;

            mSender.removeMessages(EVENT_WAKE_LOCK_TIMEOUT);
            Message msg = mSender.obtainMessage(EVENT_WAKE_LOCK_TIMEOUT);
            mSender.sendMessageDelayed(msg, mWakeLockTimeout);
        }
    }

    private void
    releaseWakeLockIfDone() {
        Rlog.d(LOG_TAG, "wakeLock()=> "
                + "mRequestMessagesPending = " + mRequestMessagesPending
                + ", mRequestsList.size() = " + mRequestsList.size());
        synchronized (mWakeLock) {
            if (mWakeLock.isHeld() &&
                    (mRequestMessagesPending == 0) &&
                    //MTK-START [mtk04070][111121][ALPS00093395]MTK modified
                    (mRequestsList.size() == 0)) {
                //MTK-END [mtk04070][111121][ALPS00093395]MTK modified
                mSender.removeMessages(EVENT_WAKE_LOCK_TIMEOUT);
                mWakeLock.release();
            }
        }
    }

    private MMTelSSRequest findAndRemoveRequestFromList(int serial) {
        synchronized (mRequestsList) {
            for (int i = 0, s = mRequestsList.size() ; i < s ; i++) {
                MMTelSSRequest rr = mRequestsList.get(i);

                if (rr.mSerial == serial) {
                    mRequestsList.remove(i);
                    if (mRequestMessagesWaiting > 0)
                        mRequestMessagesWaiting--;
                    return rr;
                }
            }
        }

        return null;
    }

    static String
    requestToString(int request) {
        switch (request) {
            case MMTELSS_REQ_SET_CLIR: return "SET_CLIR";
            case MMTELSS_REQ_GET_CLIR: return "GET_CLIR";
            case MMTELSS_REQ_GET_CLIP: return "GET_CLIP";
            case MMTELSS_REQ_SET_CLIP: return "SET_CLIP";
            case MMTELSS_REQ_GET_COLP: return "GET_COLP";
            case MMTELSS_REQ_SET_COLP: return "SET_COLP";
            case MMTELSS_REQ_GET_COLR: return "GET_COLR";
            case MMTELSS_REQ_SET_COLR: return "SET_COLR";
            case MMTELSS_REQ_SET_CW: return "SET_CW";
            case MMTELSS_REQ_GET_CW: return "GET_CW";
            case MMTELSS_REQ_SET_CB: return "SET_CB";
            case MMTELSS_REQ_GET_CB: return "GET_CB";
            case MMTELSS_REQ_SET_CF: return "SET_CF";
            case MMTELSS_REQ_GET_CF: return "GET_CF";
            /// For OP01 UT @{
            case MMTELSS_REQ_SET_CF_TIME_SLOT: return "SET_CF_TIME_SLOT";
            case MMTELSS_REQ_GET_CF_TIME_SLOT: return "GET_CF_TIME_SLOT";
            /// @}
            default: return "UNKNOWN MMTELSS REQ";
        }

    }

    private void
    send(MMTelSSRequest rr) {
        Message msg;
        msg = mSender.obtainMessage(EVENT_SEND, rr);
        acquireWakeLock();
        msg.sendToTarget();

    }
}
