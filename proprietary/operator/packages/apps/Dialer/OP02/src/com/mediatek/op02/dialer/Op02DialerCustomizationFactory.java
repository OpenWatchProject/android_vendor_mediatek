package com.mediatek.op02.dialer;

import android.content.Context;
import android.util.Log;

import com.mediatek.dialer.ext.ICallLogExt;
import com.mediatek.dialer.ext.OpDialerCustomizationFactoryBase;

public class Op02DialerCustomizationFactory extends OpDialerCustomizationFactoryBase {
    private static final String TAG = "Op02DialerCustomizationFactory ";
    public Context mContext;
    public Op02DialerCustomizationFactory (Context context) {
        mContext = context;
    }

    @Override
    public ICallLogExt makeCallLogExt() {
        Log.d(TAG, "Op02CallLogExt mContext:" + mContext);
        return new Op02CallLogExt(mContext);
    }
}
