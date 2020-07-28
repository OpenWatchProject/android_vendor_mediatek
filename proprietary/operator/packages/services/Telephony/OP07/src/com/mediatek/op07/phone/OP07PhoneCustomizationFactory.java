package com.mediatek.op07.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.ICallFeaturesSettingExt;
import com.mediatek.phone.ext.IDisconnectCauseExt;
import com.mediatek.phone.ext.OpPhoneCustomizationFactoryBase;

/**
 * OP07 Phone Customization Factory.
 */
public class OP07PhoneCustomizationFactory extends OpPhoneCustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP07PhoneCustomizationFactory(Context context) {
        mContext = context;
    }

    @Override
    public ICallFeaturesSettingExt makeCallFeaturesSettingExt() {
        Log.d("OP07PhoneCustomizationFactory", "makeCallFeaturesSettingExt");
        return new OP07CallFeaturesSettingExt(mContext);
    }
}
