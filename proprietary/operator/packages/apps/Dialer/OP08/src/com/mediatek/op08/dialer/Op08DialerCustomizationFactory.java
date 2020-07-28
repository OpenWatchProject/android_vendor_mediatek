package com.mediatek.op08.dialer;

import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;
import com.mediatek.dialer.ext.ICallDetailsExtension;
import com.mediatek.dialer.ext.ICallLogExt;
import com.mediatek.dialer.ext.IDialPadExtension;
import com.mediatek.dialer.ext.IDialerSearchExtension;
import com.mediatek.dialer.ext.IDialerOthersExtension;
import com.mediatek.dialer.calllog.presence.CallLogExt;
import com.mediatek.dialer.search.presence.DialerSearchExtension;
import com.mediatek.dialer.calldetails.presence.CallDetailsExtension;
import com.mediatek.dialer.ext.OpDialerCustomizationFactoryBase;

public class Op08DialerCustomizationFactory extends OpDialerCustomizationFactoryBase {
    private static final String TAG = "Op08DialerCustomizationFactory";
    private Context mPluginContext;
    private static final boolean sIsSupportUce = SystemProperties
            .get("persist.vendor.mtk_uce_support").toString().equalsIgnoreCase("1");

    public Op08DialerCustomizationFactory(Context context) {
        Log.d(TAG, "Op08DialerCustomizationFactory sIsSupportUce:" + sIsSupportUce);
        mPluginContext = context;
    }

    public ICallLogExt makeCallLogExt() {
        if (sIsSupportUce) {
            return new CallLogExt(mPluginContext);
        } else {
            Log.d(TAG, "sIsSupportUce false super.makeCallLogExt();");
            return super.makeCallLogExt();
        }
    }

    @Override
    public IDialerSearchExtension makeDialerSearchExt() {
        if (sIsSupportUce) {
            return new DialerSearchExtension(mPluginContext);
        } else {
            Log.d(TAG, "sIsSupportUce false super.makeDialerSearchExt()");
            return super.makeDialerSearchExt();
        }
    }

    public ICallDetailsExtension makeCallDetailsExt() {
        if (sIsSupportUce) {
            return new CallDetailsExtension(mPluginContext);
        } else {
            Log.d(TAG, "sIsSupportUce false call super.makeCallDetailsExt()");
            return super.makeCallDetailsExt();
        }
    }

    public IDialerOthersExtension makeDialerOthersExt() {
        boolean isRcsOn = true;
        if (isRcsOn) {
            return new Op08DialerOthersExtension(mPluginContext);
        } else {
            Log.d(TAG, "isRcsOn false call super.DefaultDialerSettingExt()");
            return super.makeDialerOthersExt();
        }
    }
}
