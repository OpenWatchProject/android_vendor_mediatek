package com.mediatek.op18.phone;

import android.content.Context;
import android.util.Log;



import com.mediatek.phone.ext.ICallFeaturesSettingExt;
import com.mediatek.phone.ext.IPhoneGlobalsExt;
import com.mediatek.phone.ext.OpPhoneCustomizationFactoryBase;

public class Op18PhoneCustomizationFactory extends OpPhoneCustomizationFactoryBase {
    private Context mContext;

    public Op18PhoneCustomizationFactory(Context context) {
        mContext = context;
    }

    public ICallFeaturesSettingExt makeCallFeaturesSettingExt() {
        return new Op18CallFeaturesSettingExt(mContext);
    }


    public IPhoneGlobalsExt makePhoneGlobalsExt() {
        Log.i("OP18PhoneCustomizationFactory", "makePhoneGlobalsExt");
        return new OP18PhoneGlobalsExt(mContext);
    }
}
