package com.mediatek.op18.incallui;

import android.content.Context;

import com.mediatek.incallui.ext.IInCallExt;
import com.mediatek.incallui.ext.IStatusBarExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

/**
 * InCallUI customization factory implementation.
 */
public class OP18InCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP18InCallUICustomizationFactory(Context context) {
        mContext = context;
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public IInCallExt getInCallExt() {
        return new OP18InCallExt(mContext);
    }

    /** Interface to create the StatusBar extension implementation object.
     * @return IStatusBarExt Object for StatusBarExt interface implementation
     */
    public IStatusBarExt getStatusBarExt() {
        return new OP18StatusBarExt(mContext);
    }
}