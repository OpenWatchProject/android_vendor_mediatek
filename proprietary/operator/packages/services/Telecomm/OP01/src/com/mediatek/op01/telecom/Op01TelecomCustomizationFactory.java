package com.mediatek.op01.telecom;

import android.content.Context;

import com.mediatek.server.telecom.ext.ICallMgrExt;
import com.mediatek.server.telecom.ext.OpTelecomCustomizationFactoryBase;

public class Op01TelecomCustomizationFactory extends OpTelecomCustomizationFactoryBase {
    public Context mContext;
    public Op01TelecomCustomizationFactory (Context context){
        mContext = context;
    }

    public ICallMgrExt makeCallMgrExt() {
        return new Op01CallMgrExt();
    }
}
