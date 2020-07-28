package com.mediatek.op08.incallui;

import android.content.Context;
import android.util.Log;

import com.mediatek.incallui.ext.ICallCardExt;
import com.mediatek.incallui.ext.IInCallButtonExt;
import com.mediatek.incallui.ext.IInCallExt;
import com.mediatek.incallui.ext.IStatusBarExt;
import com.mediatek.incallui.ext.IVideoCallExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

/**
 * InCallUI customization factory implementation.
 */
public class OP08InCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP08InCallUICustomizationFactory(Context context) {
        mContext = context;
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public IInCallExt getInCallExt() {
        return new OP08InCallExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return IVideoCallExt Object for VideoCallExt interface implementation
     */
    public IVideoCallExt getVideoCallExt() {
        return new OP08VideoCallExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public IInCallButtonExt getInCallButtonExt() {
        return new OP08InCallButtonExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return ICallCardExt Object for ICallCardExt interface implementation
     */
    public ICallCardExt getCallCardExt() {
        return new OP08CallCardExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return IStatusBarExt Object for IStatusBarExt interface implementation
     */
    public IStatusBarExt getStatusBarExt() {
        return new OP08StatusBarExt(mContext);
    }
}