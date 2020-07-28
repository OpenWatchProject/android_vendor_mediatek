package com.mediatek.op20.systemui;

import android.content.Context;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.view.View;
import android.widget.ImageView;
import android.util.Log;

import com.mediatek.op20.systemui.R;
import com.mediatek.systemui.ext.DefaultSystemUIStatusBarExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import mediatek.telecom.MtkTelecomManager;

/**
 * M: Op20 implementation of Plug-in definition of Status bar.
 */
public class Op20SystemUIStatusBarExt extends DefaultSystemUIStatusBarExt {
    private static final String TAG = "Op20SystemUIStatusBarExt";
    private static final boolean DEBUG = true; //!isUserLoad();
    
    // Network type icons
    private final int NETWORK_TYPE_G  = R.drawable.stat_sys_network_type_g;
    //private final int NETWORK_TYPE_E = R.drawable.stat_sys_network_type_e;
    private final int NETWORK_TYPE_2G = R.drawable.stat_sys_network_type_2g;
    private final int NETWORK_TYPE_1X = R.drawable.stat_sys_network_type_1x;
    private final int NETWORK_TYPE_3G = R.drawable.stat_sys_network_type_3g;
    private final int NETWORK_TYPE_4G = R.drawable.stat_sys_network_type_4g;

    private boolean mInC2kCsCall = false;
    private boolean mInCsCall;
    private int mNetworkTypeIconId = 0;

    private Context mContext;

    /**
     * Constructs a new Op20SystemUIStatusBarExt instance with Context.
     * @param context The Context object
     */
    public Op20SystemUIStatusBarExt(Context context) {
        super(context);
        mContext = context;
    }


    @Override
    public int getNetworkTypeIcon(int subId, int iconId, int networkType,
                    ServiceState serviceState) {


        if (DEBUG) {
            Log.d(TAG, "getNetworkTypeIcon(), revise networkType,"
                + " subId=" + subId
                + ", oldType=" + networkType);
        }

        ///M: modify nw icon to 2G, because c2k cs call, md ps can not fall back. @{
        if (mInCsCall &&
                (serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE &&
                serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE &&
                serviceState.getVoiceNetworkType() == TelephonyManager.NETWORK_TYPE_1xRTT) 
                 && networkType != TelephonyManager.NETWORK_TYPE_IWLAN) {
            networkType = TelephonyManager.NETWORK_TYPE_1xRTT;
            Log.d(TAG, "getNetworkTypeIcon(), in c2k cs call, modify nw icon to 2G");
        }
        /// @}
        /// @}

        if (isSimOffline(subId, serviceState) ||
                !hasService(serviceState)) {
            mNetworkTypeIconId = 0;
        } else {
            if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                networkType = serviceState.getVoiceNetworkType();
                Log.d(TAG, "getNetworkTypeIcon unknown, read from voice = " + networkType);
            }
            mNetworkTypeIconId = getNetworkTypeIconId(networkType);
        }

        return mNetworkTypeIconId;
    }

    private int getNetworkTypeIconId(int networkType) {
        int iconId = 0;

        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                break;

            case TelephonyManager.NETWORK_TYPE_GSM:
            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_CDMA:
                iconId = NETWORK_TYPE_G;
                break;

            case TelephonyManager.NETWORK_TYPE_1xRTT:
                iconId = NETWORK_TYPE_1X;
                break;

            case TelephonyManager.NETWORK_TYPE_EDGE:
                iconId = NETWORK_TYPE_2G;
                break;

            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                iconId = NETWORK_TYPE_3G;
                break;

            case TelephonyManager.NETWORK_TYPE_LTE:
            case TelephonyManager.NETWORK_TYPE_LTE_CA:
                iconId = NETWORK_TYPE_4G;
                break;

            default:
                break;
        }

        return iconId;
    }

    private boolean isSimOffline(int subId, ServiceState serviceState) {
        boolean ret;

        boolean isService = hasService(serviceState);
        int slotId = SubscriptionManager.getSlotIndex(subId);

        if (slotId < 0) {
            ret = true;
        } else {
            ret = !isService;
        }
        return ret;
    }

    /*private boolean isSimInsert(int subId) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        boolean inserted = false;
        if (slotId >= 0) {
            inserted = getSimInserted(slotId);
        }
        return inserted;
    }*/

    private boolean hasService(ServiceState serviceState) {
        if (serviceState != null) {
            // Consider the device to be in service if either voice or data
            // service is available. Some SIM cards are marketed as data-only
            // and do not support voice service, and on these SIM cards, we
            // want to show signal bars for data service as well as the "no
            // service" or "emergency calls only" text that indicates that voice
            // is not available.
            switch (serviceState.getVoiceRegState()) {
                case ServiceState.STATE_POWER_OFF:
                    return false;
                case ServiceState.STATE_OUT_OF_SERVICE:
                case ServiceState.STATE_EMERGENCY_ONLY:
                    return serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE;
                default:
                    return true;
            }
        } else {
            return false;
        }
    }    

    @Override
    public void setCustomizedNetworkTypeView(int subId,
                    int networkTypeId, ImageView networkTypeView) {
        if (networkTypeView != null) {

            if (networkTypeId != 0) {
                networkTypeView.setImageDrawable(
                    mContext.getResources().getDrawable(networkTypeId));
                networkTypeView.setVisibility(View.VISIBLE);
            } else {
                networkTypeView.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public boolean handleCallStateChanged(int subId, int state, String incomingNumber,
         ServiceState serviceState) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
         if ((state == TelephonyManager.CALL_STATE_OFFHOOK
                 || state == TelephonyManager.CALL_STATE_RINGING)
              && MtkTelephonyManagerEx.getDefault().isInCsCall(slotId)) {
             Log.d(TAG, "here set mInC2kCsCall true");
             mInCsCall = true;
         } else {
             Log.d(TAG, "here set mInC2kCsCall false");
             mInCsCall = false;
         }
         if (DEBUG) {
             Log.d(TAG, "handleCallStateChanged(), subId: " + subId + ",state: " + state
                     + ",mInCsCall:" + mInCsCall);
         }
         return true;
    }

    @Override
    public boolean isInCsCall() {
        return mInCsCall;
    }

}
