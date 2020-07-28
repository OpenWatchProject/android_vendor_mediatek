package com.mediatek.op18.dialer;

import android.content.Context;

import com.mediatek.dialer.ext.ICallLogExt;
import com.mediatek.dialer.ext.IDialPadExtension;
import com.mediatek.dialer.ext.OpDialerCustomizationFactoryBase;
import com.mediatek.op18.dialer.calllog.Op18CallLogExtension;

public class Op18DialerCustomizationFactory extends OpDialerCustomizationFactoryBase {
    private Context mContext;

    public Op18DialerCustomizationFactory(Context context) {
        mContext = context;
    }

    public IDialPadExtension makeDialPadExt() {
        return new Op18DialPadExtension(mContext);
    }

    public ICallLogExt makeCallLogExt() {
        return new Op18CallLogExtension(mContext);
    }
}
