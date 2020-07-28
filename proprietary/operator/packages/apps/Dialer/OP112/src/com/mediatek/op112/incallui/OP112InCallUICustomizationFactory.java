package com.mediatek.op112.incallui;

import android.content.Context;

import com.mediatek.incallui.ext.IInCallExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

/**
 * InCallUI customization factory implementation.
 */
public class OP112InCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP112InCallUICustomizationFactory(Context context) {
        mContext = context;
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public IInCallExt getInCallExt() {
        return new OP112InCallExt(mContext);
    }
}