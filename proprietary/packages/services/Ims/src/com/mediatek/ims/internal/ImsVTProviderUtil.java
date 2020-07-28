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

package com.mediatek.ims.internal;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.PatternMatcher;
import android.os.SystemProperties;
import android.telecom.VideoProfile.CameraCapabilities;
import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.view.Surface;

// for sync operation of Util to a individual thread (not in call)
import android.os.HandlerThread;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ConditionVariable;
import com.android.internal.os.SomeArgs;

// for VilTE switch change
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.TelephonyManager;
import com.android.ims.ImsConfig;
import com.android.ims.ImsConfigListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

// for roaming check
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;

// for SIM state check
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import android.hardware.camera2.CameraCharacteristics;
import android.telecom.VideoProfile;

// basic class
import java.util.StringTokenizer;
import java.util.Map;
import android.util.Log;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.HashMap;
import java.util.List;
import java.util.StringTokenizer;
import java.util.HashSet;
import java.lang.Integer;
import java.lang.Long;

import com.mediatek.ims.internal.ImsVTProvider;
import com.mediatek.ims.internal.VTSource;
import com.mediatek.ims.internal.ImsVTUsageManager;
import com.mediatek.ims.internal.ImsVTUsageManager.ImsVTUsage;

// for External component
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ExtensionPluginFactory;
import com.mediatek.ims.plugin.OemPluginFactory;
import com.mediatek.ims.plugin.ImsCallOemPlugin;
import com.mediatek.ims.plugin.impl.ImsSelfActivatorBase;
import com.mediatek.ims.plugin.impl.ImsCallPluginBase;
import com.mediatek.ims.plugin.ImsSelfActivator;
import com.mediatek.ims.plugin.ImsCallPlugin;

import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.ImsService;

public class ImsVTProviderUtil {

    private static final String TAG = "ImsVT Util";
    private static final String VILTE_SUPPORT = "persist.vendor.vilte_support";
    private static final String VIWIFI_SUPPORT = "persist.vendor.viwifi_support";
    private static final String PROPERTY_VILTE_ENABLE = "persist.vendor.mtk.vilte.enable";
    private static final String PROPERTY_VIWIFI_ENABLE = "persist.vendor.mtk.viwifi.enable";
    public static final String PROPERTY_RIL_ICCID_SIM = "vendor.ril.iccid.sim";
    public static final String PROPERTY_MAX_DRAM_SIZE = "ro.vendor.mtk_config_max_dram_size";
    private static final String PROPERTY_NO_CAMERA_MODE = "persist.vendor.vt.no_camera_mode";
    public static final String PROPERTY_TEL_LOG = "persist.log.tag.tel_dbg";

    private static final int SIM_NUM = TelephonyManager.getDefault().getPhoneCount();

    private static final int MSG_INIT_REFVTP            = 1;
    private static final int MSG_BIND                   = 2;
    private static final int MSG_SETUIMODE              = 3;
    private static final int MSG_SWITCH_FEATURE         = 4;
    private static final int MSG_SWITCH_ROAMING         = 5;
    private static final int MSG_RESET_WRAPPER          = 6;
    private static final int MSG_REINIT_REFVTP          = 7;

    public static final int HIDE_ME_TYPE_NONE           = 0;
    public static final int HIDE_ME_TYPE_DISABLE        = 1;
    public static final int HIDE_ME_TYPE_FREEZE         = 2;
    public static final int HIDE_ME_TYPE_PICTURE        = 3;

    public static final int HIDE_YOU_TYPE_DISABLE       = 0;
    public static final int HIDE_YOU_TYPE_ENABLE        = 1;

    public static final int TURN_OFF_CAMERA             = -1;

    // Only used to differentiate between do session modify and set Fg/Bg.
    public static final int UI_MODE_UNCHANGED           = -1;

    public static final int UI_MODE_FG                  = 0;
    public static final int UI_MODE_BG                  = 1;
    public static final int UI_MODE_FULL_SCREEN         = 2;
    public static final int UI_MODE_NORMAL_SCREEN       = 3;
    public static final int UI_MODE_RESET               = 4;

    // for notify call is end
    // IMCB sometimes won't notify (in some error case)
    public static final int UI_MODE_DESTROY             = 65536;

    public static final int TAG_VILTE_MOBILE            = 0xFF000000;
    public static final int TAG_VILTE_WIFI              = 0xFF100000;

    public static final int VT_SIM_ID_ABSENT            = -1;

    public static final int CALL_RAT_LTE                = 0;
    public static final int CALL_RAT_WIFI               = 1;
    public static final int CALL_RAT_NR                 = 2;

    // for ImsConfig
    private final static String EXTRA_PHONE_ID = "phone_id";
    public static boolean sIsNoCameraMode =
            (SystemProperties.getInt(PROPERTY_NO_CAMERA_MODE, 0) == 1);

    public static class Size {
        /**
         * Sets the dimensions for pictures.
         *
         * @param w the photo width (pixels)
         * @param h the photo height (pixels)
         */
        public Size(int w, int h) {
            width = w;
            height = h;
        }

        /**
         * Compares {@code obj} to this size.
         *
         * @param obj the object to compare this size with.
         * @return {@code true} if the width and height of {@code obj} is the
         *         same as those of this size. {@code false} otherwise.
         */
        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof Size)) {
                return false;
            }
            Size s = (Size) obj;
            return width == s.width && height == s.height;
        }

        @Override
        public int hashCode() {
            return width * 32713 + height;
        }

        /** width of the picture */
        public int width;
        /** height of the picture */
        public int height;
    };

    public class FeatureValueReceiver extends BroadcastReceiver {

        private ImsVTProviderUtil   mOwner;
        private ArrayList<Boolean>  mViLTEValue = new ArrayList<Boolean>();
        private ArrayList<Boolean>  mViWifiValue = new ArrayList<Boolean>();

        public FeatureValueReceiver() {

            for (int i = 0; i < ImsVTProviderUtil.SIM_NUM; i++) {

                int propValueBit = 0;

                if (ImsCommonUtil.supportMims()) {
                    propValueBit = i;
                }

                boolean enable = (SystemProperties.getInt(PROPERTY_VILTE_ENABLE, 0) & (1 << propValueBit)) > 0;
                Log.d(TAG, "Getprop [" + PROPERTY_VILTE_ENABLE + "][" + i + "]=" + enable);
                mViLTEValue.add(enable);

                enable = (SystemProperties.getInt(PROPERTY_VIWIFI_ENABLE, 0) & (1 << propValueBit)) > 0;
                Log.d(TAG, "Getprop [" + PROPERTY_VIWIFI_ENABLE + "][" + i + "]=" + enable);
                mViWifiValue.add(enable);
            }
        }

        public void setOwner(ImsVTProviderUtil owner) {
            mOwner = owner;
        }

        public boolean getInitViLTEValue(int phondId) {

            return mViLTEValue.get(phondId).booleanValue();
        }

        public boolean getInitViWifiValue(int phondId) {

            return mViWifiValue.get(phondId).booleanValue();
        }

        @Override
        public void onReceive(Context context, Intent intent) {

            if (intent == null || intent.getAction() == null) {
                return;
            }

            if (intent.getAction().equals(ImsConfig.ACTION_IMS_FEATURE_CHANGED)) {

                int feature = intent.getIntExtra(ImsConfig.EXTRA_CHANGED_ITEM, -1);
                int phoneId = intent.getIntExtra(EXTRA_PHONE_ID, -1);
                int status  = intent.getIntExtra(ImsConfig.EXTRA_NEW_VALUE, -1);

                if (phoneId < 0) {
                    Log.d(TAG, "ignore it for invalid SIM id");
                    return;
                }

                if (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE) {
                    Log.d(TAG, "onRecevied feature changed phoneId: " + phoneId +
                            ", feature: " + feature + ", status: " + status);

                    if(status == ImsConfig.FeatureValueConstants.OFF) {
                        mViLTEValue.set(phoneId, Boolean.FALSE);
                    } else if (status == ImsConfig.FeatureValueConstants.ON) {
                        mViLTEValue.set(phoneId, Boolean.TRUE);
                    }
                    mOwner.switchFeature(phoneId, TAG_VILTE_MOBILE, mViLTEValue.get(phoneId).booleanValue());

                } else if (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI) {
                    Log.d(TAG, "onRecevied feature changed phoneId: " + phoneId +
                            ", feature: " + feature + ", status: " + status);

                    if(status == ImsConfig.FeatureValueConstants.OFF) {
                        mViWifiValue.set(phoneId, Boolean.FALSE);
                    } else if (status == ImsConfig.FeatureValueConstants.ON) {
                        mViWifiValue.set(phoneId, Boolean.TRUE);
                    }
                    mOwner.switchFeature(phoneId, TAG_VILTE_WIFI, mViWifiValue.get(phoneId).booleanValue());
                }
            }
        }
    };

    public class VTPhoneStateListener extends PhoneStateListener {

        private ImsVTProviderUtil   mOwner;
        private Context             mContext;

        public VTPhoneStateListener() {
            super();
        }

        public void setOwner(ImsVTProviderUtil owner) {
            mOwner = owner;
        }

        public void setContext(Context context) {
            mContext = context;
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {

            if (serviceState == null) {
                Log.d(TAG, "[onServiceStateChanged] " + this.mSubId + ": serviceState is null");
                return;
            }

            int phondId = SubscriptionManager.getPhoneId(this.mSubId);

            if (!SubscriptionManager.isValidPhoneId(phondId)) {
                Log.d(TAG, "[onServiceStateChanged] " + ": phondId is invalid");
                return;
            }

            boolean isDataRoaming = serviceState.getDataRoaming();
            boolean isVoiceRoaming = serviceState.getVoiceRoaming();

            //switchRoaming(phondId, isDataRoaming);

            // update the ViLTE config (On/Off) depend on roaming state
            if (TelephonyManager.getDefault().getSimState() == TelephonyManager.SIM_STATE_READY) {
                //ImsManager.updateImsServiceConfig(mContext, phondId, true);
            }
        }

    };


    private class SimStateReceiver extends BroadcastReceiver {

        private ImsVTProviderUtil mOwner;

        public void setOwner(ImsVTProviderUtil owner) {
            mOwner = owner;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                    SubscriptionManager.INVALID_SIM_SLOT_INDEX);
            int simState = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                                    TelephonyManager.SIM_STATE_UNKNOWN);

            if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
                    return;
            }

            if (action.equals(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED)) {
                Log.d(TAG, "Received ACTION_SIM_CARD_STATE_CHANGED, slotId:" + slotId
                     + ", simState:" + simState);

                setSimCardState(slotId, simState);

            // We didn't register ACTION_SIM_APPLICATION_STATE_CHANGED currently.
            // Just for future use.
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                Log.d(TAG, "Received ACTION_SIM_APPLICATION_STATE_CHANGED, slotId:" + slotId
                     + ", simState:" + simState);

                setSimAppState(slotId, simState);
            }
            //

            if(isSimStateStable(slotId)) {
                mSimReadyVariable[slotId].open();
            }
        }
    }

    private class NetworkAvailableCallback extends NetworkCallback {
        @Override
        public void onAvailable(Network network) {
            Log.d(TAG, "NetworkAvailableCallback.onAvailable: network=" + network);

            if(network != null) {
                LinkProperties linkProp = mConnectivityManager.getLinkProperties(network);

                if (linkProp != null) {
                    String ifName = linkProp.getInterfaceName();
                    Log.d(TAG, "NetworkAvailableCallback.onAvailable: (network_id, if_name) = ("
                            + network.netId + ", " + ifName + ")");

                    if (isVideoCallOnByPlatform()) {
                        ImsVTProvider.nUpdateNetworkTable(true, network.netId, ifName);
                    }

                } else {
                    Log.w(TAG, "NetworkAvailableCallback.onAvailable: linkProp = null");
                }

            } else {
                Log.w(TAG, "NetworkAvailableCallback.onAvailable: network = null");
            }
        }

        @Override
        public void onLost(Network network) {
           Log.d(TAG, "NetworkAvailableCallback.onLost: network=" + network);

           if(network != null) {

               if (isVideoCallOnByPlatform()) {
                   ImsVTProvider.nUpdateNetworkTable(false, network.netId, null);
               }

           } else {
               Log.w(TAG, "NetworkAvailableCallback.onLost: network = null");
           }
        }
    }

    public class ImsVTMessagePacker {

        public String packFromVdoProfile(VideoProfile videoProfile) {

            StringBuilder flattened = new StringBuilder();

            flattened.append("mVideoState");
            flattened.append("=");
            flattened.append("" + videoProfile.getVideoState());
            flattened.append(";");
            flattened.append("mQuality");
            flattened.append("=");
            flattened.append("" + videoProfile.getQuality());
            flattened.append(";");

            // chop off the extra semicolon at the end
            flattened.deleteCharAt(flattened.length() - 1);

            Log.d(TAG, "[packFromVdoProfile] profile = " + flattened.toString());

            return flattened.toString();
        }

        public VideoProfile unPackToVdoProfile(String flattened) {

            Log.d(TAG, "[unPackToVdoProfile] flattened = " + flattened);

            StringTokenizer tokenizer = new StringTokenizer(flattened, ";");
            int state = VideoProfile.STATE_BIDIRECTIONAL;
            int qty = VideoProfile.QUALITY_DEFAULT;

            while (tokenizer.hasMoreElements()) {
                String kv = tokenizer.nextToken();
                int pos = kv.indexOf('=');
                if (pos == -1) {
                    continue;
                }
                String k = kv.substring(0, pos);
                String v = kv.substring(pos + 1);

                Log.d(TAG, "[unPackToVdoProfile] k = " + k + ", v = " + v);

                if (k.equals("mVideoState")) {
                    state = Integer.valueOf(v).intValue();
                } else if (k.equals("mQuality")) {
                    qty = Integer.valueOf(v).intValue();
                }
            }
            Log.d(TAG, "[unPackToVdoProfile] state = " + state + ", qty = " + qty);
            return new VideoProfile(state, qty);
        }

    }

    private ImsVTMessagePacker                      mPacker = new ImsVTMessagePacker();
    private FeatureValueReceiver                    mFeatureValueReceiver;
    private SimStateReceiver                        mSimStateReceiver;
    private NetworkAvailableCallback                mNetworkAvailableCallback;

    private Map<String, Object>                     mProviderById = new ConcurrentHashMap<>();
    private Map<String, Object>                     mDataUsageById = new HashMap<>();

    private int[]                                   mSimCardState = new int[SIM_NUM];
    private int[]                                   mSimAppState = new int[SIM_NUM];
    private ConditionVariable[]                     mSimReadyVariable = new ConditionVariable[SIM_NUM];

    public  Context                                 mContext;

    protected HandlerThread                         mProviderHandlerThread;
    private Handler                                 mProviderHandler;

    private TelephonyManager                        mTelephonyManager;
    private ConnectivityManager                     mConnectivityManager;

    private SubscriptionManager                     mSubscriptionManager;
    private Map<Integer, PhoneStateListener>        mPhoneServicesStateListeners = new ConcurrentHashMap<>();
    private SubscriptionManager.OnSubscriptionsChangedListener mSubscriptionsChangedlistener =
        new SubscriptionManager.OnSubscriptionsChangedListener() {

        @Override
        public void onSubscriptionsChanged() {
            updateServiceStateListeners();
        }
    };

    // Singleton instance
    private static ImsVTProviderUtil mInstance = getInstance();

    private ImsVTProviderUtil() {

        mProviderHandlerThread = new HandlerThread("ProviderHandlerThread");
        mProviderHandlerThread.start();

        mProviderHandler = new Handler(mProviderHandlerThread.getLooper()) {

            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_INIT_REFVTP: {
                        setContextAndInitRefVTPInternal((Context) msg.obj);
                        break;
                    }
                    case MSG_BIND: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            ImsVTProvider vtp = (ImsVTProvider) args.arg1;
                            int cid = (int) args.arg2;
                            int pid = (int) args.arg3;

                            bindInternal(vtp, cid, pid);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_SETUIMODE: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            ImsVTProvider vtp = (ImsVTProvider) args.arg1;
                            int mode = (int) args.arg2;

                            setUIModeInternal(vtp, mode);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_SWITCH_FEATURE: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            int phoneId = (int) args.arg1;
                            int feature = (int) args.arg2;
                            boolean isOn = (boolean) args.arg3;

                            switchFeatureInternal(phoneId, feature, isOn);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_SWITCH_ROAMING:{
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            int phoneId = (int) args.arg1;
                            boolean isRoaming = (boolean) args.arg2;

                            switchRoamingInternal(phoneId, isRoaming);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_RESET_WRAPPER: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            ImsVTProvider vtp = (ImsVTProvider) args.arg1;

                            resetWrapperInternal(vtp);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_REINIT_REFVTP: {
                        if (isVideoCallOnByPlatform()) {
                            Log.d(TAG, "reInitRefVTP, ViLTE on, do natvie ReInit");
                            ImsVTProvider.nInitRefVTP();
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        };

        //because use in constructor, we need create it here firstly
        mFeatureValueReceiver = new FeatureValueReceiver();
        mFeatureValueReceiver.setOwner(this);

        for (int i = 0; i < SIM_NUM; i++) {
            mSimCardState[i] = getImsExtCallUtil().getSimCardState(i);
            mSimAppState[i] = getImsExtCallUtil().getSimApplicationState(i);
            mSimReadyVariable[i] = new ConditionVariable();
        }

        mSimStateReceiver = new SimStateReceiver();
        mSimStateReceiver.setOwner(this);

        mNetworkAvailableCallback = new NetworkAvailableCallback();
    }

    public static ImsVTProviderUtil getInstance() {
        if (mInstance == null) {
            synchronized(ImsVTProviderUtil.class){
                mInstance = new ImsVTProviderUtil();
            }
        }
        return mInstance;
    }

    public ImsCallPluginBase getImsExtCallUtil() {

        ExtensionPluginFactory facotry = ExtensionFactory.makeExtensionPluginFactory(mContext);
        return facotry.makeImsCallPlugin(mContext);
    }

    public ImsCallOemPlugin getImsOemCallUtil() {

        OemPluginFactory facotry = ExtensionFactory.makeOemPluginFactory(mContext);
        return facotry.makeImsCallPlugin(mContext);
    }

    private void updateServiceStateListeners() {
        if (mSubscriptionManager == null) {
            Log.d(TAG, "[updateServiceStateListeners] Unexpected error, mSubscriptionManager=null");
            return;
        }

        if (mTelephonyManager == null) {
            Log.d(TAG, "[updateServiceStateListeners] Unexpected error, mTelephonyManager=null");
            return;
        }

        HashSet<Integer> unUsedSubscriptions = new HashSet<Integer>(mPhoneServicesStateListeners.keySet());
        final List<SubscriptionInfo> slist = mSubscriptionManager.getActiveSubscriptionInfoList();

        if (slist != null) {

            for (SubscriptionInfo subInfoRecord : slist) {

                int subId = subInfoRecord.getSubscriptionId();

                if (mPhoneServicesStateListeners.get(Integer.valueOf(subId)) == null) {

                    // Create listeners for new subscriptions.
                    Log.d(TAG, "[updateServiceStateListeners] create ServicesStateListener for " + subId);

                    VTPhoneStateListener listener = new VTPhoneStateListener();

                    mTelephonyManager.listen(listener, PhoneStateListener.LISTEN_SERVICE_STATE);
                    mPhoneServicesStateListeners.put(Integer.valueOf(subId), listener);

                } else {

                    // this is still a valid subscription.
                    //Log.d(TAG, "[updateServiceStateListeners] ServicesStateListener-" + subId + " is used.");
                    unUsedSubscriptions.remove(Integer.valueOf(subId));
                }
            }
        }

        for (Integer key: unUsedSubscriptions) {
            Log.d(TAG, "[updateServiceStateListeners] remove unused ServicesStateListener for " + key);
            mTelephonyManager.listen(mPhoneServicesStateListeners.get(key), 0);
            mPhoneServicesStateListeners.remove(key);
        }

    }

    public String packFromVdoProfile(VideoProfile VideoProfile) {
        return mPacker.packFromVdoProfile(VideoProfile);
    }

    public VideoProfile unPackToVdoProfile(String flattened) {
        return mPacker.unPackToVdoProfile(flattened);
    }

    public void usageSet(int Id, ImsVTUsage usage) {
        Log.d(TAG, "[usageSet][id =" + Id + "]" + usage.toString());
        mDataUsageById.put("" + Id, new ImsVTUsage("Record", usage));
    }

    public ImsVTUsage usageGet(int Id) {
        ImsVTUsage usage = (ImsVTUsage) mDataUsageById.get("" + Id);

        if (usage == null) {
            return new ImsVTUsage("Dummy");
        } else {
            Log.d(TAG, "[usageGet][id =" + Id + "]" + usage.toString());
            return usage;
        }
    }

    public void recordAdd(int Id, ImsVTProvider p) {
        Log.d(TAG, "recordAdd id = " + Id + ", size = " + recordSize());
        mProviderById.put("" + Id, p);
        return;
    }

    public void recordRemove(int Id) {
        Log.d(TAG, "recordRemove id = " + Id + ", size = " + recordSize());
        mProviderById.remove("" + Id);
        return;
    }

    public void recordRemoveAll() {
        Log.d(TAG, "recordRemoveAll, size = " + recordSize());
        mProviderById.clear();
        return;
    }

    public ImsVTProvider recordGet(int Id) {
        if (isTelephonyLogEnable()) {
            Log.d(TAG, "recordGet id = " + Id + ", size = " + recordSize());
        }
        return (ImsVTProvider) mProviderById.get("" + Id);
    }

    public int recordPopId() {

        if (mProviderById.size() != 0) {
            for (Object p : mProviderById.values()) {
                return ((ImsVTProvider) p).getId();
            }
        }
        return ImsVTProvider.VT_PROVIDER_INVALIDE_ID;
    }

    public boolean recordContain(int Id) {
        return mProviderById.containsKey(Id);
    }

    public int recordSize() {
        return mProviderById.size();
    }

    public synchronized void setSimCardState(int simId, int state) {
        if (0 <= simId && simId < SIM_NUM) {
            mSimCardState[simId] = state;
        }
    }

    public synchronized int getSimCardState(int simId) {
        if (0 <= simId && simId < SIM_NUM) {
            return mSimCardState[simId];
        }

        return TelephonyManager.SIM_STATE_ABSENT;
    }

    public synchronized void setSimAppState(int simId, int state) {
        if (0 <= simId && simId < SIM_NUM) {
            mSimAppState[simId] = state;
        }
    }

    public synchronized int getSimAppState(int simId) {
        if (0 <= simId && simId < SIM_NUM) {
            return mSimAppState[simId];
        }

        return TelephonyManager.SIM_STATE_ABSENT;
    }

    /**
        * IccCardConstants.INTENT_VALUE_ICC_ABSENT
        * IccCardConstants.INTENT_VALUE_ICC_LOCKED
        * IccCardConstants.INTENT_VALUE_ICC_READY
        * IccCardConstants.INTENT_VALUE_ICC_LOADED
        */
    public boolean isSimStateStable(int simId) {
        int cardState = getSimCardState(simId);
        // int appState = getSimAppState(simId);

        return cardState == TelephonyManager.SIM_STATE_ABSENT ||
                cardState == TelephonyManager.SIM_STATE_PRESENT;
                // && appState != TelephonyManager.SIM_STATE_NOT_READY);
    }

    public void waitSimStateStable(int simId) {
        if (!isSimStateStable(simId)) {
            Log.d(TAG, "waitSimStateStable, simId = " + simId);

            mSimReadyVariable[simId].close();
            mSimReadyVariable[simId].block();
        }
    }

    public void quitAllThread() {
        if (mProviderById.size() != 0) {
            for (Object p : mProviderById.values()) {
                Log.d(TAG, "quitThread, id = " + ((ImsVTProvider) p).getId());
                ((ImsVTProvider) p).quitThread();
            }
        }
    }

    public void updateCameraUsage(int Id) {
        Log.d(TAG, "updateCameraUsage");
        if (mProviderById.size() != 0) {
            for (Object p : mProviderById.values()) {
                //Only release other call's camera
                if(((ImsVTProvider) p).getId() != Id) {
                    ((ImsVTProvider) p).setCameraInternal(null);
                }
            }
        }
    }

    public void releaseVTSourceAll(){
        if (mProviderById.size() != 0) {
            for (Object p : mProviderById.values()) {
                Log.d(TAG, "releaseVTSourceAll, id = " + ((ImsVTProvider) p).getId());
                ((ImsVTProvider) p).mSource.release();
            }
        }
    }

    public boolean isVideoCallOn(int phoneId) {
        return mFeatureValueReceiver.getInitViLTEValue(phoneId);
    }

    public boolean isViWifiOn(int phoneId) {
        return mFeatureValueReceiver.getInitViWifiValue(phoneId);
    }

    public static boolean isVideoCallOnByPlatform() {
        return (SystemProperties.get(VILTE_SUPPORT, "0").equals("1") ||
                SystemProperties.get(VIWIFI_SUPPORT, "0").equals("1"));
    }

    public static boolean is512mbProject() {
        // Ex: 512MB: 0x20000000, 1G: 0x40000000, ...
        return SystemProperties.get(PROPERTY_MAX_DRAM_SIZE, "0x40000000").equals("0x20000000");
    }

    public static boolean isVideoQualityTestMode() {
        //assume it is VQ test if lab op is set to op01 or op09
        int labOp = SystemProperties.getInt("persist.vendor.vt.lab_op_code", 0);
        return (labOp == 1 || labOp == 9);
    }

    public static boolean isCameraAvailable() {
        VTSource.Resolution [] cams_info;
        cams_info = VTSource.getAllCameraResolutions();

        return (null != cams_info)? true: false;
    }

    public static boolean isTelephonyLogEnable() {
        return SystemProperties.get(PROPERTY_TEL_LOG, "0").equals("1");
    }

    public void setContextAndInitRefVTP(Context context) {
        mProviderHandler.obtainMessage(MSG_INIT_REFVTP, context).sendToTarget();
    }

    public void reInitRefVTP() {
        mProviderHandler.obtainMessage(MSG_REINIT_REFVTP).sendToTarget();
    }

    public void bind(ImsVTProvider p, int CallId, int PhoneId) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = p;
        args.arg2 = CallId;
        args.arg3 = PhoneId;
        mProviderHandler.obtainMessage(MSG_BIND, args).sendToTarget();
    }

    public void setUIMode(ImsVTProvider p, int mode) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = p;
        args.arg2 = mode;
        mProviderHandler.obtainMessage(MSG_SETUIMODE, args).sendToTarget();
    }

    public void switchFeature(int phoneId, int feature, boolean isOn) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = phoneId;
        args.arg2 = feature;
        args.arg3 = isOn;
        mProviderHandler.obtainMessage(MSG_SWITCH_FEATURE, args).sendToTarget();
    }

    public void switchRoaming(int phoneId, boolean isRoaming) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = phoneId;
        args.arg2 = isRoaming;
        mProviderHandler.obtainMessage(MSG_SWITCH_ROAMING, args).sendToTarget();
    }

    public void resetWrapper(ImsVTProvider p) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = p;
        mProviderHandler.obtainMessage(MSG_RESET_WRAPPER, args).sendToTarget();
    }

    public void setContextAndInitRefVTPInternal(Context context) {
        Log.d(TAG, "setContextAndInitRefVTPInternal(), context =" + context);

        mContext = context;

        if (sIsNoCameraMode) {
            VTDummySource.setContext(context);
        } else {
            VTSource.setContext(context);
        }
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        if (mFeatureValueReceiver != null) {
            Log.d(TAG, "setContextAndInitRefVTP, register FeatureValueReceiver");
            IntentFilter filter = new IntentFilter();
            filter.addAction(ImsConfig.ACTION_IMS_FEATURE_CHANGED);
            mContext.registerReceiver(mFeatureValueReceiver, filter);
        }

        if (mSimStateReceiver != null) {
            Log.d(TAG, "setContextAndInitRefVTP, register SimStateReceiver");

            IntentFilter intentFilter = new IntentFilter(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);
            // intentFilter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
            mContext.registerReceiver(mSimStateReceiver, intentFilter);
        }

        mSubscriptionManager = SubscriptionManager.from(mContext);
        mSubscriptionManager.addOnSubscriptionsChangedListener(mSubscriptionsChangedlistener);

        registerNetworkRequestWithCallback(NetworkCapabilities.NET_CAPABILITY_IMS);
        registerNetworkRequestWithCallback(NetworkCapabilities.NET_CAPABILITY_EIMS);

        if (isVideoCallOnByPlatform()) {
            Log.d(TAG, "setContextAndInitRefVTPInternal(), ViLTE on, do natvie init");
            ImsVTProvider.nInitRefVTP();
        }
    }

    public void registerNetworkRequestWithCallback(int cap) {
        NetworkRequest.Builder builder = new NetworkRequest.Builder();
        builder.addCapability(cap);
        NetworkRequest networkRequest = builder.build();

        Log.d(TAG, "registerNetworkRequestwithCallback(), networkRequest:" + networkRequest);

        if (mConnectivityManager == null) {
            mConnectivityManager =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        }
        mConnectivityManager.registerNetworkCallback(networkRequest, mNetworkAvailableCallback);
    }

    public void bindInternal(ImsVTProvider p, int CallId, int PhoneId) {

        Log.d(TAG, "bindInternal(), vtp = " + p + ", id = " + CallId + ", phone id = " + PhoneId);

        // In some case setUIMode destroy will come first, no need to bind and add record
        if (ImsVTProviderUtil.UI_MODE_DESTROY == p.mMode) {
            Log.d(TAG, "Ignore bind ImsVTProvider because UI_MODE_DESTROY");
            return;
        }

        if (p == null) {
            Log.d(TAG, "ImsVTProvider == null");
            return;
        }

        // Note that "PhoneId" means the call from which sim slot (0/1).
        // If not support dual IMS (IMS = 1), we can just assume PhoneId as 0 (same as IMCB simslot ID)
        // For the construct id, the first 2 bytes are phone Id, the last 2 bytes are call id
        int id = CallId;
        int ImsCount = 1;

        ImsCount = ImsService.getInstance(mContext).getModemMultiImsCount();
        if (ImsCount > 1) {
            id = (PhoneId << 16) | CallId;
        }

        if (p.getId() == ImsVTProvider.VT_PROVIDER_INVALIDE_ID) {

            // Check id if exist in map
            // The same id exist mean the last call with the same id
            // does not disconnected yet at native layer.
            int wait_time = 0;
            Log.d(TAG, "bind ImsVTProvider check if exist the same id");
            while (null != recordGet(id)) {
                Log.d(TAG, "bind ImsVTProvider the same id exist, wait ...");

                try {
                    Thread.sleep(400);
                } catch(InterruptedException ex) {
                }

                wait_time += 1;
                if (wait_time > 10) {
                    Log.d(TAG, "bind ImsVTProvider the same id exist, break!");
                    break;
                }
            }

            p.setId(id);
            p.setSimId(PhoneId);
            ImsVTUsage initUsage = new ImsVTUsage("Init", usageGet(id));
            p.mUsager.setInitUsage(initUsage);

            recordAdd(id, p);

            if (sIsNoCameraMode) {
                p.mSource = new VTDummySource();
            } else {
                p.mSource = new VTSource(VTSource.VT_SRV_CALL_4G, p.getId(), p);
            }
            p.mSource.setReplacePicture(ImsVTProvider.REPLACE_PICTURE_PATH);

            p.nInitialization(id, p.getSimId());
            p.mInitComplete = true;
        }
    }

    public void setUIModeInternal(ImsVTProvider p, int mode) {
        if (p != null) {
            p.onSetUIMode(mode);
        }
    }

    public void switchFeatureInternal(int phoneId, int feature, boolean isOn) {

        Log.d(TAG, "switchFeatureInternal, feature = " + feature + "isOn = " + isOn);

        if (mProviderById.size() != 0) {
            for (Object p : mProviderById.values()) {

                if (phoneId == ((ImsVTProvider) p).getSimId()) {

                    Log.d(TAG, "switchFeatureInternal, id = " + ((ImsVTProvider) p).getId());

                    ((ImsVTProvider) p).onSwitchFeature(feature, isOn);
                }
            }
        }
    }

    public void switchRoamingInternal(int phoneId, boolean isRoaming) {

        Log.d(TAG, "switchRoamingInternal, phoneId = " + phoneId + "isRoaming = " + isRoaming);

        if (mProviderById.size() != 0) {
            for (Object p : mProviderById.values()) {

                if (phoneId == ((ImsVTProvider) p).getSimId()) {

                    Log.d(TAG, "switchRoamingInternal, id = " + ((ImsVTProvider) p).getId());

                    ((ImsVTProvider) p).onSwitchRoaming(isRoaming);
                }
            }
        }
    }

    public void resetWrapperInternal(ImsVTProvider p) {
        if (p != null) {
            p.onResetWrapper();
        }
    }
}
