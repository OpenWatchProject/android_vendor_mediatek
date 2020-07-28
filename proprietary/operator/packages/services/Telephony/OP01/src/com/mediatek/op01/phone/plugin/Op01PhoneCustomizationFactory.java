package com.mediatek.op01.phone.plugin;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.IIncomingCallExt;
import com.mediatek.phone.ext.OpPhoneCustomizationFactoryBase;

public class Op01PhoneCustomizationFactory extends OpPhoneCustomizationFactoryBase {

    public Context mContext;
    public Op01PhoneCustomizationFactory (Context context){
        mContext = context;
    }

    public IIncomingCallExt makeIncomingCallExt() {
        Log.i("Op01PhoneCustomizationFactory", "makeIncomingCallExt");
        return new Op01IncomingCallExt(mContext);
    }

    public OP01SimDialogExt makeSimDialogExt() {
        Log.i("Op01PhoneCustomizationFactory", "makeSimDialogExt");
        return new OP01SimDialogExt(mContext);
    }
}
