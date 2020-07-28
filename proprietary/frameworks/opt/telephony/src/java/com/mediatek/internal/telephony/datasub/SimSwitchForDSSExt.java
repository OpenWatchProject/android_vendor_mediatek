package com.mediatek.internal.telephony.datasub;

import android.content.Context;
import android.content.Intent;

import android.os.Build;

import android.text.TextUtils;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import static com.mediatek.internal.telephony.datasub.DataSubConstants.SIM_SWITCH_UNKNOWN;

public class SimSwitchForDSSExt implements ISimSwitchForDSSExt  {
    public static final boolean USER_BUILD = TextUtils.equals(Build.TYPE, "user");
    public static boolean DBG = true;

    private static DataSubSelector mDataSubSelector = null;

    static protected Context mContext = null;

    public SimSwitchForDSSExt(Context context) {

    }

    public void init(DataSubSelector dataSubSelector) {
        mDataSubSelector = dataSubSelector;
    }

    public boolean checkCapSwitch(int policy) {
        return false;
    }

    public int isNeedSimSwitch() {
        return SIM_SWITCH_UNKNOWN;
    }
}