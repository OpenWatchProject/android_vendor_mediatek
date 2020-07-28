package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;

import com.mediatek.telephony.MtkTelephonyManagerEx;


public class DefaultOpSubSelectActivityExt implements IOpSubSelectActivityExt {
    /* P0 migration*/
    public static final boolean MTK_C2K_SUPPORT = SystemProperties.get("ro.vendor.mtk_ps1_rat")
            .contains("C"); //.equals("1");
    private static final String TAG = "Mms/DefaultOpSubSelectActivityExt";

    @Override
    public void onCreate(Activity hostActivity) {

    }

    public boolean onListItemClick(Activity hostActivity, final int subId) {
        return false;
    }

    @Override
    public String [] setSaveLocation() {
        return null;
    }

    @Override
    public boolean isSimSupported(int subId) {
        Log.d(TAG, "[isSimSupported]: subId=" + subId);
        if (MTK_C2K_SUPPORT && isUSimType(subId)) {
            Log.d(TAG, "[isSimSupported]: false");
            return false;
        }
        Log.d(TAG, "[isSimSupported]: true");
        return true;
    }

    @Override
    public boolean getView(CheckBox subCheckBox, String preferenceKey) {
        return true;
    }

    private boolean isUSimType(int subId) {
        String phoneType = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
        if (phoneType == null) {
            Log.d(TAG, "[isUIMType]: phoneType = null");
            return false;
        }
        Log.d(TAG, "[isUIMType]: phoneType = " + phoneType);
        return phoneType.equalsIgnoreCase("CSIM") || phoneType.equalsIgnoreCase("UIM")
            || phoneType.equalsIgnoreCase("RUIM");
    }

    @Override
    public View  getView(String preferenceKey, View view) {
        return null;
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
    }

    public void onSimStateChanged() {
    }

    @Override
    public boolean getView(Context context, int position, CheckBox subCheckBox,
            String preferenceKey) {
        return true;
    }

}
