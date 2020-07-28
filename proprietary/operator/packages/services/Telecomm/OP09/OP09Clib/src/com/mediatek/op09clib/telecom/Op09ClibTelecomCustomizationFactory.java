package com.mediatek.op09clib.telecom;

import android.content.Context;

import com.mediatek.server.telecom.ext.ICallMgrExt;
import com.mediatek.server.telecom.ext.OpTelecomCustomizationFactoryBase;

public class Op09ClibTelecomCustomizationFactory extends OpTelecomCustomizationFactoryBase {
    public Context mContext;
    public Op09ClibTelecomCustomizationFactory (Context context){
        mContext = context;
    }

    public ICallMgrExt makeCallMgrExt() {
        return new Op09ClibCallMgrExt();
    }

}
