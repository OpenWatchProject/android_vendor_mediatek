package com.mediatek.op08.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.ICallFeaturesSettingExt;
import com.mediatek.phone.ext.ISimDialogExt;
import com.mediatek.phone.ext.OpPhoneCustomizationFactoryBase;

public class Op08PhoneCustomizationFactory extends OpPhoneCustomizationFactoryBase {
    private Context mContext;

    public Op08PhoneCustomizationFactory(Context context) {
        mContext = context;
    }

    @Override
    public ICallFeaturesSettingExt makeCallFeaturesSettingExt() {
        Log.d("OP08PhoneCustomizationFactory", "makeCallFeaturesSettingExt");
        return new Op08CallFeaturesSettingExt(mContext);
    }

    public ISimDialogExt makeSimDialogExt() {
        Log.i("Op08PhoneCustomizationFactory", "makeSimDialogExt");
        return new OP08SimDialogExt(mContext);
    }
}
