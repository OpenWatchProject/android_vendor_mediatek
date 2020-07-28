package com.mediatek.op18.omacp;

import android.content.Context;
import android.util.Log;

//import com.android.settings.widget.SwitchBar;

//import com.mediatek.common.PluginImpl;
import com.mediatek.omacp.ext.DefaultOmacpExt;

/**
 * Plugin implementation for WFC Settings.
 */

//@PluginImpl(interfaceName = "com.mediatek.omacp.ext.IOmacpExt")
public class OP18OmacpExt extends DefaultOmacpExt {

    private static final String TAG = "OP18OmacpExt";

    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP18OmacpExt(Context context) {
        super();
        mContext = context;
    }

    @Override
    /**
     * Get customized ringtone key.
     * @param defaultTone defaultTone
     * @param subId subId
     * @ return String
     */
    public String getRingtoneKey(String defaultRingtoneKey, int subId) {
        String ringtoneKey = Integer.toString(subId) + "_" + defaultRingtoneKey;
        Log.d("@M_" + TAG, "Ringtone key:" + ringtoneKey);
        return ringtoneKey;
    }
}

