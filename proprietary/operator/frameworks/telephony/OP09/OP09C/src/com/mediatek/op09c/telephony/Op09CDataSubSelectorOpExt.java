package com.mediatek.op09c.telephony;

import android.content.Context;
import android.content.Intent;
import android.content.ContentResolver;
import android.database.ContentObserver;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.SubscriptionController;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.datasub.CapabilitySwitch;
import com.mediatek.internal.telephony.datasub.DataSubConstants;
import com.mediatek.internal.telephony.datasub.DataSubSelector;
import com.mediatek.internal.telephony.datasub.DataSubSelectorOpExt;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;
import com.mediatek.internal.telephony.datasub.ISimSwitchForDSSExt;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class Op09CDataSubSelectorOpExt extends DataSubSelectorOpExt {
    private static boolean DBG = true;
    private static String LOG_TAG = "Op09CDSSelector";
    private static Context mContext = null;
    private static DataSubSelector mDataSubSelector = null;
    private static ISimSwitchForDSSExt mSimSwitchForDSS = null;
    private static CapabilitySwitch mCapabilitySwitch = null;
    private boolean mSimOnOffFeatureEnable = false;
    private int mModeSetting = -1;

    public Op09CDataSubSelectorOpExt(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public void init(DataSubSelector dataSubSelector, ISimSwitchForDSSExt simSwitchForDSS) {
        super.init(dataSubSelector, simSwitchForDSS);
        mDataSubSelector = dataSubSelector;
        mCapabilitySwitch = CapabilitySwitch.getInstance(mContext, dataSubSelector);
        mSimSwitchForDSS = simSwitchForDSS;
        mSimOnOffFeatureEnable = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        if (!mSimOnOffFeatureEnable) {
            log("listen radio off event op09c.");
            mModeSetting = Settings.Global.getInt(mContext.getContentResolver(),
                    MtkSettingsExt.Global.MSIM_MODE_SETTING, -1);
            mContext.getContentResolver().registerContentObserver(
                    Settings.Global.getUriFor(MtkSettingsExt.Global.MSIM_MODE_SETTING),
                    false,
                    new ContentObserver(null) {
                        @Override
                        public void onChange(boolean selfChange) {
                            int mode = Settings.Global.getInt(mContext.getContentResolver(),
                                    MtkSettingsExt.Global.MSIM_MODE_SETTING, -1);
                            log("previous mode:" + mModeSetting +", new mode:" + mode);
                            // 0:sim1/sim2 both off
                            // 1:sim1 on/sim2 off
                            // 2:sim1 off/sim2 on
                            // 3:sim1/sim2 both on
                            // if previous state is both off, ignore it, for the state change
                            // sim1 off/sim2(default) off -> sim1 on/sim2(default) off
                            // will switch unexpectedly for this open minor sim case.
                            if (mModeSetting != 0 && mModeSetting != mode) {
                                updateDefaultDataSimIfNeeded();
                            }
                            mModeSetting = mode;
                        }
                    });
        }
    }

    private boolean isRadioOffBySimManagement(int phoneId) {
        boolean result = false;
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
        try {
            IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub
                    .asInterface(ServiceManager.getService("phoneEx"));
            if (null == iTelEx) {
                log("iTelEx is null!");
                return false;
            }
            result = iTelEx.isRadioOffBySimManagement(subId);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
        return result;
    }

    private boolean isSimClosed(int phoneId){
        if (mSimOnOffFeatureEnable) {
            return !RadioCapabilitySwitchUtil.isSimOn(phoneId);
        } else {
            return isRadioOffBySimManagement(phoneId);
        }
    }

    private void updateDefaultDataSimIfNeeded() {
        int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
        int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
        int insertedSimCount = 0;
        int radioOffSimCount = 0;
        int radioOffPhoneId = -1;
        int radioOnPhoneId = -1;
        int phoneNum = mDataSubSelector.getPhoneNum();
        String[] currIccId = new String[phoneNum];
        for (int i = 0; i < phoneNum; i++) {
            currIccId[i] = DataSubSelectorUtil.getIccidFromProp(i);
            if (currIccId[i] != null && !"".equals(currIccId[i]) &&
                    !DataSubConstants.NO_SIM_VALUE.equals(currIccId[i])) {
                insertedSimCount++;
                if (isSimClosed(i)) {
                    radioOffSimCount++;
                    radioOffPhoneId = i;
                } else if (radioOnPhoneId == -1) {
                    radioOnPhoneId = i;
                }
            }
        }
        log("insertedSimCount:"+ insertedSimCount + ",radioOffSimCount:"+ radioOffSimCount
                + ",radioOffPhoneId:" + radioOffPhoneId + ", default:" + defDataPhoneId
                + ",radioOnPhoneId:" + radioOnPhoneId);
        if (insertedSimCount != radioOffSimCount && radioOffSimCount == 1 &&
                radioOffPhoneId == defDataPhoneId) {
            // OP09C case request, if close default sim, need change default data to another.
            mCapabilitySwitch.setCapability(radioOnPhoneId);
            mDataSubSelector.setDefaultData(radioOnPhoneId);
        }
    }

    @Override
    public void handleSimStateChanged(Intent intent) {
        super.handleSimStateChanged(intent);
        if (!mSimOnOffFeatureEnable) {
            log("SimOnOff not enabled, do nothing for OP09C sim state change.");
            return;
        }
        int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                TelephonyManager.SIM_STATE_UNKNOWN);
        if (simStatus == TelephonyManager.SIM_STATE_NOT_READY) {
            updateDefaultDataSimIfNeeded();
        }
    }

    @Override
    public boolean enableAospDisableDataSwitch() {
        return false;
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
