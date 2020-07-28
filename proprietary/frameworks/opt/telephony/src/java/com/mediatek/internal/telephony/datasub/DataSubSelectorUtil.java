package com.mediatek.internal.telephony.datasub;

import android.content.Context;
import android.content.Intent;

import android.os.RemoteException;
import android.os.SystemProperties;
import android.os.ServiceManager;

import android.telephony.RadioAccessFamily;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.ProxyController;

import com.android.internal.telephony.ITelephony;

import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;

import static com.mediatek.internal.telephony.datasub.DataSubConstants.*;

public class DataSubSelectorUtil {
    private static final String LOG_TAG = "DSSelectorUtil";
    private static boolean DBG = true;

    private static DataSubSelectorUtil mInstance = null;

    private static DataSubSelector mDataSubSelector = null;

    public static String[] PROPERTY_ICCID = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };

    static public String getIccidFromProp(int phoneId) {
        return SystemProperties.get(PROPERTY_ICCID[phoneId]);
    }

    static public int getIccidNum() {
        return PROPERTY_ICCID.length;
    }

    public boolean isSimInserted(int phoneId) {
        String iccid = SystemProperties.get(PROPERTY_ICCID[phoneId], "");
        return !TextUtils.isEmpty(iccid) && !NO_SIM_VALUE.equals(iccid);
    }

    static public boolean isC2kProject() {
        return RatConfiguration.isC2kSupported();
    }

    public static int getMaxIccIdCount() {
        return PROPERTY_ICCID.length;
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
