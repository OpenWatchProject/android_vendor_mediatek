package com.mediatek.op20.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.ITtyModeListPreferenceExt;
import com.mediatek.phone.ext.OpPhoneCustomizationFactoryBase;

public class Op20PhoneCustomizationFactory extends OpPhoneCustomizationFactoryBase {
    private Context mContext;

    public Op20PhoneCustomizationFactory(Context context) {
        mContext = context;
    }


    @Override
    public ITtyModeListPreferenceExt makeTtyModeListPreferenceExt() {
        Log.i("OP20PhoneCustomizationFactory", "makeTtyModeListPreferenceExt");
        return new Op20TtyModeListPreferenceExt(mContext);
    }
}
