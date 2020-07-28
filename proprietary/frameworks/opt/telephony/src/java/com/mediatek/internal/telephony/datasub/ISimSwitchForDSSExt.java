package com.mediatek.internal.telephony.datasub;

import com.mediatek.internal.telephony.datasub.DataSubSelector;


public interface ISimSwitchForDSSExt {
    void init(DataSubSelector dataSubSelector);

    boolean checkCapSwitch(int policy);

    int isNeedSimSwitch();
}