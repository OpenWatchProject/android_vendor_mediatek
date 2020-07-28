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

// for Data usage
import android.telephony.TelephonyManager;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.net.IConnectivityManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.NetworkCapabilities;
import android.net.NetworkStats;
import android.net.LinkProperties;
import android.os.INetworkManagementService;
import android.os.IBinder;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.os.PersistableBundle;
import java.util.Objects;
import static android.net.NetworkStats.UID_ALL;
import static android.net.NetworkStats.INTERFACES_ALL;

import com.mediatek.ims.common.SubscriptionManagerHelper;

// for judge if need count usage
import android.telephony.CarrierConfigManager;
import android.os.SystemProperties;

import android.os.RegistrantList;

import android.content.Context;

import android.util.Log;

import com.mediatek.ims.internal.ImsVTProviderUtil;

public class ImsVTUsageManager {

    public static class ImsVTUsage {
        public static final int STATE_TX   = 0x1;
        public static final int STATE_RX   = 0x2;
        public static final int STATE_TXRX = STATE_TX | STATE_RX;

        private String mUsedForName;

        private long mLteUsage;
        private long mWifiUsage;

        private long mLteTxUsage;
        private long mLteRxUsage;
        private long mWifiTxUsage;
        private long mWifiRxUsage;

        public ImsVTUsage(String name) {
            mUsedForName = name;
            mLteUsage = 0;
            mLteTxUsage = 0;
            mLteRxUsage = 0;
            mWifiUsage = 0;
            mWifiTxUsage = 0;
            mWifiRxUsage = 0;

            Log.d(TAG, "[ImsVTUsage]" + toString());
        }

        public ImsVTUsage(String name,
                long lteTx, long lteRx, long lteTxRx,
                long wifiTx, long wifiRx, long wifiTxRx) {
            mUsedForName = name;

            mLteTxUsage = lteTx;
            mLteRxUsage = lteRx;
            mLteUsage = lteTxRx;

            mWifiTxUsage = wifiTx;
            mWifiRxUsage = wifiRx;
            mWifiUsage = wifiTxRx;

            Log.d(TAG, "[ImsVTUsage]" + toString());
        }

        public ImsVTUsage(String name,ImsVTUsage usage) {
            mUsedForName = name;

            mLteTxUsage = usage.getLteUsage(STATE_TX);
            mLteRxUsage = usage.getLteUsage(STATE_RX);
            mLteUsage = usage.getLteUsage(STATE_TXRX);

            mWifiTxUsage = usage.getWifiUsage(STATE_TX);
            mWifiRxUsage = usage.getWifiUsage(STATE_RX);;
            mWifiUsage = usage.getWifiUsage(STATE_TXRX);;

            Log.d(TAG, "[ImsVTUsage]" + toString());
        }

        public long getLteUsage(int state) {
            long usage = 0;

            if (STATE_TX == state) {
                usage = mLteTxUsage;

            } else if (STATE_RX == state) {
                usage = mLteRxUsage;

            } else if (STATE_TXRX == state) {
                usage = mLteUsage;
            }

            return usage;
        }

        public long getWifiUsage(int state) {
            long usage = 0;

            if (STATE_TX == state) {
                usage = mWifiTxUsage;

            } else if (STATE_RX == state) {
                usage = mWifiRxUsage;

            } else if (STATE_TXRX == state) {
                usage = mWifiUsage;
            }

            return usage;
        }

        public void setLteUsage(int state, long usage) {
            if (STATE_TX == state) {
                mLteTxUsage = usage;

            } else if (STATE_RX == state) {
                mLteRxUsage = usage;

            } else if (STATE_TXRX == state) {
                mLteUsage = usage;
            }

            Log.d(TAG, "[setLTEUsage] state: " + state + ", usage: " + usage);
        }

        public void setWifiUsage(int state, long usage) {
            if (STATE_TX == state) {
                mWifiTxUsage = usage;

            } else if (STATE_RX == state) {
                mWifiRxUsage = usage;

            } else if (STATE_TXRX == state) {
                mWifiUsage = usage;
            }

            Log.d(TAG, "[setWifiUsage] state: " + state + ", usage: " + usage);
        }

        public void setAllUsage(long lteTx, long lteRx, long lteTxRx,
                long wifiTx, long wifiRx, long wifiTxRx) {
            mLteTxUsage = lteTx;
            mLteRxUsage = lteRx;
            mLteUsage = lteTxRx;

            mWifiTxUsage = wifiTx;
            mWifiRxUsage = wifiRx;
            mWifiUsage = wifiTxRx;

            Log.d(TAG, "[setAllUsage]" + toString());
        }

        public void updateFrom(ImsVTUsage usage) {
            mLteTxUsage = usage.getLteUsage(STATE_TX);
            mLteRxUsage = usage.getLteUsage(STATE_RX);
            mLteUsage = usage.getLteUsage(STATE_TXRX);

            mWifiTxUsage = usage.getWifiUsage(STATE_TX);
            mWifiRxUsage = usage.getWifiUsage(STATE_RX);
            mWifiUsage = usage.getWifiUsage(STATE_TXRX);

            Log.d(TAG, "[updateFrom]" + toString());
        }

        public void subtraction(ImsVTUsage subUsage) {
            mLteTxUsage = mLteTxUsage - subUsage.getLteUsage(STATE_TX);
            mLteRxUsage = mLteRxUsage - subUsage.getLteUsage(STATE_RX);
            mLteUsage = mLteUsage - subUsage.getLteUsage(STATE_TXRX);

            mWifiTxUsage = mWifiTxUsage - subUsage.getWifiUsage(STATE_TX);
            mWifiRxUsage = mWifiRxUsage - subUsage.getWifiUsage(STATE_RX);
            mWifiUsage = mWifiUsage - subUsage.getWifiUsage(STATE_TXRX);

            Log.d(TAG, "[subtraction]" + toString());
        }

        public String toString() {
            return ("[" + mUsedForName + "] "
                + "lteTx=" + mLteTxUsage + ", lteRx=" + mLteRxUsage + ", lteTxRx=" + mLteUsage
                + ", wifiTx=" + mWifiTxUsage + ", wifiRx=" + mWifiRxUsage + ", wifiTxRx=" + mWifiUsage);
        }

    }

    static final String                         TAG = "ImsVT Usage";

    public static final int                     VILTE_UID = 1000;

    public int                                  mId;
    protected int                               mSimId;

    private Context                             mContext;

    // Usage when call init
    private ImsVTUsage                          mInitialUsage;
    private ImsVTUsage                          mCurrentUsage;
    private ImsVTUsage                          mPreviousUsage;

    public ImsVTProviderUtil                    mVTProviderUtil = ImsVTProviderUtil.getInstance();

    private boolean                             mNeedReportDataUsage = true;
    private RegistrantList mDataUsageUpdateRegistrants = new RegistrantList();

    public ImsVTUsageManager() {
        mCurrentUsage = new ImsVTUsage("Current");
        mPreviousUsage = new ImsVTUsage("Previous");
    }

    public void setId(int id) {
        mId = id;
    }

    public void setSimId(int simId) {
        mSimId = simId;
    }

    public void setInitUsage(ImsVTUsage initUsage) {
        mInitialUsage = initUsage;
    }

    public void setContext(Context context) {
        mContext = context;
    }

    public ImsVTUsage requestCallDataUsage() {

        Log.d(TAG, "[ID=" + mId + "] [onRequestCallDataUsage] Start");

        if (!canRequestDataUsage()) {
            return null;
        }

        // ====================================================================
        // get IMS/WIFI Interface and data from CONNECTIVITY SERVICE
        //
        String mActiveImsIface = "";
        String mActiveWifiIface = "";

        String subIdStr = "" + SubscriptionManagerHelper.getSubIdUsingPhoneId(mSimId);
        ConnectivityManager sConnMgr = (ConnectivityManager) mVTProviderUtil.mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        Network [] nets = sConnMgr.getAllNetworks();
        NetworkInfo nwInfo;
        LinkProperties lkProp;

        if (null != nets) {

            for (Network net : nets) {

                nwInfo = sConnMgr.getNetworkInfo(net);

                if (nwInfo != null && nwInfo.isConnected() ) {

                    NetworkCapabilities netCap = sConnMgr.getNetworkCapabilities(net);

                    if (null == netCap) {
                        Log.d(TAG, "[onRequestCallDataUsage] netCap = null");
                        continue;
                    }

                    Log.d(TAG, "[onRequestCallDataUsage] nwInfo:" + nwInfo.toString() +
                            ", checking net=" + net + " cap=" + netCap);

                    lkProp = sConnMgr.getLinkProperties(net);
                    if (null != lkProp) {
                        if (netCap.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                            mActiveWifiIface = lkProp.getInterfaceName();
                            Log.d(TAG, "[onRequestCallDataUsage] mActiveWifiIface=" + mActiveWifiIface);

                        } else if (netCap.hasCapability(NetworkCapabilities.NET_CAPABILITY_IMS)) {
                            String networkSpecifier = "" + netCap.getNetworkSpecifier();
                            if (!subIdStr.equals(networkSpecifier)) {
                                Log.d(TAG, "[onRequestCallDataUsage] Get Ims interface with different sub, " +
                                        "net=" + net + " specifier=" + networkSpecifier + " sub=" + subIdStr +
                                        "specifier.length=" + networkSpecifier.length() + " sub.length=" +
                                        subIdStr.length());
                                continue;
                            }
                            mActiveImsIface = lkProp.getInterfaceName();
                            Log.d(TAG, "[onRequestCallDataUsage] mActiveImsIface=" + mActiveImsIface);

                        } else {
                            Log.d(TAG, "[onRequestCallDataUsage] netCap neither contain WIF nor LTE.");
                        }
                    } else {
                        Log.e(TAG, "[onRequestCallDataUsage] sConnMgr.getLinkProperties(net) = NULL");
                    }
                }
            }

            if ("" == mActiveImsIface) {
                Log.e(TAG, "[onRequestCallDataUsage] mActiveImsIface is empty");
                return null;
            }

        } else {
            Log.d(TAG, "[onRequestCallDataUsage] getAllNetworks returns null.");
            return null;
        }

        // ====================================================================
        // Calculate the usage of ViLTE IMS
        //
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        INetworkManagementService mNetworkManager = INetworkManagementService.Stub.asInterface(b);

        try {
            // should use UID_ALL = -1
            NetworkStats uidSnapshot = mNetworkManager.getNetworkStatsUidDetail(UID_ALL, INTERFACES_ALL);

            ImsVTUsage tagUsage = getTagUsage(uidSnapshot, mActiveImsIface, mActiveWifiIface);
            updateUsage(tagUsage);

            Log.d(TAG, "[ID=" + mId + "] [onRequestCallDataUsage] Finish (VIWIFI usage:" +
                    mCurrentUsage.getWifiUsage(ImsVTUsage.STATE_TXRX) + ")");

            return mCurrentUsage;

        } catch (RemoteException e) {
            Log.d(TAG, "Exception:" + e);
            return null;
        }
    }

    private boolean canRequestDataUsage() {
        Log.d(TAG, "[canRequestDataUsage]");

        boolean forceRequest = SystemProperties.get("persist.vendor.vt.data_simulate").equals("1");
        if (forceRequest) {
            return true;
        }

        int subId = SubscriptionManagerHelper.getSubIdUsingPhoneId(mSimId);

        boolean ignoreDataEnabledChanged = getBooleanCarrierConfig(
                mVTProviderUtil.mContext,
                CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS,
                subId);

        if (mNeedReportDataUsage && ignoreDataEnabledChanged) {

            Log.d(TAG, "[canRequestDataUsage] set dataUsage as false");
            mNeedReportDataUsage = false;
        }

        return mNeedReportDataUsage;
    }

    private boolean getBooleanCarrierConfig(Context context, String key, int subId) {

        CarrierConfigManager configManager = (CarrierConfigManager) context.getSystemService(Context.CARRIER_CONFIG_SERVICE);

        PersistableBundle carrierConfig = null;

        if (configManager != null) {
            carrierConfig = configManager.getConfigForSubId(subId);
        }

        if (carrierConfig != null) {
            return carrierConfig.getBoolean(key);
        } else {
            // Return static default defined in CarrierConfigManager.
            return CarrierConfigManager.getDefaultConfig().getBoolean(key);
        }
    }

    private ImsVTUsage getTagUsage(NetworkStats uidSnapshot, String lte_iface, String wifi_iface) {
        Log.i(TAG, "getTagUsage uid:" + VILTE_UID + ", LTE iface:" + lte_iface + ", WiFi iface:" + wifi_iface);

        long TotalLteTxBytes = 0;
        long TotalLteRxBytes = 0;
        long TotalWifiTxBytes = 0;
        long TotalWifiRxBytes = 0;

        NetworkStats.Entry entry = null;

        int lte_tag = ImsVTProviderUtil.TAG_VILTE_MOBILE + mId;
        int wifi_tag = ImsVTProviderUtil.TAG_VILTE_WIFI + mId;

        for (int j = 0; j < uidSnapshot.size(); j++) {

            entry = uidSnapshot.getValues(j, entry);

            // Get LTE taggedSnapshot
            if (Objects.equals(entry.iface, lte_iface) && entry.uid == VILTE_UID && entry.tag == lte_tag) {

                Log.i(TAG, "getTaggedSnapshot LTE entry:" + entry.toString());

                TotalLteTxBytes += entry.txBytes;
                TotalLteRxBytes += entry.rxBytes;
            }

            // Get WiFi taggedSnapshot
            if (Objects.equals(entry.iface, wifi_iface) && entry.uid == VILTE_UID && entry.tag == wifi_tag) {

                Log.i(TAG, "getTaggedSnapshot WiFi entry:" + entry.toString());

                TotalWifiTxBytes += entry.txBytes;
                TotalWifiRxBytes += entry.rxBytes;
            }

        }
        Log.i(TAG, "getTaggedSnapshot LTE: Tx=" + Long.toString(TotalLteTxBytes) +
                    ", Rx=" + Long.toString(TotalLteRxBytes) +
                    ", Total=" + Long.toString(TotalLteTxBytes + TotalLteRxBytes));
        Log.i(TAG, "getTaggedSnapshot WiFi: Tx=" + Long.toString(TotalWifiTxBytes) +
                    ", Rx=" + Long.toString(TotalWifiRxBytes) +
                    ", Total=" + Long.toString(TotalWifiTxBytes + TotalWifiRxBytes));

        return (new ImsVTUsage("Tag",
                TotalLteTxBytes, TotalLteRxBytes, TotalLteRxBytes + TotalLteTxBytes,
                TotalWifiTxBytes, TotalWifiRxBytes, TotalWifiRxBytes + TotalWifiTxBytes));
    }

    private void updateUsage(ImsVTUsage tagUsage) {
        int subId = SubscriptionManagerHelper.getSubIdUsingPhoneId(mSimId);
        TelephonyManager telephony = (TelephonyManager) mVTProviderUtil.mContext.getSystemService(Context.TELEPHONY_SERVICE);
        String subscriberId = telephony.getSubscriberId(subId);

        long initWifiTxBytes   = mInitialUsage.getWifiUsage(ImsVTUsage.STATE_TX);
        long initWifiRxBytes   = mInitialUsage.getWifiUsage(ImsVTUsage.STATE_RX);
        long initWifiTxRxBytes = mInitialUsage.getWifiUsage(ImsVTUsage.STATE_TXRX);

        long curWifiTxBytes   = mCurrentUsage.getWifiUsage(ImsVTUsage.STATE_TX);
        long curWifiRxBytes   = mCurrentUsage.getWifiUsage(ImsVTUsage.STATE_RX);
        long curWifiTxRxBytes = mCurrentUsage.getWifiUsage(ImsVTUsage.STATE_TXRX);

        mPreviousUsage.updateFrom(mCurrentUsage);

        // Here are 2 cases will get viwifi usage tagged snapshot 0
        // Case 1: DUT has not made any viwifi call yet
        // Case 2: WiFi is off.
        // For both cases we set viwifi usage as current (means no changed).
        if (tagUsage.getWifiUsage(ImsVTUsage.STATE_TX) == 0 &&
            tagUsage.getWifiUsage(ImsVTUsage.STATE_RX) == 0) {

            tagUsage.setWifiUsage(ImsVTUsage.STATE_TX, curWifiTxBytes + initWifiTxBytes);
            tagUsage.setWifiUsage(ImsVTUsage.STATE_RX, curWifiRxBytes + initWifiRxBytes);
            tagUsage.setWifiUsage(ImsVTUsage.STATE_TXRX, curWifiTxRxBytes + initWifiTxRxBytes);
        }

        mVTProviderUtil.usageSet(mId, tagUsage);

        tagUsage.subtraction(mInitialUsage);
        mCurrentUsage.updateFrom(tagUsage);

        // Send delta usage broadcast
        ImsVTUsage deltaUsage = new ImsVTUsage("Delta", mCurrentUsage);
        deltaUsage.subtraction(mPreviousUsage);

        Intent VTUsageIntent = new Intent(mVTProviderUtil.getImsOemCallUtil().getVTUsageAction());
        VTUsageIntent.putExtra("lterxbytes",  deltaUsage.getLteUsage(ImsVTUsage.STATE_RX));
        VTUsageIntent.putExtra("ltetxbytes",  deltaUsage.getLteUsage(ImsVTUsage.STATE_TX));
        VTUsageIntent.putExtra("wifirxbytes", deltaUsage.getWifiUsage(ImsVTUsage.STATE_RX));
        VTUsageIntent.putExtra("wifiTxbytes", deltaUsage.getWifiUsage(ImsVTUsage.STATE_TX));
        VTUsageIntent.putExtra("subscriberId", subscriberId);
        Log.i(TAG, "sendVTusageBroadcast:" +
                    "  lterxbytes="  + Long.toString(deltaUsage.getLteUsage(ImsVTUsage.STATE_RX)) +
                    ", ltetxbytes="  + Long.toString(deltaUsage.getLteUsage(ImsVTUsage.STATE_TX)) +
                    ", wifirxbytes=" + Long.toString(deltaUsage.getWifiUsage(ImsVTUsage.STATE_RX)) +
                    ", wifiTxbytes=" + Long.toString(deltaUsage.getWifiUsage(ImsVTUsage.STATE_TX)) +
                    ", subscriberId=" + subscriberId);
        mVTProviderUtil.mContext.sendBroadcast(VTUsageIntent, mVTProviderUtil.getImsOemCallUtil().getVTUsagePermission());
    }
    //
}
