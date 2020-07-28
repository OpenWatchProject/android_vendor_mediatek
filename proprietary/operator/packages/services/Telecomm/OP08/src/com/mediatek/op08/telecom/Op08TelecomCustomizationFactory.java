package com.mediatek.op08.telecom;

import android.content.Context;
import com.mediatek.server.telecom.ext.ICallMgrExt;

import com.mediatek.server.telecom.ext.OpTelecomCustomizationFactoryBase;

/**
 * Telecomm customization factory implementation.
 */
public class Op08TelecomCustomizationFactory extends OpTelecomCustomizationFactoryBase {
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public Op08TelecomCustomizationFactory(Context context) {
        mContext = context;
    }

    /** Interface to create the InCall extension implementation object.
     * @return IIncallExt Object for InCallExt interface implementation
     */
    public ICallMgrExt makeCallMgrExt() {
        return new Op08CallMgrExt(mContext);
    }
}
