package com.mediatek.op12.telecom;

import android.content.Context;

import com.mediatek.server.telecom.ext.ICallMgrExt;
import com.mediatek.server.telecom.ext.OpTelecomCustomizationFactoryBase;

/**
 * Telecomm customization factory implementation.
 */
public class Op12TelecomCustomizationFactory extends OpTelecomCustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public Op12TelecomCustomizationFactory(Context context) {
        mContext = context;
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public ICallMgrExt makeCallMgrExt() {
        return new Op12CallMgrExt(mContext);
    }
}