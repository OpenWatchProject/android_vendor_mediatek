package com.mediatek.presence.core.ims.service.presence.extension;

import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.core.ims.service.presence.PresenceService;
import com.mediatek.presence.core.ims.service.presence.extension.PresenceExtensionListener;
import com.mediatek.presence.utils.logger.Logger;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.telephony.ServiceState;
import android.net.Network;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;


public class ViLTEExtension extends PresenceExtension {

    public final String TAG = "ViLTEExtension";
    private Context mContext = AndroidFactory.getApplicationContext();
    private volatile boolean mVoiceSupported;
    private volatile boolean mVideoSupported;
    private volatile boolean mDuplexSupported;
    private int mCurrentNetworkType;
    private int mCurrentMobileNetworkClass;

    // Network Type
    private static final int NONE_NETWORK      = 0;
    private static final int GERAN_NETWORK     = 1; // 2g
    private static final int UTRAN_NETWORK     = 2; // 3g
    private static final int EUTRAN_NETWORK    = 3; // LTE
    private static final int WIFI_NETWORK      = 4; // WIFI

    private int mSlotId = 0;

    public ViLTEExtension(int slotId) {
        super(PresenceExtension.EXTENSION_VILTE);
        mSlotId = slotId;
        logger = Logger.getLogger(mSlotId, "ViLTEExtension");
        extensionName = TAG;
        mVoiceSupported = false;
        mVideoSupported = false;
        mDuplexSupported = false;
    }

    @Override
    public void attachExtension(PresenceExtensionListener presenceExtensionListener) {
       //intialize the value
        super.attachExtension(presenceExtensionListener);
        // TODO Auto-generated method stub
        logger.debug("attachExtension : " +extensionName);
        switch(getNetworkType(mSlotId)) {
            //2G NETWORK
            case TelephonyManager.NETWORK_TYPE_GPRS:   // 1
            case TelephonyManager.NETWORK_TYPE_EDGE:   // 2
            case TelephonyManager.NETWORK_TYPE_CDMA:   // 4
            case TelephonyManager.NETWORK_TYPE_1xRTT:  // 7
            case TelephonyManager.NETWORK_TYPE_IDEN:   // 11
                mCurrentNetworkType = GERAN_NETWORK;
                break;
             //3G NETWORK
            case TelephonyManager.NETWORK_TYPE_UMTS:   // 3
            case TelephonyManager.NETWORK_TYPE_EVDO_0: // 5
            case TelephonyManager.NETWORK_TYPE_EVDO_A: // 6
            case TelephonyManager.NETWORK_TYPE_HSDPA:  // 8
            case TelephonyManager.NETWORK_TYPE_HSUPA:  // 9
            case TelephonyManager.NETWORK_TYPE_HSPA:   // 10
            case TelephonyManager.NETWORK_TYPE_EVDO_B: // 12
            case TelephonyManager.NETWORK_TYPE_EHRPD:  // 14
            case TelephonyManager.NETWORK_TYPE_HSPAP:  // 15
                mCurrentNetworkType = UTRAN_NETWORK;
                break;
             //LTE NETWORK
            case TelephonyManager.NETWORK_TYPE_LTE:    // 13
                mCurrentNetworkType = EUTRAN_NETWORK;
                break;
            //WIFI NETWORK
            case TelephonyManager.NETWORK_TYPE_IWLAN:  // 18
                mCurrentNetworkType = WIFI_NETWORK;
                break;
            default:
                mCurrentNetworkType = NONE_NETWORK;
                break;
        }
        logger.debug("currentNetworkType : " + mCurrentNetworkType);

        //init value.
        mVoiceSupported = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .isIR92VoiceCallSupported();
        mVideoSupported = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .isIR94VideoCallSupported();
        mDuplexSupported = mVoiceSupported || mVideoSupported;

        //workaround for 3G video tag.
        if (mCurrentNetworkType >= 3) {
            logger.debug("set mVideoSupported = true");
            mVideoSupported = true;
        } else {
            logger.debug("set mVideoSupported = false");
            mVideoSupported = false;
        }

        logger.debug("init volte: " + mVoiceSupported + " vilte: "
                + mVideoSupported + " duplex: " + mDuplexSupported);
    }

    @Override
    public void detachExtension() {
        logger.debug("detachExtension : " +extensionName);
        // TODO Auto-generated method stub
        super.detachExtension();
    }

    public void handleNetworkChanged(int networkType) {
        logger.debug("handleNetworkChanged:" + networkType);

        if (mCurrentNetworkType == networkType)
            return;
        if (networkType <= 2) {
            //2/3G
            mVoiceSupported = false;
            mVideoSupported = false;
            mDuplexSupported = false;
        } else if (networkType >= 3) {
            //LTE/Wifi
            mVoiceSupported = true;
            mVideoSupported = true;
            mDuplexSupported = true;
        }
        mCurrentNetworkType = networkType;
    }

    public void handleCapabilityChanged(boolean enabled) {
        logger.debug("handleCapabilityChanged capabilities: " + enabled);
        mVideoSupported = enabled;
        mDuplexSupported = enabled;
    }

    public boolean isVoiceCallSupported() {
        logger.debug("isVoiceCallSupported:" + mVoiceSupported);
        return mVoiceSupported;
    }

    public boolean isVideoCallSupported() {
        logger.debug("isVideoCallSupported:" + mVideoSupported);
        return mVideoSupported;
    }

    public boolean isDuplexSupported() {
        logger.debug("isDuplexSupported:" + mDuplexSupported);
        return mDuplexSupported;
    }

    @Override
    public void notifyListener() {
        // TODO Auto-generated method stub
        super.notifyListener();
    }

    private int getNetworkType(int slotId) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds == null) {
            logger.debug("getNetworkType fail. subIds is null");
            return -1;
        }
        TelephonyManager tm = TelephonyManager.from(mContext).createForSubscriptionId(subIds[0]);
        return tm.getNetworkType();
    }

    private static int getMainCapabilityPhoneId() {
    int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        }
    return phoneId;
    }
}
