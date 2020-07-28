package com.mediatek.op01.dialer;

import android.content.Context;
import android.util.Log;

import com.mediatek.dialer.ext.ICallDetailsExtension;
import com.mediatek.dialer.ext.ICallLogExt;
import com.mediatek.dialer.ext.IDialerSearchExtension;
import com.mediatek.dialer.ext.IDialPadExtension;
import com.mediatek.dialer.ext.OpDialerCustomizationFactoryBase;
import com.mediatek.op01.dialer.calllog.Op01CallLogExt;
import com.mediatek.op01.dialer.calldetails.Op01CallDetailsExtension;
import com.mediatek.op01.dialer.search.Op01DialerSearchExtension;
import com.mediatek.op01.dialer.speeddial.Op01DialPadExtension;

public class Op01DialerCustomizationFactory extends OpDialerCustomizationFactoryBase {
    private static final String TAG = "Op01DialerCustomizationFactory ";
    public Context mContext;
    public Op01DialerCustomizationFactory (Context context){
        mContext = context;
    }

    @Override
    public ICallLogExt makeCallLogExt() {
        Log.d(TAG, "makeCallLogExt mContext:" + mContext);
        return new Op01CallLogExt(mContext);
    }

    @Override
    public IDialPadExtension makeDialPadExt() {
        Log.d(TAG, "makeDialPadExt mContext:" + mContext);
        return new Op01DialPadExtension(mContext);
    }

    @Override
    public IDialerSearchExtension makeDialerSearchExt() {
        Log.d(TAG, "makeDialerSearchExt mContext:" + mContext);
        return new Op01DialerSearchExtension(mContext);
    }

    public ICallDetailsExtension makeCallDetailsExt() {
        return new Op01CallDetailsExtension(mContext);
    }
}
