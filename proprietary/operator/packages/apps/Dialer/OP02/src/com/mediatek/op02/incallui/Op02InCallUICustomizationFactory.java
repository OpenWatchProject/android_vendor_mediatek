package com.mediatek.op02.incallui;
import android.content.Context;

import com.mediatek.incallui.ext.ICallCardExt;
import com.mediatek.incallui.ext.IStatusBarExt;
import com.mediatek.incallui.ext.IVilteAutoTestHelperExt;

import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

public class Op02InCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;
    public Op02InCallUICustomizationFactory(Context context) {
        mContext = context;
    }
    public IVilteAutoTestHelperExt getVilteAutoTestHelperExt() {
        return new Op02VilteAutoTestHelperExt();
    }

    public ICallCardExt getCallCardExt() {
        return new Op02CallCardExtension(mContext);
    }

    public IStatusBarExt getStatusBarExt() {
        return new Op02StatusBarExtension(mContext);
    }

}
