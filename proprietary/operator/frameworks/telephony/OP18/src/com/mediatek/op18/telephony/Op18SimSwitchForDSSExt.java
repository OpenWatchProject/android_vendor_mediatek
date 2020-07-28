package com.mediatek.op18.telephony;

import android.content.Context;
import android.content.Intent;

import android.os.Build;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SubscriptionController;
import com.mediatek.internal.telephony.datasub.DataSubConstants;
import com.mediatek.internal.telephony.datasub.DataSubSelector;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;
import com.mediatek.internal.telephony.datasub.ISimSwitchForDSSExt;
import com.mediatek.internal.telephony.datasub.CapabilitySwitch;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import java.util.Arrays;

public class Op18SimSwitchForDSSExt implements ISimSwitchForDSSExt {
    private static String LOG_TAG = "Op18SimSwitch";

    private static final boolean USER_BUILD = TextUtils.equals(Build.TYPE, "user");
    private static boolean DBG = true;


    private static DataSubSelector mDataSubSelector = null;

    private static CapabilitySwitch mCapabilitySwitch = null;

    private Context mContext = null;

    public Op18SimSwitchForDSSExt(Context context) {
        mContext = context;
    }

    public void init(DataSubSelector dataSubSelector) {
        log("init Op18SimSwitchForDSSExt");
        mDataSubSelector = dataSubSelector;
        mCapabilitySwitch = CapabilitySwitch.getInstance(mContext, dataSubSelector);
    }

    public boolean checkCapSwitch(int policy) {
        return checkCapSwitch();
    }

    public boolean checkCapSwitch() {
        // op18 has multi policies, check configuration is default
        if (mCapabilitySwitch.getCapabilitySwitchPolicy() != DataSubConstants.POLICY_POLICY1) {
            log("checkOp18CapSwitch: config is not policy1, do nothing");
            return true;
        }
        // check if need to switch capability
        // op18 > others
        int[] simOpInfo = new int[mDataSubSelector.getPhoneNum()];
        int[] simType = new int[mDataSubSelector.getPhoneNum()];
        int targetSim = -1;
        int insertedSimCount = 0;
        int insertedStatus = 0;
        boolean[] op18Usim = new boolean[mDataSubSelector.getPhoneNum()];
        int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
        int defDataPhoneId = -1;
        String[] currIccId = new String[mDataSubSelector.getPhoneNum()];

        log("checkOp18CapSwitch start");

        if (defDataSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
        }
        for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
            currIccId[i] = DataSubSelectorUtil.getIccidFromProp(i);
            if (currIccId[i] == null || "".equals(currIccId[i])) {
                log("error: iccid not found, wait for next sub ready");
                return false;
            }
            if (!DataSubConstants.NO_SIM_VALUE.equals(currIccId[i])) {
                ++insertedSimCount;
                insertedStatus = insertedStatus | (1 << i);
            }
        }
        log("checkOp18CapSwitch : Inserted SIM count: " + insertedSimCount
                + ", insertedStatus: " + insertedStatus);
        if (RadioCapabilitySwitchUtil.getSimInfo(simOpInfo, simType, insertedStatus) == false) {
            return false;
        }
        // check pin lock
        if (mCapabilitySwitch.isSimUnLocked() == false) {
            log("checkOp18CapSwitch : set need wait unlock");
            CapabilitySwitch.setNeedWaitUnlock("true");
        }
        int capabilitySimId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();

        log("op18: capabilitySimId:" + capabilitySimId);
        for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
            // update SIM status
            if (simOpInfo[i] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP18) {
                op18Usim[i] = true;
            }
        }
        // dump sim op info
        log("op18Usim: " + Arrays.toString(op18Usim));

        for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
            if (defDataPhoneId == i) {
                if (op18Usim[i] == true) {
                    targetSim = i;
                } else {
                    for (int j = 0; j < mDataSubSelector.getPhoneNum(); j++) {
                        if (op18Usim[j] == true) {
                            targetSim = j;
                        }
                    }
                }
                log("op18: i = " + i + "targetSim : " + targetSim);
                // default capability SIM is inserted
                if (op18Usim[i] == true) {
                    log("op18-C1: cur is old op18 USIM, no change");
                    if (capabilitySimId != i) {
                        log("op18-C1a: old op18 SIM change slot, change!");
                        mCapabilitySwitch.setCapability(i);
                    }
                    mDataSubSelector.setDefaultData(i);
                    mDataSubSelector.setDataEnabled(i, true);
                    return true;
                } else if (targetSim != -1) {
                    log("op18-C2: cur is not op18 SIM but find op18 SIM, change!");
                    mCapabilitySwitch.setCapability(targetSim);
                    mDataSubSelector.setDefaultData(targetSim);
                    mDataSubSelector.setDataEnabled(targetSim, true);
                    return true;
                }
                mDataSubSelector.setDefaultData(capabilitySimId);
                mDataSubSelector.setDataEnabled(capabilitySimId, true);
                log("op18-C6: no higher priority SIM, no cahnge");
                return true;
            }
        }
        // cannot find default capability SIM, check if higher priority SIM exists
        if (op18Usim[capabilitySimId] == true) {
            targetSim = capabilitySimId;
        } else {
            for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                if (op18Usim[i] == true) {
                    targetSim = i;
                }
            }
        }
        log("op18: target SIM :" + targetSim);
        if (op18Usim[capabilitySimId] == true) {
            log("op18-C7: cur is new op18 USIM, no change");
            mDataSubSelector.setDefaultData(capabilitySimId);
            mDataSubSelector.setDataEnabled(capabilitySimId, true);
            return true;
        } else if (targetSim != -1) {
            log("op18-C8: find op18 USIM, change!");
            mCapabilitySwitch.setCapability(targetSim);
            mDataSubSelector.setDefaultData(targetSim);
            mDataSubSelector.setDataEnabled(targetSim, true);
            return true;
        } else {
            mDataSubSelector.setDefaultData(capabilitySimId);
            mDataSubSelector.setDataEnabled(capabilitySimId, true);
            log("op18-C12: no higher priority SIM, no cahnge");
        }
        return true;
    }

    public int isNeedSimSwitch() {
        return DataSubConstants.SIM_SWITCH_UNKNOWN;
    }

    private void log(String txt) {
        if (DBG) {
            Rlog.d(LOG_TAG, txt);
        }
    }

    private void loge(String txt) {
        if (DBG) {
            Rlog.e(LOG_TAG, txt);
        }
    }
}
