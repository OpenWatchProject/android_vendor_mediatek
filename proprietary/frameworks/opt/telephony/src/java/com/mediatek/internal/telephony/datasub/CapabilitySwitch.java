package com.mediatek.internal.telephony.datasub;

import android.content.Context;
import android.content.Intent;

import android.os.SystemProperties;

import android.telephony.RadioAccessFamily;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ProxyController;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import static com.mediatek.internal.telephony.datasub.DataSubConstants.*;

public class CapabilitySwitch {
    private static final String LOG_TAG = "CapaSwitch";
    private static boolean DBG = true;

    private static CapabilitySwitch mInstance = null;
    static private Context mContext = null;

    private static DataSubSelector mDataSubSelector = null;

    private final static int capability_switch_policy =
            SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH_POLICY, POLICY_DEFAULT);

    static public CapabilitySwitch getInstance(Context context, DataSubSelector dataSubSelector) {
        if (mInstance == null) {
            mInstance = new CapabilitySwitch(context, dataSubSelector);
        }
        return mInstance;
    }

    public CapabilitySwitch(Context context, DataSubSelector dataSubSelector) {
        mContext = context;
        mDataSubSelector = dataSubSelector;
    }

    public boolean isCanSwitch() {
        boolean ret = true;
        if (mDataSubSelector.getAirPlaneModeOn()){
            ret = false ;
            log("DataSubselector,isCanSwitch AirplaneModeOn = " +
                    mDataSubSelector.getAirPlaneModeOn());
            return ret;
        }
        return isSimUnLocked();
    }

    public boolean isSimUnLocked() {
        int simState = 0;
        int simNum = mDataSubSelector.getPhoneNum();

        for (int i = 0; i < simNum; i++) {
            simState = TelephonyManager.from(mContext).getSimState(i);
            if (simState == TelephonyManager.SIM_STATE_PIN_REQUIRED ||
                    simState == TelephonyManager.SIM_STATE_PUK_REQUIRED ||
                    simState == TelephonyManager.SIM_STATE_NETWORK_LOCKED ||
                    simState == TelephonyManager.SIM_STATE_NOT_READY ||
                    simState == TelephonyManager.SIM_STATE_UNKNOWN){
                log("isSimUnLocked, sim locked, simState = " + simState + ", slot=" + i);

                if ((simState != TelephonyManager.SIM_STATE_NOT_READY) ||
                        (RadioCapabilitySwitchUtil.isSimOn(i) == true)) {
                    return false;
                }
            }
        }
        return true;
    }

    static public boolean isNeedWaitUnlock() {
        return (SystemProperties.getBoolean(NEED_TO_WAIT_UNLOCKED, false));
    }

    static public boolean isNeedWaitUnlockRoaming() {
        return (SystemProperties.getBoolean(NEED_TO_WAIT_UNLOCKED_ROAMING, false));
    }

    static public void setNeedWaitUnlock(String value) {
        SystemProperties.set(NEED_TO_WAIT_UNLOCKED, value);
    }

    static public void setNeedWaitUnlockRoaming(String value) {
        SystemProperties.set(NEED_TO_WAIT_UNLOCKED_ROAMING, value);
    }

    static public boolean isNeedWaitImsi() {
        return (SystemProperties.getBoolean(NEED_TO_WAIT_IMSI, false));
    }

    static public boolean isNeedWaitImsiRoaming() {
        return (SystemProperties.getBoolean(NEED_TO_WAIT_IMSI_ROAMING, false));
    }

    static public void setNeedWaitImsi(String value) {
        SystemProperties.set(NEED_TO_WAIT_IMSI, value);
    }

    static public void setNeedWaitImsiRoaming(String value) {
        SystemProperties.set(NEED_TO_WAIT_IMSI_ROAMING, value);
    }

    static public void setSimStatus(Intent intent) {
        if (intent == null) {
            log("setSimStatus, intent is null => return");
            return;
        }
        log("setSimStatus");

        int detectedType = intent.getIntExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS, 0);
        if (detectedType != getSimStatus()) {
            SystemProperties.set(SIM_STATUS, Integer.toString(detectedType));
        }
    }

    static public void resetSimStatus() {
        log("resetSimStatus");
        SystemProperties.set(SIM_STATUS, "");
    }

    static public int getSimStatus() {
        log("getSimStatus");
        return SystemProperties.getInt(SIM_STATUS, 0);
    }

    public boolean setCapability(int phoneId) {
        int phoneNum = mDataSubSelector.getPhoneNum();
        int[] phoneRat = new int[phoneNum];
        boolean isSwitchSuccess = true;

        String curr3GSim = SystemProperties.get(PROPERTY_3G_SIM, "");
        log("setCapability: " + phoneId + ", current 3G Sim = " + curr3GSim);

        ProxyController proxyController = ProxyController.getInstance();

        RadioAccessFamily[] rat = new RadioAccessFamily[phoneNum];
        for (int i = 0; i < phoneNum; i++) {
            if (phoneId == i) {
                phoneRat[i] = proxyController.getMaxRafSupported();
            } else {
                phoneRat[i] = proxyController.getMinRafSupported();
            }
            rat[i] = new RadioAccessFamily(i, phoneRat[i]);
        }

        if (false  == proxyController.setRadioCapability(rat)) {
            log("Set phone rat fail!!! MaxPhoneRat=" + phoneRat[phoneId]);
            isSwitchSuccess = false;
        }

        return isSwitchSuccess;
    }

    public boolean setCapabilityIfNeeded(int phoneId) {
        if (phoneId != RadioCapabilitySwitchUtil.getMainCapabilityPhoneId()) {
            return setCapability(phoneId);
        }
        return true;
    }

    public int getCapabilitySwitchPolicy() {
        return capability_switch_policy;
    }

    public void handleSimImsiStatus(Intent intent) {
        int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                TelephonyManager.SIM_STATE_UNKNOWN);
        int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY, PhoneConstants.SIM_ID_1);
        if (simStatus == TelephonyManager.SIM_STATE_LOADED) {
            RadioCapabilitySwitchUtil.updateSimImsiStatus(slotId,
                    RadioCapabilitySwitchUtil.IMSI_READY);
        } else if (simStatus == TelephonyManager.SIM_STATE_NOT_READY) {
            RadioCapabilitySwitchUtil.updateSimImsiStatus(slotId,
                    RadioCapabilitySwitchUtil.IMSI_NOT_READY);
        }
    }

    static private void log(String txt) {
        if (DBG) {
            Rlog.d(LOG_TAG, txt);
        }
    }

    static private void loge(String txt) {
        if (DBG) {
            Rlog.e(LOG_TAG, txt);
        }
    }
}
