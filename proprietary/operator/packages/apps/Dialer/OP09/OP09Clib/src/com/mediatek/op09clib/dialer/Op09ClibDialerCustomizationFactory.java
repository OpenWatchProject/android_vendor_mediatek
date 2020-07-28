package com.mediatek.op09clib.dialer;

import android.content.Context;

import com.mediatek.dialer.ext.ICallLogExt;
import com.mediatek.dialer.ext.IDialPadExtension;
import com.mediatek.dialer.ext.OpDialerCustomizationFactoryBase;
import com.mediatek.op09clib.dialer.dialpad.Op09ClibDialPadExtension;

public class Op09ClibDialerCustomizationFactory extends OpDialerCustomizationFactoryBase {
    public Context mContext;
    public Op09ClibDialerCustomizationFactory(Context context) {
        mContext = context;
    }

    @Override
    public IDialPadExtension makeDialPadExt() {
        return new Op09ClibDialPadExtension(mContext);
    }

    @Override
    public ICallLogExt makeCallLogExt() {
        return new Op09CallLogExt(mContext);
    }
}
