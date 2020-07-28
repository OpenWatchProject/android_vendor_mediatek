package com.mediatek.keyguard.op09clip;

import android.content.Context;
import android.util.Log;

import com.mediatek.keyguard.ext.DefaultCarrierTextExt;

/**
 * Customize the carrier text for OP09 C lib.
 */
public class OP09CarrierTextExt extends DefaultCarrierTextExt {
    private static final String TAG = "OP09CarrierTextExt";

    private Context mContext;

    /**
     * The constructor and to save the plugin's context for resource access.
     *
     * @param context the context of plugin.
     */
    public OP09CarrierTextExt(Context context) {
        super();
        mContext = context;
    }

    @Override
    public CharSequence customizeCarrierTextCapital(CharSequence carrierText) {
        //Log.d(TAG, "customizeCarrierTextCapital, carrierText=" + carrierText);
        return carrierText;
    }

}
