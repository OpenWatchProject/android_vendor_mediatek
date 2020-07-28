package com.mediatek.op18.mms;

import android.content.Context;
import android.util.Log;

import com.mediatek.mms.ext.IOpMessagePluginExt;
import com.mediatek.mms.ext.OpMmsCustomizationFactoryBase;

public class Op18MmsCustomizationFactory extends OpMmsCustomizationFactoryBase {

    private static String TAG = "Op18MmsCustomizationFactory";

    public Op18MmsCustomizationFactory(Context context) {
        super(context);
        Log.d(TAG, "[Op18MmsCustomizationFactory]context=" + context.getApplicationInfo());
    }

    @Override
    public IOpMessagePluginExt makeOpMessagePluginExt() {
        return new Op18MessagePluginExt(mContext);
    }
}
