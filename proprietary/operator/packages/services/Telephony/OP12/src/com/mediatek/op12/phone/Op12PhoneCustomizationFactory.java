package com.mediatek.op12.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.IEmergencyDialerExt;
import com.mediatek.phone.ext.IAccessibilitySettingsExt;
import com.mediatek.phone.ext.ICallFeaturesSettingExt;
import com.mediatek.phone.ext.ISCBMHelperExt;
import com.mediatek.phone.ext.OpPhoneCustomizationFactoryBase;
/**
 * Initialize Phone plugin.
 */
public class Op12PhoneCustomizationFactory extends OpPhoneCustomizationFactoryBase {
    private Context mContext;

    /**
     * Init context Phone plugin.
     * @param context phone app context.
     */
    public Op12PhoneCustomizationFactory(Context context) {
        mContext = context;
    }

    public IAccessibilitySettingsExt makeAccessibilitySettingsExt() {
        return new Op12AccessibilitySettingsExt(mContext);
    }

    /**
     * Start Emergency dialer.
     * @return emergency dialer plugin instance.
     */
    public IEmergencyDialerExt makeEmergencyDialerExt() {
        Log.i("Op12EmergencyDialerExt", "makeEmergencyDialerExt");
        return new Op12EmergencyDialerExt(mContext);
    }

    /**
     * Start Call setting. Op12CallFeaturesSettingExt
     * @return emergency dialer plugin instance.
     */
    public ICallFeaturesSettingExt makeCallFeaturesSettingExt() {
        Log.i("Op12PhoneCustomizationFactory: ", "Op12CallFeaturesSettingExt");
        return new Op12CallFeaturesSettingExt(mContext);
    }

    public ISCBMHelperExt makeSCBMHelperExt() {
        Log.i("OP12PhoneCustomizationFactory", "makeSCBMHelperExt");
        return new Op12SCBMHelperExt(mContext);
    }
}
