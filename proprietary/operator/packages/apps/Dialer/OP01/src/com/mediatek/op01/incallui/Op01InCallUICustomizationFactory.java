package com.mediatek.op01.incallui;
import android.content.Context;
import com.mediatek.incallui.ext.IVilteAutoTestHelperExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;
public class Op01InCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;
    public Op01InCallUICustomizationFactory(Context context) {
        mContext = context;
    }
    public IVilteAutoTestHelperExt getVilteAutoTestHelperExt() {
        return new Op01VilteAutoTestHelperExt();
    }
}
