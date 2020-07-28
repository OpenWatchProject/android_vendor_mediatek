package com.mediatek.op09clib.incallui;
import android.content.Context;
import com.mediatek.incallui.ext.IVilteAutoTestHelperExt;
import com.mediatek.incallui.ext.IVideoCallExt;

import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;
public class Op09ClibInCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;
    public Op09ClibInCallUICustomizationFactory(Context context) {
        mContext = context;
    }
    public IVilteAutoTestHelperExt getVilteAutoTestHelperExt() {
        return new Op09ClibVilteAutoTestHelperExt();
    }

    public IVideoCallExt getVideoCallExt() {
        return new Op09ClibVideoCallExt(mContext);
    }

}
