package com.mediatek.op18.telecom;

import android.content.Context;

import com.mediatek.server.telecom.ext.ICallMgrExt;
import com.mediatek.server.telecom.ext.OpTelecomCustomizationFactoryBase;

/**
 * Telecomm customization factory implementation.
 */
public class OP18TelecomCustomizationFactory extends OpTelecomCustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP18TelecomCustomizationFactory(Context context) {
        mContext = context;
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public ICallMgrExt makeCallMgrExt() {
        return new Op18CallMgrExt(mContext);
    }
}