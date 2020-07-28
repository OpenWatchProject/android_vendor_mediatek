package com.mediatek.rcse.plugin.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.incallui.ext.IRCSeCallButtonExt;
import com.mediatek.incallui.ext.IRCSeCallCardExt;
import com.mediatek.incallui.ext.IRCSeInCallExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

public class RCSeInCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;
    
    /**
     * The Constant LOG_TAG.
     */
    private static final String LOG_TAG = "RCSeInCallUICustomizationFactory";

    public RCSeInCallUICustomizationFactory(Context context) {
        mContext = context;
    }

    public IRCSeCallButtonExt getRCSeCallButtonExt() {
        Log.d(LOG_TAG, "return RCSeCallButtonExtension");
        return new RCSeCallButtonExtension(mContext);
    }

    public IRCSeCallCardExt getRCSeCallCardExt() {
        Log.d(LOG_TAG, "return RCSeCallCardExtension");
        return new RCSeCallCardExtension(mContext);
    }

    public IRCSeInCallExt getRCSeInCallExt() {
        Log.d(LOG_TAG, "return RCSeInCallUIExtension");
        return new RCSeInCallUIExtension(mContext);
    }    
    
}
