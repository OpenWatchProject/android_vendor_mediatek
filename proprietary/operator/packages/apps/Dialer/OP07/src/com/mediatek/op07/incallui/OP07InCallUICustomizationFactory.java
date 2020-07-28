package com.mediatek.op07.incallui;

import android.content.Context;

import com.mediatek.incallui.ext.IAnswerExt;
import com.mediatek.incallui.ext.ICallCardExt;
import com.mediatek.incallui.ext.IInCallButtonExt;
import com.mediatek.incallui.ext.IInCallExt;
import com.mediatek.incallui.ext.IVideoCallExt;
import com.mediatek.incallui.ext.IStatusBarExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

/**
 * InCallUI customization factory implementation.
 */
public class OP07InCallUICustomizationFactory extends OpInCallUICustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP07InCallUICustomizationFactory(Context context) {
        mContext = context;
    }

    /** Interface to create the InCall extension implementation object.
     * @return IVideoCallExt Object for VideoCallExt interface implementation
     */
    public IVideoCallExt getVideoCallExt() {
        return new OP07VideoCallExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public IInCallExt getInCallExt() {
        return new OP07InCallExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallButtonExt Object for InCallButtonExt interface implementation
     */
    public IInCallButtonExt getInCallButtonExt() {
        return new OP07InCallButtonExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return IAnswerExt Object for IAnswerExt interface implementation
     */
    public IAnswerExt getAnswerExt() {
        return new OP07AnswerExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return ICallCardExt Object for ICallCardExt interface implementation
     */
    public ICallCardExt getCallCardExt() {
        return new OP07CallCardExt(mContext);
    }

    /** Interface to create the InCall extension implementation object.
     * @return IStatusBarExt Object for IStatusBarExt interface implementation
     */
    public IStatusBarExt getStatusBarExt() {
        return new OP07StatusBarExt(mContext);
    }
}