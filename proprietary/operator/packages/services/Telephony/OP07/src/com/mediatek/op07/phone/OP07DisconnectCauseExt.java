package com.mediatek.op07.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.DefaultDisconnectCauseExt;

import mediatek.telephony.MtkDisconnectCause;


/**
 * Plugin implementation for OP07.
 */
public class OP07DisconnectCauseExt extends DefaultDisconnectCauseExt {
    private static final String TAG = "OP07DisconnectCauseExt";
    private Context mContext = null;

    /**
     * Plugin implementation for CallfeatureSettings.
     * @param context context
     */
    public OP07DisconnectCauseExt(Context context) {
        mContext = context;
    }

    @Override
    public int toTelecomDisconnectCauseCode(int telephonyDisconnectCause, int error) {
        boolean result = false;
        Log.d(TAG, "toTelecomDisconnectCauseCode " + telephonyDisconnectCause);
        switch (telephonyDisconnectCause) {
            case MtkDisconnectCause.ECC_OVER_WIFI_UNSUPPORTED:
            case MtkDisconnectCause.WFC_UNAVAILABLE_IN_CURRENT_LOCATION:
                return mediatek.telecom.MtkDisconnectCause.SIP_INVITE_ERROR;
            default:
                return error;
        }
    }

    @Override
    public CharSequence toTelecomDisconnectCauseLabel(int telephonyDisconnectCause) {
        Log.d(TAG, "toTelecomDisconnectCauseLabel " + telephonyDisconnectCause);
        CharSequence resource = "";
        switch (telephonyDisconnectCause) {
            case MtkDisconnectCause.ECC_OVER_WIFI_UNSUPPORTED:
                resource = mContext.getResources().getString(R.string.ecc_na);
                break;
            case MtkDisconnectCause.WFC_UNAVAILABLE_IN_CURRENT_LOCATION:
                resource = mContext.getResources().getString(R.string.service_na);
                break;
            default :
                break;
        }
        return resource;
    }

    @Override
    public CharSequence toTelecomDisconnectCauseDescription(int telephonyDisconnectCause) {
        Log.d(TAG, "toTelecomDisconnectCauseDescription " + telephonyDisconnectCause);
        CharSequence resource = "";
        switch (telephonyDisconnectCause) {
            case MtkDisconnectCause.ECC_OVER_WIFI_UNSUPPORTED:
                resource = mContext.getResources().getString(R.string.ecc_over_wifi_na_location);
                break;
            case MtkDisconnectCause.WFC_UNAVAILABLE_IN_CURRENT_LOCATION:
                resource = mContext.getResources().getString(R.string.service_na_location);
                break;
            default :
                break;
        }
        return resource;
     }
}