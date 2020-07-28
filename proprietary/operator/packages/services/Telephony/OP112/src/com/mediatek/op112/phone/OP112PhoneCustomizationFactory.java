package com.mediatek.op112.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.IDisconnectCauseExt;
import com.mediatek.phone.ext.OpPhoneCustomizationFactoryBase;

/**
 * OP112 Phone Customization Factory.
 */
public class OP112PhoneCustomizationFactory extends OpPhoneCustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP112PhoneCustomizationFactory(Context context) {
        mContext = context;
    }

    @Override
    public IDisconnectCauseExt makeDisconnectCauseExt() {
        Log.d("OP112PhoneCustomizationFactory", "makeDisconnectCauseExt");
        return new OP112DisconnectCauseExt(mContext);
    }
}
