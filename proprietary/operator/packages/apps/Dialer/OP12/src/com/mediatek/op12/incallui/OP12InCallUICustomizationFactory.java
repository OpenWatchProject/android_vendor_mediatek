package com.mediatek.op12.incallui;

import android.content.Context;

import com.mediatek.incallui.ext.IInCallButtonExt;
import com.mediatek.incallui.ext.IInCallExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

/**
 * InCallUI customization factory implementation.
 */
public class OP12InCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP12InCallUICustomizationFactory(Context context) {
        mContext = context;
    }

    @Override
    public IInCallButtonExt getInCallButtonExt() {
        return new OP12InCallButtonExt(mContext);
    }

    @Override
    public IInCallExt getInCallExt() {
        return new OP12InCallExt(mContext);
    }
}