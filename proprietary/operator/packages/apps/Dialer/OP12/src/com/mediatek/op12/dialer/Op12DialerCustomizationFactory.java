package com.mediatek.op12.dialer;

import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;
import com.mediatek.dialer.ext.ICallDetailsExtension;
import com.mediatek.dialer.ext.ICallLogExt;
import com.mediatek.dialer.ext.IDialerSearchExtension;
import com.mediatek.dialer.calllog.presence.CallLogExt;
import com.mediatek.dialer.search.presence.DialerSearchExtension;
import com.mediatek.dialer.calldetails.presence.CallDetailsExtension;
import com.mediatek.dialer.ext.OpDialerCustomizationFactoryBase;

public class Op12DialerCustomizationFactory extends OpDialerCustomizationFactoryBase {
    private static final String TAG = "Op12DialerCustomizationFactory";
    private Context mContext;
    private static final boolean sIsSupportUce = SystemProperties
            .get("persist.vendor.mtk_uce_support").toString().equalsIgnoreCase("1");

    public Op12DialerCustomizationFactory(Context context) {
        mContext = context;
    }

    public ICallLogExt makeCallLogExt() {
        if (sIsSupportUce) {
            return new CallLogExt(mContext);
        } else {
            Log.d(TAG, "sIsSupportUce false super.makeCallLogExt();");
            return super.makeCallLogExt();
        }
    }

    @Override
    public IDialerSearchExtension makeDialerSearchExt() {
        if (sIsSupportUce) {
            return new DialerSearchExtension(mContext);
        } else {
            Log.d(TAG, "sIsSupportUce false super.makeDialerSearchExt()");
            return super.makeDialerSearchExt();
        }
    }

    public ICallDetailsExtension makeCallDetailsExt() {
        if (sIsSupportUce) {
            return new CallDetailsExtension(mContext);
        } else {
            Log.d(TAG, "sIsSupportUce false call super.makeCallDetailsExt()");
            return super.makeCallDetailsExt();
        }
    }
}
