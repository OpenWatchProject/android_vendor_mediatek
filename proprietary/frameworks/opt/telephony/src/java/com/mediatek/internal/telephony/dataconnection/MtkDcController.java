/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony.dataconnection;

import android.content.Context;
import android.hardware.radio.V1_4.DataConnActiveStatus;
import android.net.LinkAddress;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.NetworkUtils;
import android.net.LinkProperties;
import android.net.LinkProperties.CompareResult;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.AccessNetworkConstants;
import android.telephony.DataFailCause;
import android.telephony.TelephonyManager;
import android.telephony.PhoneStateListener;
import android.telephony.data.DataCallResponse;

import com.android.internal.telephony.DctConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.dataconnection.CellularDataService;
import com.android.internal.telephony.dataconnection.DataConnection.UpdateLinkPropertyResult;
import com.android.internal.telephony.dataconnection.DataConnection;
import com.android.internal.telephony.dataconnection.DataServiceManager;
import com.android.internal.telephony.dataconnection.DcController;
import com.android.internal.telephony.dataconnection.DcTracker;

import com.mediatek.internal.telephony.MtkDctConstants;
import com.mediatek.internal.telephony.MtkPhoneConstants;

import java.util.ArrayList;
import java.util.ConcurrentModificationException;
import java.util.HashMap;
import java.util.List;

public class MtkDcController extends DcController {
    private static final boolean DBG = Build.IS_DEBUGGABLE;
    /// M: CDMA LTE mode system property
    private static final String PROP_MTK_CDMA_LTE_MODE = "ro.vendor.mtk_c2k_support";
    private static final boolean MTK_SVLTE_SUPPORT = (SystemProperties.getInt(
            PROP_MTK_CDMA_LTE_MODE, 0) == 1);
    private static final boolean MTK_SRLTE_SUPPORT = (SystemProperties.getInt(
            PROP_MTK_CDMA_LTE_MODE, 0) == 2);

    private CellularDataService mCellularDataService;

    public MtkDcController(String name, Phone phone, DcTracker dct,
            DataServiceManager dataServiceManager, Handler handler) {
        super(name, phone, dct, dataServiceManager, handler);
        mCellularDataService = new CellularDataService();
    }

    @Override
    public void removeDc(DataConnection dc) {
        super.removeDc(dc);
        if (DBG) log("removeDc: " + dc);
    }

    @Override
    public void addActiveDcByCid(DataConnection dc) {
        super.addActiveDcByCid(dc);
        if (DBG) log("addActiveDcByCid: " + dc);
    }

    @Override
    protected void removeActiveDcByCid(DataConnection dc) {
        super.removeActiveDcByCid(dc);
        if (DBG) log("removeActiveDcByCid: " + dc);
    }

    protected class MtkDccDefaultState extends DccDefaultState {
        @Override
        public void enter() {
            super.enter();
            mDataServiceManager.unregisterForDataCallListChanged(getHandler());
            mPhone.mCi.registerForDataCallListChanged(getHandler(),
                    DataConnection.EVENT_DATA_STATE_CHANGED, null);
        }

        @Override
        public void exit() {
            super.exit();
            mPhone.mCi.unregisterForDataCallListChanged(getHandler());
        }

        @Override
        public boolean processMessage(Message msg) {
            AsyncResult ar;
            boolean retVal = HANDLED;

            switch (msg.what) {
                case DataConnection.EVENT_RIL_CONNECTED:
                    ar = (AsyncResult)msg.obj;
                    if (ar.exception == null) {
                        if (DBG) {
                            log("DccDefaultState: msg.what=EVENT_RIL_CONNECTED mRilVersion=" +
                                ar.result);
                        }
                        // M: Get data call list after ril connected, fix the issue of
                        // can't setup data call after phone process is killed. @{
                        mPhone.mCi.getDataCallList(obtainMessage(
                                MtkDataConnection.EVENT_GET_DATA_CALL_LIST));
                        // @}
                    } else {
                        log("DccDefaultState: Unexpected exception on EVENT_RIL_CONNECTED");
                    }
                    break;

                case MtkDataConnection.EVENT_GET_DATA_CALL_LIST:
                    ar = (AsyncResult)msg.obj;
                    if (ar.exception == null) {
                        onDataStateChanged((ArrayList<DataCallResponse>)ar.result);
                    } else {
                        log("DccDefaultState: EVENT_DATA_STATE_CHANGED:" +
                                    " exception; likely radio not available, ignore");
                    }
                    break;
                default:
                    retVal = super.processMessage(msg);
                    break;
            }
            return retVal;
        }

        /**
         * Process the new list of "known" Data Calls
         * @param dcsList as sent by RIL_UNSOL_DATA_CALL_LIST_CHANGED
         */
        @Override
        protected void onDataStateChanged(ArrayList<DataCallResponse> dcsList) {
            final ArrayList<DataConnection> dcListAll;
            final HashMap<Integer, DataConnection> dcListActiveByCid;
            synchronized (mDcListAll) {
                dcListAll = new ArrayList<>(mDcListAll);
                dcListActiveByCid = new HashMap<>(mDcListActiveByCid);
            }

            if (DBG) {
                lr("onDataStateChanged: dcsList=" + dcsList
                        + " dcListActiveByCid=" + dcListActiveByCid);
            }
            if (VDBG) {
                log("onDataStateChanged: mDcListAll=" + dcListAll);
            }

            // Create hashmap of cid to DataCallResponse
            HashMap<Integer, DataCallResponse> dataCallResponseListByCid =
                    new HashMap<Integer, DataCallResponse>();
            for (DataCallResponse dcs : dcsList) {
                dataCallResponseListByCid.put(dcs.getId(), dcs);
            }

            // Add a DC that is active but not in the
            // dcsList to the list of DC's to retry
            ArrayList<DataConnection> dcsToRetry = new ArrayList<DataConnection>();
            for (DataConnection dc : dcListActiveByCid.values()) {
                if (dataCallResponseListByCid.get(dc.mCid) == null) {
                    if (DBG) log("onDataStateChanged: add to retry dc=" + dc);
                    dcsToRetry.add(dc);
                }
            }
            if (DBG) log("onDataStateChanged: dcsToRetry=" + dcsToRetry);

            // Find which connections have changed state and send a notification or cleanup
            // and any that are in active need to be retried.
            ArrayList<ApnContext> apnsToCleanup = new ArrayList<ApnContext>();

            boolean isAnyDataCallDormant = false;
            boolean isAnyDataCallActive = false;

            /// M: Get the cid of default apn from ril property @{
            int cidForDefault = -1;
            String strPdnInfo =
                    SystemProperties.get("vendor.ril.data.pdn_info"+mPhone.getPhoneId(), "");
            String[] pdnInfos = strPdnInfo.split(",");
            for (String pdnInfo : pdnInfos) {
                String[] info = pdnInfo.split("_");
                if (info.length == 2
                        && PhoneConstants.APN_TYPE_DEFAULT.equals(info[0])) {
                    cidForDefault = Integer.parseInt(info[1]);
                    loge("cidForDefault = " + cidForDefault);
                    break;
                }
            }
            /// @}

            for (DataCallResponse newState : dcsList) {
                int dcActive = Integer.MAX_VALUE;

                // Support iwlan AP assisted mode
                if (ignoreTransportModeNotMatch(newState)) {
                    log("TransportMode not match, ignore");
                    continue;
                }

                DataConnection dc = dcListActiveByCid.get(newState.getId());
                if (dc == null) {
                    // UNSOL_DATA_CALL_LIST_CHANGED arrived before SETUP_DATA_CALL completed.
                    loge("onDataStateChanged: no associated DC yet, ignore");

                    // Remember to change deactivatePdpByCid() function if also has
                    // other types deactivate.
                    // TBD: Currently we only consider default PDN since only it had this behavior
                    // in reality, maybe we should take on-demand PDNs into account later.
                    if (((newState.getId() % 100) != cidForDefault)
                            || (mDct.getState(PhoneConstants.APN_TYPE_DEFAULT) ==
                                DctConstants.State.IDLE)) {
                        loge("Deactivate unlinked PDP context.");
                        ((MtkDcTracker) mDct).deactivatePdpByCid(newState.getId());
                    } else {
                        loge("Default pdn is not in IDLE state, get data call list again");
                        mPhone.mCi.getDataCallList(obtainMessage(
                                MtkDataConnection.EVENT_GET_DATA_CALL_LIST));
                    }

                    continue;
                }

                List<ApnContext> apnContexts = dc.getApnContexts();
                if (apnContexts.size() == 0) {
                    if (DBG) loge("onDataStateChanged: no connected apns, ignore");
                } else {
                    // Determine if the connection/apnContext should be cleaned up
                    // or just a notification should be sent out.
                    if (DBG) {
                        log("onDataStateChanged: Found ConnId=" + newState.getId()
                                + " newState=" + newState.toString());
                    }
                    /// SSC Mode 3 @{
                    if (DataFailCause.getFailCause(newState.getCause())
                            == DataFailCause.MTK_PDU_SSC_MODE_3) {
                        if (DBG) {
                            log("onDataStateChanged: trySetupDataOnEvent. "
                                    + "EVENT_DATA_SETUP_SSC_MODE3, cid=" + newState.getId()
                                    + ", lifetime=" + newState.getSuggestedRetryTime());
                        }
                        ((MtkDcTracker) mDct).trySetupDataOnEvent(
                                MtkDctConstants.EVENT_DATA_SETUP_SSC_MODE3,
                                newState.getId(), newState.getSuggestedRetryTime());
                        continue;
                    }
                    /// @}
                    // M: [OD over ePDG] start
                    ((MtkDataConnection) dc).setConnectionRat(
                            MtkDcHelper.decodeRat(newState.getLinkStatus()),
                            "data call list");
                    dcActive = newState.getLinkStatus() % MtkPhoneConstants.RAT_TYPE_KEY;
                    // M: [OD over ePDG] end
                    if (dcActive == DataConnActiveStatus.INACTIVE) {
                        if (mDct.isCleanupRequired.get()) {
                            apnsToCleanup.addAll(apnContexts);
                            mDct.isCleanupRequired.set(false);
                        } else {
                            int failCause = DataFailCause.getFailCause(newState.getCause());
                            if (DataFailCause.isRadioRestartFailure(mPhone.getContext(), failCause,
                                        mPhone.getSubId())) {
                                if (DBG) {
                                    log("onDataStateChanged: X restart radio, failCause="
                                            + failCause);
                                }
                                mDct.sendRestartRadio();
                            } else if (mDct.isPermanentFailure(failCause)) {
                                if (DBG) {
                                    log("onDataStateChanged: inactive, add to cleanup list. "
                                            + "failCause=" + failCause);
                                }
                                apnsToCleanup.addAll(apnContexts);
                            } else {
                                if (DBG) {
                                    log("onDataStateChanged: inactive, add to retry list. "
                                            + "failCause=" + failCause);
                                }
                                dcsToRetry.add(dc);
                            }
                        }
                    } else {
                        // Its active so update the DataConnections link properties
                        UpdateLinkPropertyResult result = dc.updateLinkProperty(newState);
                        if (result.oldLp.equals(result.newLp)) {
                            if (DBG) log("onDataStateChanged: no change");
                        } else {
                            if (result.oldLp.isIdenticalInterfaceName(result.newLp)) {
                                if (! result.oldLp.isIdenticalDnses(result.newLp) ||
                                        ! result.oldLp.isIdenticalRoutes(result.newLp) ||
                                        ! result.oldLp.isIdenticalHttpProxy(result.newLp) ||
                                        ! isIpMatched(result.oldLp, result.newLp)) {
                                    // If the same address type was removed and
                                    // added we need to cleanup
                                    CompareResult<LinkAddress> car =
                                        result.oldLp.compareAddresses(result.newLp);
                                    if (DBG) {
                                        log("onDataStateChanged: oldLp=" + result.oldLp +
                                                " newLp=" + result.newLp + " car=" + car);
                                    }
                                    boolean needToClean = false;
                                    for (LinkAddress added : car.added) {
                                        for (LinkAddress removed : car.removed) {
                                            if (NetworkUtils.addressTypeMatches(
                                                    removed.getAddress(),
                                                    added.getAddress())) {
                                                needToClean = true;
                                                break;
                                            }
                                        }
                                    }
                                    /// M: Don't clean DC when IP changed for IRAT.
                                    if ((MTK_SVLTE_SUPPORT || MTK_SRLTE_SUPPORT) &&
                                            mPhone.getPhoneType() ==
                                            PhoneConstants.PHONE_TYPE_CDMA) {
                                        if (DBG) {
                                            log("onDataStateChanged: IRAT set needToClean false");
                                        }
                                        needToClean = false;
                                    } else if ("OP07".equals(
                                            SystemProperties.get("persist.vendor.operator.optr"))) {
                                        if (DBG) {
                                            log("onDataStateChanged: OP07 set needToClean false");
                                        }
                                        needToClean = false;
                                    }
                                    if (needToClean) {
                                        if (DBG) {
                                            log("onDataStateChanged: addr change,"
                                                    + " cleanup apns=" + apnContexts
                                                    + " oldLp=" + result.oldLp
                                                    + " newLp=" + result.newLp);
                                        }
                                        apnsToCleanup.addAll(apnContexts);
                                    } else {
                                        if (DBG) log("onDataStateChanged: simple change");

                                        for (ApnContext apnContext : apnContexts) {
                                            mPhone.notifyDataConnection(apnContext.getApnType());
                                        }
                                    }
                                } else {
                                    if (DBG) {
                                        log("onDataStateChanged: no changes");
                                    }
                                }
                            } else {
                                apnsToCleanup.addAll(apnContexts);
                                if (DBG) {
                                    log("onDataStateChanged: interface change, cleanup apns="
                                            + apnContexts);
                                }
                            }
                        }
                    }
                }

                if (dcActive == DataConnActiveStatus.ACTIVE) {
                    isAnyDataCallActive = true;
                }
                if (dcActive == DataConnActiveStatus.DORMANT) {
                    isAnyDataCallDormant = true;
                }
            }

            if (isAnyDataCallDormant && !isAnyDataCallActive) {
                // There is no way to indicate link activity per APN right now. So
                // Link Activity will be considered dormant only when all data calls
                // are dormant.
                // If a single data call is in dormant state and none of the data
                // calls are active broadcast overall link state as dormant.
                if (DBG) {
                    log("onDataStateChanged: Data Activity updated to DORMANT. stopNetStatePoll");
                }
                mDct.sendStopNetStatPoll(DctConstants.Activity.DORMANT);
            } else {
                if (DBG) {
                    log("onDataStateChanged: Data Activity updated to NONE. " +
                            "isAnyDataCallActive = " + isAnyDataCallActive +
                            " isAnyDataCallDormant = " + isAnyDataCallDormant);
                }
                if (isAnyDataCallActive) {
                    mDct.sendStartNetStatPoll(DctConstants.Activity.NONE);
                }
            }

            if (DBG) {
                lr("onDataStateChanged: dcsToRetry=" + dcsToRetry
                        + " apnsToCleanup=" + apnsToCleanup);
            }

            // Cleanup connections that have changed
            for (ApnContext apnContext : apnsToCleanup) {
                mDct.cleanUpConnection(apnContext);
            }

            // Retry connections that have disappeared
            for (DataConnection dc : dcsToRetry) {
                if (DBG) log("onDataStateChanged: send EVENT_LOST_CONNECTION dc.mTag=" + dc.mTag);
                dc.sendMessage(DataConnection.EVENT_LOST_CONNECTION, dc.mTag);
            }

            if (VDBG) log("onDataStateChanged: X");
        }
    }

    /** M: if new IP contains old IP, we treat is equally
     *  This is to handle that network response both IPv4 and IPv6 IP,
     *  but APN settings is IPv4 or IPv6
     */
    private boolean isIpMatched(LinkProperties oldLp, LinkProperties newLp) {
        if (oldLp.isIdenticalAddresses(newLp)) {
            return true;
        } else {
            if (DBG) log("isIpMatched: address count is different but matched");
            return newLp.getAddresses().containsAll(oldLp.getAddresses());
        }
    }

    /**
     * Anchor method to replace states implementation in the state machine initialization procedure.
     */
    @Override
    protected void mtkReplaceStates() {
        mDccDefaultState = new MtkDccDefaultState();
    }

    /*  M: Support iwlan AP assisted mode
     *     ignore check if rat type not match the transport mode
     */
    boolean ignoreTransportModeNotMatch(DataCallResponse newDcr) {
        boolean ignore = false;
        if (!mPhone.getTransportManager().isInLegacyMode()) {  // alway check in legacy mode
            int mRat = MtkDcHelper.decodeRat(newDcr.getLinkStatus());
            if (mRat > 0 ) {
                if ((mDataServiceManager.getTransportType() ==
                        AccessNetworkConstants.TRANSPORT_TYPE_WLAN) &&
                        (mRat == MtkPhoneConstants.RAT_TYPE_MOBILE_3GPP ||
                        mRat == MtkPhoneConstants.RAT_TYPE_MOBILE_3GPP2)) {
                    ignore = true;
                }
                if (mDataServiceManager.getTransportType() ==
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN &&
                        mRat == MtkPhoneConstants.RAT_TYPE_WIFI) {
                    ignore = true;
                }
            } else {
                if (DBG) log("ignoreTransportModeNotMatch: can't handle mRat=" + mRat);
            }
        } else {
            if (mPhone.getHalVersion().less(RIL.RADIO_HAL_VERSION_1_4)) {
                if (DBG) log("ignoreTransportModeNotMatch HalVersion:" + mPhone.getHalVersion());
            }
        }
        if (DBG) log("ignoreTransportModeNotMatch: ignore=" + ignore);
        return ignore;
    }

    /** M: To get active connection count for
     *     E911 bearer management
     */
    int getActiveDcCount() {
        return mDcListActiveByCid.size();
    }

    int getActiveDcTagSupportInternet() {
        final HashMap<Integer, DataConnection> dcListActiveByCid;
        synchronized (mDcListAll) {
            dcListActiveByCid = new HashMap<>(mDcListActiveByCid);
        }

        List<ApnContext> apnContexts = null;
        for (DataConnection dc : dcListActiveByCid.values()) {
            apnContexts = dc.getApnContexts();
            if (apnContexts.size() == 0) {
                if (DBG) loge("getActiveDcTagSupportInternet: no connected apns, ignore");
                continue;
            }
            for (ApnContext apnContext : apnContexts) {
                for (NetworkRequest nr : apnContext.getNetworkRequests()) {
                    if (nr.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)) {
                        if (DBG) log("getActiveDcTagSupportInternet: get dc.mTag=" + dc.mTag);
                        return dc.mTag;
                    }
                }
            }
        }
        if (DBG) log("getActiveDcTagSupportInternet: no actvie DC support internet");
        return 0;
    }
}
